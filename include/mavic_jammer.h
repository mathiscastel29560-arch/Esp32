#pragma once
#include <Arduino.h>
#include <vector>

namespace MavicJammer {

// DJI Mavic/Air operates on 2.4GHz WiFi + proprietary ISM band
// WiFi control: channels 1-13 (2.412-2.472 GHz, 20MHz wide)
// Proprietary link: 2.4GHz ISM band with frequency hopping

struct JammerConfig {
    uint32_t durationMs;           // Jamming duration
    uint8_t method;                // 0=noise, 1=sweep, 2=sync
    bool targetWifi;               // Jam WiFi control link
    bool targetProprietary;        // Jam proprietary link
    uint8_t txPower;               // TX power (0-20 dBm)
};

struct JammerResult {
    bool success;
    uint32_t durationMs;
    uint32_t packetsJammed;
    uint32_t frequencyChanges;
    String error;
};

// Jam DJI Mavic control links via 2.4GHz interference
// Method 0: Continuous noise on control channels
// Method 1: Frequency sweep (2.400-2.483 GHz)
// Method 2: Sync-based jamming (follow hopping pattern)
JammerResult jammMavicController(const JammerConfig& config);

// Advanced: Replay last captured DJI packet (spoof remote command)
JammerResult replayMavicCommand(const std::vector<uint8_t>& capturedFrame);

// Analyze DJI frequency hopping pattern for sync jamming
JammerResult analyzeMavicHoppingPattern(uint32_t scanDurationMs);

}  // namespace MavicJammer
