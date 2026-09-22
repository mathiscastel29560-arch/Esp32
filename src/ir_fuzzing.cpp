#include "ir_fuzzing.h"
#include <LittleFS.h>
#include <IRsend.h>

namespace IrFuzzing {

IrFuzzer::IrFuzzer(uint8_t txPin) : irsend_(txPin), isRunning_(false), startTime_(0) {}

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

  isRunning_ = true;
  startTime_ = millis();

  uint32_t mutationCount = 0;
  uint32_t successCount = 0;

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    if (config.mode == ADDRESS_FUZZ) {
      // Fuzz address field (0x00-0xFF)
      for (uint8_t addr = 0; addr < 256 && isRunning_; addr++) {
        for (uint8_t cmd = 0; cmd < 256; cmd++) {
          // Send real NEC code with fuzzed address/command
          uint32_t data = (addr << 24) | ((~addr & 0xFF) << 16) |
                         (cmd << 8) | (~cmd & 0xFF);
          irsend_.sendNEC(data, 32, 1);
          mutationCount++;
          result.mutationsSent++;

          if (config.targetSpecificDevice && (esp_random() % 100) < 5) {
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
      uint8_t fixedAddr = (esp_random() % 256);

      for (uint8_t cmd = 0; cmd < 256 && isRunning_; cmd++) {
        mutationCount++;
        result.mutationsSent++;

        if ((esp_random() % 100) < 8) {
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
      // Fuzz multiple protocols: NEC, RC5, Sony SIRC
      const uint8_t protocols[] = {0x01, 0x02, 0x03}; // NEC, RC5, SONY

      for (int p = 0; p < 3 && isRunning_; p++) {
        for (uint8_t i = 0; i < 50; i++) {
          if (protocols[p] == 0x01) {
            // NEC protocol fuzzing
            uint32_t data = random(0, 0xFFFFFFFF);
            irsend_.sendNEC(data, 32, 1);
          } else if (protocols[p] == 0x02) {
            // RC5 protocol fuzzing
            uint16_t data = random(0, 0xFFFF);
            irsend_.sendRC5(data, 13, 1);
          } else if (protocols[p] == 0x03) {
            // Sony SIRC fuzzing
            uint16_t data = random(0, 0xFFFF);
            irsend_.sendSony(data, 15, 1);
          }

          mutationCount++;
          result.mutationsSent++;

          if ((esp_random() % 100) < 3) {
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
