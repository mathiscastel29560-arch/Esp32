#pragma once
#include <Arduino.h>

namespace SpectrumAnalyzerPlus {

struct FrequencyPeak {
    float frequency;
    int8_t amplitude;
    uint32_t duration;
};

struct ScanResult {
    bool success;
    uint32_t peaksFound;
    float dominantFrequency;
    int8_t dominantAmplitude;
    uint32_t durationMs;
    String analysis;
};

// Advanced spectrum analysis with peak tracking
ScanResult analyzeSpectrum(float startFreq = 400.0, float endFreq = 5000.0, uint32_t durationMs = 10000);

// Get frequency peaks
const FrequencyPeak* getFrequencyPeaks(uint32_t& outCount);

// Identify signal modulation from spectrum
String identifyModulation(float frequency);

// Detect signal pattern repetition
struct PatternResult {
    bool success;
    uint32_t patternLength;
    uint32_t repetitions;
    String patternType;  // BEACON, CONTINUOUS, PERIODIC, SPORADIC
};
PatternResult detectSignalPattern(uint32_t durationMs = 20000);

}  // namespace SpectrumAnalyzerPlus
