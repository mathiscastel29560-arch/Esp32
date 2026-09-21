#pragma once
#include <Arduino.h>

namespace BLEPairingAttack {

struct PairingResult {
    bool success;
    String targetDevice;
    String method;
    uint32_t attemptsCount;
};

// Attempt to hijack BLE pairing process
PairingResult attackPairing(const String &targetDevice, uint32_t timeoutMs = 10000);

}  // namespace BLEPairingAttack
