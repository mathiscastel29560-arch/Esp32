#ifndef BLE_TRACKER_DETECTION_H
#define BLE_TRACKER_DETECTION_H

#include <Arduino.h>
#include <vector>

namespace BleTrackerDetection {

enum TrackerType {
  AIRTAG,
  TILE,
  CHIPOLO,
  SAMSUNG_SMARTTAG,
  OTHER
};

struct TrackerDevice {
  uint8_t addr[6];
  TrackerType type;
  int32_t rssi;
  String beaconFormat;
  uint32_t lastSeen;
};

struct TrackerConfig {
  uint32_t scanDurationMs;
  bool trackMovement;
  bool analyzeBeacons;
};

struct TrackerResult {
  bool success;
  std::vector<TrackerDevice> trackersFound;
  uint32_t totalTrackers;
  uint32_t elapsedMs;
  String logFile;
  String error;
};

class TrackerDetector {
public:
  TrackerDetector();
  TrackerResult scanTrackers(const TrackerConfig& config);
  TrackerType identifyTracker(const uint8_t* advData, uint32_t advLen);
  TrackerResult trackMovement(const uint8_t* addr, uint32_t durationMs);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;

  void logTracker(const TrackerDevice& device);
};

} // namespace BleTrackerDetection

#endif
