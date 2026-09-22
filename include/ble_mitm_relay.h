#ifndef BLE_MITM_RELAY_H
#define BLE_MITM_RELAY_H

#include <Arduino.h>
#include <vector>
#include <NimBLEDevice.h>

namespace BleMitmRelay {

struct RelayConfig {
  uint8_t targetAddr[6];
  uint32_t durationMs;
  bool passiveMode;
  bool keyLogging;
  bool dataInterception;
};

struct RelayResult {
  bool success;
  uint32_t packetsRelayed;
  uint32_t keysLogged;
  uint32_t bytesIntercepted;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class MitmRelay {
public:
  MitmRelay();
  RelayResult startRelay(const RelayConfig& config);
  RelayResult interceptData(uint8_t* data, uint32_t len);
  void logKeys(const uint8_t* keyData, uint32_t len);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;

  void onDeviceConnected(NimBLEClient* pClient);
  void onDeviceDisconnected(NimBLEClient* pClient);
};

} // namespace BleMitmRelay

#endif
