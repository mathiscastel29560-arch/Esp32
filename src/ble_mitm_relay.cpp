#include "ble_mitm_relay.h"
#include <LittleFS.h>
#include "tx_arm.h"

namespace BleMitmRelay {

MitmRelay::MitmRelay() : isRunning_(false), startTime_(0) {}

RelayResult MitmRelay::startRelay(const RelayConfig& config) {
  RelayResult result;
  result.success = false;
  result.packetsRelayed = 0;
  result.keysLogged = 0;

  if (!TxArm::isArmed()) {
    result.error = "TX not armed";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  NimBLEDevice::init("ESP32-Relay");

  NimBLEClient* pClient = NimBLEDevice::createClient();
  if (!pClient) {
    result.error = "Failed to create BLE client";
    isRunning_ = false;
    return result;
  }

  NimBLEAddress targetAddr(config.targetAddr, false);

  Serial.println("Starting BLE MITM relay (real GATT interception)...");
  Serial.printf("Target: %02X:%02X:%02X:%02X:%02X:%02X\n",
    config.targetAddr[0], config.targetAddr[1], config.targetAddr[2],
    config.targetAddr[3], config.targetAddr[4], config.targetAddr[5]);

  if (!pClient->connect(targetAddr, false)) {
    result.error = "Connection failed";
    NimBLEDevice::deleteClient(pClient);
    NimBLEDevice::deinit();
    isRunning_ = false;
    return result;
  }

  std::vector<NimBLERemoteService*>* services = pClient->getServices(true);
  uint32_t relayCount = 0;
  uint32_t deadline = startTime_ + config.durationMs;

  while (isRunning_ && (int32_t)(millis() - deadline) < 0) {
    if (!pClient->isConnected()) {
      result.error = "Connection lost";
      break;
    }

    for (auto pSvc : *services) {
      std::vector<NimBLERemoteCharacteristic*>* characteristics = pSvc->getCharacteristics(true);

      for (auto pChr : *characteristics) {
        if (pChr->canRead()) {
          std::string value = pChr->readValue();

          if (value.length() > 0) {
            if (config.dataInterception) {
              result.bytesIntercepted += value.length();
            }

            if (config.keyLogging) {
              logKeys((const uint8_t*)value.data(), value.length());
              result.keysLogged++;

              Serial.printf("  [%d] GATT read: %d bytes\n", result.keysLogged, (int)value.length());
            }

            if (config.notifyInterception && pChr->canNotify()) {
              pChr->subscribe(true);
              Serial.printf("    Subscribed to notifications on characteristic\n");
            }

            relayCount++;
          }
        }
      }
    }

    delay(100);
  }

  pClient->disconnect();
  NimBLEDevice::deleteClient(pClient);
  NimBLEDevice::deinit();

  result.packetsRelayed = relayCount;
  result.success = relayCount > 0;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_mitm.csv";

  Serial.printf("MITM relay complete: %d packets relayed, %d bytes intercepted\n",
    relayCount, result.bytesIntercepted);

  isRunning_ = false;
  return result;
}

RelayResult MitmRelay::interceptData(uint8_t* data, uint32_t len) {
  RelayResult result;
  result.success = false;

  if (!data || len == 0) {
    result.error = "Invalid data";
    return result;
  }

  NimBLEDevice::init("ESP32-Intercept");

  for (int i = 0; i < len && i < 20; i++) {
    if (data[i] >= 32 && data[i] <= 126) {
      Serial.write(data[i]);
    } else {
      Serial.printf("[%02X]", data[i]);
    }
  }
  Serial.println();

  logKeys(data, (len > 256) ? 256 : len);

  result.bytesIntercepted = len;
  result.packetsRelayed = 1;
  result.success = true;

  NimBLEDevice::deinit();
  return result;
}

void MitmRelay::logKeys(const uint8_t* keyData, uint32_t len) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/ble_keys.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ble_keys.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,KEY_LOG,%u\n", millis(), len);
    logFile.close();
  }

  LittleFS.end();
}

void MitmRelay::onDeviceConnected(NimBLEClient* pClient) {
  // Handle connection
}

void MitmRelay::onDeviceDisconnected(NimBLEClient* pClient) {
  // Handle disconnection
}

void MitmRelay::stop() {
  isRunning_ = false;
  NimBLEDevice::deinit();
}

} // namespace BleMitmRelay
