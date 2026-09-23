#pragma once

#include <string>
#include <vector>
#include <RadioLib.h>

namespace SubGhzFuzzing {

enum ModulationType {
  MOD_2FSK = 0,
  MOD_GFSK = 1,
  MOD_ASK = 2,
  MOD_OOK = 3
};

struct FuzzConfig {
  uint32_t baseFreq;
  ModulationType modulation;
  uint32_t durationMs;
  bool randomizeFreq;
  bool varyModulation;
  uint32_t bitRate; // Bits per second
};

struct FuzzResult {
  bool success;
  uint32_t packetsSent;
  uint32_t mutationsApplied;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class FuzzingEngine {
public:
  FuzzingEngine(int8_t cs = 5, int8_t irq = 2, int8_t gpio = 4); // CC1101 pins
  FuzzResult fuzzSubGhz(const FuzzConfig& config);
  bool transmitFuzzPayload(const std::vector<uint8_t>& payload, uint32_t freq, ModulationType mod);
  void generateFuzzVector(std::vector<uint8_t>& payload);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  CC1101 radio_;
  bool isRunning_;
  unsigned long startTime_;

  bool setModulation(ModulationType mod);
};

}  // namespace SubGhzFuzzing
