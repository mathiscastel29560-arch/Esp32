#include "nfc_relay_attack.h"
#include <LittleFS.h>

namespace NfcRelayAttack {

NfcRelay::NfcRelay() : isRunning_(false) {}

RelayResult NfcRelay::performRelay(const RelayConfig& config) {
  RelayResult result;
  result.success = false;

  if (!TxArm::isArmed()) return result;

  isRunning_ = true;
  unsigned long startTime = millis();

  uint32_t relayCount = 0;
  uint32_t capturedCount = 0;

  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    // Simulate NFC relay attack
    if (random(0, 100) < 20) { // 20% detection rate
      relayCount++;

      if (config.capturePayload) {
        // Simulate payload capture
        uint8_t payload[64];
        for (int i = 0; i < 64; i++) {
          payload[i] = random(0, 256);
        }
        capturedCount++;
      }

      if (config.modifyPayload) {
        // Simulate payload modification for MitM attack
        delay(10);
      }
    }

    delay(100);
  }

  result.transactionsRelayed = relayCount;
  result.payloadsCaptured = capturedCount;
  result.success = relayCount > 0;
  result.logFile = "/logs/handshakes/nfc_relay.csv";

  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/nfc_relay.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/nfc_relay.csv", "a");
  }
  if (logFile) {
    logFile.printf("%lu,%u,%u\n", millis(), relayCount, capturedCount);
    logFile.close();
  }
  LittleFS.end();

  isRunning_ = false;
  return result;
}

void NfcRelay::stop() {
  isRunning_ = false;
}

} // namespace NfcRelayAttack
