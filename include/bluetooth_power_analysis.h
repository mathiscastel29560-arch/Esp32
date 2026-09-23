#ifndef BLUETOOTH_POWER_ANALYSIS_H
#define BLUETOOTH_POWER_ANALYSIS_H

#include <Arduino.h>
#include <vector>

namespace BluetoothPowerAnalysis {

struct PowerSample {
  uint32_t timestamp;
  float milliamps;
  float voltage;
  float powerMw;
};

struct AnalysisConfig {
  uint32_t samplingRateMs;
  uint32_t durationMs;
  bool analyzePatterns;
};

struct AnalysisResult {
  bool success;
  std::vector<PowerSample> samples;
  float avgPower;
  float peakPower;
  uint32_t elapsedMs;
  String logFile;
};

class PowerAnalyzer {
public:
  PowerAnalyzer();
  AnalysisResult analyzeBlePower(const AnalysisConfig& config);
  float estimateBatteryLife();
  void stop();

private:
  bool isRunning_;
};

} // namespace BluetoothPowerAnalysis

#endif
