#pragma once
#include <Arduino.h>
#include <vector>

// BLE recon (FEATURES.md Module 1): advertisement scanning/inventory only.
// Nothing here transmits advertising data or connects to anything.
namespace BleTools {

struct BleDevice {
    String address;
    bool randomAddress = false;
    String name;              // may be empty if the device doesn't advertise one
    int rssi = 0;
    int manufacturerId = -1;  // -1 if no manufacturer data advertised
    String manufacturerName;  // looked-up name, empty if unknown/unset
    std::vector<String> serviceUuids;
};

void begin();

// Sorted strongest-signal first.
std::vector<BleDevice> scan(uint32_t durationSeconds);

} // namespace BleTools
