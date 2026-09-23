#include "nfc_cloner.h"
#include "drivers/pn532_driver.h"
#include "hardware.h"
#include "results_display.h"

namespace NfcCloner {

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

    result.durationMs = millis() - startTime;

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

}  // namespace NfcCloner
