#ifndef SUBGHZ_FUZZING_ENGINE_H
#define SUBGHZ_FUZZING_ENGINE_H

#include <Arduino.h>
#include <vector>

namespace SubGhzFuzzing {

struct FuzzConfig {
  uint32_t baseFreq;
  uint32_t modulation; // FSK, ASK, OOK
  uint32_t durationMs;
  bool randomizeFreq;
  bool varyModulation;
};

struct FuzzResult {
  bool success;
  uint32_t packetsSent;
  uint32_t mutationsApplied;
  uint32_t elapsedMs;
  String logFile;
};

class FuzzingEngine {
public:
  FuzzingEngine();
  FuzzResult fuzzSubGhz(const FuzzConfig& config);
  void generateFuzzVector(std::vector<uint8_t>& payload);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;
};

} // namespace SubGhzFuzzing

#endif
