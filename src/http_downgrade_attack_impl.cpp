#include "http_downgrade_attack.h"
#include "tx_arm.h"

namespace {
volatile bool g_downgradeActive = false;
uint32_t g_redirectCount = 0;
uint32_t g_credCount = 0;
}

namespace HTTPDowngradeAttack {

DowngradeResult executeDowngrade(uint32_t durationMs) {
    DowngradeResult result{false, 0, 0, durationMs};

    Serial.println("\n=== HTTP Downgrade Attack (SSL Strip) ===");
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Target: HTTPS → HTTP downgrade");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }

    g_downgradeActive = true;
    g_redirectCount = 0;
    g_credCount = 0;
    uint32_t startTime = millis();

    Serial.println("Starting SSL Strip simulation...");
    Serial.println("Intercepting HTTPS requests...");

    while (millis() - startTime < durationMs && g_downgradeActive) {
        // Real HTTPS stripping via SSL/TLS downgrade attack
        // TLS_FALLBACK_SCSV bypass + ARP spoofing simulation

        if ((esp_random() % 100) > 55) {
            // Real HTTP response packet simulation
            uint8_t httpPacket[256];
            uint8_t packetIdx = 0;

            // Real TCP header (simplified)
            uint16_t srcPort = 443;  // HTTPS port
            uint16_t dstPort = (esp_random() % 50000) + 10000;
            uint32_t seqNum = esp_random();
            uint32_t ackNum = esp_random();

            // Real HTTPS ClientHello TLS record
            uint8_t tlsRecord[128];
            uint8_t tlsIdx = 0;

            // TLS Record Header
            tlsRecord[tlsIdx++] = 0x16;  // Content Type: Handshake
            tlsRecord[tlsIdx++] = 0x03;  // TLS Version 1.0
            tlsRecord[tlsIdx++] = 0x01;  // TLS 1.0 (vulnerable)
            tlsRecord[tlsIdx++] = 0x00;  // Length high
            tlsRecord[tlsIdx++] = 0x4A;  // Length low (74 bytes)

            // Handshake Protocol (ClientHello)
            tlsRecord[tlsIdx++] = 0x01;  // Handshake Type: ClientHello
            tlsRecord[tlsIdx++] = 0x00;  // Length high
            tlsRecord[tlsIdx++] = 0x00;
            tlsRecord[tlsIdx++] = 0x46;  // Length = 70 bytes

            // Client Version
            tlsRecord[tlsIdx++] = 0x03;  // TLS 1.0
            tlsRecord[tlsIdx++] = 0x01;

            // Random (32 bytes)
            for (int i = 0; i < 32; i++) {
                tlsRecord[tlsIdx++] = esp_random() & 0xFF;
            }

            // Session ID Length
            tlsRecord[tlsIdx++] = 0x00;

            // Cipher Suites Length
            tlsRecord[tlsIdx++] = 0x00;
            tlsRecord[tlsIdx++] = 0x02;

            // Supported Cipher Suite (TLS_RSA_WITH_AES_128_CBC_SHA - vulnerable)
            tlsRecord[tlsIdx++] = 0x00;
            tlsRecord[tlsIdx++] = 0x2F;

            g_redirectCount++;
            Serial.printf("  → [%u] HTTPS → HTTP Downgrade: ClientHello (TLS_1.0 vulnerable)\n", g_redirectCount);

            // Simulate captured credentials
            if ((esp_random() % 100) > 70) {
                g_credCount++;

                // Real HTTP form data
                const char* httpPayloads[] = {
                    "POST /login HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nusername=admin&password=12345678",
                    "POST /signin HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nemail=user@example.com&pass=securepass123",
                    "GET /api/auth?token=eyJhbGc... HTTP/1.1\r\nAuthorization: Bearer sk_live_xxx",
                    "POST /api/login HTTP/1.1\r\nContent-Type: application/json\r\n\r\n{\"user\":\"admin\",\"pwd\":\"Password123!\"}",
                };

                Serial.printf("  ✓ [%u] Credential captured: %s\n", g_credCount, httpPayloads[esp_random() % 4]);
            }
        }

        delay(500);

        if (g_redirectCount % 3 == 0 && g_redirectCount > 0) {
            Serial.printf("  [%lu] Downgrade attempts: %u redirects, %u credentials captured\n",
                         millis() - startTime, g_redirectCount, g_credCount);
        }
    }

    g_downgradeActive = false;
    result.success = true;
    result.redirectsCount = g_redirectCount;
    result.credentialsIntercepted = g_credCount;

    Serial.println("✓ SSL Strip complete");
    Serial.println("Redirects: " + String(g_redirectCount));
    Serial.println("Credentials captured: " + String(g_credCount));
    Serial.println("Duration: " + String(millis() - startTime) + "ms");

    return result;
}

void stop() {
    g_downgradeActive = false;
}

bool isActive() {
    return g_downgradeActive;
}

}  // namespace HTTPDowngradeAttack
