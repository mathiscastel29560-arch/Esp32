#ifndef WPS_BRUTEFORCE_H
#define WPS_BRUTEFORCE_H

#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include <mbedtls/md.h>
#include <mbedtls/sha1.h>
#include <mbedtls/hmac_drbg.h>

namespace WpsBruteforce {

enum AttackMode {
  PIN_ENUMERATION,
  PIXIE_DUST,
  HYBRID_MODE
};

// WPA2 4-way handshake EAPOL frame
struct EapolFrame {
  uint8_t version;
  uint8_t type; // EAPOL-Key
  uint16_t length;
  uint8_t keyInfo[2];
  uint16_t keyLength;
  uint8_t replayCounter[8];
  uint8_t nonce[32];
  uint8_t keyIv[16];
  uint8_t keyRsc[8];
  uint8_t keyId[8];
  uint8_t keyMic[16];
  uint16_t keyDataLength;
  uint8_t keyData[256];
};

struct WpsConfig {
  char targetBssid[18];
  uint8_t targetChannel;
  AttackMode mode;
  uint32_t startPin;
  uint32_t endPin;
  uint32_t timeoutMs;
  bool aggressiveMode;
  bool captureHandshake; // Capture real handshake
};

struct WpsResult {
  bool success;
  uint32_t validPin;
  String psk;
  String ssid;
  uint32_t attemptsCompleted;
  uint32_t elapsedMs;
  String error;
  String logFile;
};

class WpsBruteforcer {
public:
  WpsBruteforcer();
  WpsResult bruteforcePin(const WpsConfig& config);
  WpsResult pixieDustAttack(const WpsConfig& config);
  WpsResult hybridAttack(const WpsConfig& config);

  // Real WPA2 handshake handling
  bool captureWpa2Handshake(const char* bssid, uint8_t channel);
  bool deriveWpa2Keys(const String& passphrase, const String& ssid,
                      uint8_t* psk, uint8_t* pmk, uint8_t* ptk);
  bool verifyMic(const EapolFrame& eapol, const uint8_t* kck);

  bool validatePin(uint32_t pin);
  String generateWpsNonce();
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  uint32_t attemptCount_;
  unsigned long startTime_;

  EapolFrame handshakeFrames_[4];
  uint8_t apMac_[6];
  uint8_t staMac_[6];
  uint8_t apNonce_[32];
  uint8_t staNonce_[32];

  void logAttempt(const char* mode, uint32_t pin, bool success);
  uint32_t calculateChecksum(uint32_t pin);
  String crackPsk(uint32_t pin);

  // Cryptographic helpers
  void pbkdf2(const uint8_t* password, uint32_t pwLen,
             const uint8_t* salt, uint32_t saltLen,
             uint32_t iterations, uint32_t outLen, uint8_t* output);
  void hmacSha1(const uint8_t* key, uint32_t keyLen,
               const uint8_t* data, uint32_t dataLen,
               uint8_t* output);
  uint32_t lfsr32(uint32_t state); // For Pixie Dust LFSR
};

} // namespace WpsBruteforce

#endif
