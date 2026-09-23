#ifndef IR_FUZZING_H
#define IR_FUZZING_H

#include <Arduino.h>
#include <IRsend.h>

namespace IrFuzzing {

enum FuzzMode {
  ADDRESS_FUZZ,
  COMMAND_FUZZ,
  TIMING_FUZZ,
  PROTOCOL_FUZZ
};

struct FuzzConfig {
  uint8_t txPin;
  FuzzMode mode;
  uint32_t frequency;
  uint32_t durationMs;
  bool rapidFire;
  bool targetSpecificDevice; // If true, iterate addr/cmd sequentially
};

struct FuzzResult {
  bool success;
  uint32_t mutationsSent;
  uint32_t devicesAffected;
  uint32_t codesFound;
  uint32_t elapsedMs;
  String logFile;
};

class IrFuzzer {
public:
  IrFuzzer(uint8_t txPin = 14); // GPIO14 for IR LED
  FuzzResult fuzzIrDevices(const FuzzConfig& config);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  IRsend irsend_;
  bool isRunning_;
  unsigned long startTime_;

  void logFuzz(uint32_t count);
};

} // namespace IrFuzzing

#endif
