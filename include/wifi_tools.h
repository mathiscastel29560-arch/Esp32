#pragma once
#include <Arduino.h>
#include <vector>

// Passive Wi-Fi recon: network scanning and receive-only client observation.
// Nothing in this module transmits a frame; it only listens.
namespace WifiTools {

struct ApInfo {
    String ssid;
    String bssid;
    int32_t rssi;
    uint8_t channel;
    String enc;
};

// Brings the radio up in WIFI_MODE_APSTA and starts the control-panel
// SoftAP (apSsid/apPassword). Must be called once at boot before anything
// else in this namespace.
void begin(const String &apSsid, const String &apPassword);

std::vector<ApInfo> scan();

// Puts the radio in promiscuous mode on `channel` for `durationMs` and
// collects the MAC addresses of stations seen talking to `bssid`.
// Best-effort: it inspects addr1/addr2 of every captured frame, it does not
// fully parse 802.11 frame semantics.
std::vector<String> sniffClients(const String &bssid, uint8_t channel, uint32_t durationMs);

} // namespace WifiTools
