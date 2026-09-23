#include "subghz_frequency_scanner.h"
#include <LittleFS.h>
#include <RF24.h>
#include "tx_arm.h"

namespace SubGhzFrequencyScanner {

FrequencyScanner::FrequencyScanner() : isRunning_(false), startTime_(0) {}

ScannerResult FrequencyScanner::scanFrequencies(const ScannerConfig& config) {
  ScannerResult result;
  result.success = false;
  result.activeFrequencies = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  RF24 radio(22, 21); // CE, CSN pins
  if (!radio.begin()) {
    result.error = "RF24 initialization failed";
    isRunning_ = false;
    return result;
  }

  // Scan frequency range
  for (uint32_t freq = config.startFreq; freq <= config.endFreq && isRunning_; freq += config.stepHz) {
    if (millis() - startTime_ > (config.endFreq - config.startFreq) / config.stepHz * config.durationPerFreqMs) {
      break;
    }

    // Tune RF24 to specific frequency
    uint8_t channel = (freq - 2400) / 1; // Convert MHz to RF24 channel
    if (channel > 125) channel = 125; // Max channel

    radio.setChannel(channel);
    radio.startListening();

    // Scan for signals
    ScannerResult freqResult = scanSpecificFreq(freq, config.durationPerFreqMs);

    result.frequencies.insert(result.frequencies.end(),
                             freqResult.frequencies.begin(),
                             freqResult.frequencies.end());

    if (freqResult.activeFrequencies > 0) {
      result.activeFrequencies++;
    }

    radio.stopListening();
    delay(50);
  }

  result.success = result.activeFrequencies > 0;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/subghz_scan.csv";

  radio.powerDown();
  isRunning_ = false;
  return result;
}

ScannerResult FrequencyScanner::scanSpecificFreq(uint32_t freq, uint32_t durationMs) {
  ScannerResult result;
  result.success = false;

  unsigned long freqStartTime = millis();
  uint32_t packetCount = 0;
  int32_t avgRssi = -100;

  // Simulate signal detection on frequency
  while ((millis() - freqStartTime) < durationMs) {
    // Random signal simulation
    if ((esp_random() % 100) < 30) { // 30% chance of detecting signal
      int32_t rssi = ((esp_random() % 60) + -90);
      avgRssi = (avgRssi + rssi) / 2;
      packetCount++;
    }

    delay(10);
  }

  FrequencyScan scan;
  scan.frequency = freq;
  scan.rssi = avgRssi;
  scan.signalStrength = (avgRssi > -70) ? 3 : (avgRssi > -85) ? 2 : 1;
  scan.packetCount = packetCount;

  if (packetCount > 0) {
    result.frequencies.push_back(scan);
    result.activeFrequencies = 1;
    result.success = true;
    logFrequency(scan);
  }

  return result;
}

void FrequencyScanner::identifyActiveFrequencies() {
  // Identify common ISM band frequencies with activity:
  // 433 MHz: Remote controls, weather stations, car key fobs
  // 868 MHz: European ISM band
  // 915 MHz: North American ISM band
  // 2.4 GHz: WiFi, Bluetooth, Zigbee, LoRa
}

void FrequencyScanner::logFrequency(const FrequencyScan& scan) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/subghz_scan.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/subghz_scan.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,%u,%d,%u,%u\n", millis(), scan.frequency, scan.rssi,
                   scan.signalStrength, scan.packetCount);
    logFile.close();
  }

  LittleFS.end();
}

void FrequencyScanner::stop() {
  isRunning_ = false;
}

} // namespace SubGhzFrequencyScanner
