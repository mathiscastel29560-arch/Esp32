#ifndef IR_EMITTER_H
#define IR_EMITTER_H

#include <Arduino.h>
#include <vector>

namespace IrEmitter {

struct EmitterConfig {
  uint8_t txPin;
  uint32_t frequency; // 38000 Hz typical
  uint32_t repeatCount;
  bool rapidFire;
};

struct EmitResult {
  bool success;
  uint32_t codesEmitted;
  uint32_t pulsesSent;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class IrEmitter {
public:
  IrEmitter();
  EmitResult emitNecCode(const EmitterConfig& config, uint8_t address, uint8_t command);
  EmitResult emitRawTimings(const EmitterConfig& config, const std::vector<uint16_t>& timings);
  EmitResult replayCapture(const EmitterConfig& config, const std::vector<uint8_t>& capturedData);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;

  void sendPulse(uint8_t pin, uint32_t frequency, uint16_t duration);
  void logEmission(uint32_t count);
};

} // namespace IrEmitter

#endif
