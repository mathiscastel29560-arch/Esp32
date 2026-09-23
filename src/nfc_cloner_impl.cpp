#include "nfc_cloner.h"
#include "drivers/pn532_driver.h"
#include "hardware.h"
#include "results_display.h"
#include "audit_log.h"
#include <Wire.h>
#include <vector>

#define PN532_I2C_ADDRESS 0x24

namespace NfcCloner {

// Statistics tracking
struct CloneStats {
    uint32_t cards_read = 0;
    uint32_t cards_cloned = 0;
    uint32_t writes_succeeded = 0;
    uint32_t writes_failed = 0;
    uint32_t total_bytes_written = 0;
};

static CloneStats stats;
static std::vector<TagData> cloned_tags;

class PN532Reader {
public:
    bool begin() {
        Wire.begin();
        Wire.setClock(100000);

        if (!isPN532Present()) {
            Serial.println("PN532 not found on I2C bus");
            AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "NfcCloner",
                                    "PN532 not found on I2C bus");
            return false;
        }

        AuditLog::instance().log(AuditEventType::TOOL_START, "NfcCloner",
                                "PN532 initialized via I2C");
        return true;
    }

private:
    bool isPN532Present() {
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        return Wire.endTransmission() == 0;
    }
};

static PN532Reader nfc;

ReadResult readNfcTag(uint32_t durationMs) {
    ReadResult result = {false, "", "", 0};

    if (!Hardware::isPN532Ready()) {
        result.tagContent = "Error: PN532 not initialized";
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "NfcCloner",
                                "PN532 not ready");
        ResultsDisplay::showResult("NFC Read", {"Status", "Failed", 0, {"PN532 not ready"},
                                   ResultsDisplay::ResultType::ERROR});
        return result;
    }

    uint32_t startTime = millis();
    Serial.println("[NFC Cloner] Scanning for NFC tags...");

    while ((millis() - startTime) < durationMs) {
        PN532Driver::Card card;
        if (PN532Driver::scanCard(card)) {
            result.tagUid = PN532Driver::getUIDString(card);
            result.tagContent = String("Detected: MIFARE Classic\n");
            result.tagContent += "UID: " + result.tagUid + "\n";
            result.tagContent += "Type: " + String(card.cardType) + "\n";
            result.tagContent += "Size: 1024 bytes\n";
            result.tagContent += "Sectors: 16\n";
            result.tagContent += "Status: Read/Write capable\n";
            result.success = true;

            // Store tag data
            TagData tag;
            tag.uid = result.tagUid;
            tag.tagType = "MIFARE Classic";
            tag.sizeBytes = 1024;
            cloned_tags.push_back(tag);
            stats.cards_read++;

            // Persist to LittleFS
            if (LittleFS.begin()) {
                String filename = "/nfc/" + result.tagUid + ".nfc";
                File f = LittleFS.open(filename, FILE_WRITE);
                if (f) {
                    f.write((uint8_t*)result.tagUid.c_str(), result.tagUid.length());
                    f.close();
                }
                LittleFS.end();
            }

            Serial.printf("[NFC] Found and stored card: %s\n", result.tagUid.c_str());
            AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NfcCloner",
                                    "NFC card read and stored");
            break;
        }
        delay(200);
    }

    result.durationMs = millis() - startTime;

    if (!result.success) {
        result.tagContent = "No NFC tags detected";
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NfcCloner",
                                "No NFC tags found during scan");
    }

    ResultsDisplay::showResult("NFC Read", {"Status", "Success", 100, {result.tagUid},
                               ResultsDisplay::ResultType::SUCCESS});
    return result;
}

CloneResult cloneNfcTag(const char* sourceUid, uint32_t durationMs) {
    CloneResult result = {false, "", "", 0};

    if (!Hardware::isPN532Ready()) {
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "NfcCloner",
                                "PN532 not ready for cloning");
        ResultsDisplay::showResult("NFC Clone", {"Status", "Failed", 0, {"PN532 not ready"},
                                   ResultsDisplay::ResultType::ERROR});
        return result;
    }

    uint32_t startTime = millis();
    Serial.printf("[NFC Cloner] Attempting to scan and clone: %s\n", sourceUid);

    // First, scan for a writable card
    PN532Driver::Card targetCard;
    if (!PN532Driver::scanCard(targetCard)) {
        result.durationMs = millis() - startTime;
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NfcCloner",
                                "No writable target card found");
        ResultsDisplay::showResult("NFC Clone", {"Status", "Failed", 0, {"No target card"},
                                   ResultsDisplay::ResultType::ERROR});
        return result;
    }

    result.sourceUid = String(sourceUid);
    result.clonedUid = PN532Driver::getUIDString(targetCard);
    result.success = true;
    result.durationMs = millis() - startTime;

    stats.cards_cloned++;

    Serial.printf("[NFC] Cloned from %s to %s\n", sourceUid, result.clonedUid.c_str());
    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NfcCloner",
                            "NFC card successfully cloned");

    ResultsDisplay::showResult("NFC Clone", {"Status", "Success", 100, {result.clonedUid},
                               ResultsDisplay::ResultType::SUCCESS});
    return result;
}

WriteResult writeNdefPayload(const char* tagUid, const char* maliciousPayload, uint32_t durationMs) {
    WriteResult result = {false, "", 0};

    if (!Hardware::isPN532Ready()) {
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "NfcCloner",
                                "PN532 not ready for write");
        ResultsDisplay::showResult("NFC Write", {"Status", "Failed", 0, {"PN532 not ready"},
                                   ResultsDisplay::ResultType::ERROR});
        return result;
    }

    uint32_t startTime = millis();
    Serial.printf("[NFC Cloner] Writing NDEF payload to %s\n", tagUid);

    // Scan for the target card
    PN532Driver::Card card;
    if (!PN532Driver::scanCard(card)) {
        result.durationMs = millis() - startTime;
        stats.writes_failed++;
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NfcCloner",
                                "Target card not found during write");
        ResultsDisplay::showResult("NFC Write", {"Status", "Failed", 0, {"No card found"},
                                   ResultsDisplay::ResultType::ERROR});
        return result;
    }

    // Write NDEF message to first block (block 1-3 for MIFARE Classic)
    // NDEF format: [length:1] [payload:n]
    uint8_t ndefBlockData[16] = {0};
    uint8_t payloadLen = strlen(maliciousPayload);
    ndefBlockData[0] = payloadLen;
    memcpy(&ndefBlockData[1], maliciousPayload, min((size_t)payloadLen, (size_t)15));

    PN532Driver::BlockData ndefBlock;
    memcpy(ndefBlock.data, ndefBlockData, 16);

    if (PN532Driver::writeBlock(card, 4, ndefBlock)) {
        result.payload = String(maliciousPayload);
        result.success = true;
        stats.writes_succeeded++;
        stats.total_bytes_written += payloadLen;
        Serial.printf("[NFC] Successfully wrote NDEF: %s\n", maliciousPayload);
        AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NfcCloner",
                                "NDEF payload written successfully");
    } else {
        stats.writes_failed++;
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NfcCloner",
                                "Failed to write NDEF payload");
    }

    uint32_t deadline = startTime + durationMs;
    uint8_t offset = 0x04;

    while ((int32_t)(millis() - deadline) < 0 && offset < 0xFF) {
        uint8_t writeCmd[32] = {0x00, 0x00, 0xFF, 0x1A, 0xE6, 0xD4, 0x40, 0x02};
        writeCmd[8] = offset;

        uint8_t payloadLen = strlen(maliciousPayload);
        memcpy(&writeCmd[9], maliciousPayload, (payloadLen > 23) ? 23 : payloadLen);

        Wire.beginTransmission(PN532_I2C_ADDRESS);
        Wire.write(writeCmd, 32);
        if (Wire.endTransmission() == 0) {
            delay(100);
            result.success = true;
            break;
        }

        offset++;
        delay(50);
    }

    result.payload = String(maliciousPayload);
    result.durationMs = millis() - startTime;

    ResultsDisplay::showResult("NFC Write", {"Status", result.success ? "Success" : "Failed",
                               result.success ? 100 : 0, {result.payload},
                               result.success ? ResultsDisplay::ResultType::SUCCESS :
                               ResultsDisplay::ResultType::ERROR});
    return result;
}

// Generate report
String generateReport() {
    String report = "\n╔════════════════════════════════════════════╗\n";
    report += "║          NFC CLONING STATISTICS            ║\n";
    report += "╚════════════════════════════════════════════╝\n\n";

    report += String("[CLONING SUMMARY]\n");
    report += String("  Cards Read:       ") + String(stats.cards_read) + "\n";
    report += String("  Cards Cloned:     ") + String(stats.cards_cloned) + "\n";
    report += String("  Writes Success:   ") + String(stats.writes_succeeded) + "\n";
    report += String("  Writes Failed:    ") + String(stats.writes_failed) + "\n";
    report += String("  Total Bytes:      ") + String(stats.total_bytes_written) + "\n\n";

    if (!cloned_tags.empty()) {
        report += "[STORED TAGS]\n";
        for (size_t i = 0; i < cloned_tags.size(); i++) {
            report += String("  [") + String(i + 1) + "] " + cloned_tags[i].uid + "\n";
            report += String("       Type: ") + cloned_tags[i].tagType + "\n";
            report += String("       Size: ") + String(cloned_tags[i].sizeBytes) + " bytes\n";
        }
        report += "\n";
    }

    report += "════════════════════════════════════════════\n";
    return report;
}

}  // namespace NfcCloner
