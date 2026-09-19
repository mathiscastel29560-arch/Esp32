#include "wifi_tools.h"
#include "config.h"
#include "mac_utils.h"

#include <WiFi.h>
#include <esp_wifi.h>

namespace {

// ---------------- passive client observation ----------------
volatile bool g_sniffing = false;
uint8_t g_targetBssid[6];
std::vector<String> g_sniffResults;

void IRAM_ATTR promiscCb(void *buf, wifi_promiscuous_pkt_type_t type) {
    if (!g_sniffing) return;
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) return;
    auto *pkt = (wifi_promiscuous_pkt_t *)buf;
    if (pkt->rx_ctrl.sig_len < 24) return;
    const uint8_t *addr1 = pkt->payload + 4;
    const uint8_t *addr2 = pkt->payload + 10;

    const uint8_t *client = nullptr;
    if (memcmp(addr1, g_targetBssid, 6) == 0 && memcmp(addr2, g_targetBssid, 6) != 0) {
        client = addr2;
    } else if (memcmp(addr2, g_targetBssid, 6) == 0 &&
               memcmp(addr1, MacUtils::BROADCAST, 6) != 0 &&
               memcmp(addr1, g_targetBssid, 6) != 0) {
        client = addr1;
    }
    if (!client) return;

    String macStr = MacUtils::toString(client);
    for (auto &s : g_sniffResults) if (s == macStr) return;
    if (g_sniffResults.size() < 64) g_sniffResults.push_back(macStr);
}

} // namespace

namespace WifiTools {

void begin(const String &apSsid, const String &apPassword) {
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(apSsid.c_str(), apPassword.c_str(), AP_CHANNEL);
    esp_wifi_set_promiscuous_rx_cb(&promiscCb);
}

std::vector<ApInfo> scan() {
    std::vector<ApInfo> out;
    int n = WiFi.scanNetworks(false, true);
    for (int i = 0; i < n; i++) {
        ApInfo a;
        a.ssid = WiFi.SSID(i);
        a.bssid = WiFi.BSSIDstr(i);
        a.rssi = WiFi.RSSI(i);
        a.channel = WiFi.channel(i);
        switch (WiFi.encryptionType(i)) {
            case WIFI_AUTH_OPEN: a.enc = "OPEN"; break;
            case WIFI_AUTH_WEP: a.enc = "WEP"; break;
            case WIFI_AUTH_WPA_PSK: a.enc = "WPA"; break;
            case WIFI_AUTH_WPA2_PSK: a.enc = "WPA2"; break;
            case WIFI_AUTH_WPA_WPA2_PSK: a.enc = "WPA/WPA2"; break;
            case WIFI_AUTH_WPA3_PSK: a.enc = "WPA3"; break;
            default: a.enc = "?"; break;
        }
        out.push_back(a);
    }
    WiFi.scanDelete();
    return out;
}

std::vector<String> sniffClients(const String &bssid, uint8_t channel, uint32_t durationMs) {
    g_sniffResults.clear();
    if (!MacUtils::parse(bssid, g_targetBssid)) return g_sniffResults;

    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    g_sniffing = true;
    esp_wifi_set_promiscuous(true);

    uint32_t start = millis();
    while (millis() - start < durationMs) {
        delay(10);
    }

    esp_wifi_set_promiscuous(false);
    g_sniffing = false;
    return g_sniffResults;
}

} // namespace WifiTools
