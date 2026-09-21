#pragma once
#include <Arduino.h>

namespace GenericPacketTools {

struct InjectionResult {
    bool success;
    uint32_t packetsSent;
    uint32_t durationMs;
    String radioType;
};

// Generic packet injection across all radios (WiFi, NRF24, CC1101)
InjectionResult injectCustomPacket(const char* payload, const char* radioType = "auto", uint32_t durationMs = 10000);

// Generic packet replay from captured data
struct ReplayResult {
    bool success;
    uint32_t packetsReplayed;
    uint32_t durationMs;
    String radioUsed;
};
ReplayResult replayPackets(const uint8_t* capturedData, uint32_t dataLength, uint32_t durationMs = 10000);

// Packet fuzzing tool
struct FuzzResult {
    bool success;
    uint32_t fuzzedPackets;
    uint32_t crashesFound;
    uint32_t durationMs;
};
FuzzResult fuzzPackets(const char* radioType = "nrf24", uint32_t durationMs = 30000);

}  // namespace GenericPacketTools
