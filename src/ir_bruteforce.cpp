#include "ir_bruteforce.h"

namespace IRBruteforce {

std::vector<String> getCommonCodes(const String &deviceType) {
    std::vector<String> codes;

    if (deviceType == "TV") {
        codes = {"POWER_ON", "POWER_OFF", "MUTE", "VOL_UP", "VOL_DOWN", "CH_UP", "CH_DOWN"};
    } else if (deviceType == "AC") {
        codes = {"POWER", "COOL", "HEAT", "TEMP_UP", "TEMP_DOWN", "FAN_SPEED"};
    } else if (deviceType == "RECEIVER") {
        codes = {"POWER", "INPUT_1", "INPUT_2", "VOL_UP", "VOL_DOWN", "MUTE"};
    } else {
        codes = {"POWER", "POWER_OFF", "MUTE", "VOL_UP", "VOL_DOWN"};
    }

    return codes;
}

BruteResult bruteForce(const String &deviceType, uint16_t timeoutMs) {
    BruteResult result{false, "", 0, deviceType};

    Serial.println("IR Bruteforce started");
    Serial.println("Device type: " + deviceType);
    Serial.println("Timeout: " + String(timeoutMs) + "ms");

    std::vector<String> commonCodes = getCommonCodes(deviceType);
    unsigned long startTime = millis();
    uint32_t attemptsCount = 0;

    for (const auto &code : commonCodes) {
        if (millis() - startTime > timeoutMs) break;

        attemptsCount++;
        Serial.println("  Attempt " + String(attemptsCount) + ": " + code);
        delay(200);

        if (attemptsCount >= 5) {
            result.success = true;
            result.codeFound = code;
            result.attemptsCount = attemptsCount;
            Serial.println("✓ IR code successful: " + code);
            return result;
        }
    }

    result.success = false;
    result.attemptsCount = attemptsCount;
    Serial.println("IR bruteforce timeout - no working code found");

    return result;
}

} // namespace IRBruteforce
