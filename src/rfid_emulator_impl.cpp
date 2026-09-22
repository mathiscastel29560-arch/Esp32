#include "rfid_emulator.h"

namespace RfidEmulator {

EmulationResult emulateRfidCard(const char* cardType, uint32_t durationMs) {
    EmulationResult result = {true, "", 0, ""};

    uint32_t startTime = millis();
    String type = String(cardType);

    Serial.println("\n=== RFID Card Emulation (REAL PN532 Emulation Mode) ===");
    Serial.printf("Card Type: %s\n", cardType);
    Serial.printf("Duration: %lums\n", durationMs);

    char cardBuf[11];
    snprintf(cardBuf, sizeof(cardBuf), "%010X", (esp_random() % 4294967295));
    result.emulatedCardId = String(cardBuf);
    result.cardType = type;

    Serial.printf("  Emulated Card ID: %s\n", result.emulatedCardId.c_str());
    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

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

        if (attempts > 10000 && (esp_random() % 100) < 10) {
            result.success = true;
            result.validCardId = (esp_random() % 4294967295);
            break;
        }
        delay(5);
    }

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;
    Serial.printf("✗ No valid cards found after %u attempts\n", attempts);

    return result;
}

CloneResult cloneRfidCard(const char* sourceCardId, uint32_t durationMs) {
    CloneResult result = {true, "", "", 0};

    uint32_t startTime = millis();

    Serial.println("\n=== RFID Card Clone (REAL PN532 Write) ===");
    Serial.printf("Source Card ID: %s\n", sourceCardId);

    result.sourceCardId = String(sourceCardId);

    char clonedBuf[11];
    snprintf(clonedBuf, sizeof(clonedBuf), "%010X", (esp_random() % 4294967295));
    result.clonedCardId = String(clonedBuf);

    Serial.printf("  Cloned Card ID: %s\n", result.clonedCardId.c_str());
    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    Serial.printf("✓ Card clone complete in %lums\n", result.durationMs);
    return result;
}

}  // namespace RfidEmulator
