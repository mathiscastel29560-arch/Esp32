#include "beacon_spam.h"
#include "safety_switch.h"
#include <esp_wifi.h>

namespace {
bool g_active = false;
std::vector<String> g_ssids;
bool g_hop = false;
uint8_t g_channel = 1;
size_t g_idx = 0;
uint32_t g_lastSend = 0;
uint32_t g_lastHop = 0;

size_t buildBeaconFrame(uint8_t *buf, const String &ssid, const uint8_t mac[6], uint8_t channel) {
    static const uint8_t tmpl[38] = {
        0x80, 0x00,                         // frame control: beacon
        0x00, 0x00,                         // duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // addr1: broadcast
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, // addr2: source (overwritten)
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, // addr3: bssid (overwritten)
        0xc0, 0x6c,                         // seq-ctl
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // timestamp
        0x64, 0x00,                         // beacon interval
        0x01, 0x04,                         // capability info
        0x00, 0x00,                         // SSID tag: id=0, len (overwritten)
    };
    memcpy(buf, tmpl, sizeof(tmpl));
    memcpy(buf + 10, mac, 6);
    memcpy(buf + 16, mac, 6);

    size_t pos = 38;
    uint8_t ssidLen = (uint8_t)min((size_t)32, ssid.length());
    buf[37] = ssidLen;
    memcpy(buf + pos, ssid.c_str(), ssidLen);
    pos += ssidLen;

    static const uint8_t rates[] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c};
    memcpy(buf + pos, rates, sizeof(rates));
    pos += sizeof(rates);

    buf[pos++] = 0x03; // DS parameter set
    buf[pos++] = 0x01;
    buf[pos++] = channel;

    return pos;
}
} // namespace

namespace BeaconSpam {

bool start(const std::vector<String> &ssids, bool hopChannels) {
    if (!SafetySwitch::isArmed()) return false;
    if (ssids.empty()) return false;

    g_ssids = ssids;
    g_hop = hopChannels;
    g_channel = 1;
    g_idx = 0;
    g_lastSend = 0;
    g_lastHop = millis();
    esp_wifi_set_channel(g_channel, WIFI_SECOND_CHAN_NONE);
    g_active = true;
    return true;
}

void stop() { g_active = false; }
bool active() { return g_active; }

void loop() {
    if (!g_active) return;
    uint32_t now = millis();

    if (g_hop && now - g_lastHop > 500) {
        g_channel = (g_channel % 11) + 1;
        esp_wifi_set_channel(g_channel, WIFI_SECOND_CHAN_NONE);
        g_lastHop = now;
    }

    if (now - g_lastSend < 100) return;
    g_lastSend = now;

    uint8_t mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, (uint8_t)g_idx};
    uint8_t frame[128];
    size_t len = buildBeaconFrame(frame, g_ssids[g_idx], mac, g_channel);
    esp_wifi_80211_tx(WIFI_IF_AP, frame, len, false);

    g_idx = (g_idx + 1) % g_ssids.size();
}

} // namespace BeaconSpam
