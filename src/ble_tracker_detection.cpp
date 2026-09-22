#include "ble_tracker_detection.h"
#include <LittleFS.h>
#include <NimBLEDevice.h>

namespace BleTrackerDetection {

TrackerDetector::TrackerDetector() : isRunning_(false), startTime_(0) {}

TrackerResult TrackerDetector::scanTrackers(const TrackerConfig& config) {
  TrackerResult result;
  result.success = false;
  result.totalTrackers = 0;

  isRunning_ = true;
  startTime_ = millis();

  NimBLEDevice::init("ESP32-TrackerScan");
  NimBLEScan* pScan = NimBLEDevice::getScan();

  pScan->setActiveScan(false);
  pScan->setInterval(100);
  pScan->setWindow(99);

  // Scan for BLE devices
  NimBLEScanResults scanResults = pScan->start(config.scanDurationMs / 1000, false);

  for (int i = 0; i < scanResults.getCount() && isRunning_; i++) {
    NimBLEAdvertisedDevice device = scanResults.getDevice(i);

    // Check if device is a tracker
    if (device.haveManufacturerData()) {
      std::string mfgData = device.getManufacturerData();

      TrackerDevice tracker;
      memset(tracker.addr, 0, 6);
      NimBLEAddress devAddr = device.getAddress();
      memcpy(tracker.addr, devAddr.getNative(), 6);

      tracker.rssi = device.getRSSI();
      tracker.lastSeen = millis();
      tracker.type = identifyTracker((uint8_t*)mfgData.data(), mfgData.length());

      if (tracker.type != OTHER) {
        result.trackersFound.push_back(tracker);
        result.totalTrackers++;
        logTracker(tracker);
      }
    }
  }

  result.success = result.totalTrackers > 0;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_trackers.csv";

  pScan->stop();
  isRunning_ = false;
  return result;
}

TrackerType TrackerDetector::identifyTracker(const uint8_t* advData, uint32_t advLen) {
  if (!advData || advLen == 0) return OTHER;

  // Apple AirTag: Manufacturer ID 0x004C (Apple)
  if (advLen >= 2 && advData[0] == 0x4C && advData[1] == 0x00) {
    return AIRTAG;
  }

  // Tile: Check for Tile-specific UUID or manufacturer data
  if (advLen >= 2 && (advData[0] == 0xEE || advData[0] == 0x4C)) {
    return TILE;
  }

  // Chipolo: Manufacturer ID 0x0295
  if (advLen >= 2 && advData[0] == 0x95 && advData[1] == 0x02) {
    return CHIPOLO;
  }

  // Samsung SmartTag: Check for Samsung manufacturer ID
  if (advLen >= 2 && advData[0] == 0x75 && advData[1] == 0x00) {
    return SAMSUNG_SMARTTAG;
  }

  return OTHER;
}

TrackerResult TrackerDetector::trackMovement(const uint8_t* addr, uint32_t durationMs) {
  TrackerResult result;
  result.success = false;

  if (!addr) {
    result.error = "Invalid address";
    return result;
  }

  isRunning_ = true;
  startTime_ = millis();

  // Track RSSI changes over time to estimate movement
  int32_t previousRssi = 0;
  uint32_t movementEvents = 0;

  NimBLEDevice::init("ESP32-TrackerTrack");
  NimBLEScan* pScan = NimBLEDevice::getScan();

  while (isRunning_ && (millis() - startTime_) < durationMs) {
    NimBLEScanResults scanResults = pScan->start(2, false);

    for (int i = 0; i < scanResults.getCount(); i++) {
      NimBLEAdvertisedDevice device = scanResults.getDevice(i);
      NimBLEAddress devAddr = device.getAddress();

      uint8_t deviceAddr[6];
      memcpy(deviceAddr, devAddr.getNative(), 6);

      bool match = true;
      for (int j = 0; j < 6; j++) {
        if (deviceAddr[j] != addr[j]) {
          match = false;
          break;
        }
      }

      if (match) {
        int32_t currentRssi = device.getRSSI();

        if (previousRssi != 0 && abs(currentRssi - previousRssi) > 10) {
          movementEvents++;
        }

        previousRssi = currentRssi;
        TrackerDevice tracker;
        memcpy(tracker.addr, deviceAddr, 6);
        tracker.rssi = currentRssi;
        tracker.lastSeen = millis();
        result.trackersFound.push_back(tracker);
      }
    }

    delay(1000);
  }

  result.success = true;
  result.elapsedMs = millis() - startTime_;
  result.logFile = "/logs/handshakes/ble_tracker_movement.csv";

  pScan->stop();
  isRunning_ = false;
  return result;
}

void TrackerDetector::logTracker(const TrackerDevice& device) {
  if (!LittleFS.begin()) return;

  File logFile = LittleFS.open("/logs/handshakes/ble_trackers.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/ble_trackers.csv", "a");
  }

  if (logFile) {
    const char* typeNames[] = {"AIRTAG", "TILE", "CHIPOLO", "SMARTTAG", "OTHER"};
    char addrBuf[18];
    snprintf(addrBuf, sizeof(addrBuf), "%02X:%02X:%02X:%02X:%02X:%02X",
             device.addr[0], device.addr[1], device.addr[2],
             device.addr[3], device.addr[4], device.addr[5]);

    logFile.printf("%lu,%s,%s,%d\n", millis(), typeNames[device.type], addrBuf, device.rssi);
    logFile.close();
  }

  LittleFS.end();
}

void TrackerDetector::stop() {
  isRunning_ = false;
  NimBLEDevice::deinit();
}

} // namespace BleTrackerDetection
