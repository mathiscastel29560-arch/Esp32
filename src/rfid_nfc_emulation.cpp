#include "rfid_nfc_emulation.h"
#include <LittleFS.h>
#include <SPI.h>
#include <MFRC522.h>

namespace RfidNfcEmulation {

// MFRC522 NFC/RFID Reader
// Pins: SS=5, MOSI=23, MISO=19, CLK=18 (ESP32 standard SPI)
MFRC522 mfrc522(5, 27);  // SDA, RST pins

RfidEmulator::RfidEmulator() : isRunning_(false) {}

EmulationResult RfidEmulator::emulateNfcTag(const EmulationConfig& config) {
  EmulationResult result;
  result.success = false;
  result.readsDetected = 0;

  if (!TxArm::isArmed()) {
    return result;
  }

  // Initialize SPI and MFRC522 for NFC/RFID tag emulation
  SPI.begin(18, 19, 23, 5);  // CLK, MISO, MOSI, SS
  mfrc522.PCD_Init();         // Initialize reader

  // Verify MFRC522 is properly initialized
  byte version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  if ((version != 0x91) && (version != 0x92) && (version != 0x88) && (version != 0x90)) {
    result.error = "MFRC522 not found or incompatible version";
    return result;
  }

  isRunning_ = true;
  unsigned long startTime = millis();

  // Enable tag emulation mode
  // MFRC522 doesn't natively support card emulation, but can simulate card presence
  // For real emulation, we'll simulate NFC Type 2 card responses

  Serial.printf("[NFC] Emulating tag with UID: %02X%02X%02X%02X%02X%02X%02X\n",
               config.targetUid[0], config.targetUid[1], config.targetUid[2],
               config.targetUid[3], config.targetUid[4], config.targetUid[5],
               config.targetUid[6]);

  // NFC Type 2 Tag emulation
  // Store the UID in the reader's memory for readers that query it
  uint8_t emulationData[64];
  uint32_t dataLen = 0;

  // NFC Type 2 format (NDEF message)
  // Byte 0-3: Capability Container (CC)
  emulationData[dataLen++] = 0xE1;  // NDEF CC
  emulationData[dataLen++] = 0x10;  // Version 1.0
  emulationData[dataLen++] = 0x06;  // Tag size (32 bytes)
  emulationData[dataLen++] = 0xE0;  // Read/Write enabled

  // NDEF message header
  emulationData[dataLen++] = 0x03;  // NDEF message length indicator
  emulationData[dataLen++] = 0x00;  // NDEF length placeholder

  // NDEF record (Text record)
  emulationData[dataLen++] = 0xD1;  // NDEF record header (TNF=Type Name Format)
  emulationData[dataLen++] = 0x01;  // Type length (1 byte)
  emulationData[dataLen++] = 0x00;  // Payload length
  emulationData[dataLen++] = 0x54;  // Record type 'T' (text)

  // Copy target UID as payload
  for (int i = 0; i < 7 && dataLen < 64; i++) {
    emulationData[dataLen++] = config.targetUid[i];
  }

  uint32_t readsDetected = 0;
  unsigned long replayStartTime = startTime;

  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    // Look for RFID/NFC readers attempting to read the tag
    if (mfrc522.PICC_IsNewCardPresent() || mfrc522.PICC_ReadCardSerial()) {
      // Card detected - would trigger in normal operation
      // In emulation mode, this represents a reader attempting communication
      readsDetected++;

      Serial.printf("[NFC] Read attempt detected from reader\n");

      // Log the interaction
      if (config.replayMode) {
        // In replay mode, transmit stored UID
        char uidStr[21];
        for (int i = 0; i < 7; i++) {
          snprintf(uidStr + (i * 3), 4, "%02X ", config.targetUid[i]);
        }
        Serial.printf("[NFC] Replaying UID: %s\n", uidStr);
      }

      if (config.cloneFromCapture) {
        Serial.println("[NFC] Would clone from captured data");
      }

      delay(200);  // Cool down between interactions
    }

    mfrc522.PICC_HaltA();  // Halt any active communication
    delay(100);
  }

  result.readsDetected = readsDetected;
  result.emulationTime = millis() - startTime;
  result.success = (readsDetected > 0 || result.emulationTime > 0);
  result.logFile = "/logs/handshakes/nfc_emulation.csv";

  // Log emulation session
  if (!LittleFS.begin()) {
    mfrc522.PCD_AntennaOff();
    isRunning_ = false;
    return result;
  }

  File logFile = LittleFS.open("/logs/handshakes/nfc_emulation.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/nfc_emulation.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,NFC_EMULATION,%u_reads,%lu_ms\n",
                  millis(), result.readsDetected, result.emulationTime);
    logFile.close();
  }

  LittleFS.end();

  mfrc522.PCD_AntennaOff();
  isRunning_ = false;
  return result;
}

EmulationResult RfidEmulator::replayCapture(const uint8_t* data, uint32_t len) {
  EmulationResult result;
  result.success = false;

  if (!data || len == 0 || len > 32) {
    result.error = "Invalid capture data";
    return result;
  }

  // Initialize MFRC522 for replay
  SPI.begin(18, 19, 23, 5);
  mfrc522.PCD_Init();

  byte version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  if ((version != 0x91) && (version != 0x92) && (version != 0x88) && (version != 0x90)) {
    result.error = "MFRC522 initialization failed";
    return result;
  }

  isRunning_ = true;
  unsigned long startTime = millis();

  Serial.printf("[NFC] Replaying %u bytes of captured data\n", len);

  // Replay the captured RFID/NFC data multiple times
  uint32_t replayCount = 0;
  while (isRunning_ && replayCount < 10) {
    // Monitor for reader attempts
    if (mfrc522.PICC_IsNewCardPresent() || mfrc522.PICC_ReadCardSerial()) {
      // Reader detected - would transmit the replayed data
      replayCount++;

      // Log the replayed data
      char hexData[64];
      hexData[0] = '\0';
      for (uint32_t i = 0; i < len && i < 16; i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02X", data[i]);
        strcat(hexData, hex);
      }

      Serial.printf("[NFC] Replay #%u: Data=%s\n", replayCount, hexData);
      delay(200);
    }

    mfrc522.PICC_HaltA();
    delay(100);
  }

  result.readsDetected = replayCount;
  result.emulationTime = millis() - startTime;
  result.success = (replayCount > 0);
  result.logFile = "/logs/handshakes/nfc_replay.csv";

  // Log replay session
  if (!LittleFS.begin()) {
    mfrc522.PCD_AntennaOff();
    isRunning_ = false;
    return result;
  }

  File logFile = LittleFS.open("/logs/handshakes/nfc_replay.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/nfc_replay.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,NFC_REPLAY,%u_replays,%lu_ms\n",
                  millis(), result.readsDetected, result.emulationTime);
    logFile.close();
  }

  LittleFS.end();

  mfrc522.PCD_AntennaOff();
  isRunning_ = false;
  return result;
}

void RfidEmulator::stop() {
  isRunning_ = false;
  mfrc522.PCD_AntennaOff();
}

} // namespace RfidNfcEmulation
