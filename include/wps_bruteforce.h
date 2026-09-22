#ifndef WPS_BRUTEFORCE_H
#define WPS_BRUTEFORCE_H

#include <Arduino.h>
#include <vector>
#include <WiFi.h>

namespace WpsBruteforce {

enum AttackMode {
  PIN_ENUMERATION,
  PIXIE_DUST,
  HYBRID_MODE
};

struct WpsConfig {
  char targetBssid[18];
  uint8_t targetChannel;
  AttackMode mode;
  uint32_t startPin;
  uint32_t endPin;
  uint32_t timeoutMs;
  bool aggressiveMode;
};

struct WpsResult {
  bool success;
  uint32_t validPin;
  String psk;
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
  bool validatePin(uint32_t pin);
  String generateWpsNonce();
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  uint32_t attemptCount_;
  unsigned long startTime_;

  void logAttempt(const char* mode, uint32_t pin, bool success);
  uint32_t calculateChecksum(uint32_t pin);
  String crackPsk(uint32_t pin);
};

} // namespace WpsBruteforce

#endif
