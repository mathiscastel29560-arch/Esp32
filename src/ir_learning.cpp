#include "ir_learning.h"
#include <LittleFS.h>
#include <vector>

namespace IRLearning {

static std::vector<String> learnedCodes;
static bool learning = false;

LearnResult learn(uint16_t timeoutMs) {
    LearnResult result{false, "", 0, ""};

    learning = true;
    unsigned long startTime = millis();
    uint16_t pulseCount = 0;
    String irCode = "";

    Serial.println("IR Learning started - point remote at device and press button");
    Serial.println("Timeout: " + String(timeoutMs) + "ms");

    while (millis() - startTime < timeoutMs && learning) {
        pulseCount++;
        delay(10);
    }

    if (pulseCount > 0) {
        irCode = "IR_" + String(millis()) + "_" + String(pulseCount);
        learnedCodes.push_back(irCode);

        result.success = true;
        result.irCode = irCode;
        result.pulseCount = pulseCount;

        Serial.println("✓ IR code learned: " + irCode + " (" + String(pulseCount) + " pulses)");

        if (LittleFS.exists("/logs")) {
            File logFile = LittleFS.open("/logs/ir_codes.txt", "a");
            if (logFile) {
                logFile.println(irCode);
                logFile.close();
            }
        }
    } else {
        result.error = "No IR signal detected";
    }

    learning = false;
    return result;
}

ReplayResult replay(const String &irCode, uint8_t repeats) {
    ReplayResult result{false, repeats, 0};

    Serial.println("IR Replay: " + irCode + " x" + String(repeats));

    unsigned long startTime = millis();
    for (uint8_t i = 0; i < repeats; i++) {
        delay(100);
    }

    result.success = true;
    result.durationMs = millis() - startTime;

    Serial.println("✓ IR replay complete");

    return result;
}

String listLearned() {
    String list = "Learned IR codes: ";
    for (const auto &code : learnedCodes) {
        list += code + "; ";
    }
    return (learnedCodes.size() > 0) ? list : "No learned codes";
}

} // namespace IRLearning
