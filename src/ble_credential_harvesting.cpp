#include "ble_credential_harvesting.h"
#include "hex_utils.h"
#include <LittleFS.h>
#include <NimBLEDevice.h>

namespace BleCredentialHarvesting {

CredentialHarvester::CredentialHarvester() : isRunning_(false), startTime_(0) {}

HarvestResult CredentialHarvester::harvestCredentials(const HarvestConfig& config) {
  HarvestResult result;
  result.success = false;
  result.credentialsFound = 0;
  result.pairingAttempts = 0;

  isRunning_ = true;
  startTime_ = millis();

  NimBLEDevice::init("ESP32-CredHarvest");
  NimBLEScan* pScan = NimBLEDevice::getScan();

  pScan->setActiveScan(true);
  pScan->setInterval(100);
  pScan->setWindow(99);

  // Scan for BLE devices advertising pairing/authentication services
  NimBLEScanResults scanResults = pScan->start(config.scanDurationMs / 1000, false);

  for (int i = 0; i < scanResults.getCount() && isRunning_; i++) {
    NimBLEAdvertisedDevice device = scanResults.getDevice(i);

    // Check for authentication/pairing services
    // GATT Service UUIDs that might contain credentials:
    // - User Defined Service (FFxx)
    // - Proprietary services
    // - Device Information Service (180A)

    if (device.haveServiceUUID()) {
      NimBLEUUID serviceUUID = device.getServiceUUID();

      // Simulate credential extraction from service data
      if (config.captureCharacteristics) {
        HarvestResult charResult = captureCharacteristics((uint8_t*)device.getAddress().getNative());
        result.credentials.insert(result.credentials.end(),
                                 charResult.credentials.begin(),
                                 charResult.credentials.end());
        result.credentialsFound += charResult.credentialsFound;
      }
    }

    // Simulate pairing interception
    if (config.interceptPairing) {
      if ((esp_random() % 100) < 30) { // 30% simulated success rate
        result.pairingAttempts++;

        BleCredential cred;
        memcpy(cred.deviceAddr, device.getAddress().getNative(), 6);
        cred.timestamp = millis();
        cred.credentialType = "PAIRING_KEY";
        cred.rssi = device.getRSSI();

        // Simulate pairing key extraction
        uint8_t keyData[16];
        for (int j = 0; j < 16; j++) {
          keyData[j] = (esp_random() % 256);
        }

        // Build hex string using utility
        cred.harvestedData = HexUtils::toHexString(keyData, 16);

        result.credentials.push_back(cred);
        result.credentialsFound++;
        logCredential(cred);
      }
    }
  }

  result.success = result.credentialsFound > 0;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_credentials.csv";

  pScan->stop();
  isRunning_ = false;
  return result;
}

HarvestResult CredentialHarvester::interceptPairingData(const uint8_t* pairingData, uint32_t len) {
  HarvestResult result;
  result.success = false;

  if (!pairingData || len == 0) {
    result.error = "Invalid pairing data";
    return result;
  }

  // Parse BLE pairing PDU (Protocol Data Unit)
  // Pairing Request/Response contains:
  // - IO Capability
  // - OOB Data Flag
  // - Auth Req
  // - Max Encryption Key Size
  // - Initiator Key Distribution
  // - Responder Key Distribution

  BleCredential cred;
  cred.timestamp = millis();
  cred.credentialType = "PAIRING_PDU";

  cred.harvestedData = "";
  for (uint32_t i = 0; i < len && i < 32; i++) {
    char hexBuf[3];
    snprintf(hexBuf, sizeof(hexBuf), "%02X", pairingData[i]);
    cred.harvestedData += hexBuf;
  }

  result.credentials.push_back(cred);
  result.credentialsFound = 1;
  result.success = true;

  logCredential(cred);
  return result;
}

HarvestResult CredentialHarvester::captureCharacteristics(const uint8_t* addr) {
  HarvestResult result;
  result.success = false;
  result.credentialsFound = 0;

  if (!addr) {
    result.error = "Invalid address";
    return result;
  }

  // Simulate GATT characteristic enumeration and capture
  // In real implementation: connect, discover services, read characteristics

  uint32_t charCount = ((esp_random() % 6) + 2);
  for (uint32_t i = 0; i < charCount; i++) {
    BleCredential cred;
    memcpy(cred.deviceAddr, addr, 6);
    cred.timestamp = millis();
    cred.credentialType = "GATT_CHAR";
    cred.rssi = ((esp_random() % 50) + -80);

    // Simulate characteristic data
    cred.harvestedData = "";
    uint32_t dataLen = ((esp_random() % 28) + 4);
    for (uint32_t j = 0; j < dataLen; j++) {
      char hexBuf[3];
      snprintf(hexBuf, sizeof(hexBuf), "%02X", (esp_random() % 256));
      cred.harvestedData += hexBuf;
    }

    result.credentials.push_back(cred);
    result.credentialsFound++;
    logCredential(cred);
  }

  result.success = true;
  return result;
}

void CredentialHarvester::logCredential(const BleCredential& cred) {
  if (!LittleFS.begin()) {
    Serial.println("ERROR: Failed to mount LittleFS");
    return;
  }

  // Ensure cleanup even on early return
  auto cleanup = [](){ LittleFS.end(); };

  File logFile = LittleFS.open("/logs/handshakes/ble_credentials.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ble_credentials.csv", "a");
  }

  if (logFile) {
    char addrBuf[18];
    snprintf(addrBuf, sizeof(addrBuf), "%02X:%02X:%02X:%02X:%02X:%02X",
             cred.deviceAddr[0], cred.deviceAddr[1], cred.deviceAddr[2],
             cred.deviceAddr[3], cred.deviceAddr[4], cred.deviceAddr[5]);

    logFile.printf("%lu,%s,%s,%s,%d\n", cred.timestamp, cred.credentialType.c_str(),
                   addrBuf, cred.harvestedData.c_str(), cred.rssi);
    logFile.close();
  } else {
    Serial.println("ERROR: Failed to open credential log file");
  }

  LittleFS.end();
}

String CredentialHarvester::parseCredentialPayload(const uint8_t* data, uint32_t len) {
  if (!data || len == 0) return "";

  // Use utility for efficient hex conversion
  return HexUtils::toHexString(data, len);
}

void CredentialHarvester::stop() {
  isRunning_ = false;
  NimBLEDevice::deinit();
}

} // namespace BleCredentialHarvesting
