#include "ir_emitter.h"
#include <LittleFS.h>

namespace IrEmitter {

IrEmitter::IrEmitter() : isRunning_(false), startTime_(0) {}

EmitResult IrEmitter::emitNecCode(const EmitterConfig& config, uint8_t address, uint8_t command) {
  EmitResult result;
  result.success = false;
  result.codesEmitted = 0;
  result.pulsesSent = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  pinMode(config.txPin, OUTPUT);
  isRunning_ = true;
  startTime_ = millis();

  for (uint32_t rep = 0; rep < config.repeatCount && isRunning_; rep++) {
    // NEC Protocol: 9ms header + 4.5ms space + 32 bits + stop bit
    sendPulse(config.txPin, config.frequency, 9000); // 9ms header
    delayMicroseconds(4500); // 4.5ms space

    // Send 32 bits (address x2, command x2)
    for (int i = 0; i < 32; i++) {
      uint8_t bit = (i < 8) ? ((address >> i) & 1) :
                    (i < 16) ? ((~address >> (i-8)) & 1) :
                    (i < 24) ? ((command >> (i-16)) & 1) :
                    ((~command >> (i-24)) & 1);

      if (bit) {
        sendPulse(config.txPin, config.frequency, 560);  // 1 bit
        delayMicroseconds(1690);
      } else {
        sendPulse(config.txPin, config.frequency, 560);  // 0 bit
        delayMicroseconds(560);
      }

      result.pulsesSent++;
    }

    // Stop bit
    sendPulse(config.txPin, config.frequency, 560);
    result.codesEmitted++;

    if (config.rapidFire) {
      delay(50); // 50ms between rapid-fire codes
    } else {
      delay(100); // 100ms between normal codes
    }
  }

  result.elapsedMs = millis() - startTime_;
  result.success = true;
  result.logFile = "/logs/handshakes/ir_emit.csv";
  logEmission(result.codesEmitted);

  digitalWrite(config.txPin, LOW);
  isRunning_ = false;
  return result;
}

EmitResult IrEmitter::emitRawTimings(const EmitterConfig& config, const std::vector<uint16_t>& timings) {
  EmitResult result;
  result.success = false;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  if (timings.empty()) {
    result.error = "Empty timings";
    return result;
  }

  pinMode(config.txPin, OUTPUT);
  isRunning_ = true;
  startTime_ = millis();

  bool isCarrier = true; // Start with carrier on

  for (const auto& timing : timings) {
    if (isCarrier) {
      sendPulse(config.txPin, config.frequency, timing);
    } else {
      delayMicroseconds(timing);
    }

    isCarrier = !isCarrier;
    result.pulsesSent++;
  }

  result.codesEmitted = 1;
  result.elapsedMs = millis() - startTime_;
  result.success = true;
  result.logFile = "/logs/handshakes/ir_emit.csv";
  logEmission(result.codesEmitted);

  digitalWrite(config.txPin, LOW);
  isRunning_ = false;
  return result;
}

EmitResult IrEmitter::replayCapture(const EmitterConfig& config, const std::vector<uint8_t>& capturedData) {
  EmitResult result;
  result.success = false;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  // Parse captured data and replay
  std::vector<uint16_t> timings;
  for (uint32_t i = 0; i < capturedData.size() && i < 128; i++) {
    timings.push_back(capturedData[i] * 100); // Scale up
  }

  result = emitRawTimings(config, timings);
  return result;
}

void IrEmitter::sendPulse(uint8_t pin, uint32_t frequency, uint16_t duration) {
  // Simple PWM simulation at specified frequency
  uint32_t period = 1000000 / frequency; // Period in microseconds
  uint32_t pulseWidth = period / 2;
  uint32_t cycles = (duration * frequency) / 1000000;

  for (uint32_t i = 0; i < cycles; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(pin, LOW);
    delayMicroseconds(pulseWidth);
  }
}

void IrEmitter::logEmission(uint32_t count) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/ir_emit.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ir_emit.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,EMIT,%u\n", millis(), count);
    logFile.close();
  }

  LittleFS.end();
}

void IrEmitter::stop() {
  isRunning_ = false;
}

} // namespace IrEmitter
