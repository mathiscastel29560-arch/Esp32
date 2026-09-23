#include "subghz_protocol_analyzer.h"

namespace SubghzProtocolAnalyzer {

ProtocolAnalysis analyzeSignal(const std::vector<uint16_t> &pulses) {
    ProtocolAnalysis analysis{"Unknown", 0, "OOK", pulses.size(), 0.0f};
    
    if (pulses.empty()) return analysis;
    
    Serial.println("\n=== Sub-GHz Protocol Analyzer ===");
    Serial.println("Analyzing " + String(pulses.size()) + " pulses...");
    
    uint32_t avgPulse = 0;
    for (uint16_t p : pulses) avgPulse += p;
    avgPulse /= pulses.size();
    
    uint32_t bitrate = 1000000 / (avgPulse * 2);
    analysis.bitrate = bitrate;
    
    Serial.println("Average pulse: " + String(avgPulse) + "µs");
    Serial.println("Estimated bitrate: " + String(bitrate) + " bps");
    
    if (bitrate > 4000 && bitrate < 6000) {
        analysis.protocolName = "PT2262 (Remote)";
        analysis.confidence = 0.85f;
    } else if (bitrate > 2000 && bitrate < 3000) {
        analysis.protocolName = "Garage Door (OOK)";
        analysis.confidence = 0.75f;
    } else if (bitrate > 1000 && bitrate < 2000) {
        analysis.protocolName = "Weather Station";
        analysis.confidence = 0.70f;
    } else {
        analysis.protocolName = "Generic OOK";
        analysis.confidence = 0.50f;
    }
    
    Serial.println("Protocol: " + analysis.protocolName);
    Serial.println("Confidence: " + String(analysis.confidence * 100, 0) + "%");
    
    return analysis;
}

}  // namespace SubghzProtocolAnalyzer
