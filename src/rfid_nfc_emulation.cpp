#include "rfid_nfc_emulation.h"
#include <LittleFS.h>

namespace RfidNfcEmulation {

RfidEmulator::RfidEmulator() : isRunning_(false) {}

EmulationResult RfidEmulator::emulateNfcTag(const EmulationConfig& config) {
  EmulationResult result;
  result.success = false;
  result.readsDetected = 0;

  if (!TxArm::isArmed()) {
    return result;
  }

  isRunning_ = true;
  unsigned long startTime = millis();

  // Simulate NFC tag emulation
  // NFC Type 2 or 4 emulation
  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    if (random(0, 100) < 20) { // 20% chance of detection per cycle
      result.readsDetected++;
    }
    delay(100);
  }

  result.emulationTime = millis() - startTime;
  result.success = true;
  result.logFile = "/logs/handshakes/nfc_emulation.csv";

  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/nfc_emulation.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/nfc_emulation.csv", "a");
  }
  if (logFile) {
    logFile.printf("%lu,EMULATION,%u\n", millis(), result.readsDetected);
    logFile.close();
  }
  LittleFS.end();

  isRunning_ = false;
  return result;
}

EmulationResult RfidEmulator::replayCapture(const uint8_t* data, uint32_t len) {
  EmulationResult result;
  result.success = false;

  if (!data || len == 0) return result;

  isRunning_ = true;
  unsigned long startTime = millis();

  // Simulate replay attack
  for (int i = 0; i < 10 && isRunning_; i++) {
    result.readsDetected++;
    delay(200);
  }

  result.emulationTime = millis() - startTime;
  result.success = true;
  result.logFile = "/logs/handshakes/nfc_replay.csv";

  isRunning_ = false;
  return result;
}

void RfidEmulator::stop() {
  isRunning_ = false;
}

} // namespace RfidNfcEmulation
