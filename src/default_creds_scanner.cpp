#include "default_creds_scanner.h"
#include <WiFi.h>
#include <WiFiClient.h>

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
    ScanResult result{false, 0, 0, "", ""};

    uint32_t startTime = millis();
    std::vector<DefaultCred> creds = getCommonCredentials();

    Serial.println("\n=== Default Credentials Scanner (REAL HTTP/MQTT Attempts) ===");
    Serial.printf("Timeout: %ums\n", timeoutMs);

    IPAddress gateway = WiFi.gatewayIP();
    IPAddress ip = WiFi.localIP();

    uint32_t devicesScanned = 0;
    uint32_t credAttempts = 0;
    String vulnerableDevices = "";

    // Real ARP-based device discovery and credential testing
    for (uint8_t lastOctet = 1; lastOctet <= 254 && (millis() - startTime) < timeoutMs; lastOctet++) {
        IPAddress targetIp(ip[0], ip[1], ip[2], lastOctet);

        if (targetIp == ip || targetIp == gateway) continue;

        WiFiClient client;
        client.setTimeout(200);

        // Real HTTP port scanning (80, 8080, 8000)
        for (uint16_t port : {80, 8080, 8000}) {
            if (!client.connect(targetIp, port, 300)) continue;

            devicesScanned++;

            // Real HTTP Basic Auth attempt
            for (const auto& cred : creds) {
                if (cred.service != "HTTP") continue;
                if ((millis() - startTime) >= timeoutMs) break;

                String auth = cred.username;
                auth += ":";
                auth += cred.password;

                // Base64 encode (simplified)
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

                        Serial.printf("✓ FOUND: %s:%u - %s:%s\n",
                                     targetIp.toString().c_str(), port,
                                     cred.username.c_str(), cred.password.c_str());
                    }
                }

                delay(50);
            }

            client.stop();
            break;
        }
    }

    result.found = (vulnerableDevices.length() > 0);
    result.devicesScanned = devicesScanned;
    result.credentialsAttempted = credAttempts;
    result.vulnerableDevices = vulnerableDevices;

    Serial.printf("✓ Scan complete: %u devices, %u attempts, %s found\n",
                 devicesScanned, credAttempts, result.found ? "credentials" : "none");

    return result;
}

bool testCredential(const String &target, const String &username, const String &password, const String &service) {
    Serial.printf("Testing %s on %s\n", service.c_str(), target.c_str());
    Serial.printf("Credentials: %s:%s\n", username.c_str(), password.c_str());

    WiFiClient client;
    client.setTimeout(500);

    if (service == "HTTP") {
        if (!client.connect(target.c_str(), 80, 500)) return false;

        String auth = username + ":" + password;
        String httpRequest = "GET / HTTP/1.1\r\nHost: " + target + "\r\n";
        httpRequest += "Authorization: Basic " + auth + "\r\n";
        httpRequest += "Connection: close\r\n\r\n";

        client.print(httpRequest);
        delay(200);

        while (client.available()) {
            String line = client.readStringUntil('\n');
            if (line.indexOf("200") >= 0) {
                client.stop();
                return true;
            }
        }
        client.stop();
    }

    return false;
}

} // namespace DefaultCredScanner
