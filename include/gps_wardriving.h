#pragma once
#include <Arduino.h>
#include <vector>

namespace GpsWardriving {

struct WardriveEntry {
    float latitude;
    float longitude;
    uint32_t timestamp;
    String ssid;
    int8_t rssi;
    String bssid;
    String type;  // "WiFi" or "BLE"
};

struct WardriveSession {
    uint32_t startTime;
    uint32_t endTime;
    std::vector<WardriveEntry> entries;
    float startLat, startLon;
    float endLat, endLon;
    uint32_t totalDistance;  // meters
};

// Start GPS wardriving session
void beginSession();

// Log WiFi network at current GPS position
void logWiFiNetwork(const String &ssid, const String &bssid, int8_t rssi);

// Log BLE device at current GPS position
void logBLEDevice(const String &name, const String &address, int8_t rssi);

// End session and export to CSV
WardriveSession endSession();

// Get current GPS fix status
bool hasGPSFix();

// Export all wardrive entries to file
bool exportToKML(const String &filename, const WardriveSession &session);

}  // namespace GpsWardriving
