#pragma once
#include <Arduino.h>

namespace ZwaveScanner {

struct ZwaveNode {
    uint8_t nodeId;
    String deviceType;
    int8_t rssi;
    uint8_t securityLevel;  // 0=none, 1=s0, 2=s2
    String manufacturer;
    uint32_t timestamp;
};

struct ScanResult {
    bool success;
    uint32_t nodeCount;
    uint32_t durationMs;
    int8_t strongestRssi;
    uint8_t controllerNode;
};

// Scan for Z-Wave devices (868-915 MHz depending on region)
ScanResult scanZwaveNetwork(uint32_t durationMs = 8000);

// Get discovered Z-Wave nodes
const ZwaveNode* getDiscoveredNodes(uint32_t& outCount);

// Attack: Z-Wave command injection
struct InjectionResult {
    bool success;
    uint32_t commandsSent;
    uint32_t durationMs;
    String commandType;
};
InjectionResult injectZwaveCommands(uint8_t targetNode, uint32_t durationMs, const char* cmdType = "BASIC_SET");

// Attack: S0/S2 security bypass
struct SecurityBypassResult {
    bool success;
    uint32_t attemptCount;
    uint32_t durationMs;
    String vulnerabilityFound;
};
SecurityBypassResult bypassZwaveSecurity(uint32_t durationMs = 30000);

// Attack: Network key recovery
struct KeyRecoveryResult {
    bool success;
    String networkKey;
    uint32_t durationMs;
};
KeyRecoveryResult recoverZwaveNetworkKey(uint32_t durationMs = 20000);

// Get Z-Wave statistics
struct ZwaveStats {
    uint32_t totalNodesFound;
    uint32_t secureNodes;
    uint32_t vulnerableNodes;
    uint8_t networkChannel;
    float averageRssi;
};
ZwaveStats getZwaveStats();

}  // namespace ZwaveScanner
