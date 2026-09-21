#pragma once
#include <Arduino.h>

namespace WiFiDeauth {

struct DeauthResult {
    bool success;
    uint32_t deauthCount;
    String targetSSID;
    String targetBSSID;
    uint32_t durationMs;
};

// Send deauthentication frames to disconnect clients from WiFi
// targetBSSID: MAC address of WiFi AP (e.g., "AA:BB:CC:DD:EE:FF")
// broadcastClients: if true, target all connected clients; if false, target specific BSSID
DeauthResult sendDeauthFrames(const String &targetBSSID, uint32_t durationMs = 10000, bool broadcastClients = true);

// Stop active deauth attack
void stop();

// Check if deauth is active
bool isActive();

}  // namespace WiFiDeauth
