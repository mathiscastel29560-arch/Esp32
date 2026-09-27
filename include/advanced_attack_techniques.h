#ifndef ADVANCED_ATTACK_TECHNIQUES_H
#define ADVANCED_ATTACK_TECHNIQUES_H

#include <Arduino.h>
#include <vector>
#include "attack_framework.h"

// ============= ADVANCED WIFI ATTACK TECHNIQUES =============

class WiFiAdvancedScanner : public Attack {
public:
  WiFiAdvancedScanner() : Attack("WiFi Advanced Scanner"),
                          channelHopping(true),
                          passiveScan(false),
                          maxNetworks(50) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

  bool setParameter(const char* key, const char* value) override;
  const char* getParameter(const char* key) override;

private:
  bool channelHopping;
  bool passiveScan;
  uint16_t maxNetworks;
  uint8_t currentChannel;
  uint32_t channelSwitchTime;
  uint32_t lastChannelSwitch;

  void scanChannel(uint8_t channel);
  void captureHiddenSSID();
  void analyzeSignalStrength();
};

class WiFiPacketCapture : public Attack {
public:
  WiFiPacketCapture() : Attack("WiFi Packet Capture"),
                       captureFilter(0),
                       packetCount(0),
                       maxPackets(10000) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

  bool setParameter(const char* key, const char* value) override;

private:
  uint8_t captureFilter;
  uint32_t packetCount;
  uint32_t maxPackets;
  uint32_t captureStartTime;

  void captureBeacons();
  void captureData();
  void captureProbes();
};

class WiFiHandshakeCapture : public Attack {
public:
  WiFiHandshakeCapture() : Attack("WiFi Handshake Capture"),
                          targetBSSID(""),
                          handshakesCaught(0),
                          deauthFramesNeeded(10) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

  bool setParameter(const char* key, const char* value) override;

private:
  char targetBSSID[18];
  uint16_t handshakesCaught;
  uint8_t deauthFramesNeeded;
  uint32_t lastDeauthTime;

  void sendDeauthFrames();
  void captureHandshake();
  void validateHandshake();
};

// ============= ADVANCED BLE ATTACK TECHNIQUES =============

class BLEAdvancedScanner : public Attack {
public:
  BLEAdvancedScanner() : Attack("BLE Advanced Scanner"),
                        scanDuration(30000),
                        captureAdvData(true),
                        sniffConnections(true) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

  bool setParameter(const char* key, const char* value) override;

private:
  uint32_t scanDuration;
  bool captureAdvData;
  bool sniffConnections;

  void extractGattServices();
  void captureConnections();
  void analyzeAdvertisements();
};

class BLEConnectionHijack : public Attack {
public:
  BLEConnectionHijack() : Attack("BLE Connection Hijack"),
                         targetAddress(""),
                         hijackAttempts(0) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

  bool setParameter(const char* key, const char* value) override;

private:
  char targetAddress[18];
  uint16_t hijackAttempts;

  void performMitmAttack();
  void replayPackets();
  void spoofMaster();
};

// ============= ADVANCED RF ATTACK TECHNIQUES =============

class RFAdvancedScanner : public Attack {
public:
  RFAdvancedScanner() : Attack("RF Advanced Scanner"),
                       frequencyHopping(true),
                       analysisDepth(2) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

private:
  bool frequencyHopping;
  uint8_t analysisDepth;

  void frequencyAnalysis();
  void modulationDetection();
  void patternRecognition();
};

// ============= ADVANCED NFC ATTACK TECHNIQUES =============

class NFCAdvancedAttack : public Attack {
public:
  NFCAdvancedAttack() : Attack("NFC Advanced Attack"),
                       mifareCloning(false),
                       ndef_injection(false) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

private:
  bool mifareCloning;
  bool ndef_injection;

  void cloneMifareCard();
  void injectNDEF();
  void bypassProtection();
};

// ============= RESILIENCE & ANTI-DETECTION =============

class AttackStealthMode : public Attack {
public:
  AttackStealthMode() : Attack("Stealth Mode"),
                       slowdownFactor(10),
                       randomizeTiming(true),
                       useChannelHopping(true) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

  void setSlowdownFactor(uint16_t factor) { slowdownFactor = factor; }

private:
  uint16_t slowdownFactor;
  bool randomizeTiming;
  bool useChannelHopping;
  uint32_t nextActionTime;

  uint32_t calculateRandomDelay();
};

class AttackPersistence : public Attack {
public:
  AttackPersistence() : Attack("Attack Persistence"),
                       resumeOnFailure(true),
                       maxRetries(10),
                       retryCount(0) {}

  bool begin() override;
  bool start() override;
  void update() override;
  bool stop() override;

private:
  bool resumeOnFailure;
  uint8_t maxRetries;
  uint8_t retryCount;

  void handleFailure();
  void retryAttack();
};

#endif
