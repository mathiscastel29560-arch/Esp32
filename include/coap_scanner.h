#pragma once
#include <Arduino.h>

namespace CoapScanner {

struct CoapServer {
    String ipAddress;
    uint16_t port;
    String resources;  // Discovered CoAP resources
    int8_t rssi;
    bool requiresAuth;
    uint32_t timestamp;
};

struct ScanResult {
    bool success;
    uint32_t serverCount;
    uint32_t resourceCount;
    uint32_t durationMs;
};

// Scan for CoAP (Constrained Application Protocol) servers on network
ScanResult scanCoapServers(uint32_t durationMs = 10000);

// Get discovered CoAP servers
const CoapServer* getDiscoveredServers(uint32_t& outCount);

// Attack: CoAP resource enumeration
struct EnumerationResult {
    bool success;
    uint32_t resourcesFound;
    String resourcePaths;
    uint32_t durationMs;
};
EnumerationResult enumerateCoapResources(const char* serverIp, uint32_t durationMs = 15000);

// Attack: CoAP message injection
struct InjectionResult {
    bool success;
    uint32_t messagesSent;
    uint32_t durationMs;
    String payloadType;
};
InjectionResult injectCoapMessages(const char* serverIp, const char* resourcePath, uint32_t durationMs = 10000);

// Attack: DTLS bypass (if encrypted)
struct DtlsBypassResult {
    bool success;
    uint32_t attemptCount;
    uint32_t durationMs;
    String vulnerabilityFound;
};
DtlsBypassResult bypassDtlsSecurity(const char* serverIp, uint32_t durationMs = 30000);

// Get CoAP statistics
struct CoapStats {
    uint32_t totalServersFound;
    uint32_t secureServers;
    uint32_t encryptedServers;
    uint32_t totalResourcesDiscovered;
};
CoapStats getCoapStats();

}  // namespace CoapScanner
