#pragma once
#include <Arduino.h>

namespace LoRawanRecon {

struct LoRawanGateway {
    String gwId;
    String region;      // EU868, US915, AS923, etc.
    int8_t rssi;
    uint32_t latitude;
    uint32_t longitude;
    String location;
    uint32_t timestamp;
};

struct ScanResult {
    bool success;
    uint32_t gatewayCount;
    uint32_t deviceCount;
    uint32_t durationMs;
    int8_t strongestRssi;
};

// Scan for LoRaWAN gateways and devices
ScanResult scanLoRawanNetwork(uint32_t durationMs = 10000);

// Get discovered gateways
const LoRawanGateway* getDiscoveredGateways(uint32_t& outCount);

// Attack: LoRaWAN frame injection
struct InjectionResult {
    bool success;
    uint32_t framesSent;
    uint32_t durationMs;
    String frameType;
};
InjectionResult injectLoRawanFrames(uint32_t durationMs = 15000);

// Attack: Join Request forging
struct JoinForgeResult {
    bool success;
    uint32_t joinAttemptsCount;
    uint32_t durationMs;
    String statusMessage;
};
JoinForgeResult forgeJoinRequests(uint32_t durationMs = 20000);

// Attack: Session key recovery
struct KeyRecoveryResult {
    bool success;
    String appKey;
    String nwkKey;
    uint32_t durationMs;
};
KeyRecoveryResult recoverLoRawanKeys(uint32_t durationMs = 30000);

// Get LoRaWAN statistics
struct LoRawanStats {
    uint32_t totalGatewaysFound;
    uint32_t supportedRegions;
    uint32_t devicesDiscovered;
    String mostActiveGateway;
};
LoRawanStats getLoRawanStats();

}  // namespace LoRawanRecon
