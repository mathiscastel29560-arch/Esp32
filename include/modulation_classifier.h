#pragma once
#include <Arduino.h>

namespace ModulationClassifier {

struct ClassificationResult {
    bool success;
    String modulationType;    // "OOK", "FSK", "PSK", "GFSK", "MSK", etc.
    String modulationFamily;  // "Digital", "Analog", "Hybrid"
    float confidence;
    uint32_t durationMs;
};

// Automatically detect and classify modulation type from RF signal
ClassificationResult classifyModulation(uint32_t durationMs = 20000);

// Detect signal bandwidth
float estimateSignalBandwidth();

// Identify bit rate
uint32_t estimateBitrate();

}  // namespace ModulationClassifier
