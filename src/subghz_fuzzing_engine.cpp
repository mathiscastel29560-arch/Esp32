#include "subghz_fuzzing_engine.h"
#include <LittleFS.h>
#include "tx_arm.h"
#include "config.h"
#include <RadioLib.h>

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);
}

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

  // Initialize radio at base frequency
  float freqMHz = config.baseFreq / 1000000.0f;
  if (radio.begin(freqMHz) != RADIOLIB_ERR_NONE) {
    Serial.println("✗ Sub-GHz radio init failed");
    result.logFile = "/logs/handshakes/subghz_fuzz.csv";
    return result;
  }

  radio.setOOK(true);
  radio.transmitDirectAsync();
  pinMode(PIN_CC1101_GDO0, OUTPUT);

  isRunning_ = true;
  startTime_ = millis();
  uint32_t mutations = 0;
  uint32_t deadline = startTime_ + config.durationMs;

  std::vector<uint8_t> payload;
  payload.reserve(64);

  while (isRunning_ && (int32_t)(millis() - deadline) < 0) {
    payload.clear();
    generateFuzzVector(payload);

    uint32_t freq = config.baseFreq;
    if (config.randomizeFreq) {
      freq += ((esp_random() % 200000) - 100000);
      freqMHz = freq / 1000000.0f;
      radio.setFrequency(freqMHz);
    }

    uint32_t modulation = config.modulation;
    if (config.varyModulation) {
      modulation = (esp_random() % 3);
      mutations++;
    }

    // Transmit preamble
    for (int i = 0; i < 16; i++) {
      digitalWrite(PIN_CC1101_GDO0, i % 2);
      delayMicroseconds(200);
    }

    // Transmit fuzz payload as bits
    for (uint8_t byte : payload) {
      for (int i = 7; i >= 0; i--) {
        bool bit = (byte >> i) & 1;
        digitalWrite(PIN_CC1101_GDO0, bit);
        delayMicroseconds(200);
      }
    }

    // Send trailing zeros
    for (int i = 0; i < 8; i++) {
      digitalWrite(PIN_CC1101_GDO0, 0);
      delayMicroseconds(200);
    }

    result.packetsSent++;
    delay(50);
  }

  digitalWrite(PIN_CC1101_GDO0, LOW);
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
