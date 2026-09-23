#pragma once
#include <Arduino.h>

namespace JammingSignalGenerator {

struct JamResult {
    bool success;
    uint32_t signalsGenerated;
    uint32_t durationMs;
    String noiseType;
};

// Generate pure jamming noise/signals
// noiseType: "WHITE" (white noise), "PINK" (pink noise), "SWEEP" (frequency sweep)
JamResult generateJammingSignal(uint32_t durationMs = 10000, const String &noiseType = "WHITE");

void stop();
bool isActive();

}  // namespace JammingSignalGenerator
