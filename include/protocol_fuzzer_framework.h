#pragma once
#include <Arduino.h>
#include <vector>
#include <cstdint>

namespace ProtocolFuzzerFramework {

struct FuzzResult {
    bool success;
    uint32_t payloadsSent;
    uint32_t seedLength;
    uint32_t crashes;
    uint32_t mutationsApplied;
};

struct FuzzingStats {
    uint32_t totalPayloads;
    uint32_t totalMutations;
    uint32_t avgPayloadSize;
};

FuzzResult fuzzeWiFiProtocol(const uint8_t* seed, uint16_t seedLength, uint32_t durationMs);
FuzzResult fuzzeDNSProtocol(uint32_t durationMs);
FuzzingStats getStatistics();

} // namespace ProtocolFuzzerFramework
