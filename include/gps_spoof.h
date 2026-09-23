#pragma once
#include <Arduino.h>

namespace GPSSpoof {

struct SpoofResult {
    bool success;
    uint32_t packetsCount;
    float spoofedLat;
    float spoofedLon;
    uint32_t durationMs;
    String method;
};

struct SpoofLocation {
    float latitude;
    float longitude;
    const char *name;
};

// Spoof GPS coordinates and transmit via raw RF or GNSS signal simulation
// location: target coordinates to spoof
// method: "SIGNAL" (RF simulation), "GRADUAL" (slow drift), "RANDOM" (jitter)
SpoofResult spoofGPS(float latitude, float longitude, uint32_t durationMs = 10000,
                     const String &method = "SIGNAL");

// List preset locations for spoofing
const SpoofLocation PRESET_LOCATIONS[] = {
    {51.5074f, -0.1278f, "London, UK"},
    {48.8566f, 2.3522f, "Paris, France"},
    {40.7128f, -74.0060f, "New York, USA"},
    {35.6762f, 139.6503f, "Tokyo, Japan"},
    {37.7749f, -122.4194f, "San Francisco, USA"},
    {39.9526f, -75.1652f, "Philadelphia, USA"},
    {30.0000f, 31.0000f, "Egypt"},
};

// Stop active GPS spoofing
void stop();

// Check if spoofing is active
bool isActive();

}  // namespace GPSSpoof
