#include "subghz_fuzzing_engine.h"
#include <LittleFS.h>
#include "tx_arm.h"

namespace SubGhzFuzzing {

FuzzingEngine::FuzzingEngine() : isRunning_(false), startTime_(0) {}

FuzzResult FuzzingEngine::fuzzSubGhz(const FuzzConfig& config) {
  FuzzResult result;
  result.success = false;
  result.packetsSent = 0;

  if (!TxArm::isArmed()) {
    result.logFile = "/logs/handshakes/subghz_fuzz.csv";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();
  uint32_t mutations = 0;

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    std::vector<uint8_t> payload;
    generateFuzzVector(payload);

    uint32_t freq = config.baseFreq;
    if (config.randomizeFreq) {
      freq += ((esp_random() % 200000) + -100000);
    }

    uint32_t modulation = config.modulation;
    if (config.varyModulation) {
      modulation = (esp_random() % 3);
      mutations++;
    }

    result.packetsSent++;
    delay(50);
  }

  result.mutationsApplied = mutations;
  result.elapsedMs = millis() - startTime_;
  result.success = true;
  result.logFile = "/logs/handshakes/subghz_fuzz.csv";

  isRunning_ = false;
  return result;
}

void FuzzingEngine::generateFuzzVector(std::vector<uint8_t>& payload) {
  uint32_t len = ((esp_random() % 56) + 8);
  for (uint32_t i = 0; i < len; i++) {
    payload.push_back((esp_random() % 256));
  }
}

void FuzzingEngine::stop() {
  isRunning_ = false;
}

} // namespace SubGhzFuzzing
