#include "ir_emitter.h"
#include "tx_arm.h"
#include <LittleFS.h>

namespace IrEmitter {

IrEmitter::IrEmitter(uint8_t txPin) : irsend_(txPin), isRunning_(false), startTime_(0) {}

EmitResult IrEmitter::emitNecCode(const EmitterConfig& config, uint8_t address, uint8_t command) {
  EmitResult result;
  result.success = false;
  result.codesEmitted = 0;
  result.pulsesSent = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  for (uint32_t rep = 0; rep < config.repeatCount && isRunning_; rep++) {
    // Use IRremoteESP8266 library for real NEC encoding
    // NEC: 32-bit data (address + command encoded with checksum)
    uint32_t data = (address << 24) | ((~address & 0xFF) << 16) |
                    (command << 8) | (~command & 0xFF);

    irsend_.sendNEC(data, 32, config.repeatCount);
    result.codesEmitted++;
    result.pulsesSent += 68; // NEC sends 68 pulses per code

    if (config.rapidFire) {
      delay(50);
    } else {
      delay(100);
    }
  }

  result.elapsedMs = millis() - startTime_;
  result.success = true;
  result.logFile = "/logs/handshakes/ir_emit.csv";
  logEmission(result.codesEmitted);

  isRunning_ = false;
  return result;
}

EmitResult IrEmitter::emitRcCode(const EmitterConfig& config, uint8_t address, uint8_t command, uint8_t toggle) {
  EmitResult result;
  result.success = false;
  result.codesEmitted = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  for (uint32_t rep = 0; rep < config.repeatCount && isRunning_; rep++) {
    // RC5/RC6 protocol via IRremoteESP8266
    // RC5: 14-bit data (1 start + 1 toggle + 5 address + 6 command)
    uint16_t data = (toggle << 12) | (address << 6) | command;
    irsend_.sendRC5(data, 13, config.repeatCount);
    result.codesEmitted++;
    result.pulsesSent += 27; // RC5 sends ~27 pulses

    if (config.rapidFire) {
      delay(50);
    } else {
      delay(100);
    }
  }

  result.elapsedMs = millis() - startTime_;
  result.success = true;
  result.logFile = "/logs/handshakes/ir_emit.csv";
  logEmission(result.codesEmitted);

  isRunning_ = false;
  return result;
}

EmitResult IrEmitter::emitSonyCode(const EmitterConfig& config, uint16_t data) {
  EmitResult result;
  result.success = false;
  result.codesEmitted = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  for (uint32_t rep = 0; rep < config.repeatCount && isRunning_; rep++) {
    // Sony SIRC protocol via IRremoteESP8266
    irsend_.sendSony(data, 15, config.repeatCount);
    result.codesEmitted++;
    result.pulsesSent += 31; // Sony SIRC sends ~31 pulses

    if (config.rapidFire) {
      delay(50);
    } else {
      delay(100);
    }
  }

  result.elapsedMs = millis() - startTime_;
  result.success = true;
  result.logFile = "/logs/handshakes/ir_emit.csv";
  logEmission(result.codesEmitted);

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

  isRunning_ = true;
  startTime_ = millis();

  // Convert std::vector to uint16_t array for IRsend::sendRaw
  uint16_t* rawTimings = new uint16_t[timings.size()];
  for (size_t i = 0; i < timings.size(); i++) {
    rawTimings[i] = timings[i];
  }

  // Use IRremoteESP8266 sendRaw for arbitrary IR sequences
  irsend_.sendRaw(rawTimings, timings.size(), config.frequency);

  result.codesEmitted = 1;
  result.pulsesSent = timings.size();
  result.elapsedMs = millis() - startTime_;
  result.success = true;
  result.logFile = "/logs/handshakes/ir_emit.csv";
  logEmission(result.codesEmitted);

  delete[] rawTimings;
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

  // Parse captured data and replay - scale 8-bit samples to 16-bit timings
  std::vector<uint16_t> timings;
  for (uint32_t i = 0; i < capturedData.size() && i < 256; i++) {
    timings.push_back(capturedData[i] * 100); // Scale by 100 to get microseconds
  }

  result = emitRawTimings(config, timings);
  return result;
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
