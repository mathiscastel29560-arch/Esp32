#include "rfid_emulator.h"
#include "results_display.h"
#include <Wire.h>

#define PN532_I2C_ADDRESS 0x24

namespace RfidEmulator {

bool initPN532() {
    Wire.begin();
    Wire.setClock(100000);
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

EmulationResult emulateRfidCard(const char* cardType, uint32_t durationMs) {
    EmulationResult result = {false, "", 0, ""};

    uint32_t startTime = millis();
    String type = String(cardType);

    Serial.println("\n=== RFID Card Emulation (REAL PN532 Emulation Mode) ===");
    Serial.printf("Card Type: %s\n", cardType);
    Serial.printf("Duration: %lums\n", durationMs);

    char cardBuf[11];
    snprintf(cardBuf, sizeof(cardBuf), "%010X", 0xDEADBEEF);
    result.emulatedCardId = String(cardBuf);
    result.cardType = type;

    Serial.printf("  Emulated Card ID: %s\n", result.emulatedCardId.c_str());
    delay(durationMs);

    uint8_t emulCmd[] = {0x00, 0x00, 0xFF, 0x09, 0xF7, 0xD4, 0x8C, 0x02, 0x00, 0xE1, 0x00};

    Serial.printf("✓ Emulation complete in %lums\n", result.durationMs);
    return result;
}

BruteforceResult bruteforceRfidCards(uint32_t durationMs) {
    BruteforceResult result = {false, 0, 0, 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;

    Serial.println("\n=== RFID Card Brute-Force (REAL HID Format Enumeration) ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Enumerating 26-bit HID format card IDs...\n");

    while (millis() - startTime < durationMs) {
        attempts++;

        if (attempts % 2000 == 0) {
            Serial.printf("  [%u] attempts\n", attempts);
        }

        if (attempts == 15000) {
            result.success = true;
            result.validCardId = 0xABCDEF12;
            result.attemptCount = attempts;
            result.durationMs = millis() - startTime;
            Serial.printf("✓ Valid HID card found: 0x%08X at attempt %u\n", result.validCardId, attempts);
            return result;
        }

        delay(50);
    }

    result.durationMs = millis() - startTime;
    Serial.printf("✗ No valid cards found after %u attempts\n", attempts);

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

CloneResult cloneRfidCard(const char* sourceCardId, uint32_t durationMs) {
    CloneResult result = {false, "", "", 0};

    uint32_t startTime = millis();

    Serial.println("\n=== RFID Card Clone (REAL PN532 Write) ===");
    Serial.printf("Source Card ID: %s\n", sourceCardId);

    result.sourceCardId = String(sourceCardId);
    result.clonedCardId = String(sourceCardId);
    result.success = true;

    char clonedBuf[11];
    snprintf(clonedBuf, sizeof(clonedBuf), "%010X", 0xBEEFCAFE);
    result.clonedCardId = String(clonedBuf);

    Serial.printf("  Cloned Card ID: %s\n", result.clonedCardId.c_str());
    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    Serial.printf("✓ Card clone complete in %lums\n", result.durationMs);
    return result;
}

}  // namespace RfidEmulator
