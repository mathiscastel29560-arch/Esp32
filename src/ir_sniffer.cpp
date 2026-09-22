#include "ir_sniffer.h"
#include <LittleFS.h>

namespace IrSniffer {

IrSniffer::IrSniffer() : isRunning_(false), startTime_(0) {}

SnifferResult IrSniffer::captureIrCodes(const SnifferConfig& config) {
  SnifferResult result;
  result.success = false;
  result.codesCapTured = 0;

  pinMode(config.rxPin, INPUT);

  isRunning_ = true;
  startTime_ = millis();

  uint32_t lastLevel = LOW;
  uint32_t lastTime = micros();
  uint32_t codeCount = 0;

  while (isRunning_ && (millis() - startTime_) < config.captureTimeMs) {
    uint32_t currentLevel = digitalRead(config.rxPin);
    uint32_t currentTime = micros();

    if (currentLevel != lastLevel) {
      uint32_t timing = currentTime - lastTime;

      // Simulate IR code capture
      if (timing > 100) { // Filter out noise (< 100µs)
        IrCode code;
        code.timestamp = millis();
        code.timings.push_back(timing);

        // Generate simulated code
        if (codeCount == 0 || (esp_random() % 100) < 5) {
          code.protocol = "NEC";
          code.address = (esp_random() % 256);
          code.command = (esp_random() % 256);
          code.rssi = ((esp_random() % 40) + -60);

          result.codes.push_back(code);
          result.codesCapTured++;
          codeCount++;

          logCode(code);
        }
      }

      lastLevel = currentLevel;
      lastTime = currentTime;
    }

    if (config.continuousCapture) {
      delay(1);
    } else {
      delay(10);
    }
  }

  // Identify dominant protocol
  result.dominantProtocol = "NEC"; // Default to NEC (most common)
  result.success = result.codesCapTured > 0;
  result.logFile = "/logs/handshakes/ir_capture.csv";

  isRunning_ = false;
  return result;
}

String IrSniffer::identifyProtocol(const std::vector<uint16_t>& timings) {
  if (timings.empty()) return "UNKNOWN";

  // NEC: ~9ms header + 560µs units
  if (timings.size() > 2 && timings[0] > 8000 && timings[0] < 10000) {
    return "NEC";
  }

  // RC5: 1777µs units
  if (timings.size() > 2 && timings[0] > 1500 && timings[0] < 2000) {
    return "RC5";
  }

  // Sony: 2400µs header
  if (timings.size() > 2 && timings[0] > 2000 && timings[0] < 2800) {
    return "SONY";
  }

  return "UNKNOWN";
}

void IrSniffer::logCode(const IrCode& code) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/ir_capture.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ir_capture.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,%s,%02X,%02X,%u\n", code.timestamp, code.protocol.c_str(),
                   code.address, code.command, (uint32_t)code.timings.size());
    logFile.close();
  }

  LittleFS.end();
}

void IrSniffer::stop() {
  isRunning_ = false;
}

} // namespace IrSniffer
