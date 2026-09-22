#include "bluetooth_power_analysis.h"
#include <LittleFS.h>

namespace BluetoothPowerAnalysis {

PowerAnalyzer::PowerAnalyzer() : isRunning_(false) {}

AnalysisResult PowerAnalyzer::analyzeBlePower(const AnalysisConfig& config) {
  AnalysisResult result;
  result.success = false;
  result.avgPower = 0;
  result.peakPower = 0;

  isRunning_ = true;
  unsigned long startTime = millis();
  uint32_t sampleCount = 0;
  float totalPower = 0;

  // Simulate power measurement during BLE operations
  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    PowerSample sample;
    sample.timestamp = millis() - startTime;

    // Simulate realistic BLE power consumption
    // Idle: ~50µA
    // Scanning: ~10-20mA
    // Connecting/transmitting: ~20-50mA

    uint8_t state = (esp_random() % 100);
    if (state < 60) {
      // Idle state
      sample.milliamps = ((esp_random() % 15) + 5) / 100.0f; // 0.05-0.2mA
    } else if (state < 90) {
      // Scanning state
      sample.milliamps = ((esp_random() % 10000) + 10000) / 1000.0f; // 10-20mA
    } else {
      // Transmission state
      sample.milliamps = ((esp_random() % 30000) + 20000) / 1000.0f; // 20-50mA
    }

    sample.voltage = 3.3f + (((esp_random() % 100) + -50) / 1000.0f);
    sample.powerMw = sample.milliamps * sample.voltage;

    result.samples.push_back(sample);
    totalPower += sample.powerMw;
    sampleCount++;

    if (sample.powerMw > result.peakPower) {
      result.peakPower = sample.powerMw;
    }

    delay(config.samplingRateMs);
  }

  if (sampleCount > 0) {
    result.avgPower = totalPower / sampleCount;
  }

  result.elapsedMs = millis() - startTime;
  result.success = sampleCount > 0;
  result.logFile = "/logs/handshakes/ble_power.csv";

  // Log results
  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/ble_power.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ble_power.csv", "a");
  }
  if (logFile) {
    for (const auto& sample : result.samples) {
      logFile.printf("%lu,%.2f,%.2f,%.2f\n", sample.timestamp, sample.milliamps,
                     sample.voltage, sample.powerMw);
    }
    logFile.close();
  }
  LittleFS.end();

  isRunning_ = false;
  return result;
}

float PowerAnalyzer::estimateBatteryLife() {
  // Estimate battery life based on average power consumption
  // Assume typical BLE device battery: 100mAh
  // Battery life (hours) = Capacity (mAh) / Average Current (mA)

  float capacityMah = 100.0f;
  float avgCurrentMa = 10.0f; // Estimated average

  return capacityMah / avgCurrentMa;
}

void PowerAnalyzer::stop() {
  isRunning_ = false;
}

} // namespace BluetoothPowerAnalysis
