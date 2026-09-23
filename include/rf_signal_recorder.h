#pragma once
#include <Arduino.h>
#include <vector>

namespace RfSignalRecorder {

struct RecordingResult {
    bool success;
    uint32_t sampleCount;
    uint32_t durationMs;
    float frequency;
    float rssiMin;
    float rssiMax;
    float rssiAvg;
};

// Record raw RF signals from available radios (CC1101 433MHz, NRF24 2.4GHz)
// Stores up to 64KB of samples depending on available PSRAM
RecordingResult recordSignals(float frequencyMHz, uint32_t durationMs, const char* radioType = "auto");

// Get captured signal data (pointer valid until next record() call)
const uint8_t* getCapturedData(uint32_t& outLength);

// Clear captured data from memory
void clearRecording();

// Get signal statistics
struct SignalStats {
    uint32_t totalSamples;
    float peakSignalStrength;
    float averageSignalStrength;
    uint32_t transitionCount;
    uint32_t silenceGaps;
};
SignalStats analyzeSignal();

}  // namespace RfSignalRecorder
