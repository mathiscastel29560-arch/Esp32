#ifndef PROTOCOL_ANALYZER_H
#define PROTOCOL_ANALYZER_H

#include <Arduino.h>
#include <vector>

class ProtocolAnalyzer {
public:
    static ProtocolAnalyzer& instance() {
        static ProtocolAnalyzer pa;
        return pa;
    }

    struct SignalSample {
        uint32_t timestamp;
        uint16_t frequency;
        int8_t rssi;
        uint8_t modulation;  // 0=FSK, 1=OOK, 2=ASK
    };

    struct ProtocolSignature {
        char name[32];
        uint16_t frequency;
        uint16_t bandwidth;
        uint8_t modulation;
        uint32_t bit_rate;
        uint16_t pattern_length;
        uint8_t pattern[64];
        uint16_t packet_count;
        uint8_t confidence;  // 0-100%
    };

    struct AnalysisStats {
        uint32_t total_signals;
        uint32_t unique_protocols;
        uint32_t total_analysis_time;
        uint32_t strongest_rssi;
        uint16_t most_active_frequency;
        uint8_t common_modulation;
    };

    void begin();
    void startAnalysis();
    void stopAnalysis();
    void analyzeSignal(uint16_t frequency, int8_t rssi);

    std::vector<ProtocolSignature> identifyProtocols();
    AnalysisStats getStats();
    String generateReport();
    void exportToJSON(const char* filepath);
    void clearData();

private:
    ProtocolAnalyzer() : analyzing_(false), sample_count_(0) {}

    bool analyzing_;
    std::vector<SignalSample> samples_;
    std::vector<ProtocolSignature> identified_;
    AnalysisStats stats_;
    uint32_t sample_count_;
    uint32_t start_time_;

    void decodeFSK(const std::vector<SignalSample>& sig_samples);
    void decodeOOK(const std::vector<SignalSample>& sig_samples);
    uint8_t calculateConfidence(const ProtocolSignature& sig);
};

#endif
