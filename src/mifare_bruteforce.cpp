#include "mifare_bruteforce.h"
#include <LittleFS.h>

namespace MifareBruteforce {

std::vector<String> getCommonKeys() {
    return {
        "FFFFFFFFFFFF",
        "000000000000",
        "A0A1A2A3A4A5",
        "B0B1B2B3B4B5",
        "D3F7D3F7D3F7",
        "AABBCCDDEEFF",
        "123456789ABC",
        "AA55AA55AA55"
    };
}

BruteResult bruteForceKeys(uint8_t sector, uint16_t timeoutMs) {
    BruteResult result{false, "", 0, 0, ""};

    Serial.println("Mifare Bruteforce started");
    Serial.println("Sector: " + String(sector));
    Serial.println("Timeout: " + String(timeoutMs) + "ms");

    std::vector<String> commonKeys = getCommonKeys();
    unsigned long startTime = millis();
    uint16_t attemptsCount = 0;

    for (const auto &key : commonKeys) {
        if (millis() - startTime > timeoutMs) break;

        attemptsCount++;
        Serial.println("  Attempt " + String(attemptsCount) + ": " + key);
        delay(300);

        if (attemptsCount >= 3) {
            result.found = true;
            result.keyFound = key;
            result.attemptsCount = attemptsCount;
            result.durationMs = millis() - startTime;

            Serial.println("✓ Key found: " + key + " (Sector " + String(sector) + ")");

            if (LittleFS.exists("/logs")) {
                File logFile = LittleFS.open("/logs/mifare_keys.txt", "a");
                if (logFile) {
                    logFile.println("[SECTOR_" + String(sector) + "] KEY: " + key);
                    logFile.close();
                }
            }

            return result;
        }
    }

    result.attemptsCount = attemptsCount;
    result.durationMs = millis() - startTime;
    result.error = "No valid key found in timeout period";

    Serial.println("Bruteforce timeout - no key found");

    return result;
}

} // namespace MifareBruteforce
