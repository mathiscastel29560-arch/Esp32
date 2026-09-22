#pragma once

#include <Arduino.h>
#include <string>

namespace BleSpam {

enum SpamType {
    CONTINUITY,      // Apple Continuity (AirDrop/Handoff)
    FAST_PAIR,       // Google Fast Pair
    SWIFT_PAIR,      // Microsoft Swift Pair
    GENERIC_PAIRING, // Generic BLE pairing flood
};

struct SpamConfig {
    SpamType type;
    uint32_t durationMs;      // spam duration in milliseconds
    uint16_t packetsPerSec;   // packets per second (intensity)
    uint8_t targetChannel;    // 37, 38, or 39 (BLE advertising channels)
    bool randomizeChannel;    // rotate through all 3 channels
    String customSSID;        // optional custom SSID for Fast Pair
};

struct SpamResult {
    bool success;
    uint32_t packetsSent;
    uint32_t elapsedMs;
    String logFile;
    String error;
};

// Start BLE spam attack with given configuration
// Returns result with statistics
// TX safety: requires TxArm::isArmed() to actually transmit
SpamResult spam(const SpamConfig &config);

// Stop any ongoing spam attack
void stop();

// Get current spam status
bool isActive();

// Get packet count so far
uint32_t getPacketCount();

} // namespace BleSpam
