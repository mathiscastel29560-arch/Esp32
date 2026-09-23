#pragma once
#include <Arduino.h>

namespace BluetoothAggressiveJammer {

struct JamResult {
    bool success;
    uint32_t jamPacketsCount;
    uint32_t durationMs;
};

JamResult jamBluetooth(uint32_t durationMs = 10000);
void stop();
bool isActive();

}  // namespace BluetoothAggressiveJammer
