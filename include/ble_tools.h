#pragma once
#include <Arduino.h>
#include <vector>

// Passive BLE recon: advertisement scanning/inventory only. Nothing here
// transmits advertising data.
namespace BleTools {

struct BleDevice {
    String address;
    String name;      // may be empty if the device doesn't advertise one
    int rssi;
};

void begin();
std::vector<BleDevice> scan(uint32_t durationSeconds);

} // namespace BleTools
