#include "ir_fuzzing.h"
#include <LittleFS.h>

namespace IrFuzzing {

IrFuzzer::IrFuzzer() : isRunning_(false), startTime_(0) {}

FuzzResult IrFuzzer::fuzzIrDevices(const FuzzConfig& config) {
  FuzzResult result;
  result.success = false;
  result.mutationsSent = 0;
  result.devicesAffected = 0;
  result.codesFound = 0;

  if (!TxArm::isArmed()) {
    result.logFile = "/logs/handshakes/ir_fuzz.csv";
    return result;
  }

  pinMode(config.txPin, OUTPUT);
  isRunning_ = true;
  startTime_ = millis();

  uint32_t mutationCount = 0;
  uint32_t successCount = 0;

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    if (config.mode == ADDRESS_FUZZ) {
      // Fuzz address field (0x00-0xFF)
      for (uint8_t addr = 0; addr < 256 && isRunning_; addr++) {
        for (uint8_t cmd = 0; cmd < 256; cmd++) {
          // Simulate sending NEC command
          mutationCount++;
          result.mutationsSent++;

          if (config.targetSpecificDevice && random(0, 100) < 5) {
            successCount++;
          }

          if (config.rapidFire) {
            delay(10);
          } else {
            delay(50);
          }

          if ((millis() - startTime_) > config.durationMs) break;
        }

        if ((millis() - startTime_) > config.durationMs) break;
      }

    } else if (config.mode == COMMAND_FUZZ) {
      // Fuzz command field only (fixed address)
      uint8_t fixedAddr = random(0, 256);

      for (uint8_t cmd = 0; cmd < 256 && isRunning_; cmd++) {
        mutationCount++;
        result.mutationsSent++;

        if (random(0, 100) < 8) {
          successCount++;
        }

        delay(config.rapidFire ? 10 : 50);

        if ((millis() - startTime_) > config.durationMs) break;
      }

    } else if (config.mode == TIMING_FUZZ) {
      // Fuzz timing parameters (pulse width, space duration)
      uint32_t baseFreq = config.frequency;

      for (uint32_t freqVariation = 30000; freqVariation <= 50000; freqVariation += 1000) {
        mutationCount++;
        result.mutationsSent++;
        delay(config.rapidFire ? 20 : 100);

        if ((millis() - startTime_) > config.durationMs) break;
      }

    } else if (config.mode == PROTOCOL_FUZZ) {
      // Fuzz protocol bits and structures
      const uint8_t protocols[] = {0xNEC, 0xRC5, 0xSONY}; // Different protocol signatures

      for (int p = 0; p < 3 && isRunning_; p++) {
        for (uint8_t i = 0; i < 50; i++) {
          mutationCount++;
          result.mutationsSent++;

          if (random(0, 100) < 3) {
            successCount++;
          }

          delay(config.rapidFire ? 15 : 75);

          if ((millis() - startTime_) > config.durationMs) break;
        }

        if ((millis() - startTime_) > config.durationMs) break;
      }
    }

    if ((millis() - startTime_) > config.durationMs) break;
  }

  result.devicesAffected = successCount;
  result.codesFound = successCount / 10; // Estimate
  result.success = mutationCount > 0;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ir_fuzz.csv";

  logFuzz(mutationCount);

  digitalWrite(config.txPin, LOW);
  isRunning_ = false;
  return result;
}

void IrFuzzer::logFuzz(uint32_t count) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/ir_fuzz.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ir_fuzz.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,FUZZ,%u\n", millis(), count);
    logFile.close();
  }

  LittleFS.end();
}

void IrFuzzer::stop() {
  isRunning_ = false;
}

} // namespace IrFuzzing
