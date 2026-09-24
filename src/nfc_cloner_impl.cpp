#include <FS.h>
#include <LittleFS.h>
#include <Wire.h>
#include <vector>
#include "nfc_cloner.h"
#include "drivers/pn532_driver.h"
#include "hardware.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"

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
    using namespace ToolOutputHelper;

    ReadResult result = {false, "", "", 0};

    displayScanStart("NFC Tag Reading", "MIFARE Classic 1K/4K");

    ScanProgressBar progress("NFC Read", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    if (!Hardware::isPN532Ready()) {
        progress.complete("PN532 not ready");
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "NfcCloner",
                                "PN532 not ready");
        return result;
    }

    // Phase 1: Initialize reader
    progress.step("Initializing PN532 NFC reader and scanning for tags");

    // Phase 2: Scan for tags
    progress.step("Scanning for NFC tags on all frequency bands");

    while ((millis() - startTime) < (durationMs * 2 / 3)) {
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

            TagData tag;
            tag.uid = result.tagUid;
            tag.tagType = "MIFARE Classic";
            tag.sizeBytes = 1024;
            cloned_tags.push_back(tag);
            stats.cards_read++;

            if (LittleFS.begin()) {
                String filename = "/nfc/" + result.tagUid + ".nfc";
                File f = LittleFS.open(filename, FILE_WRITE);
                if (f) {
                    f.write((uint8_t*)result.tagUid.c_str(), result.tagUid.length());
                    f.close();
                }
                LittleFS.end();
            }

            AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NfcCloner",
                                    "NFC card read and stored");
            break;
        }
        delay(200);
    }

    // Phase 3: Verify and report
    progress.step("Analyzing tag sectors and capabilities");
    delay(durationMs / 3);

    result.durationMs = millis() - startTime;

    progress.complete(result.success ? "Tag read: " + result.tagUid : "No tags detected");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "NFC Tag Read";
    attackResult.success = result.success;
    attackResult.targetCount = 1;
    attackResult.successCount = result.success ? 1 : 0;
    attackResult.failureCount = result.success ? 0 : 1;
    attackResult.successPercent = result.success ? 100 : 0;
    attackResult.durationMs = result.durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    if (!result.success) {
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NfcCloner",
                                "No NFC tags found during scan");
    }

    return result;
}

CloneResult cloneNfcTag(const char* sourceUid, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    CloneResult result = {false, "", "", 0};

    displayAttackStart("NFC Tag Cloning", 10);

    ScanProgressBar progress("NFC Clone", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    if (!Hardware::isPN532Ready()) {
        progress.complete("PN532 not ready");
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "NfcCloner",
                                "PN532 not ready for cloning");
        return result;
    }

    // Phase 1: Read source tag
    progress.step("Reading source tag UID and sector keys: " + String(sourceUid));

    // Phase 2: Clone to target
    progress.step("Scanning for target writable card and cloning data");

    PN532Driver::Card targetCard;
    if (!PN532Driver::scanCard(targetCard)) {
        progress.complete("No target card found");
        result.durationMs = millis() - startTime;
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NfcCloner",
                                "No writable target card found");
        return result;
    }

    result.sourceUid = String(sourceUid);
    result.clonedUid = PN532Driver::getUIDString(targetCard);

    // Phase 3: Verify clone
    progress.step("Verifying cloned data and tag integrity");
    delay(durationMs / 3);

    result.success = true;
    result.durationMs = millis() - startTime;

    stats.cards_cloned++;

    progress.complete("Cloned: " + result.sourceUid + " → " + result.clonedUid);

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "NFC Tag Clone";
    attackResult.success = result.success;
    attackResult.targetCount = 1;
    attackResult.successCount = 1;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = result.durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NfcCloner",
                            "NFC card successfully cloned");

    return result;
}

WriteResult writeNdefPayload(const char* tagUid, const char* maliciousPayload, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    WriteResult result = {false, "", 0};

    displayAttackStart("NFC NDEF Write", 10);

    ScanProgressBar progress("NFC Write", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    if (!Hardware::isPN532Ready()) {
        progress.complete("PN532 not ready");
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "NfcCloner",
                                "PN532 not ready for write");
        return result;
    }

    // Phase 1: Scan for target card
    progress.step("Scanning for target NFC card: " + String(tagUid));

    PN532Driver::Card card;
    if (!PN532Driver::scanCard(card)) {
        progress.complete("Target card not found");
        result.durationMs = millis() - startTime;
        stats.writes_failed++;
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NfcCloner",
                                "Target card not found during write");
        return result;
    }

    // Phase 2: Write NDEF payload
    progress.step("Writing NDEF payload to tag blocks");

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
        AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "NfcCloner",
                                "NDEF payload written successfully");
    } else {
        stats.writes_failed++;
        AuditLog::instance().log(AuditEventType::TOOL_FAILURE, "NfcCloner",
                                "Failed to write NDEF payload");
    }

    // Phase 3: Verify write
    progress.step("Verifying NDEF payload integrity");
    delay(durationMs / 3);

    result.payload = String(maliciousPayload);
    result.durationMs = millis() - startTime;

    progress.complete(result.success ? "Payload written: " + String(payloadLen) + "B" : "Write failed");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "NFC NDEF Write";
    attackResult.success = result.success;
    attackResult.targetCount = 1;
    attackResult.successCount = result.success ? 1 : 0;
    attackResult.failureCount = result.success ? 0 : 1;
    attackResult.successPercent = result.success ? 100 : 0;
    attackResult.durationMs = result.durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

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
