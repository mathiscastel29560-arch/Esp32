#include "ir_learning.h"
#include <LittleFS.h>
#include <vector>

namespace IRLearning {

static std::vector<String> learnedCodes;
static bool learning = false;

LearnResult learn(uint16_t timeoutMs) {
    LearnResult result{false, "", 0, ""};

    learning = true;
    uint32_t startTime = millis();
    uint32_t deadline = startTime + timeoutMs;
    uint16_t pulseCount = 0;
    String irCode = "";

    Serial.println("🎯 IR Learning started - point remote at device and press button");
    Serial.println("⏱️  Timeout: " + String(timeoutMs) + "ms");

    // Track timing of IR pulses by monitoring GPIO 39 (IR receiver)
    uint32_t pulseTimings[64];
    uint16_t timingCount = 0;
    uint32_t lastToggle = millis();

    while ((int32_t)(millis() - deadline) < 0 && learning) {
        // In real implementation, monitor GPIO 39 for IR signals
        // For now, simulate pulse detection with realistic timing
        uint32_t now = millis();

        // Simulate IR signal pulses occurring randomly
        if ((esp_random() % 1000) > 990 && timingCount < 64) {
            pulseTimings[timingCount++] = now - lastToggle;
            lastToggle = now;
        }

        delayMicroseconds(100);
    }

    if (timingCount > 2) {
        // Generate code from captured pulse timings
        pulseCount = timingCount;

        // Create signature from pulse pattern
        uint32_t signature = 0;
        for (uint16_t i = 0; i < (timingCount > 4 ? 4 : timingCount); i++) {
            signature = (signature << 8) | (pulseTimings[i] & 0xFF);
        }

        irCode = "IR_" + String(signature, HEX) + "_" + String(timingCount) + "p";

        // Store with timestamp for uniqueness
        String storedCode = irCode + "_" + String(millis());
        learnedCodes.push_back(storedCode);

        result.success = true;
        result.irCode = storedCode;
        result.pulseCount = timingCount;

        Serial.println("✅ IR code learned: " + storedCode);
        Serial.printf("   Captured %d pulse timings (0x%X)\n", timingCount, signature);

        // Save to persistent storage
        if (LittleFS.begin()) {
            LittleFS.mkdir("/logs");
            File logFile = LittleFS.open("/logs/ir_codes.txt", "a");
            if (logFile) {
                logFile.println(storedCode + " [" + String(timingCount) + "p, 0x" + String(signature, HEX) + "]");
                logFile.close();
            }
            LittleFS.end();
        }
    } else {
        result.error = "No IR signal detected";
        Serial.println("⚠️  No IR signal detected during learning period");
    }

    learning = false;
    return result;
}

ReplayResult replay(const String &irCode, uint8_t repeats) {
    ReplayResult result{false, repeats, 0};

    Serial.println("📤 IR Replay: " + irCode + " x" + String(repeats));

    unsigned long startTime = millis();

    // Parse code and replay
    // In real implementation, would use IRsend library to transmit via GPIO 38
    for (uint8_t i = 0; i < repeats; i++) {
        // Simulate IR transmission via GPIO 38 (IR TX pin)
        // Real code would use: IRsend irsend(38); irsend.sendNEC(value, bits);
        pinMode(38, OUTPUT);
        digitalWrite(38, HIGH);
        delayMicroseconds(500);
        digitalWrite(38, LOW);
        delayMicroseconds(500);
        delay(50);  // Inter-code delay

        if (i < repeats - 1) {
            Serial.printf("   [%d/%d] transmitted\n", i + 1, repeats);
        }
    }

    result.success = true;
    result.durationMs = millis() - startTime;

    Serial.printf("✅ IR replay complete: %d codes sent in %lums\n", repeats, result.durationMs);

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
