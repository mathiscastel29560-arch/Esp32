#pragma once
#include <Arduino.h>
#include <vector>

namespace IoTDeviceHunter {

struct DetectedDevice {
    String vendor;
    String deviceType;
    int32_t signal;    // RSSI in dBm (typically -100 to -30)
    String protocol;   // "WiFi", "BLE", "Sub-GHz"
    uint32_t timestamp;
};

struct HuntResult {
    uint32_t devicesFound;
    std::vector<DetectedDevice> devices;
};

// Scan for common IoT devices (smart home, medical, etc)
HuntResult huntDevices(uint32_t durationMs = 15000);

}  // namespace IoTDeviceHunter
