#include "subghz_fuzzing_engine.h"
#include <LittleFS.h>

namespace SubGhzFuzzing {

FuzzingEngine::FuzzingEngine(int8_t cs, int8_t irq, int8_t gpio)
  : radio_(cs, irq, gpio), isRunning_(false), startTime_(0) {
  // Initialize CC1101 for Sub-GHz fuzzing
  radio_.begin();
}

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

  // Set base modulation and bit rate
  setModulation(config.modulation);
  radio_.setBitRate(config.bitRate);

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    std::vector<uint8_t> payload;
    generateFuzzVector(payload);

    uint32_t freq = config.baseFreq;
    if (config.randomizeFreq) {
      // Random frequency deviation (±100kHz)
      int32_t deviation = random(-100000, 100000);
      freq += deviation;
    }

    ModulationType mod = config.modulation;
    if (config.varyModulation) {
      mod = (ModulationType)random(0, 4);
      mutations++;
    }

    // Transmit fuzzed payload
    if (transmitFuzzPayload(payload, freq, mod)) {
      result.packetsSent++;
    }

    delay(50);
  }

  result.mutationsApplied = mutations;
  result.elapsedMs = millis() - startTime_;
  result.success = (result.packetsSent > 0);
  result.logFile = "/logs/handshakes/subghz_fuzz.csv";

  // Log results
  if (LittleFS.begin()) {
    File logFile = LittleFS.open("/logs/handshakes/subghz_fuzz.csv", "a");
    if (!logFile) {
      LittleFS.mkdir("/logs/handshakes");
      logFile = LittleFS.open("/logs/handshakes/subghz_fuzz.csv", "a");
    }
    if (logFile) {
      logFile.printf("%lu,%u_packets,%u_mutations\n", millis() - startTime_,
                    result.packetsSent, result.mutationsApplied);
      logFile.close();
    }
    LittleFS.end();
  }

  isRunning_ = false;
  return result;
}

bool FuzzingEngine::transmitFuzzPayload(const std::vector<uint8_t>& payload, uint32_t freq, ModulationType mod) {
  // Set frequency (convert Hz to MHz for radio)
  float freqMHz = freq / 1000000.0;
  radio_.setFrequency(freqMHz);

  // Set modulation
  setModulation(mod);

  // Transmit payload
  uint8_t state = radio_.transmit((uint8_t*)payload.data(), payload.size());

  return (state == RADIOLIB_ERR_NONE);
}

bool FuzzingEngine::setModulation(ModulationType mod) {
  uint8_t state = RADIOLIB_ERR_NONE;

  switch (mod) {
    case MOD_2FSK:
      state = radio_.setModulation(RADIOLIB_CC1101_MOD_FSK_2);
      break;
    case MOD_GFSK:
      state = radio_.setModulation(RADIOLIB_CC1101_MOD_GFSK);
      break;
    case MOD_ASK:
      state = radio_.setModulation(RADIOLIB_CC1101_MOD_ASK_OOK);
      break;
    case MOD_OOK:
      state = radio_.setModulation(RADIOLIB_CC1101_MOD_OOK);
      break;
    default:
      state = radio_.setModulation(RADIOLIB_CC1101_MOD_FSK_2);
  }

  return (state == RADIOLIB_ERR_NONE);
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
