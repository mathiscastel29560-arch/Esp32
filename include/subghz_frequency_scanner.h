#pragma once

#include <string>
#include <vector>
#include <RadioLib.h>

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
  FrequencyScanner(int8_t cs = 5, int8_t irq = 2, int8_t gpio = 4); // CC1101 pins
  ScannerResult scanFrequencies(const ScannerConfig& config);
  ScannerResult scanSpecificFreq(uint32_t freq, uint32_t durationMs);
  void identifyActiveFrequencies();
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  CC1101 radio_;
  bool isRunning_;
  unsigned long startTime_;

  void logFrequency(const FrequencyScan& scan);
};

}  // namespace SubGhzFrequencyScanner
