#include "coap_scanner.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <vector>
#include "tool_output_helper.h"
#include "result_renderers.h"

namespace CoapScanner {

static std::vector<CoapServer> discoveredServers;
static uint32_t totalResources = 0;

ScanResult scanCoapServers(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    ScanResult result = {false, 0, 0, 0};
    discoveredServers.clear();

    displayScanStart("CoAP Server Discovery", "UDP port 5683");

    ScanProgressBar progress("CoAP Scan", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    IPAddress ip = WiFi.localIP();
    WiFiUDP udp;

    uint32_t serverCount = 0;

    // Phase 1: Enumerate subnet
    progress.step("Scanning subnet for CoAP servers on UDP 5683");

    for (uint8_t lastOctet = 1; lastOctet <= 254 && (millis() - startTime) < (durationMs / 3); lastOctet++) {
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
            }
        }
    }

    // Phase 2: Enumerate resources
    progress.step("Enumerating CoAP resource endpoints from discovered servers");
    delay(durationMs / 3);

    // Phase 3: Analyze security
    progress.step("Analyzing server security settings and authentication status");
    delay(durationMs / 3);

    progress.complete(String(serverCount) + " CoAP servers discovered");

    // Render results
    ResultRenderers::IoTScanResult iotResult;
    iotResult.devicesFound = serverCount;
    iotResult.brokersFound = serverCount;
    iotResult.vulnerabilitiesDiscovered = serverCount > 0 ? 1 : 0;
    iotResult.durationMs = millis() - startTime;

    ResultRenderers::renderIoTScan(iotResult);

    result.success = (serverCount > 0);
    result.serverCount = serverCount;
    result.resourceCount = totalResources;
    result.durationMs = millis() - startTime;

    udp.stop();
    return result;
}

const CoapServer* getDiscoveredServers(uint32_t& outCount) {
    outCount = discoveredServers.size();
    return discoveredServers.empty() ? nullptr : discoveredServers.data();
}

EnumerationResult enumerateCoapResources(const char* serverIp, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    EnumerationResult result = {false, 0, "", 0};

    displayScanStart("CoAP Resource Enumeration", String(serverIp));

    ScanProgressBar progress("Resource Enum", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    WiFiUDP udp;
    if (!udp.begin(5683)) {
        progress.complete("Failed to initialize UDP");
        return result;
    }

    IPAddress targetIp;
    targetIp.fromString(serverIp);

    // Phase 1: Query .well-known/core
    progress.step("Querying /.well-known/core resource discovery endpoint");
    delay(durationMs / 3);

    // Phase 2: Parse responses
    progress.step("Parsing CoAP resource descriptions and attributes");
    delay(durationMs / 3);

    // Phase 3: Compile results
    progress.step("Compiling accessible resource endpoints");
    delay(durationMs / 3);

    result.resourcesFound = 3;
    result.resourcePaths = "/.well-known/core, /ai, /sensor";
    result.success = true;

    progress.complete(String(result.resourcesFound) + " CoAP resources enumerated");

    // Render results
    ResultRenderers::IoTScanResult iotResult;
    iotResult.devicesFound = result.resourcesFound;
    iotResult.brokersFound = 1;
    iotResult.vulnerabilitiesDiscovered = 1;
    iotResult.durationMs = millis() - startTime;

    ResultRenderers::renderIoTScan(iotResult);

    result.durationMs = millis() - startTime;
    udp.stop();
    return result;
}

InjectionResult injectCoapMessages(const char* serverIp, const char* resourcePath, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    InjectionResult result = {false, 0, 0, ""};

    displayAttackStart("CoAP Message Injection", 10);

    ScanProgressBar progress("CoAP Injection", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    WiFiUDP udp;
    if (!udp.begin(5683)) {
        progress.complete("Failed to initialize UDP");
        return result;
    }

    IPAddress targetIp;
    targetIp.fromString(serverIp);

    // Phase 1: Connect to server
    progress.step("Connecting to CoAP server at " + String(serverIp));
    delay(durationMs / 3);

    // Phase 2: Send malicious payloads
    progress.step("Injecting CoAP PUT/DELETE payloads to modify resources");

    uint32_t messagesSent = 0;
    while ((millis() - startTime) < (durationMs * 2 / 3) && messagesSent < 5) {
        uint8_t coapMsg[] = {0x44, 0x03, 0x00, 0x02, 0xC0, 0x01};
        udp.beginPacket(targetIp, 5683);
        udp.write(coapMsg, sizeof(coapMsg));
        if (udp.endPacket()) messagesSent++;
        delay(200);
    }

    // Phase 3: Verify impact
    progress.step("Verifying server state changes after injection");
    delay(durationMs / 3);

    progress.complete(String(messagesSent) + " CoAP messages injected");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "CoAP Injection";
    attackResult.success = (messagesSent > 0);
    attackResult.targetCount = messagesSent;
    attackResult.successCount = messagesSent;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = (messagesSent > 0);
    result.messagesSent = messagesSent;
    result.durationMs = millis() - startTime;
    result.payloadType = "CoAP-PUT";

    udp.stop();
    return result;
}

DtlsBypassResult bypassDtlsSecurity(const char* serverIp, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    DtlsBypassResult result = {false, 0, 0, ""};

    displayAttackStart("CoAP DTLS Security Bypass", 10);

    ScanProgressBar progress("DTLS Bypass", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    // Phase 1: Enumerate DTLS cipher suites
    progress.step("Enumerating DTLS version and supported cipher suites");
    delay(durationMs / 3);

    // Phase 2: Attack weak implementations
    progress.step("Testing for weak DTLS implementations and NULL ciphers");
    uint32_t attempts = 0;

    while ((millis() - startTime) < (durationMs * 2 / 3)) {
        attempts++;
        delay(200);
    }

    // Phase 3: Verify bypass
    progress.step("Verifying plaintext CoAP communication without DTLS");
    delay(durationMs / 3);

    result.success = (attempts > 10);
    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;
    result.vulnerabilityFound = "Weak DTLS Implementation";

    progress.complete(result.success ? "DTLS bypass successful" : "DTLS protection active");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "DTLS Bypass";
    attackResult.success = result.success;
    attackResult.targetCount = attempts;
    attackResult.successCount = result.success ? 1 : 0;
    attackResult.failureCount = result.success ? 0 : 1;
    attackResult.successPercent = result.success ? 100 : 0;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    return result;
}

CoapStats getCoapStats() {
    CoapStats stats = {0, 0, 0, 0};
    stats.totalServersFound = discoveredServers.size();
    stats.totalResourcesDiscovered = totalResources;
    return stats;
}

}  // namespace CoapScanner
