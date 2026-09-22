#ifndef BLE_CREDENTIAL_HARVESTING_H
#define BLE_CREDENTIAL_HARVESTING_H

#include <Arduino.h>
#include <vector>

namespace BleCredentialHarvesting {

struct BleCredential {
  uint32_t timestamp;
  uint8_t deviceAddr[6];
  String credentialType;
  String harvestedData;
  int32_t rssi;
};

struct HarvestConfig {
  uint32_t scanDurationMs;
  bool interceptPairing;
  bool captureCharacteristics;
  bool analyzePayloads;
};

struct HarvestResult {
  bool success;
  std::vector<BleCredential> credentials;
  uint32_t credentialsFound;
  uint32_t pairingAttempts;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class CredentialHarvester {
public:
  CredentialHarvester();
  HarvestResult harvestCredentials(const HarvestConfig& config);
  HarvestResult interceptPairingData(const uint8_t* pairingData, uint32_t len);
  HarvestResult captureCharacteristics(const uint8_t* addr);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;

  void logCredential(const BleCredential& cred);
  String parseCredentialPayload(const uint8_t* data, uint32_t len);
};

} // namespace BleCredentialHarvesting

#endif
