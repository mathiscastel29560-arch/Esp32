#pragma once
#include <Arduino.h>
#include <vector>

namespace FrequencyAnalyzer {

struct BandAnalysis {
    String bandName;
    float startFreq, endFreq;
    int8_t maxRSSI;
    uint32_t signalsDetected;
};

struct AnalysisResult {
    uint32_t totalSignals;
    std::vector<BandAnalysis> bands;
};

// Analyze multiple frequency bands (2.4GHz, 433MHz, etc)
AnalysisResult analyzeBands(uint32_t durationMs = 20000);

}  // namespace FrequencyAnalyzer
