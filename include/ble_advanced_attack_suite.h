#pragma once
#include <Arduino.h>

namespace BLEAdvancedAttackSuite {

struct AttackResult {
    bool success;
    uint32_t attackPacketsCount;
    uint32_t durationMs;
    String method;
};

// GATT jammer + Eavesdropper hybrid
// method: "GATT" (jam GATT services), "EAVES" (eavesdrop), "ALL" (both)
AttackResult attackBLE(uint32_t durationMs = 10000, const String &method = "ALL");

void stop();
bool isActive();

}  // namespace BLEAdvancedAttackSuite
