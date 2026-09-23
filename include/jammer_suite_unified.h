#pragma once
#include <Arduino.h>

namespace JammerSuite {

// 11 unified jammer types covering WiFi, BLE, RF, Sub-GHz, and signal generation
enum JammerType {
    JAMMER_WIFI_CHANNEL = 0,   // WiFi specific channel jamming
    JAMMER_WIFI_BEACON,        // WiFi beacon corruption
    JAMMER_WIFI_ALL,           // WiFi full spectrum
    JAMMER_BLE_ADVERTISING,    // BLE advertisement jamming
    JAMMER_RF_NOISE,           // RF white noise
    JAMMER_RF_SWEEP,           // RF frequency sweep
    JAMMER_RF_FOLLOW,          // RF follow hopping
    JAMMER_SUBGHZ,             // 433MHz Sub-GHz device jamming
    JAMMER_SIGNAL_WHITE,       // Pure white noise signal
    JAMMER_SIGNAL_PINK,        // Pure pink noise signal
    JAMMER_SIGNAL_SWEEP,       // Pure frequency sweep signal
};

struct JammerConfig {
    JammerType type;           // Attack type to execute
    uint32_t durationMs;       // Duration: 100ms-600s recommended
    uint32_t frequency;        // For RF jammers (default 433MHz = 433000000)
    uint8_t channel;           // For WiFi jammer (1-14, default 6)
    String method;             // Method: WiFi ("ALL","CHANNEL","BEACON"), RF ("NOISE","SWEEP","FOLLOW"), Signal ("WHITE","PINK","SWEEP")
};

struct JamResult {
    bool success;
    uint32_t packetsGenerated;
    uint32_t durationMs;
    uint32_t frequency;
    String error;
};

// Start unified jamming attack
JamResult startJamming(const JammerConfig &config);

// Stop active jamming
void stop();

// Check if jamming is active
bool isActive();

// Get current jammer type
JammerType getCurrentType();

}  // namespace JammerSuite
