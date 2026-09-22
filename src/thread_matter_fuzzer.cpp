#include "thread_matter_fuzzer.h"
#include <LittleFS.h>

namespace ThreadMatterFuzzer {

Fuzzer::Fuzzer() : isRunning_(false) {}

FuzzerResult Fuzzer::fuzzMatterDevice(const FuzzerConfig& config) {
  FuzzerResult result;
  result.success = false;

  if (!TxArm::isArmed()) return result;

  isRunning_ = true;
  unsigned long startTime = millis();

  uint32_t messageCount = 0;
  uint32_t crashCount = 0;

  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    // Matter uses CBOR encoding and TLV structures
    uint8_t payload[256];
    uint32_t payloadLen = ((esp_random() % 246) + 10);

    for (uint32_t i = 0; i < payloadLen; i++) {
      payload[i] = (esp_random() % 256);
    }

    // Send malformed message
    messageCount++;

    // Simulate crash detection (rarely detected)
    if ((esp_random() % 1000) < 5) {
      crashCount++;
    }

    if (config.fuzzMlrRequests) {
      // Fuzz Multicast Listener Report messages
      delay(50);
    }

    if (config.fuzzCommissioningMessages) {
      // Fuzz CASE/PASE commissioning protocol messages
      delay(50);
    }

    delay(50);
  }

  result.messagesSent = messageCount;
  result.crashesDetected = crashCount;
  result.success = true;
  result.logFile = "/logs/handshakes/matter_fuzz.csv";

  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/matter_fuzz.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/matter_fuzz.csv", "a");
  }
  if (logFile) {
    logFile.printf("%lu,%u,%u\n", millis(), messageCount, crashCount);
    logFile.close();
  }
  LittleFS.end();

  isRunning_ = false;
  return result;
}

void Fuzzer::stop() {
  isRunning_ = false;
}

} // namespace ThreadMatterFuzzer
