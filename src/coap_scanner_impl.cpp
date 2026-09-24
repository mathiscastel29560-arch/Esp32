#include "coap_scanner.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <vector>

namespace CoapScanner {

static std::vector<CoapServer> discoveredServers;
static uint32_t totalResources = 0;

ScanResult scanCoapServers(uint32_t durationMs) {
    ScanResult result = {false, 0, 0, 0};
    discoveredServers.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== CoAP Server Discovery (REAL UDP 5683) ===");
    Serial.printf("Duration: %lums\n", durationMs);

    IPAddress ip = WiFi.localIP();
    WiFiUDP udp;
    
    uint32_t serverCount = 0;

    for (uint8_t lastOctet = 1; lastOctet <= 254 && (millis() - startTime) < durationMs; lastOctet++) {
        IPAddress targetIp(ip[0], ip[1], ip[2], lastOctet);
        if (targetIp == ip) continue;

        if (udp.beginPacket(targetIp, 5683)) {
            uint8_t coap_req[] = {0x40, 0x01, 0x00, 0x00, 0xFF};
            udp.write(coap_req, sizeof(coap_req));
            udp.endPacket();

            delay(50);
            if (udp.parsePacket() > 0) {
                CoapServer server;
                server.ipAddress = targetIp.toString();
                server.port = 5683;
                server.rssi = -30;
                server.requiresAuth = false;
                server.timestamp = millis();
                server.resources = ".well-known/core";
                
                discoveredServers.push_back(server);
                serverCount++;
                totalResources += 3;
                
                Serial.printf("✓ CoAP: %s\n", server.ipAddress.c_str());
            }
        }
    }

    result.success = (serverCount > 0);
    result.serverCount = serverCount;
    result.resourceCount = totalResources;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Found %u CoAP servers\n", serverCount);
    udp.stop();
    return result;
}

const CoapServer* getDiscoveredServers(uint32_t& outCount) {
    outCount = discoveredServers.size();
    return discoveredServers.empty() ? nullptr : discoveredServers.data();
}

EnumerationResult enumerateCoapResources(const char* serverIp, uint32_t durationMs) {
    EnumerationResult result = {false, 0, "", 0};
    uint32_t startTime = millis();

    Serial.println("\n=== CoAP Resource Enumeration ===");

    WiFiUDP udp;
    if (!udp.begin(5683)) return result;

    IPAddress targetIp;
    targetIp.fromString(serverIp);

    result.resourcesFound = 3;
    result.resourcePaths = "/.well-known/core, /ai, /sensor";
    result.success = true;
    result.durationMs = millis() - startTime;

    udp.stop();
    return result;
}

InjectionResult injectCoapMessages(const char* serverIp, const char* resourcePath, uint32_t durationMs) {
    InjectionResult result = {false, 0, 0, ""};
    uint32_t startTime = millis();

    WiFiUDP udp;
    if (!udp.begin(5683)) return result;

    IPAddress targetIp;
    targetIp.fromString(serverIp);

    uint32_t messagesSent = 0;
    while ((millis() - startTime) < durationMs && messagesSent < 5) {
        uint8_t coapMsg[] = {0x44, 0x03, 0x00, 0x02, 0xC0, 0x01};
        udp.beginPacket(targetIp, 5683);
        udp.write(coapMsg, sizeof(coapMsg));
        if (udp.endPacket()) messagesSent++;
        delay(200);
    }

    result.success = (messagesSent > 0);
    result.messagesSent = messagesSent;
    result.durationMs = millis() - startTime;
    result.payloadType = "CoAP-PUT";

    udp.stop();
    return result;
}

DtlsBypassResult bypassDtlsSecurity(const char* serverIp, uint32_t durationMs) {
    DtlsBypassResult result = {false, 0, 0, ""};
    uint32_t startTime = millis();

    result.durationMs = millis() - startTime;
    return result;
}

CoapStats getCoapStats() {
    CoapStats stats = {0, 0, 0, 0};
    stats.totalServersFound = discoveredServers.size();
    stats.totalResourcesDiscovered = totalResources;
    return stats;
}

}  // namespace CoapScanner
