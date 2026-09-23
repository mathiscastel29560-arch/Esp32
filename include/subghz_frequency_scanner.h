#pragma once

#include <string>
#include <vector>
#include <Arduino.h>

namespace SubGhzFrequencyScanner {

struct ScannerConfig {
    uint32_t startFreq;
    uint32_t endFreq;
    uint32_t stepHz;
    uint32_t durationPerFreqMs;
};

struct FrequencyScan {
    uint32_t frequency;
    int8_t rssi;
    uint8_t signalStrength;
    uint32_t packetCount;
};

struct ScannerResult {
    bool success;
    uint32_t activeFrequencies;
    std::vector<FrequencyScan> frequencies;
    String error;
    uint32_t elapsedMs;
    String logFile;
};

class FrequencyScanner {
public:
    FrequencyScanner();
    ScannerResult scanFrequencies(const ScannerConfig& config);
    ScannerResult scanSpecificFreq(uint32_t freq, uint32_t durationMs);
    void identifyActiveFrequencies();
    void logFrequency(const FrequencyScan& scan);
    void stop();

private:
    bool isRunning_;
    uint32_t startTime_;
};

}  // namespace SubGhzFrequencyScanner
