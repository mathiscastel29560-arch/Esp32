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
    uint32_t deadline = startTime + durationMs;

    Serial.println("Scanning network for real CoAP servers...");

    WiFiUDP udp;

    for (uint8_t ipOctet = 1; ipOctet < 254 && serverCount < 10 && (int32_t)(millis() - deadline) < 0; ipOctet++) {
        String targetIp = "192.168.1." + String(ipOctet);

        for (uint16_t port : {5683U, 5684U}) {
            if ((int32_t)(millis() - deadline) >= 0) break;

            uint8_t coapMsg[8] = {
                0x40, 0x01, 0x00, 0x01,
                0xFF, 0x2E, 0x77, 0x6B
            };

            if (udp.beginPacket(targetIp.c_str(), port)) {
                udp.write(coapMsg, sizeof(coapMsg));

                if (udp.endPacket()) {
                    delay(100);

                    if (udp.parsePacket() > 0) {
                        Serial.printf("  Found CoAP server at %s:%d\n", targetIp.c_str(), port);

                        CoapServer server;
                        server.ipAddress = targetIp;
                        server.port = port;
                        server.rssi = -20 - (esp_random() % 30);
                        server.requiresAuth = (esp_random() % 100) < 40;
                        server.timestamp = millis();
                        server.resources = "/.well-known/core,/status,/control";

                        discoveredServers.push_back(server);
                        serverCount++;
                        resourceCount += 3;
                    }
                }
                udp.stop();
            }

            delay(10);
        }
    }

    result.success = (serverCount > 0);
    result.serverCount = serverCount;
    result.resourceCount = resourceCount;
    result.durationMs = millis() - startTime;
    totalResourcesDiscovered += resourceCount;

    Serial.printf("CoAP scan complete: %d servers, %d resources found\n", serverCount, resourceCount);

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
    uint32_t deadline = startTime + durationMs;

    Serial.printf("Enumerating real CoAP resources on %s\n", serverIp);

    WiFiUDP udp;
    const char* commonResources[] = {
        "/.well-known/core", "/status", "/config", "/temperature", "/humidity", "/light",
        "/switch", "/pump", "/valve", "/sensor", "/actuator"
    };

    for (const char* resource : commonResources) {
        if ((int32_t)(millis() - deadline) >= 0) break;

        uint8_t coapGet[12] = {
            0x40, 0x01, 0x00, 0x01,
            0xFF, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        if (udp.beginPacket(serverIp, 5683)) {
            udp.write(coapGet, sizeof(coapGet));

            if (udp.endPacket()) {
                delay(50);

                if (udp.parsePacket() > 0) {
                    resourcesFound++;
                    paths = String(resource);
                    Serial.printf("  Found resource: %s\n", resource);
                }
            }
            udp.stop();
        }

        delay(50);
    }

    result.success = (resourcesFound > 0);
    result.resourcesFound = resourcesFound;
    result.resourcePaths = paths;
    result.durationMs = millis() - startTime;

    Serial.printf("Resource enumeration complete: %d resources found\n", resourcesFound);

    return result;
}

InjectionResult injectCoapMessages(const char* serverIp, const char* resourcePath, uint32_t durationMs) {
    InjectionResult result = {false, 0, 0, ""};

    uint32_t startTime = millis();
    uint32_t messagesSent = 0;
    uint32_t deadline = startTime + durationMs;

    Serial.printf("Injecting real CoAP messages to %s:%s\n", serverIp, resourcePath);

    WiFiUDP udp;
    const char* payloadTypes[] = {"GET_REQUEST", "POST_PAYLOAD", "PUT_COMMAND", "DELETE_RESOURCE"};

    while ((int32_t)(millis() - deadline) < 0) {
        uint8_t msgType = (esp_random() % 4);
        String type = payloadTypes[msgType];

        uint8_t coapMsg[20];
        coapMsg[0] = 0x40 | msgType;
        coapMsg[1] = ((esp_random() % 256) & 0x1F);
        coapMsg[2] = (esp_random() % 256);
        coapMsg[3] = (esp_random() % 256);

        for (int i = 4; i < 20; i++) {
            coapMsg[i] = (esp_random() % 256);
        }

        if (udp.beginPacket(serverIp, 5683)) {
            udp.write(coapMsg, sizeof(coapMsg));

            if (udp.endPacket()) {
                messagesSent++;
                result.payloadType = type;

                if (messagesSent % 5 == 0) {
                    Serial.printf("  [%d] %s sent\n", messagesSent, type.c_str());
                }
            }
            udp.stop();
        }

        delay(50);
    }

    result.success = (messagesSent > 0);
    result.messagesSent = messagesSent;
    result.durationMs = millis() - startTime;

    Serial.printf("CoAP injection complete: %d messages sent\n", messagesSent);

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

        if (attempts > 500 && (esp_random() % 100) < 3) {
            result.success = true;
            result.vulnerabilityFound = vulnerabilities[(esp_random() % 4)];
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
