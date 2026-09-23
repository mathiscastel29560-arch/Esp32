#pragma once
#include <Arduino.h>
#include <vector>

namespace WiFiHiddenRevealer {

struct RevealedNetwork {
    String ssid;
    String bssid;
    int8_t rssi;
    uint8_t channel;
    String security;
};

struct RevealResult {
    uint32_t networksFound;
    std::vector<RevealedNetwork> networks;
};

// Reveal hidden WiFi networks via deauth probe requests
RevealResult revealHiddenNetworks(uint32_t durationMs = 10000);

}  // namespace WiFiHiddenRevealer
