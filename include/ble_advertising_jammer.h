#pragma once
#include <Arduino.h>

namespace BLEAdvertisingJammer {

struct JamResult {
    bool success;
    uint32_t jamPacketsCount;
    uint32_t durationMs;
    String method;
};

// Jam BLE advertising channels with noise/interference
// Disrupts legitimate BLE advertisements and scanning
JamResult jamAdvertising(uint32_t durationMs = 10000, const String &method = "NOISE");

// Stop active jamming
void stop();

// Check if jamming is active
bool isActive();

}  // namespace BLEAdvertisingJammer
