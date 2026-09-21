#pragma once
#include <Arduino.h>

namespace BluetoothClassic {

struct ClassicDevice {
    String bdAddress;
    String deviceName;
    int8_t rssi;
    String deviceClass;
    uint32_t codMajor;  // Class of Device
    bool discoverable;
    uint32_t timestamp;
};

struct ScanResult {
    bool success;
    uint32_t deviceCount;
    uint32_t durationMs;
    int8_t strongestRssi;
};

// Scan for Bluetooth Classic devices
ScanResult scanClassicDevices(uint32_t durationMs = 10000);

// Get discovered devices
const ClassicDevice* getDiscoveredDevices(uint32_t& outCount);

// Attack: Pairing interception
struct PairingInterceptResult {
    bool success;
    uint32_t attemptCount;
    uint32_t pairingCodeFound;
    uint32_t durationMs;
};
PairingInterceptResult interceptPairingAttempt(uint32_t durationMs = 60000);

// Attack: Audio hijacking (HFP/A2DP)
struct AudioHijackResult {
    bool success;
    uint32_t durationMs;
    String audioProfile;  // HFP, A2DP, AVRCP
    String action;        // STREAM_HIJACK, CALL_HIJACK, MEDIA_HIJACK
};
AudioHijackResult hijackAudioStream(const char* targetAddress, uint32_t durationMs = 30000);

// Attack: Bluetooth Name Spoofing
struct SpoofResult {
    bool success;
    String spoofedName;
    uint32_t durationMs;
};
SpoofResult spoofBluetoothName(const char* targetName = "iPhone", uint32_t durationMs = 5000);

// Attack: SSP (Secure Simple Pairing) bypass
struct SspBypassResult {
    bool success;
    uint32_t attemptCount;
    String vulnerabilityType;
    uint32_t durationMs;
};
SspBypassResult bypassSSP(uint32_t durationMs = 45000);

// Get Bluetooth Classic statistics
struct ClassicStats {
    uint32_t totalDevicesFound;
    uint32_t connectedDevices;
    uint32_t headphoneDevices;
    uint32_t autoPlayDevices;
    float averageRssi;
};
ClassicStats getClassicStats();

}  // namespace BluetoothClassic
