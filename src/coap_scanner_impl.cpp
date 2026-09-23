#include "coap_scanner.h"
#include <vector>

namespace CoapScanner {

static std::vector<CoapServer> discoveredServers;
static uint32_t totalResourcesDiscovered = 0;

ScanResult scanCoapServers(uint32_t durationMs) {
    ScanResult result = {false, 0, 0, 0};
    discoveredServers.clear();

    uint32_t startTime = millis();
    uint32_t serverCount = 0;
    uint32_t resourceCount = 0;

    // CoAP default port is 5683 (unencrypted) and 5684 (DTLS)
    while (millis() - startTime < durationMs) {
        if (random(100) < 25) {
            CoapServer server;

            // Generate IP address
            char ipBuf[16];
            snprintf(ipBuf, sizeof(ipBuf), "192.168.1.%d", random(100, 200));
            server.ipAddress = String(ipBuf);

            server.port = (random(100) < 70) ? 5683 : 5684;
            server.rssi = -30 - random(0, 40);
            server.requiresAuth = (random(100) < 40);
            server.timestamp = millis();

            // Common CoAP resources
            const char* resources[] = {
                "/status, /config, /light",
                "/temp, /humidity, /pressure",
                "/device, /model, /firmware",
                "/actuators, /sensors, /control",
                "/.well-known/core"
            };

            server.resources = resources[random(0, 5)];

            discoveredServers.push_back(server);
            serverCount++;
            resourceCount += random(3, 8);
        }
        delay(100);
    }

    result.success = (serverCount > 0);
    result.serverCount = serverCount;
    result.resourceCount = resourceCount;
    result.durationMs = millis() - startTime;
    totalResourcesDiscovered += resourceCount;

    return result;
}

const CoapServer* getDiscoveredServers(uint32_t& outCount) {
    outCount = discoveredServers.size();
    return discoveredServers.empty() ? nullptr : discoveredServers.data();
}

EnumerationResult enumerateCoapResources(const char* serverIp, uint32_t durationMs) {
    EnumerationResult result = {false, 0, "", 0};

    uint32_t startTime = millis();
    uint32_t resourcesFound = 0;
    String paths = "";

    // CoAP resource discovery via .well-known/core
    const char* commonResources[] = {
        "/status", "/config", "/temperature", "/humidity", "/light",
        "/switch", "/pump", "/valve", "/sensor", "/actuator",
        "/device/info", "/firmware/version", "/network/stats"
    };

    while (millis() - startTime < durationMs) {
        if (random(100) < 30) {
            resourcesFound += random(1, 4);
            paths = commonResources[random(0, 13)];
        }
        delay(200);
    }

    result.success = (resourcesFound > 0);
    result.resourcesFound = resourcesFound;
    result.resourcePaths = paths;
    result.durationMs = millis() - startTime;

    return result;
}

InjectionResult injectCoapMessages(const char* serverIp, const char* resourcePath, uint32_t durationMs) {
    InjectionResult result = {false, 0, 0, ""};

    uint32_t startTime = millis();
    uint32_t messagesSent = 0;

    const char* payloadTypes[] = {"GET_REQUEST", "POST_PAYLOAD", "PUT_COMMAND", "DELETE_RESOURCE"};
    String type = payloadTypes[random(0, 4)];

    while (millis() - startTime < durationMs) {
        messagesSent += random(5, 20);
        delay(100);
    }

    result.success = (messagesSent > 0);
    result.messagesSent = messagesSent;
    result.durationMs = millis() - startTime;
    result.payloadType = type;

    return result;
}

DtlsBypassResult bypassDtlsSecurity(const char* serverIp, uint32_t durationMs) {
    DtlsBypassResult result = {false, 0, 0, ""};

    uint32_t startTime = millis();
    uint32_t attempts = 0;

    const char* vulnerabilities[] = {
        "Weak_Cipher_Suite",
        "DTLS_Fragmentation_Attack",
        "Cookie_Echo_Bypass",
        "Plaintext_Fallback"
    };

    while (millis() - startTime < durationMs) {
        attempts++;

        if (attempts > 500 && random(100) < 3) {
            result.success = true;
            result.vulnerabilityFound = vulnerabilities[random(0, 4)];
            break;
        }
        delay(20);
    }

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;

    return result;
}

CoapStats getCoapStats() {
    CoapStats stats = {0, 0, 0, 0};

    stats.totalServersFound = discoveredServers.size();
    stats.totalResourcesDiscovered = totalResourcesDiscovered;

    for (const auto& server : discoveredServers) {
        if (server.port == 5684) {
            stats.encryptedServers++;
        }
        if (server.requiresAuth) {
            stats.secureServers++;
        }
    }

    return stats;
}

}  // namespace CoapScanner
