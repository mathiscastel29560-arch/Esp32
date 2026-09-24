#pragma once
#include <Arduino.h>

namespace IMSICatcher {

struct CellularDevice {
    String imsi;           // International Mobile Subscriber Identity
    String imei;           // International Mobile Equipment Identity
    String tmsi;           // Temporary Mobile Subscriber Identity
    String msisdn;         // Phone number
    int8_t signalStrength;
    String networkType;    // 2G, 3G, 4G, 5G
    uint32_t timestamp;
};

struct ScanResult {
    bool success;
    uint32_t devicesDetected;
    uint32_t imsiCaptured;
    uint32_t imeiCaptured;
    uint32_t durationMs;
};

// Scan for cellular devices
ScanResult scanCellularDevices(uint32_t durationMs = 30000);

// Get captured devices
const CellularDevice* getCapturedDevices(uint32_t& outCount);

// Force 4G→2G downgrade attack
struct DowngradeResult {
    bool success;
    uint32_t devicesDowngraded;
    uint32_t imsiCaptured;
    uint32_t durationMs;
};
DowngradeResult forceDowngrade4GTo2G(uint32_t durationMs = 45000);

// Join request spoofing
struct JoinSpoofResult {
    bool success;
    uint32_t fakeIMSIsGenerated;
    uint32_t devicesFooled;
    uint32_t durationMs;
};
JoinSpoofResult spoofJoinRequests(uint32_t durationMs = 30000);

// Network parameters enumeration
struct NetworkEnum {
    bool success;
    String mcc;            // Mobile Country Code
    String mnc;            // Mobile Network Code
    uint32_t cellID;
    uint32_t lac;          // Location Area Code
    uint32_t durationMs;
};
NetworkEnum enumerateNetworkParameters(uint32_t durationMs = 20000);

// Fake base station simulation
struct FakeBSResult {
    bool success;
    uint32_t connectionsAccepted;
    uint32_t imsisCaptured;
    uint32_t durationMs;
};
FakeBSResult simulateFakeBaseStation(uint32_t durationMs = 60000);

}  // namespace IMSICatcher
