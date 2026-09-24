#include "default_creds_scanner.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include "tool_output_helper.h"
#include "result_renderers.h"

namespace DefaultCredScanner {

std::vector<DefaultCred> getCommonCredentials() {
    return {
        {"admin", "admin", "HTTP"},
        {"admin", "password", "HTTP"},
        {"admin", "12345", "HTTP"},
        {"root", "root", "HTTP"},
        {"admin", "admin123", "HTTP"},
        {"admin", "1234", "HTTP"},
        {"test", "test", "HTTP"},
        {"guest", "guest", "HTTP"},
        {"mqtt", "mqtt", "MQTT"},
    };
}

ScanResult scanDefaultCredentials(uint16_t timeoutMs) {
    using namespace ToolOutputHelper;

    ScanResult result{false, 0, 0, "", ""};

    displayScanStart("Default Credentials Scanner", "HTTP/MQTT Basic Auth");

    ScanProgressBar progress("Cred Scan", timeoutMs, 3);
    progress.start();

    uint32_t startTime = millis();
    std::vector<DefaultCred> creds = getCommonCredentials();

    IPAddress gateway = WiFi.gatewayIP();
    IPAddress ip = WiFi.localIP();

    uint32_t devicesScanned = 0;
    uint32_t credAttempts = 0;
    String vulnerableDevices = "";

    // Phase 1: Scan for open HTTP services
    progress.step("Scanning subnet for open HTTP/MQTT services");

    for (uint8_t lastOctet = 1; lastOctet <= 254 && (millis() - startTime) < (timeoutMs / 3); lastOctet++) {
        IPAddress targetIp(ip[0], ip[1], ip[2], lastOctet);

        if (targetIp == ip || targetIp == gateway) continue;

        WiFiClient client;
        client.setTimeout(200);

        for (uint16_t port : {80, 8080, 8000}) {
            if (!client.connect(targetIp, port, 300)) continue;

            devicesScanned++;
            client.stop();
            break;
        }
    }

    // Phase 2: Attempt default credentials
    progress.step("Testing default credentials on discovered services");

    for (uint8_t lastOctet = 1; lastOctet <= 254 && (millis() - startTime) < (timeoutMs * 2 / 3); lastOctet++) {
        IPAddress targetIp(ip[0], ip[1], ip[2], lastOctet);

        if (targetIp == ip || targetIp == gateway) continue;

        WiFiClient client;
        client.setTimeout(200);

        for (uint16_t port : {80, 8080, 8000}) {
            if (!client.connect(targetIp, port, 300)) continue;

            for (const auto& cred : creds) {
                if (cred.service != "HTTP") continue;
                if ((millis() - startTime) >= (timeoutMs * 2 / 3)) break;

                String auth = cred.username;
                auth += ":";
                auth += cred.password;

                String encodedAuth = auth;

                String httpRequest = "GET / HTTP/1.1\r\n";
                httpRequest += "Host: ";
                httpRequest += targetIp.toString();
                httpRequest += "\r\n";
                httpRequest += "Authorization: Basic ";
                httpRequest += encodedAuth;
                httpRequest += "\r\n";
                httpRequest += "Connection: close\r\n\r\n";

                if (client.print(httpRequest)) {
                    uint32_t connStart = millis();
                    uint16_t responseCode = 0;
                    String response = "";

                    while (client.available() && (millis() - connStart) < 200) {
                        char c = client.read();
                        response += c;
                        if (response.length() > 20) {
                            if (response.indexOf("200 OK") >= 0) {
                                responseCode = 200;
                                break;
                            }
                            if (response.indexOf("401") >= 0) {
                                responseCode = 401;
                                break;
                            }
                        }
                    }

                    credAttempts++;

                    if (responseCode == 200) {
                        if (vulnerableDevices.length() > 0) vulnerableDevices += ",";
                        vulnerableDevices += targetIp.toString();
                        vulnerableDevices += ":";
                        vulnerableDevices += String(port);
                        vulnerableDevices += " (";
                        vulnerableDevices += cred.username;
                        vulnerableDevices += "/";
                        vulnerableDevices += cred.password;
                        vulnerableDevices += ")";
                    }
                }

                delay(50);
            }

            client.stop();
            break;
        }
    }

    // Phase 3: Compile results
    progress.step("Compiling vulnerable device list and credentials");
    delay(timeoutMs / 3);

    progress.complete(String(credAttempts) + " credential attempts completed");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "Default Credentials";
    attackResult.success = (vulnerableDevices.length() > 0);
    attackResult.targetCount = devicesScanned;
    attackResult.successCount = attackResult.success ? 1 : 0;
    attackResult.failureCount = devicesScanned - (attackResult.success ? 1 : 0);
    attackResult.successPercent = attackResult.success ? 100 : 0;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.found = (vulnerableDevices.length() > 0);
    result.devicesScanned = devicesScanned;
    result.credentialsAttempted = credAttempts;
    result.vulnerableDevices = vulnerableDevices;

    return result;
}

bool testCredential(const String &target, const String &username, const String &password, const String &service) {
    using namespace ToolOutputHelper;

    displayScanStart("Credential Test", target + " - " + service);

    ScanProgressBar progress("Cred Test", 1000, 2);
    progress.start();

    WiFiClient client;
    client.setTimeout(500);

    // Phase 1: Connect to service
    progress.step("Connecting to " + service + " service on " + target);

    if (service == "HTTP") {
        if (!client.connect(target.c_str(), 80, 500)) {
            progress.complete("Connection failed");
            return false;
        }

        // Phase 2: Test authentication
        progress.step("Sending authentication credentials: " + username);

        String auth = username + ":" + password;
        String httpRequest = "GET / HTTP/1.1\r\nHost: " + target + "\r\n";
        httpRequest += "Authorization: Basic " + auth + "\r\n";
        httpRequest += "Connection: close\r\n\r\n";

        client.print(httpRequest);
        delay(200);

        bool success = false;
        while (client.available()) {
            String line = client.readStringUntil('\n');
            if (line.indexOf("200") >= 0) {
                success = true;
                progress.complete("Credentials accepted - authentication successful");
                break;
            }
        }
        client.stop();

        if (!success) {
            progress.complete("Authentication failed - invalid credentials");
        }

        return success;
    }

    progress.complete("Unsupported service type");
    return false;
}

} // namespace DefaultCredScanner
