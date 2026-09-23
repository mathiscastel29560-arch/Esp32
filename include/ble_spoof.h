#pragma once
#include <Arduino.h>

namespace BLESpoof {

struct SpoofResult {
    bool success;
    String originalMAC;
    String spoofedMAC;
    String targetDevice;
};

// Spoof BLE MAC address to impersonate device
SpoofResult spoofBLEAddress(const String &targetDevice, const String &newMAC);

// Scan and list BLE devices for spoofing
void listBLEDevices();

}  // namespace BLESpoof
