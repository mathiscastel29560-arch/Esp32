#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace WiFiDeauth {

enum DeauthMode {
    TARGETED,        // Specific BSSID + MAC client
    BROADCAST,       // Deauth ALL clients on specific BSSID
    CHANNEL_SWEEP,   // Hit all channels 1-14 simultaneously
    NUCLEAR,         // All channels + all BSSIDs (total RF storm)
};

struct DeauthConfig {
    DeauthMode mode;
    uint8_t targetBssid[6];      // For TARGETED/BROADCAST
    uint8_t targetClient[6];     // For TARGETED only
    uint8_t targetChannel;       // For TARGETED/BROADCAST
    uint32_t durationMs;         // Attack duration
    uint16_t packetsPerSec;      // Intensity (default 100-300)
    bool randomizeMac;           // Spoof source MAC each packet
    bool repeatSequence;         // Repeat same sequence number (more destructive)
};

struct DeauthResult {
    bool success;
    uint32_t packetsSent;
    uint32_t elapsedMs;
    uint8_t channelsTested;      // For channel sweep
    String error;
};

// Nuclear deauth: all channels, all BSSIDs, maximum disruption
DeauthResult nuclearOption(uint32_t durationMs);

// Broadcast deauth: hit everyone on one BSSID
DeauthResult broadcastDeauth(const DeauthConfig &config);

// Channel sweep: rotate through all WiFi channels
DeauthResult channelSweep(const DeauthConfig &config);

// Targeted: specific BSSID + optional client MAC
DeauthResult targeted(const DeauthConfig &config);

// Stop active deauth
void stop();

// Check if deauth is active
bool isActive();

// Backward compatible API (simple broadcast deauth)
DeauthResult sendDeauthFrames(const String &targetBSSID, uint32_t durationMs = 10000, bool broadcastClients = true);

}  // namespace WiFiDeauth
