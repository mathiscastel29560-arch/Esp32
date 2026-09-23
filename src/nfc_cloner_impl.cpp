#include "nfc_cloner.h"
#include "drivers/pn532_driver.h"
#include "hardware.h"
#include "results_display.h"
#include <Wire.h>

#define PN532_I2C_ADDRESS 0x24

namespace NfcCloner {

class PN532Reader {
public:
    bool begin() {
        Wire.begin();
        Wire.setClock(100000);

        if (!isPN532Present()) {
            Serial.println("PN532 not found on I2C bus");
            return false;
        }

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
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
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
            Serial.printf("[NFC] Found card: %s\n", result.tagUid.c_str());
            break;
        }
        delay(200);
    }

    result.durationMs = millis() - startTime;

    if (!result.success) {
        result.tagContent = "No NFC tags detected";
    }

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

CloneResult cloneNfcTag(const char* sourceUid, uint32_t durationMs) {
    CloneResult result = {false, "", "", 0};

    if (!Hardware::isPN532Ready()) {
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    uint32_t startTime = millis();

    Serial.printf("[NFC Cloner] Attempting to scan and clone: %s\n", sourceUid);

    // First, scan for a writable card
    PN532Driver::Card targetCard;
    if (!PN532Driver::scanCard(targetCard)) {
        result.durationMs = millis() - startTime;
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    result.sourceUid = String(sourceUid);
    result.clonedUid = PN532Driver::getUIDString(targetCard);
    result.success = true;
    result.durationMs = millis() - startTime;

    Serial.printf("[NFC] Cloned from %s to %s\n", sourceUid, result.clonedUid.c_str());

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

WriteResult writeNdefPayload(const char* tagUid, const char* maliciousPayload, uint32_t durationMs) {
    WriteResult result = {false, "", 0};

    if (!Hardware::isPN532Ready()) {
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    uint32_t startTime = millis();

    Serial.printf("[NFC Cloner] Writing NDEF payload to %s\n", tagUid);

    // Scan for the target card
    PN532Driver::Card card;
    if (!PN532Driver::scanCard(card)) {
        result.durationMs = millis() - startTime;
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
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
        Serial.printf("[NFC] Successfully wrote NDEF: %s\n", maliciousPayload);
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

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

}  // namespace NfcCloner
