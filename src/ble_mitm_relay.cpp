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

  uint32_t relayCount = 0;
  uint32_t deadline = startTime_ + config.durationMs;
  while (isRunning_ && (int32_t)(millis() - deadline) < 0) {
    // Simulate packet relay
    uint8_t simulatedData[20];
    for (int i = 0; i < 20; i++) {
      simulatedData[i] = (esp_random() % 256);
    }

    if (config.keyLogging) {
      logKeys(simulatedData, 20);
      result.keysLogged++;
    }

    if (config.dataInterception) {
      result.bytesIntercepted += 20;
    }

    relayCount++;
    delay(100);
  }

  result.packetsRelayed = relayCount;
  result.success = true;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_mitm.csv";

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

  // Simulate interception of BLE data
  // In real scenario: decrypt GATT characteristics and log
  result.bytesIntercepted = len;
  result.packetsRelayed = 1;
  result.success = true;

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
