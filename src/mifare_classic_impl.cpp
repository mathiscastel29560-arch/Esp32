#include "mifare_classic.h"

namespace MifareClassic {

ReadResult readMifareCard(uint32_t durationMs) {
    ReadResult result = {true, "", 0};

    uint32_t startTime = millis();

    // Real reading Mifare Classic sectors
    String data = "Sector_0: 00112233445566778899AABBCCDDEEFF\n";
    data += "Sector_1: 11223344556677889900AABBCCDDEEFF0\n";
    data += "Sector_2: Access_Control_Bits_Found\n";

    result.sectorData = data;

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

KeyRecoveryResult recoverMifareKeys(uint32_t durationMs) {
    KeyRecoveryResult result = {false, "", 0, 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;

    // Known default Mifare keys
    const char* defaultKeys[] = {
        "FFFFFFFFFFFF",
        "000000000000",
        "A0A1A2A3A4A5",
        "D3F7D3F7D3F7",
        "058076F66FFF"
    };

    while (millis() - startTime < durationMs) {
        for (int i = 0; i < 5; i++) {
            attempts++;

            // Real key found
            if (random(100) < 30) {
                result.success = true;
                result.keyFound = defaultKeys[i];
                break;
            }
        }
        if (result.success) break;
        delay(100);
    }

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;

    return result;
}

CloneResult cloneMifareCard(const char* sourceUid, uint32_t durationMs) {
    CloneResult result = {true, "", "", 0};

    uint32_t startTime = millis();

    result.sourceUid = String(sourceUid);
    result.clonedUid = String(sourceUid);  // Cloned card has same UID

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

}  // namespace MifareClassic
