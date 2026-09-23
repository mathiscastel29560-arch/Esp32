#include "nfc_emulator.h"
#include "audit_log.h"

void NFCEmulator::begin() {
    stored_tags_.clear();
    stored_tags_.reserve(50);
    memset(&stats_, 0, sizeof(EmulationStats));
    Serial.println("[NFCEmulator] NFC tag emulator initialized");
    AuditLog::instance().log(AuditEventType::TOOL_START, "NFC_Emulator", "NFC tag emulator initialized via PN532");
}

void NFCEmulator::loadTag(const char* tag_file) {
    if (!LittleFS.begin()) return;

    if (!LittleFS.exists(tag_file)) {
        Serial.printf("[NFCEmulator] Tag file not found: %s\n", tag_file);
        LittleFS.end();
        return;
    }

    File f = LittleFS.open(tag_file, "r");
    if (!f) {
        LittleFS.end();
        return;
    }

    NFCTag tag = {0};
    if (f.read((uint8_t*)&tag, sizeof(NFCTag)) == sizeof(NFCTag)) {
        if (validateNDEF(tag.ndef_text)) {
            stored_tags_.push_back(tag);
            Serial.printf("[NFCEmulator] Loaded tag: %s\n", tag.tag_id);
            stats_.emulated_tags = stored_tags_.size();
        }
    }

    f.close();
    LittleFS.end();
}

void NFCEmulator::createTag(const char* tag_id, uint8_t type, const char* ndef_data) {
    if (!tag_id || !ndef_data) return;
    if (stored_tags_.size() >= 50) return;

    NFCTag new_tag = {0};
    strncpy(new_tag.tag_id, tag_id, sizeof(new_tag.tag_id) - 1);
    new_tag.tag_type = type;
    strncpy(new_tag.ndef_text, ndef_data, sizeof(new_tag.ndef_text) - 1);
    new_tag.ndef_length = strlen(ndef_data);
    new_tag.created_time = millis() / 1000;
    new_tag.access_count = 0;

    stored_tags_.push_back(new_tag);
    stats_.emulated_tags = stored_tags_.size();

    Serial.printf("[NFCEmulator] Created tag: %s (Type %d)\n", tag_id, type);
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NFC_Emulator", "NFC tag created and ready for emulation");
}

void NFCEmulator::startEmulation() {
    if (stored_tags_.empty()) {
        Serial.println("[NFCEmulator] No tags loaded for emulation");
        return;
    }

    emulating_ = true;
    scan_count_ = 0;
    start_time_ = millis();
    Serial.printf("[NFCEmulator] Emulation started with %u tags\n", stored_tags_.size());
    AuditLog::instance().log(AuditEventType::TOOL_START, "NFC_Emulator", "NFC tag emulation mode activated");
}

void NFCEmulator::stopEmulation() {
    emulating_ = false;
    stats_.uptime_seconds = (millis() - start_time_) / 1000;
    Serial.printf("[NFCEmulator] Emulation stopped. Total scans: %u\n", stats_.total_scans);
}

void NFCEmulator::deleteTag(const char* tag_id) {
    for (auto it = stored_tags_.begin(); it != stored_tags_.end(); ++it) {
        if (strcmp(it->tag_id, tag_id) == 0) {
            stored_tags_.erase(it);
            stats_.emulated_tags = stored_tags_.size();
            Serial.printf("[NFCEmulator] Deleted tag: %s\n", tag_id);
            return;
        }
    }
}

std::vector<NFCEmulator::NFCTag> NFCEmulator::listTags() {
    return stored_tags_;
}

NFCEmulator::EmulationStats NFCEmulator::getStats() {
    stats_.uptime_seconds = (millis() - start_time_) / 1000;
    if (stats_.read_attempts + stats_.write_attempts > 0) {
        stats_.success_rate = (stats_.read_attempts * 100) /
                             (stats_.read_attempts + stats_.write_attempts);
    }
    return stats_;
}

String NFCEmulator::generateReport() {
    String report = "\n╔════════════════════════════════════════════╗\n";
    report += "║         NFC TAG EMULATION REPORT           ║\n";
    report += "╚════════════════════════════════════════════╝\n\n";

    report += String("[EMULATION STATUS]\n");
    report += String("  Status:         ") + String(emulating_ ? "ACTIVE" : "INACTIVE") + "\n";
    report += String("  Stored Tags:    ") + String(stats_.emulated_tags) + "\n";
    report += String("  Total Scans:    ") + String(stats_.total_scans) + "\n";
    report += String("  Uptime:         ") + String(stats_.uptime_seconds) + " seconds\n\n";

    if (!stored_tags_.empty()) {
        report += "[STORED TAGS]\n";
        for (size_t i = 0; i < stored_tags_.size(); i++) {
            report += String("  Tag ") + String(i + 1) + ": " + String(stored_tags_[i].tag_id) + "\n";
            report += String("    Type:       Type ") + String(stored_tags_[i].tag_type) + "\n";
            report += String("    Data:       ") + String(stored_tags_[i].ndef_text) + "\n";
            report += String("    Accessed:   ") + String(stored_tags_[i].access_count) + " times\n\n";
        }
    } else {
        report += "[NO TAGS STORED]\n";
        report += "  Create or load tags to start emulation\n\n";
    }

    report += "════════════════════════════════════════════\n";
    return report;
}

void NFCEmulator::exportTagsToJSON(const char* filepath) {
    if (!LittleFS.begin()) return;

    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return;
    }

    f.print("{\"tags\":[");
    for (size_t i = 0; i < stored_tags_.size(); i++) {
        if (i > 0) f.print(",");
        f.printf("{\"tag_id\":\"%s\",\"type\":%u,\"data\":\"%s\",\"accesses\":%u,\"created\":%u}",
                stored_tags_[i].tag_id, stored_tags_[i].tag_type,
                stored_tags_[i].ndef_text, stored_tags_[i].access_count,
                stored_tags_[i].created_time);
    }
    f.printf("],\"total\":%u}", stats_.emulated_tags);

    f.close();
    LittleFS.end();

    Serial.printf("[NFCEmulator] Tags exported to %s\n", filepath);
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NFC_Emulator", "NFC tags exported to JSON");
}

void NFCEmulator::importTagsFromJSON(const char* filepath) {
    if (!LittleFS.begin()) return;

    if (!LittleFS.exists(filepath)) {
        Serial.printf("[NFCEmulator] Import file not found: %s\n", filepath);
        LittleFS.end();
        return;
    }

    Serial.printf("[NFCEmulator] Importing tags from %s\n", filepath);
    // Simple import - would parse JSON in production
    LittleFS.end();
}

bool NFCEmulator::validateNDEF(const char* data) {
    if (!data || strlen(data) == 0) return false;
    if (strlen(data) > 255) return false;
    return true;
}

uint8_t NFCEmulator::calculateChecksum(const NFCTag& tag) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(tag.tag_id); i++) {
        checksum ^= tag.tag_id[i];
    }
    return checksum;
}
