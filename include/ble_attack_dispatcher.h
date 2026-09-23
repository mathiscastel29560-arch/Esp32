#pragma once
#include <Arduino.h>

namespace BLEAttackDispatcher {

// 3 unified BLE attack modes for pairing, fuzzing, and audio control
enum AttackType {
    ATTACK_PAIRING = 0,     // BLE pairing hijacking (ECDH MITM)
    ATTACK_FUZZER,          // Device fuzzing (robust connection test - own device only)
    ATTACK_AUDIO_HIJACK,    // Audio device control hijacking (volume, media control)
};

struct AttackConfig {
    AttackType type;            // Attack type to execute
    String targetDevice;        // Target device address/name (pairing/fuzzer/audio)
    uint32_t durationMs;        // Attack duration in milliseconds
    uint32_t timeoutMs;         // Timeout for operations (1ms-600s)
    uint8_t targetAddr[6];      // MAC address for audio hijacking target
};

struct AttackResult {
    bool success;               // Attack succeeded
    String targetDevice;        // Target device info
    String method;              // Attack method used (e.g., "FUZZ", "AUDIO_HIJACK")
    uint32_t attemptsOrCommands;// Attempts or commands sent
    uint32_t elapsedMs;         // Elapsed time in milliseconds
    String error;               // Error message (if failed)
    String details;             // Additional details (e.g., "Oversized:2/5")
};

// Start unified BLE attack
AttackResult executeAttack(const AttackConfig &config);

// Stop active attack
void stop();

// Check if attack is active
bool isActive();

}  // namespace BLEAttackDispatcher
