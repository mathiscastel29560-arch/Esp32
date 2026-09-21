#pragma once
#include <Arduino.h>

namespace AdvancedSignalCloner {

struct CloneParams {
    float frequency;        // Target frequency (MHz)
    uint32_t bitrate;       // Target bitrate (bps)
    uint8_t powerLevel;     // TX power (0-31)
    uint32_t repeatCount;   // Number of repetitions
    uint32_t delayBetweenRepsMs;  // Delay between repeats
    String modulationType;  // "ASK", "FSK", "GFSK", "OOK"
    bool useTimingInfo;     // Use original timing
};

struct CloneResult {
    bool success;
    uint32_t transmittedBytes;
    uint32_t repetitionsCompleted;
    uint32_t durationMs;
    String radioUsed;       // "CC1101", "NRF24", "BOTH"
};

// Clone/replay captured signals with adjustable parameters
CloneResult cloneSignal(const CloneParams& params = {433.0, 2400, 20, 1, 100, "ASK", false});

// Quick replay with default parameters (433 MHz Sub-GHz)
CloneResult quickReplay(uint32_t repeatCount = 1);

// Clone at 2.4GHz (NRF24)
CloneResult cloneAt2400MHz(uint32_t repeatCount = 1);

// Clone with custom timing from captured data
CloneResult cloneWithOriginalTiming(uint32_t repeatCount = 1);

// Test signal integrity before transmission
bool validateSignal();

// Get clone operation statistics
struct CloneStats {
    uint32_t lastTransmittedBytes;
    uint32_t totalCloneAttempts;
    uint32_t successfulClones;
    float averageTransmitTime;
};
CloneStats getCloneStats();

}  // namespace AdvancedSignalCloner
