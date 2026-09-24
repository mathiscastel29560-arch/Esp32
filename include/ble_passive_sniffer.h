#pragma once
#include <Arduino.h>

namespace BLEPassiveSniffer {

struct BLEAdvertisement {
    String macAddress;
    String deviceName;
    String advertisementData;
    int rssi;
    uint8_t txPower;
    String flags;
    uint32_t timestamp;
};

struct SniffResult {
    bool success;
    uint32_t devicesDiscovered;
    uint32_t packetsCapture;
    int averageRssi;
    uint32_t durationMs;
};

// Passive BLE listening (no transmission)
SniffResult passiveSniff(uint32_t durationMs = 30000);
const BLEAdvertisement* getDiscoveredAdvertisements(uint32_t& outCount);

// Analyze specific device
struct DeviceAnalysis {
    bool success;
    uint32_t advertisementCount;
    String deviceVendor;
    int strongestRssi;
};
DeviceAnalysis analyzeDevice(const char* macAddress, uint32_t durationMs = 10000);

}  // namespace BLEPassiveSniffer
