#pragma once
#include <Arduino.h>

namespace KeylessEntryRelay {

struct RKESignal {
    uint32_t frequency;      // 315MHz or 433MHz
    uint32_t rollingCode;
    String protocol;         // Keeloq, Secplus, etc
    int8_t signalStrength;
    uint32_t timestamp;
};

struct RelayResult {
    bool success;
    uint32_t signalsCaptured;
    uint32_t relayedSuccessfully;
    uint32_t vehiclesUnlocked;
    uint32_t durationMs;
};

// Capture keyless entry signals from distance
RelayResult captureAndRelayRKESignals(uint32_t durationMs = 45000);

// Get captured RKE signals
const RKESignal* getCapturedRKESignals(uint32_t& outCount);

// Analyze rolling code for prediction
struct RollingCodeAnalysis {
    bool success;
    uint32_t codesAnalyzed;
    uint32_t patternDetected;
    uint32_t nextCodePredicted;
    uint32_t durationMs;
};
RollingCodeAnalysis analyzeRollingCodes(uint32_t durationMs = 30000);

// Replay captured code with RF jammer
struct JammingRelayResult {
    bool success;
    uint32_t jamPacketsSent;
    uint32_t vehiclesAffected;
    uint32_t unlocksAchieved;
    uint32_t durationMs;
};
JammingRelayResult jammingRelayAttack(uint32_t durationMs = 35000);

// Brute force rolling code window
struct BruteForceResult {
    bool success;
    uint32_t codesGenerated;
    uint32_t successfulUnlocks;
    uint32_t durationMs;
};
BruteForceResult bruteForceRollingCode(uint32_t startCode, uint32_t durationMs = 40000);

// Protocol fingerprinting
struct ProtocolFingerprint {
    bool success;
    String protocol;
    uint32_t frequency;
    uint32_t bitrate;
    String modulationType;
    uint32_t vulnerabilityScore;  // 0-100
};
ProtocolFingerprint fingerprintRKEProtocol(uint32_t durationMs = 25000);

}  // namespace KeylessEntryRelay
