#pragma once
#include <Arduino.h>

namespace AdvancedRFJammer {

struct JamResult {
    bool success;
    uint32_t jamPacketsCount;
    uint32_t durationMs;
    String method;
    String frequency;
};

// Jam RF signals - supports both static and frequency-hopping targets
// method: "NOISE" (white noise), "SWEEP" (frequency sweep), "FOLLOW" (track hopping)
JamResult jamRFSignals(const String &frequency = "433MHz", uint32_t durationMs = 10000,
                       const String &method = "NOISE");

// Stop active jamming
void stop();

// Check if jamming is active
bool isActive();

}  // namespace AdvancedRFJammer
