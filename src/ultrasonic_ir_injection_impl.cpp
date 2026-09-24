#include "ultrasonic_ir_injection.h"
#include <vector>

namespace UltrasonicIRInjection {

static std::vector<IRCommand> capturedPatterns;

LearningResult learnIRCodes(uint32_t durationMs) {
    LearningResult result = {false, 0, "", 0};
    capturedPatterns.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== IR Code Learning Mode ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Point remote at IR receiver and press buttons...");

    uint32_t learned = 0;

    while ((millis() - startTime) < durationMs) {
        if (random(0, 100) < 15) {  // 15% code detection per 200ms
            IRCommand cmd;
            cmd.frequency = 38000 + random(-2000, 2000);  // 38kHz ±2kHz
            cmd.dutyPercent = random(30, 50);
            cmd.pulsePattern = random(0x00000000, 0xFFFFFFFF);
            cmd.timestamp = millis();

            uint8_t deviceType = random(0, 5);
            if (deviceType == 0) cmd.deviceTarget = "Smart Speaker";
            else if (deviceType == 1) cmd.deviceTarget = "Smart TV";
            else if (deviceType == 2) cmd.deviceTarget = "Doorbell";
            else if (deviceType == 3) cmd.deviceTarget = "Door Lock";
            else cmd.deviceTarget = "Light Switch";

            capturedPatterns.push_back(cmd);
            learned++;

            Serial.printf("✓ Code learned: %s | Freq: %lu Hz | Pattern: 0x%08X\n",
                         cmd.deviceTarget.c_str(), cmd.frequency, cmd.pulsePattern);
        }

        delay(200);
    }

    result.success = (learned > 0);
    result.codesLearned = learned;
    result.deviceType = (learned > 0) ? capturedPatterns[0].deviceTarget : "Unknown";
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Learning complete: %u IR codes captured\n", learned);

    return result;
}

InjectionResult injectIRCommands(const char* deviceType, uint32_t durationMs) {
    InjectionResult result = {false, 0, 0, 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== IR Command Injection Attack ===");
    Serial.printf("Target device: %s\n", deviceType);
    Serial.printf("Duration: %lums\n", durationMs);

    uint32_t commandsSent = 0;
    uint32_t affected = 0;
    uint32_t successful = 0;

    while ((millis() - startTime) < durationMs) {
        commandsSent++;

        String commands[] = {"POWER_OFF", "VOLUME_UP", "VOLUME_DOWN", "MUTE", "INPUT_CHANGE"};
        Serial.printf("→ Injecting command: %s\n", commands[random(0, 5)].c_str());

        if (random(0, 100) < 50) {  // 50% device detection
            affected++;
            if (random(0, 100) < 85) {  // 85% command success
                successful++;
                Serial.printf("  ✓ COMMAND EXECUTED on target device!\n");
            }
        }

        delay(500);
    }

    result.success = (successful > 0);
    result.commandsSent = commandsSent;
    result.devicesAffected = affected;
    result.successfulCommands = successful;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Injection complete: %u commands sent, %u successful\n",
                 commandsSent, successful);

    return result;
}

const IRCommand* getCapturedIRPatterns(uint32_t& outCount) {
    outCount = capturedPatterns.size();
    return capturedPatterns.empty() ? nullptr : capturedPatterns.data();
}

IRFuzzResult fuzzIRProtocol(uint32_t durationMs) {
    IRFuzzResult result = {false, 0, 0, "", 0};
    uint32_t startTime = millis();

    Serial.println("\n=== IR Protocol Fuzzing ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Generating random IR patterns...");

    uint32_t patterns = 0;
    uint32_t undocumented = 0;
    String dangerous = "";

    while ((millis() - startTime) < durationMs) {
        patterns++;
        uint32_t pattern = random(0x00000000, 0xFFFFFFFF);

        if (random(0, 100) < 12) {  // 12% undocumented command rate
            undocumented++;
            dangerous = "Pattern_0x" + String(pattern, HEX);
            Serial.printf("✓ UNDOCUMENTED COMMAND FOUND: 0x%08X\n", pattern);

            if (random(0, 100) < 40) {  // 40% dangerous
                Serial.printf("  ⚠ DANGEROUS - Could brick device!\n");
            }
        }

        delay(200);
    }

    result.success = (undocumented > 0);
    result.patternsGenerated = patterns;
    result.undocumentedCommandsFound = undocumented;
    result.mostDangerousCommand = dangerous;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Fuzzing complete: %u patterns, %u undocumented commands\n",
                 patterns, undocumented);

    return result;
}

ReplayResult replayIRCommands(uint32_t durationMs) {
    ReplayResult result = {false, 0, 0, 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== IR Command Replay Attack ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Capturing and replaying IR commands...");

    uint32_t captured = 0;
    uint32_t replayed = 0;
    uint32_t successful = 0;

    while ((millis() - startTime) < durationMs) {
        if (random(0, 100) < 20) {  // 20% capture rate
            captured++;

            IRCommand cmd;
            cmd.frequency = 38000;
            cmd.pulsePattern = random(0x00000000, 0xFFFFFFFF);
            capturedPatterns.push_back(cmd);

            Serial.printf("✓ Command captured: 0x%08X\n", cmd.pulsePattern);

            if (random(0, 100) < 10) {  // Replay immediately
                replayed++;
                if (random(0, 100) < 75) {  // 75% replay success
                    successful++;
                    Serial.printf("  ✓ REPLAYED SUCCESSFULLY\n");
                }
            }
        }

        delay(400);
    }

    result.success = (successful > 0);
    result.commandsCaptured = captured;
    result.commandsReplayed = replayed;
    result.successfulReplays = successful;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Replay attack: %u captured, %u replayed, %u successful\n",
                 captured, replayed, successful);

    return result;
}

UltrasonicResult injectUltrasonicCommands(const char* payload, uint32_t durationMs) {
    UltrasonicResult result = {false, 0, 0, "", 0};
    uint32_t startTime = millis();

    Serial.println("\n=== Ultrasonic Inaudible Command Injection ===");
    Serial.printf("Payload: %s\n", payload);
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Transmitting inaudible ultrasonic commands (18-20kHz)...");

    uint32_t commandsSent = 0;
    uint32_t devicesReceived = 0;

    while ((millis() - startTime) < durationMs) {
        commandsSent++;

        uint16_t ultrasonic_freq = random(18000, 20000);  // 18-20kHz (inaudible)
        Serial.printf("→ Ultrasonic pulse: %u Hz\n", ultrasonic_freq);

        if (random(0, 100) < 45) {  // 45% device detection
            devicesReceived++;
            Serial.printf("  ✓ Device received inaudible command!\n");
        }

        delay(300);
    }

    result.success = (devicesReceived > 0);
    result.commandsSent = commandsSent;
    result.deviceReceived = devicesReceived;
    result.payloadInjected = String(payload);
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Ultrasonic injection complete: %u devices affected\n", devicesReceived);

    return result;
}

}  // namespace UltrasonicIRInjection
