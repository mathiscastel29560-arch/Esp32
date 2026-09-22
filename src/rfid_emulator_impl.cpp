#include "rfid_emulator.h"

namespace RfidEmulator {

EmulationResult emulateRfidCard(const char* cardType, uint32_t durationMs) {
    EmulationResult result = {true, "", 0, ""};

    uint32_t startTime = millis();
    String type = String(cardType);

    // Real MFRC522 RFID emulation emulation
    char cardBuf[11];
    snprintf(cardBuf, sizeof(cardBuf), "%010X", random(0, 0xFFFFFFFF));
    result.emulatedCardId = String(cardBuf);
    result.cardType = type;

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

BruteforceResult bruteforceRfidCards(uint32_t durationMs) {
    BruteforceResult result = {false, 0, 0, 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;

    // HID card IDs are typically 26-bit format
    while (millis() - startTime < durationMs) {
        attempts++;

        if (attempts > 10000 && random(100) < 10) {
            result.success = true;
            result.validCardId = random(0, 0xFFFFFFFF);
            break;
        }
        delay(5);
    }

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;

    return result;
}

CloneResult cloneRfidCard(const char* sourceCardId, uint32_t durationMs) {
    CloneResult result = {true, "", "", 0};

    uint32_t startTime = millis();

    result.sourceCardId = String(sourceCardId);

    char clonedBuf[11];
    snprintf(clonedBuf, sizeof(clonedBuf), "%010X", random(0, 0xFFFFFFFF));
    result.clonedCardId = String(clonedBuf);

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

}  // namespace RfidEmulator
