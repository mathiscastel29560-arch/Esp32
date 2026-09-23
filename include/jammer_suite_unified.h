#pragma once
#include <Arduino.h>

namespace JammerSuite {

enum JammerType {
    JAMMER_WIFI_CHANNEL,      // WiFi specific channel jamming
    JAMMER_WIFI_BEACON,       // WiFi beacon corruption
    JAMMER_WIFI_ALL,          // WiFi full spectrum
    JAMMER_BLE_ADVERTISING,   // BLE advertisement jamming
    JAMMER_RF_NOISE,          // RF white noise
    JAMMER_RF_SWEEP,          // RF frequency sweep
    JAMMER_RF_FOLLOW,         // RF follow hopping
    JAMMER_SUBGHZ,            // 433MHz Sub-GHz device jamming
    JAMMER_SIGNAL_WHITE,      // Pure white noise signal
    JAMMER_SIGNAL_PINK,       // Pure pink noise signal
    JAMMER_SIGNAL_SWEEP,      // Pure frequency sweep signal
};

struct JammerConfig {
    JammerType type;
    uint32_t durationMs;
    uint32_t frequency;      // For RF jammers (433MHz, etc.)
    uint8_t channel;         // For WiFi jammer (1-14)
    String method;           // Additional method parameter
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
