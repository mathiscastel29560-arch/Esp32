#pragma once
#include <Arduino.h>

namespace WiFiJammerSuite {

struct JamResult {
    bool success;
    uint32_t jamPacketsCount;
    uint32_t durationMs;
    String method;
};

// WiFi jamming suite - channel jammer + beacon DoS hybrid
// method: "CHANNEL" (jam specific channel), "BEACON" (corrupt beacons), "ALL" (both)
JamResult jamWiFiNetwork(uint8_t channel = 6, uint32_t durationMs = 10000,
                         const String &method = "ALL");

void stop();
bool isActive();

}  // namespace WiFiJammerSuite
