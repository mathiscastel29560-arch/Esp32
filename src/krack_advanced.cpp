#include "krack_advanced.h"
#include <LittleFS.h>
#include <mbedtls/aes.h>
#include <mbedtls/sha1.h>

namespace KrackAdvanced {

KrackAttacker::KrackAttacker() : isRunning_(false), startTime_(0) {}

KrackResult KrackAttacker::captureHandshake(const KrackConfig& config) {
  KrackResult result;
  result.success = false;
  result.packetsReplayed = 0;
  result.packetsDecrypted = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();
  handshakeData_.clear();

  // Simulate 4-way handshake capture
  // In real implementation: capture EAPOL frames and perform key recovery
  uint32_t captureTime = 0;
  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    // Simulate handshake frame reception
    // EAPOL frame 1: AP → Client (ANonce)
    // EAPOL frame 2: Client → AP (SNonce, MIC)
    // EAPOL frame 3: AP → Client (GTK, MIC)
    // EAPOL frame 4: Client → AP (ACK)

    if (captureTime == 0 || (millis() - startTime_ - captureTime) > 1000) {
      // Simulate handshake captured
      result.success = true;
      captureTime = millis() - startTime_;
      logHandshake(nullptr, 0);
    }

    delay(100);
  }

  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/krack_handshake.csv";
  isRunning_ = false;
  return result;
}

KrackResult KrackAttacker::replayPackets(const KrackConfig& config) {
  KrackResult result;
  result.success = false;
  result.packetsReplayed = 0;
  result.packetsDecrypted = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  // KRACK attack: replay encrypted packets with incremented key counter
  // Triggers key reinstallation vulnerability in affected devices
  uint32_t replayCount = 0;

  while (isRunning_ && (millis() - startTime_) < config.durationMs) {
    if (replayCount >= config.replayCount && config.replayCount > 0) {
      break;
    }

    // Simulate packet replay with counter increment
    // Real implementation: extract encrypted data, modify counter, retransmit
    if (config.aggressiveReplay) {
      // Send 10 replays per packet
      for (int i = 0; i < 10; i++) {
        replayCount++;
        delay(10); // 10ms between replays
      }
    } else {
      replayCount++;
      delay(100);
    }
  }

  result.packetsReplayed = replayCount;
  result.success = true;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/krack_replay.csv";

  isRunning_ = false;
  return result;
}

KrackResult KrackAttacker::recoverKey(const KrackConfig& config) {
  KrackResult result;
  result.success = false;
  result.packetsDecrypted = 0;

  isRunning_ = true;
  startTime_ = millis();

  // Simulate key recovery from captured handshake
  // KRACK works by:
  // 1. Forcing key reinstallation
  // 2. Exploiting key stream reuse
  // 3. Recovering plaintext via XOR analysis

  // Generate simulated PTK (Pairwise Transient Key)
  uint8_t ptkSeed[32];
  for (int i = 0; i < 32; i++) {
    ptkSeed[i] = random(0, 256);
  }

  // Derive PTK from PSK, ANonce, SNonce
  uint8_t nonce[32];
  for (int i = 0; i < 32; i++) {
    nonce[i] = random(0, 256);
  }

  result.recoveredKey = deriveKeys(ptkSeed, nonce);
  result.success = result.recoveredKey.isValid;
  result.packetsDecrypted = random(100, 1000);
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/krack_keys.csv";

  isRunning_ = false;
  return result;
}

WpaKey KrackAttacker::deriveKeys(const uint8_t* pmk, const uint8_t* nonce) {
  WpaKey key;
  key.isValid = false;
  key.keyCounter = 0;

  if (!pmk || !nonce) {
    return key;
  }

  // HMAC-SHA1 based PRF for WPA2 key derivation
  // PRF(key, label, data) = HMAC_SHA1(key, label + 0x00 + data + counter)
  mbedtls_sha1_context ctx;
  uint8_t hmacResult[20];

  // Simplified key derivation (real implementation would be full PRF)
  for (int i = 0; i < 48; i++) {
    key.ptk[i] = pmk[i % 32] ^ nonce[i % 32];
  }

  for (int i = 0; i < 32; i++) {
    key.gtk[i] = nonce[i] ^ pmk[i];
  }

  key.isValid = true;
  return key;
}

String KrackAttacker::decryptPacket(const uint8_t* encData, uint32_t len, const WpaKey& key) {
  if (!key.isValid || !encData || len == 0) {
    return "";
  }

  // RC4/TKIP or AES-CCMP decryption
  uint8_t decrypted[256] = {0};

  // Simulate decryption using AES-CCMP
  mbedtls_aes_context aes;
  mbedtls_aes_setkey_dec(&aes, key.ptk, 128);

  // In real scenario: extract IV/PN from packet, use as decryption IV
  for (uint32_t i = 0; i < len && i < sizeof(decrypted); i++) {
    decrypted[i] = encData[i] ^ key.ptk[i % 48];
  }

  String result;
  for (uint32_t i = 0; i < len && i < sizeof(decrypted); i++) {
    char hexBuf[3];
    snprintf(hexBuf, sizeof(hexBuf), "%02X", decrypted[i]);
    result += hexBuf;
  }

  return result;
}

void KrackAttacker::logHandshake(const uint8_t* data, uint32_t len) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/krack_handshake.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/krack_handshake.csv", "a");
  }

  if (logFile) {
    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "%lu,HANDSHAKE_CAPTURED,%u\n",
             millis(), len);
    logFile.print(logEntry);
    logFile.close();
  }

  LittleFS.end();
}

void KrackAttacker::stop() {
  isRunning_ = false;
}

} // namespace KrackAdvanced
