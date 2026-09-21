#pragma once
#include <Arduino.h>
#include <vector>

namespace SubghzProtocolAnalyzer {

struct ProtocolAnalysis {
    String protocolName;
    uint32_t bitrate;
    String modulation;
    uint32_t pulseCount;
    float confidence;
};

// Analyze captured Sub-GHz signal to identify protocol
ProtocolAnalysis analyzeSignal(const std::vector<uint16_t> &pulses);

}  // namespace SubghzProtocolAnalyzer
