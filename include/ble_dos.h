#pragma once
#include <Arduino.h>

namespace BLE_DOS {

struct DOSResult {
    bool success;
    uint32_t packetsCount;
    String targetDevice;
    String method;
};

// Launch BLE Denial of Service attack
DOSResult launchDOS(const String &targetDevice, uint32_t durationMs = 10000);

}  // namespace BLE_DOS
