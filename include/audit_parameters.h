#pragma once

#include <stdint.h>
#include <string>

// ============= AUDIT PARAMETERS MANAGER =============
class AuditParameters {
public:
  static AuditParameters& getInstance() {
    static AuditParameters instance;
    return instance;
  }

  // Attack Timeouts (ms)
  uint32_t getWiFiScanTimeout() const { return wifiScanTimeout; }
  void setWiFiScanTimeout(uint32_t ms) { wifiScanTimeout = ms; }

  uint32_t getBLEScanTimeout() const { return bleScanTimeout; }
  void setBLEScanTimeout(uint32_t ms) { bleScanTimeout = ms; }

  uint32_t getRFScanTimeout() const { return rfScanTimeout; }
  void setRFScanTimeout(uint32_t ms) { rfScanTimeout = ms; }

  uint32_t getDefaultAttackTimeout() const { return defaultAttackTimeout; }
  void setDefaultAttackTimeout(uint32_t ms) { defaultAttackTimeout = ms; }

  // Result Limits
  uint16_t getMaxResults() const { return maxResults; }
  void setMaxResults(uint16_t count) { maxResults = count; }

  uint16_t getMaxWiFiNetworks() const { return maxWiFiNetworks; }
  void setMaxWiFiNetworks(uint16_t count) { maxWiFiNetworks = count; }

  uint16_t getMaxBLEDevices() const { return maxBLEDevices; }
  void setMaxBLEDevices(uint16_t count) { maxBLEDevices = count; }

  // Signal Filtering
  int8_t getRSSIThreshold() const { return rssiThreshold; }
  void setRSSIThreshold(int8_t rssi) { rssiThreshold = rssi; }

  uint8_t getSignalStrengthMin() const { return signalStrengthMin; }
  void setSignalStrengthMin(uint8_t strength) { signalStrengthMin = strength; }

  // Scan Modes
  enum ScanMode {
    FAST = 0,      // 5 seconds
    NORMAL = 1,    // 15 seconds
    THOROUGH = 2   // 30 seconds
  };

  ScanMode getScanMode() const { return scanMode; }
  void setScanMode(ScanMode mode) { scanMode = mode; }
  const char* getScanModeString() const;

  // Logging & Export
  bool isLoggingEnabled() const { return enableLogging; }
  void setLoggingEnabled(bool enable) { enableLogging = enable; }

  bool isAutoExportEnabled() const { return enableAutoExport; }
  void setAutoExportEnabled(bool enable) { enableAutoExport = enable; }

  // Export Formats
  bool getExportHTML() const { return exportHTML; }
  void setExportHTML(bool enable) { exportHTML = enable; }

  bool getExportJSON() const { return exportJSON; }
  void setExportJSON(bool enable) { exportJSON = enable; }

  bool getExportCSV() const { return exportCSV; }
  void setExportCSV(bool enable) { exportCSV = enable; }

  // Performance Monitoring
  bool isPerformanceMonitoring() const { return enablePerformanceMonitoring; }
  void setPerformanceMonitoring(bool enable) { enablePerformanceMonitoring = enable; }

  // Duplicate Detection
  bool isDuplicateDetection() const { return enableDuplicateDetection; }
  void setDuplicateDetection(bool enable) { enableDuplicateDetection = enable; }

  // Frequency Hopping (RF)
  bool isFrequencyHoppingEnabled() const { return enableFrequencyHopping; }
  void setFrequencyHopping(bool enable) { enableFrequencyHopping = enable; }

  uint16_t getHopInterval() const { return hopInterval; }
  void setHopInterval(uint16_t ms) { hopInterval = ms; }

  // Stealth Mode
  bool isStealthMode() const { return stealthMode; }
  void setStealthMode(bool enable) { stealthMode = enable; }

  uint16_t getStealthDelay() const { return stealthDelay; }
  void setStealthDelay(uint16_t ms) { stealthDelay = ms; }

  // Concurrent Attacks
  uint8_t getMaxConcurrentAttacks() const { return maxConcurrentAttacks; }
  void setMaxConcurrentAttacks(uint8_t count) { maxConcurrentAttacks = count; }

  // Load & Save
  void loadFromNVS();
  void saveToNVS();
  void resetToDefaults();
  void printParameters() const;

private:
  AuditParameters();

  // Attack Timeouts (ms)
  uint32_t wifiScanTimeout = 15000;
  uint32_t bleScanTimeout = 15000;
  uint32_t rfScanTimeout = 20000;
  uint32_t defaultAttackTimeout = 30000;

  // Result Limits
  uint16_t maxResults = 100;
  uint16_t maxWiFiNetworks = 50;
  uint16_t maxBLEDevices = 50;

  // Signal Filtering
  int8_t rssiThreshold = -100;  // dBm
  uint8_t signalStrengthMin = 0;

  // Scan Modes
  ScanMode scanMode = NORMAL;

  // Logging & Export
  bool enableLogging = true;
  bool enableAutoExport = false;

  // Export Formats
  bool exportHTML = true;
  bool exportJSON = true;
  bool exportCSV = false;

  // Performance Monitoring
  bool enablePerformanceMonitoring = false;

  // Duplicate Detection
  bool enableDuplicateDetection = true;

  // Frequency Hopping
  bool enableFrequencyHopping = false;
  uint16_t hopInterval = 500;

  // Stealth Mode
  bool stealthMode = false;
  uint16_t stealthDelay = 100;

  // Concurrent Attacks
  uint8_t maxConcurrentAttacks = 2;
};

#endif // AUDIT_PARAMETERS_H
