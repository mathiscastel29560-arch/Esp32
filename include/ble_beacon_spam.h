#pragma once
#include <Arduino.h>

namespace BLEBeaconSpam {

struct SpamResult {
    bool success;
    uint32_t beaconsCount;
    uint32_t durationMs;
    String beaconType;
};

// Spam BLE beacons (Apple Continuity, Google Fast Pair, Microsoft Swift Pair)
SpamResult spamBeacons(const String &beaconType = "ALL", uint32_t durationMs = 10000);

// Stop active beacon spam
void stop();

// Check if spam is active
bool isActive();

}  // namespace BLEBeaconSpam
