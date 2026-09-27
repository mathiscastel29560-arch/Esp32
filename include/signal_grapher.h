#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <cstring>

// ============= SIGNAL GRAPHING & DATA VISUALIZATION =============
class SignalGrapher {
public:
  static SignalGrapher& getInstance() {
    static SignalGrapher instance;
    return instance;
  }

  // Data point structure
  struct DataPoint {
    uint32_t timestamp;
    int8_t rssi;              // Signal strength (dBm)
    uint8_t signalPercent;    // 0-100%
  };

  // Add a RSSI reading
  void addRSSIReading(int8_t rssi);

  // Add WiFi device detected
  void addWiFiDevice(const char* ssid, int8_t rssi, uint16_t channel);

  // Add BLE device
  void addBLEDevice(const char* address, int8_t rssi);

  // Get recent readings
  std::vector<DataPoint> getRecentReadings(uint16_t count) const;

  // Print ASCII graph (terminal output)
  void printASCIIGraph(const char* title, uint16_t width = 40, uint16_t height = 10) const;

  // Print signal strength histogram
  void printSignalHistogram() const;

  // Print heatmap (by channel for WiFi)
  void printChannelHeatmap() const;

  // Statistics
  int8_t getMaxRSSI() const;
  int8_t getMinRSSI() const;
  int8_t getAvgRSSI() const;
  uint16_t getReadingCount() const { return readingCount; }

  // Clear all data
  void clearData();

  // Get summary string for display
  std::string getSummaryString() const;

private:
  SignalGrapher();

  static constexpr uint16_t MAX_READINGS = 256;  // Ring buffer

  struct WiFiRecord {
    char ssid[33];
    int8_t rssi;
    uint16_t channel;
    uint32_t lastSeen;
  };

  struct BLERecord {
    char address[18];  // MAC address
    int8_t rssi;
    uint32_t lastSeen;
  };

  DataPoint readings[MAX_READINGS];
  uint16_t readingCount = 0;
  uint16_t nextIndex = 0;

  std::vector<WiFiRecord> wifiDevices;
  std::vector<BLERecord> bleDevices;

  // Helper functions
  char getRSSIChar(int8_t rssi) const;
  uint8_t rssiToPercent(int8_t rssi) const;
  const char* getRSSILabel(int8_t rssi) const;
};

#endif // SIGNAL_GRAPHER_H
