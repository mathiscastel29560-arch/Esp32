#pragma once
#include <Arduino.h>

namespace RFIDProtocolFuzzer {

struct FuzzResult {
    bool success;
    String protocol;           // ISO14443A, ISO14443B, ISO15693, Mifare, etc
    uint32_t fuzzPayloadsSent;
    uint32_t crashes;
    uint32_t unexpectedResponses;
    uint32_t vulnerabilitiesFound;
    uint32_t durationMs;
};

struct ProtocolVulnerability {
    String protocol;
    String vulnerabilityType;  // BufferOverflow, ProtocolBypass, WeakCrypto, etc
    String description;
    uint8_t severityScore;     // 0-100
};

// Fuzz ISO14443A protocol
FuzzResult fuzzeISO14443A(uint32_t durationMs = 45000);

// Fuzz ISO14443B protocol
FuzzResult fuzzeISO14443B(uint32_t durationMs = 45000);

// Fuzz ISO15693 protocol (high frequency, long range)
FuzzResult fuzzeISO15693(uint32_t durationMs = 45000);

// Fuzz Mifare Classic protocol
FuzzResult fuzzeMifareClassic(uint32_t durationMs = 40000);

// Comprehensive multi-protocol fuzzing
struct ComprehensiveFuzzResult {
    bool success;
    uint32_t protocolsTested;
    uint32_t totalVulnerabilitiesFound;
    uint32_t criticalVulnerabilities;
    uint32_t durationMs;
};
ComprehensiveFuzzResult comprehensiveRFIDFuzzing(uint32_t durationMs = 120000);

// Get discovered vulnerabilities
const ProtocolVulnerability* getDiscoveredVulnerabilities(uint32_t& outCount);

// Exploit specific vulnerability
struct ExploitResult {
    bool success;
    String protocol;
    String exploitType;
    uint32_t successfulAttempts;
    uint32_t durationMs;
};
ExploitResult exploitDiscoveredVulnerability(const char* protocol, uint32_t durationMs = 30000);

}  // namespace RFIDProtocolFuzzer
