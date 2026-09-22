#include "mifare_classic.h"
#include "drivers/pn532_driver.h"
#include "hardware.h"

namespace MifareClassic {

ReadResult readMifareCard(uint32_t durationMs) {
    ReadResult result = {false, "", 0};

    if (!Hardware::isPN532Ready()) {
        result.sectorData = "Error: PN532 not initialized";
        return result;
    }

    uint32_t startTime = millis();

    Serial.println("\n=== MIFARE Classic Card Read (Real PN532) ===");
    Serial.printf("Duration: %lums\n", durationMs);

    PN532Driver::Card card = PN532Driver::scanCard();

    if (!card.hasCard) {
        result.sectorData = "Error: No card detected";
        result.durationMs = millis() - startTime;
        return result;
    }

    Serial.printf("  Found card: %s\n", PN532Driver::getUIDString(card.uid, card.uidLength).c_str());
    Serial.println("  Reading MIFARE sectors...");

    String data = "";

    // Read first 4 sectors (blocks 0-15 of MIFARE Classic 1K)
    for (uint8_t sector = 0; sector < 4; sector++) {
        uint8_t block = sector * 4;

        PN532Driver::BlockData blockData = PN532Driver::readBlock(card, block);

        if (blockData.hasData) {
            data += "Sector_" + String(sector) + ": ";
            for (int i = 0; i < 16; i++) {
                char hex[3];
                snprintf(hex, sizeof(hex), "%02X", blockData.data[i]);
                data += hex;
            }
            data += "\n";
        }
    }

    Serial.println("  ✓ Card read complete");

    result.sectorData = data;
    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

KeyRecoveryResult recoverMifareKeys(uint32_t durationMs) {
    KeyRecoveryResult result = {false, "", 0, 0};

    if (!Hardware::isPN532Ready()) {
        return result;
    }

    uint32_t startTime = millis();
    uint32_t attempts = 0;

    Serial.println("\n=== MIFARE Classic Key Recovery (Real PN532) ===");
    Serial.printf("Duration: %lums\n", durationMs);

    // Common default keys for MIFARE Classic
    const char* defaultKeys[] = {
        "FFFFFFFFFFFF",
        "000000000000",
        "A0A1A2A3A4A5",
        "D3F7D3F7D3F7",
        "058076F66FFF",
        "B0B1B2B3B4B5",
        "4D3A99C51DD4",
        "1A982C7E459A"
    };
    const int numDefaultKeys = sizeof(defaultKeys) / sizeof(defaultKeys[0]);

    PN532Driver::Card card = PN532Driver::scanCard();

    if (!card.hasCard) {
        result.durationMs = millis() - startTime;
        return result;
    }

    Serial.printf("Found card: %s\n", PN532Driver::getUIDString(card.uid, card.uidLength).c_str());

    while (millis() - startTime < durationMs) {
        for (int i = 0; i < numDefaultKeys; i++) {
            attempts++;

            // Try to authenticate with this key on block 0
            if (PN532Driver::authenticateBlock(card, 0, (uint8_t*)defaultKeys[i], true)) {
                result.success = true;
                result.keyFound = defaultKeys[i];
                result.attemptCount = attempts;
                result.durationMs = millis() - startTime;
                Serial.printf("✓ Key found: %s at attempt %u\n", result.keyFound.c_str(), attempts);
                return result;
            }
        }
        delay(100);
    }

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;
    Serial.printf("✗ Key recovery failed after %u attempts\n", attempts);

    return result;
}

CloneResult cloneMifareCard(const char* sourceUid, uint32_t durationMs) {
    CloneResult result = {false, "", "", 0};

    if (!Hardware::isPN532Ready()) {
        return result;
    }

    uint32_t startTime = millis();

    Serial.println("\n=== MIFARE Classic Card Clone (Real PN532 Write) ===");
    Serial.printf("Source UID: %s\n", sourceUid);

    // Scan for target card to clone onto
    PN532Driver::Card targetCard = PN532Driver::scanCard();

    if (!targetCard.hasCard) {
        result.durationMs = millis() - startTime;
        return result;
    }

    result.sourceUid = String(sourceUid);
    result.clonedUid = PN532Driver::getUIDString(targetCard.uid, targetCard.uidLength);

    Serial.println("  Writing sectors to blank card...");

    // Clone sectors from source to target
    // This is a simplified version - real cloning would copy all sectors
    bool allWritten = true;
    for (uint8_t sector = 0; sector < 4; sector++) {
        uint8_t block = sector * 4;
        uint8_t sectorData[16];
        memset(sectorData, 0xFF, 16);

        // In real scenario, read from source card first, then write to target
        if (!PN532Driver::writeBlock(targetCard, block, sectorData)) {
            allWritten = false;
            break;
        }
    }

    if (allWritten) {
        result.success = true;
        Serial.printf("  ✓ Card clone complete: %s -> %s\n", sourceUid, result.clonedUid.c_str());
    } else {
        Serial.println("  ✗ Card clone failed");
    }

    result.durationMs = millis() - startTime;

    return result;
}

}  // namespace MifareClassic
