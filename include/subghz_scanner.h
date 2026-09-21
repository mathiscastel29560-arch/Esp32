#pragma once
#include <Arduino.h>
#include <vector>

namespace SubghzScanner {

struct SignalDetection {
    float frequency;
    int8_t rssi;
    uint32_t timestamp;
    String signalType;  // "Strong", "Medium", "Weak"
};

struct ScanResult {
    uint32_t detectionCount;
    int8_t strongestSignal;
    float busyFrequency;
    uint32_t scanDurationMs;
    std::vector<SignalDetection> detections;
};

// Scan 433MHz band (433.05 - 434.79 MHz) for activity
ScanResult scanBand(uint32_t durationMs = 10000);

// Scan specific frequency
int8_t scanFrequency(float freqMHz);

// Full spectrum analysis (multiple bands)
std::vector<ScanResult> analyzeSpectrum(uint32_t durationMs = 5000);

}  // namespace SubghzScanner
