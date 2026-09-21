#pragma once
#include <Arduino.h>

namespace ZigbeeScanner {

struct ZigbeeDevice {
    uint16_t panId;
    uint16_t shortAddr;
    uint64_t ieeeAddr;
    int8_t rssi;
    uint8_t channel;
    String deviceType;
    uint32_t timestamp;
};

struct ScanResult {
    bool success;
    uint32_t deviceCount;
    uint32_t durationMs;
    int8_t strongestRssi;
    uint8_t busyChannel;
};

// Scan for Zigbee devices on 2.4GHz channels (11-26)
ScanResult scanZigbeeDevices(uint32_t durationMs = 5000);

// Get discovered devices
const ZigbeeDevice* getDiscoveredDevices(uint32_t& outCount);

// Attack: Zigbee frame injection
struct InjectionResult {
    bool success;
    uint32_t framesSent;
    uint32_t durationMs;
    String attackType;
};
InjectionResult injectZigbeeFrames(uint32_t durationMs, const char* attackType = "BEACON_FLOOD");

// Attack: Zigbee key recovery simulation
struct KeyRecoveryResult {
    bool success;
    String keyRecovered;
    uint32_t attemptCount;
    uint32_t durationMs;
};
KeyRecoveryResult attemptKeyRecovery(uint32_t durationMs = 30000);

// Get scan statistics
struct ZigbeeStats {
    uint32_t totalDevicesFound;
    uint32_t secureDevices;
    uint32_t insecureDevices;
    uint8_t mostUsedChannel;
    float averageRssi;
};
ZigbeeStats getZigbeeStats();

}  // namespace ZigbeeScanner
