#pragma once
#include <Arduino.h>

namespace BLEAttackDispatcher {

enum AttackType {
    ATTACK_PAIRING,         // BLE pairing hijacking
    ATTACK_FUZZER,          // Device fuzzing (robust connection test)
    ATTACK_AUDIO_HIJACK,    // Audio device control hijacking
};

struct AttackConfig {
    AttackType type;
    String targetDevice;        // For pairing/fuzzer/audio
    uint32_t durationMs;        // Attack duration
    uint32_t timeoutMs;         // Timeout for operations
    uint8_t targetAddr[6];      // MAC address for audio hijacking
};

struct AttackResult {
    bool success;
    String targetDevice;
    String method;
    uint32_t attemptsOrCommands;
    uint32_t elapsedMs;
    String error;
    String details;
};

// Start unified BLE attack
AttackResult executeAttack(const AttackConfig &config);

// Stop active attack
void stop();

// Check if attack is active
bool isActive();

}  // namespace BLEAttackDispatcher
