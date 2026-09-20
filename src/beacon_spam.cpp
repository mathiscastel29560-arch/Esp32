#include "beacon_spam.h"
#include "tx_arm.h"
#include <esp_wifi.h>
#include <esp_wifi_types.h>

namespace BeaconSpam {

typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    uint16_t seq_ctrl;
    uint32_t timestamp;
    uint16_t beacon_interval;
    uint16_t capabilities;
} __attribute__((packed)) BeaconFrame;

void spam(const std::vector<String> &ssidList, uint8_t channel, uint16_t durationMs) {
    if (!TxArm::isArmed()) {
        Serial.println("TX not armed - hold BACK to enable");
        return;
    }

    Serial.println("Starting beacon spam on channel " + String(channel));
    Serial.println("SSIDs: " + String(ssidList.size()));

    unsigned long startTime = millis();
    uint16_t seq = 0;

    while (millis() - startTime < durationMs && TxArm::isArmed()) {
        for (const auto &ssid : ssidList) {
            BeaconFrame frame;
            frame.frame_control = 0x80;
            frame.duration = 0;
            frame.beacon_interval = 100;
            frame.capabilities = 0x0401;
            frame.seq_ctrl = (seq++ << 4);
            frame.timestamp = millis() * 1000;

            memset(frame.da, 0xff, 6);
            memset(frame.sa, 0x11, 6);
            memset(frame.bssid, 0x22, 6);

            esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            esp_wifi_80211_tx(WIFI_IF_STA, (void *)&frame, sizeof(frame), false);

            delay(50);
        }
    }

    Serial.println("Beacon spam complete ✓");
}

} // namespace BeaconSpam
