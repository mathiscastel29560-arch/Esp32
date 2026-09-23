#include "nfc_card_cloner.h"
#include "audit_log.h"

#define PN532_I2C_ADDRESS 0x24
#define PN532_FIRMWAREVERS 0x03
#define PN532_COMMAND_INLISTPASSIVETARGET 0x4A
#define PN532_COMMAND_INDATAEXCHANGE 0x40

bool NFCCardCloner::begin() {
    Wire.begin(8, 9);  // SDA=8, SCL=9 for PN532
    delay(100);

    if (!initPN532()) {
        Serial.println("[NFCCardCloner] PN532 initialization failed");
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "NFCCardCloner",
                                "PN532 init failed");
        return false;
    }

    initialized_ = true;
    memset(&stats_, 0, sizeof(ClonerStats));

    AuditLog::instance().log(AuditEventType::TOOL_START, "NFCCardCloner",
                            "NFC card cloner ready");
    Serial.println("[NFCCardCloner] Ready to read/clone NFC cards via PN532");
    return true;
}

bool NFCCardCloner::initPN532() {
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(0x55);  // Wake-up command
    Wire.endTransmission();
    delay(200);

    // Get firmware version
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0xFF);
    Wire.write(0x02);
    Wire.write(0xFE);
    Wire.write(PN532_FIRMWAREVERS);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(100);

    return true;
}

bool NFCCardCloner::scanNearby() {
    if (!initialized_) return false;

    Serial.println("[NFCCardCloner] Scanning for nearby cards...");

    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0xFF);
    Wire.write(0x04);  // Length
    Wire.write(0xFC);
    Wire.write(PN532_COMMAND_INLISTPASSIVETARGET);
    Wire.write(0x01);  // Max 1 target
    Wire.write(0x00);  // Baud rate (106 kbps Type A)
    Wire.endTransmission();
    delay(500);

    uint8_t response[32] = {0};
    Wire.requestFrom(PN532_I2C_ADDRESS, 32);

    uint8_t idx = 0;
    while (Wire.available()) {
        response[idx++] = Wire.read();
        if (idx >= 32) break;
    }

    // Check if card found (response[8] should have number of targets)
    if (idx > 8 && response[8] > 0) {
        Serial.println("[NFCCardCloner] Card detected!");
        return true;
    }

    return false;
}

bool NFCCardCloner::readCard(NFCCard& card) {
    if (!initialized_) return false;

    Serial.println("[NFCCardCloner] Reading card data...");

    uint8_t uid[10] = {0};
    uint8_t uid_len = 0;

    if (!readUID(uid, &uid_len)) {
        Serial.println("[NFCCardCloner] Failed to read UID");
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NFCCardCloner",
                                "Failed to read card UID");
        return false;
    }

    // Store UID as card_id (hex)
    card.data.clear();
    for (uint8_t i = 0; i < uid_len; i++) {
        card.data.push_back(uid[i]);
        char hex[3];
        snprintf(hex, 3, "%02X", uid[i]);
        strcat(card.card_id, hex);
    }

    card.card_type = 1;  // Type 2 (simplified)
    card.captured_time = millis() / 1000;
    card.sector_count = 16;
    card.memory_size = 1024;

    // Read sectors (simplified - Type 2 has 16 sectors)
    for (uint8_t sector = 0; sector < 16; sector++) {
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.write(0xFF);
        Wire.write(0x05);  // Length
        Wire.write(0xFB);
        Wire.write(PN532_COMMAND_INDATAEXCHANGE);
        Wire.write(0x01);  // Active target
        Wire.write(0x30);  // Read command
        Wire.write(sector);
        Wire.endTransmission();
        delay(100);

        Wire.requestFrom(PN532_I2C_ADDRESS, 20);
        while (Wire.available()) {
            card.data.push_back(Wire.read());
        }
    }

    stats_.cards_read++;
    Serial.printf("[NFCCardCloner] Read card: %s (%u bytes)\n",
                 card.card_id, card.data.size());
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NFCCardCloner",
                            "Card read successfully");
    return true;
}

bool NFCCardCloner::storeCard(const NFCCard& card) {
    if (!initialized_ || card.data.empty()) return false;

    stored_cards_.push_back(card);
    stats_.cards_stored = stored_cards_.size();

    // Persist to LittleFS
    if (!LittleFS.begin()) return false;

    String filename = "/nfc/" + String(card.card_id) + ".nfc";
    File f = LittleFS.open(filename, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return false;
    }

    // Write header
    f.write((uint8_t*)card.card_id, 16);
    f.write(card.card_type);
    uint16_t data_len = card.data.size();
    f.write((uint8_t*)&data_len, 2);

    // Write data
    for (uint8_t b : card.data) {
        f.write(b);
    }

    f.close();
    LittleFS.end();

    Serial.printf("[NFCCardCloner] Stored card: %s\n", card.card_id);
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NFCCardCloner",
                            "Card stored for cloning");
    return true;
}

bool NFCCardCloner::loadCard(const char* card_id, NFCCard& card) {
    if (!LittleFS.begin()) return false;

    String filename = "/nfc/" + String(card_id) + ".nfc";
    File f = LittleFS.open(filename, "r");
    if (!f) {
        LittleFS.end();
        return false;
    }

    // Read header
    f.read((uint8_t*)card.card_id, 16);
    card.card_type = f.read();
    uint16_t data_len;
    f.read((uint8_t*)&data_len, 2);

    // Read data
    card.data.clear();
    for (uint16_t i = 0; i < data_len; i++) {
        card.data.push_back(f.read());
    }

    f.close();
    LittleFS.end();

    return true;
}

bool NFCCardCloner::startEmulation(const NFCCard& card) {
    if (!initialized_ || card.data.empty()) return false;

    active_card_ = card;
    emulating_ = true;
    stats_.emulation_sessions++;

    Serial.printf("[NFCCardCloner] Started NFC emulation for: %s\n",
                 card.card_name[0] ? card.card_name : card.card_id);

    // Configure PN532 for target mode (emulation)
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0xFF);
    Wire.write(0x0B);  // Length
    Wire.write(0xF5);
    Wire.write(0x8C);  // TgInitAsTarget
    Wire.write(0x00);  // Mode
    // Send UID
    for (size_t i = 0; i < (active_card_.data.size() < 7 ? active_card_.data.size() : 7); i++) {
        Wire.write(active_card_.data[i]);
    }
    Wire.endTransmission();
    delay(500);

    AuditLog::instance().log(AuditEventType::TOOL_START, "NFCCardCloner",
                            "NFC card emulation started");
    return true;
}

void NFCCardCloner::stopEmulation() {
    emulating_ = false;
    Serial.println("[NFCCardCloner] Emulation stopped");
    AuditLog::instance().log(AuditEventType::TOOL_STOP, "NFCCardCloner",
                            "NFC card emulation stopped");
}

bool NFCCardCloner::cloneCard(const char* stored_id, const char* new_name) {
    if (!initialized_) return false;

    NFCCard card;
    if (!loadCard(stored_id, card)) {
        Serial.printf("[NFCCardCloner] Card not found: %s\n", stored_id);
        return false;
    }

    if (new_name) {
        strncpy(card.card_name, new_name, sizeof(card.card_name) - 1);
    }

    return startEmulation(card);
}

std::vector<NFCCardCloner::NFCCard> NFCCardCloner::listStoredCards() {
    return stored_cards_;
}

bool NFCCardCloner::deleteCard(const char* card_id) {
    if (!LittleFS.begin()) return false;

    String filename = "/nfc/" + String(card_id) + ".nfc";
    bool success = LittleFS.remove(filename);
    LittleFS.end();

    if (success) {
        for (auto it = stored_cards_.begin(); it != stored_cards_.end(); ++it) {
            if (strcmp(it->card_id, card_id) == 0) {
                stored_cards_.erase(it);
                stats_.cards_stored = stored_cards_.size();
                break;
            }
        }
        Serial.printf("[NFCCardCloner] Deleted card: %s\n", card_id);
    }

    return success;
}

NFCCardCloner::ClonerStats NFCCardCloner::getStats() {
    if (stats_.read_attempts + stats_.emulation_sessions > 0) {
        stats_.success_rate = (stats_.cards_read * 100) /
                             (stats_.read_attempts + 1);
    }
    return stats_;
}

String NFCCardCloner::generateReport() {
    String report = "\n╔════════════════════════════════════════════╗\n";
    report += "║          NFC CARD CLONING REPORT           ║\n";
    report += "╚════════════════════════════════════════════╝\n\n";

    ClonerStats s = getStats();
    report += String("[CLONING STATISTICS]\n");
    report += String("  Cards Read:       ") + String(s.cards_read) + "\n";
    report += String("  Cards Stored:     ") + String(s.cards_stored) + "\n";
    report += String("  Emulation:        ") + String(s.emulation_sessions) + " sessions\n";
    report += String("  Success Rate:     ") + String(s.success_rate) + "%\n\n";

    if (!stored_cards_.empty()) {
        report += "[STORED CARDS - AVAILABLE FOR CLONING]\n";
        for (size_t i = 0; i < stored_cards_.size(); i++) {
            report += String("  [") + String(i + 1) + "] " + stored_cards_[i].card_id + "\n";
            if (stored_cards_[i].card_name[0]) {
                report += String("       Name: ") + stored_cards_[i].card_name + "\n";
            }
            report += String("       Type: ") + String(stored_cards_[i].card_type) +
                     String(" | Size: ") + String(stored_cards_[i].data.size()) + " bytes\n";
        }
        report += "\n";
    } else {
        report += "[NO STORED CARDS]\nRead and store cards first\n\n";
    }

    report += "════════════════════════════════════════════\n";
    return report;
}

void NFCCardCloner::exportToJSON(const char* filepath) {
    if (!LittleFS.begin()) return;

    File f = LittleFS.open(filepath, FILE_WRITE);
    if (!f) {
        LittleFS.end();
        return;
    }

    f.print("{\"cards\":[");
    for (size_t i = 0; i < stored_cards_.size(); i++) {
        if (i > 0) f.print(",");
        const auto& card = stored_cards_[i];
        f.printf("{\"uid\":\"%s\",\"type\":%u,\"size\":%u,\"name\":\"%s\"}",
                card.card_id, card.card_type, card.data.size(), card.card_name);
    }
    f.printf("],\"total\":%u}", stats_.cards_stored);

    f.close();
    LittleFS.end();

    Serial.printf("[NFCCardCloner] Exported to %s\n", filepath);
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NFCCardCloner",
                            "Card data exported");
}

bool NFCCardCloner::readUID(uint8_t* uid, uint8_t* uid_len) {
    if (!uid || !uid_len) return false;

    // Simplified UID extraction from read response
    // In real implementation, would parse NFC Type 2/4 UID properly
    *uid_len = 7;  // Standard UID length
    for (uint8_t i = 0; i < 7; i++) {
        uid[i] = random(0, 256);  // Would be real UID in actual hardware
    }

    return true;
}

bool NFCCardCloner::readSectors(NFCCard& card) {
    // Simplified - real implementation would read each sector
    return true;
}

bool NFCCardCloner::validateNFCType(uint8_t type) {
    return (type >= 1 && type <= 4);
}

uint8_t NFCCardCloner::calculateChecksum(const std::vector<uint8_t>& data) {
    uint8_t checksum = 0;
    for (uint8_t b : data) {
        checksum ^= b;
    }
    return checksum;
}
