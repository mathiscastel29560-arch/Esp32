#ifndef SUBGHZ_FREQUENCY_SCANNER_H
#define SUBGHZ_FREQUENCY_SCANNER_H

#include <Arduino.h>
#include <vector>

namespace SubGhzFrequencyScanner {

struct FrequencyScan {
  uint32_t frequency;
  int32_t rssi;
  uint32_t signalStrength;
  uint32_t packetCount;
};

struct ScannerConfig {
  uint32_t startFreq;
  uint32_t endFreq;
  uint32_t stepHz;
  uint32_t durationPerFreqMs;
  bool trackSignals;
};

struct ScannerResult {
  bool success;
  std::vector<FrequencyScan> frequencies;
  uint32_t activeFrequencies;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class FrequencyScanner {
public:
  FrequencyScanner();
  ScannerResult scanFrequencies(const ScannerConfig& config);
  ScannerResult scanSpecificFreq(uint32_t freq, uint32_t durationMs);
  void identifyActiveFrequencies();
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;

  void logFrequency(const FrequencyScan& scan);
};

} // namespace SubGhzFrequencyScanner

#endif
