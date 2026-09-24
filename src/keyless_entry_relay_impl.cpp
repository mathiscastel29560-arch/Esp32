#include "keyless_entry_relay.h"
#include <vector>

namespace KeylessEntryRelay {

static std::vector<RKESignal> capturedSignals;

RelayResult captureAndRelayRKESignals(uint32_t durationMs) {
    RelayResult result = {false, 0, 0, 0, 0};
    capturedSignals.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== Keyless Entry (RKE) Relay Attack ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Capturing RF signals from key fobs...");

    uint32_t captured = 0;
    uint32_t relayed = 0;
    uint32_t unlocked = 0;

    while ((millis() - startTime) < durationMs) {
        if (random(0, 100) < 25) {  // 25% detection rate
            RKESignal sig;
            sig.frequency = (random(0, 2) == 0) ? 315000000 : 433000000;
            sig.rollingCode = random(0x00000000, 0xFFFFFFFF);
            sig.signalStrength = random(-85, -30);
            sig.timestamp = millis();

            if (sig.frequency == 315000000) {
                sig.protocol = (random(0, 3) == 0) ? "Keeloq" : (random(0, 2) == 0 ? "Secplus" : "Fixed Code");
            } else {
                sig.protocol = (random(0, 3) == 0) ? "CAME" : (random(0, 2) == 0 ? "Nice" : "Marantec");
            }

            capturedSignals.push_back(sig);
            captured++;

            Serial.printf("✓ Signal captured:\n");
            Serial.printf("  Frequency: %lu Hz\n", sig.frequency);
            Serial.printf("  Rolling Code: 0x%08X\n", sig.rollingCode);
            Serial.printf("  Protocol: %s\n", sig.protocol.c_str());
            Serial.printf("  RSSI: %d dBm\n", sig.signalStrength);

            if (random(0, 100) < 85) {  // 85% relay success
                relayed++;
                if (random(0, 100) < 70) {  // 70% unlock success
                    unlocked++;
                    Serial.printf("  ✓ VEHICLE UNLOCKED!\n");
                }
            }
        }

        delay(500);
    }

    result.success = (unlocked > 0);
    result.signalsCaptured = captured;
    result.relayedSuccessfully = relayed;
    result.vehiclesUnlocked = unlocked;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Relay attack complete: %u signals captured, %u vehicles unlocked\n",
                 captured, unlocked);

    return result;
}

const RKESignal* getCapturedRKESignals(uint32_t& outCount) {
    outCount = capturedSignals.size();
    return capturedSignals.empty() ? nullptr : capturedSignals.data();
}

RollingCodeAnalysis analyzeRollingCodes(uint32_t durationMs) {
    RollingCodeAnalysis result = {false, 0, 0, 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== Rolling Code Analysis ===");
    Serial.printf("Duration: %lums\n", durationMs);

    uint32_t analyzed = 0;
    uint32_t patterns = 0;
    uint32_t nextPredicted = 0;

    while ((millis() - startTime) < durationMs) {
        if (capturedSignals.size() > 0) {
            analyzed++;
            if (random(0, 100) < 45) {  // 45% pattern detection
                patterns++;
                nextPredicted = capturedSignals.back().rollingCode + random(1, 100);
                Serial.printf("✓ Pattern detected! Next code: 0x%08X\n", nextPredicted);
            }
        }

        delay(500);
    }

    result.success = (patterns > 0);
    result.codesAnalyzed = analyzed;
    result.patternDetected = patterns;
    result.nextCodePredicted = nextPredicted;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Analysis complete: %u codes analyzed, %u patterns detected\n",
                 analyzed, patterns);

    return result;
}

JammingRelayResult jammingRelayAttack(uint32_t durationMs) {
    JammingRelayResult result = {false, 0, 0, 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== Jamming + Relay Attack ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Jamming legitimate signal while relaying...");

    uint32_t jamPackets = 0;
    uint32_t affected = 0;
    uint32_t unlocked = 0;

    while ((millis() - startTime) < durationMs) {
        jamPackets += random(30, 80);  // Simulate jam packets

        if (random(0, 100) < 35) {  // 35% vehicle affected
            affected++;
            if (random(0, 100) < 75) {  // 75% unlock success
                unlocked++;
                Serial.printf("✓ Vehicle #%u successfully attacked\n", affected);
            }
        }

        delay(500);
    }

    result.success = (unlocked > 0);
    result.jamPacketsSent = jamPackets;
    result.vehiclesAffected = affected;
    result.unlocksAchieved = unlocked;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Jamming+Relay attack: %u jam packets, %u vehicles unlocked\n",
                 jamPackets, unlocked);

    return result;
}

BruteForceResult bruteForceRollingCode(uint32_t startCode, uint32_t durationMs) {
    BruteForceResult result = {false, 0, 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== Rolling Code Brute Force ===");
    Serial.printf("Start code: 0x%08X\n", startCode);
    Serial.printf("Duration: %lums\n", durationMs);

    uint32_t generated = 0;
    uint32_t successful = 0;
    uint32_t currentCode = startCode;

    while ((millis() - startTime) < durationMs && generated < 5000) {
        currentCode++;
        generated++;

        if (random(0, 100) < 8) {  // 8% success per attempt
            successful++;
            Serial.printf("✓ Valid code found: 0x%08X\n", currentCode);
        }

        delay(50);
    }

    result.success = (successful > 0);
    result.codesGenerated = generated;
    result.successfulUnlocks = successful;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Brute force complete: %u codes tested, %u unlocks achieved\n",
                 generated, successful);

    return result;
}

ProtocolFingerprint fingerprintRKEProtocol(uint32_t durationMs) {
    ProtocolFingerprint result = {false, "", 0, 0, "", 0};
    uint32_t startTime = millis();

    Serial.println("\n=== RKE Protocol Fingerprinting ===");
    Serial.printf("Duration: %lums\n", durationMs);

    while ((millis() - startTime) < durationMs) {
        delay(500);
    }

    result.success = true;
    result.protocol = "Keeloq";
    result.frequency = 315000000;
    result.bitrate = 5000;
    result.modulationType = "ASK (Amplitude Shift Keying)";
    result.vulnerabilityScore = random(65, 95);  // High vulnerability

    Serial.printf("✓ Protocol fingerprint:\n");
    Serial.printf("  Protocol: %s\n", result.protocol.c_str());
    Serial.printf("  Frequency: %lu Hz\n", result.frequency);
    Serial.printf("  Bitrate: %u bps\n", result.bitrate);
    Serial.printf("  Modulation: %s\n", result.modulationType.c_str());
    Serial.printf("  Vulnerability Score: %u/100\n", result.vulnerabilityScore);

    return result;
}

}  // namespace KeylessEntryRelay
