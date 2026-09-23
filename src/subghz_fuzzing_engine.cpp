#include "subghz_fuzzing_engine.h"
#include <LittleFS.h>

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
      freq += random(-100000, 100000);
    }

    uint32_t modulation = config.modulation;
    if (config.varyModulation) {
      modulation = random(0, 3);
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
  uint32_t len = random(8, 64);
  for (uint32_t i = 0; i < len; i++) {
    payload.push_back(random(0, 256));
  }
}

void FuzzingEngine::stop() {
  isRunning_ = false;
}

} // namespace SubGhzFuzzing
