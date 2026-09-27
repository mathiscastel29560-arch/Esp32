#include "audit_parameters.h"
#include "debug_logger.h"
#include <nvs.h>
#include <nvs_flash.h>

AuditParameters::AuditParameters() {
  loadFromNVS();
  DebugLogger::println("[AuditParams] Initialized");
}

const char* AuditParameters::getScanModeString() const {
  switch (scanMode) {
    case FAST:      return "FAST (5s)";
    case NORMAL:    return "NORMAL (15s)";
    case THOROUGH:  return "THOROUGH (30s)";
    default:        return "UNKNOWN";
  }
}

void AuditParameters::loadFromNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("audit_params", NVS_READONLY, &handle);

  if (err != ESP_OK) {
    DebugLogger::println("[AuditParams] NVS not found, using defaults");
    return;
  }

  nvs_get_u32(handle, "wifi_timeout", &wifiScanTimeout);
  nvs_get_u32(handle, "ble_timeout", &bleScanTimeout);
  nvs_get_u32(handle, "rf_timeout", &rfScanTimeout);
  nvs_get_u32(handle, "default_timeout", &defaultAttackTimeout);

  nvs_get_u16(handle, "max_results", &maxResults);
  nvs_get_u16(handle, "max_wifi", &maxWiFiNetworks);
  nvs_get_u16(handle, "max_ble", &maxBLEDevices);

  int8_t rssi;
  if (nvs_get_i8(handle, "rssi_threshold", &rssi) == ESP_OK) {
    rssiThreshold = rssi;
  }

  uint8_t mode;
  if (nvs_get_u8(handle, "scan_mode", &mode) == ESP_OK) {
    scanMode = (ScanMode)mode;
  }

  uint8_t logging;
  if (nvs_get_u8(handle, "logging", &logging) == ESP_OK) {
    enableLogging = (logging != 0);
  }

  uint8_t autoExport;
  if (nvs_get_u8(handle, "auto_export", &autoExport) == ESP_OK) {
    enableAutoExport = (autoExport != 0);
  }

  uint8_t html;
  if (nvs_get_u8(handle, "export_html", &html) == ESP_OK) {
    exportHTML = (html != 0);
  }

  uint8_t json;
  if (nvs_get_u8(handle, "export_json", &json) == ESP_OK) {
    exportJSON = (json != 0);
  }

  uint8_t csv;
  if (nvs_get_u8(handle, "export_csv", &csv) == ESP_OK) {
    exportCSV = (csv != 0);
  }

  uint8_t perf;
  if (nvs_get_u8(handle, "perf_monitor", &perf) == ESP_OK) {
    enablePerformanceMonitoring = (perf != 0);
  }

  uint8_t dupDetect;
  if (nvs_get_u8(handle, "dup_detect", &dupDetect) == ESP_OK) {
    enableDuplicateDetection = (dupDetect != 0);
  }

  uint8_t hopEnabled;
  if (nvs_get_u8(handle, "hop_enabled", &hopEnabled) == ESP_OK) {
    enableFrequencyHopping = (hopEnabled != 0);
  }

  nvs_get_u16(handle, "hop_interval", &hopInterval);

  uint8_t stealth;
  if (nvs_get_u8(handle, "stealth", &stealth) == ESP_OK) {
    stealthMode = (stealth != 0);
  }

  nvs_get_u16(handle, "stealth_delay", &stealthDelay);
  nvs_get_u8(handle, "max_concurrent", &maxConcurrentAttacks);

  nvs_close(handle);
  DebugLogger::println("[AuditParams] Loaded from NVS");
}

void AuditParameters::saveToNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("audit_params", NVS_READWRITE, &handle);

  if (err != ESP_OK) {
    DebugLogger::println("[AuditParams] Error opening NVS");
    return;
  }

  nvs_set_u32(handle, "wifi_timeout", wifiScanTimeout);
  nvs_set_u32(handle, "ble_timeout", bleScanTimeout);
  nvs_set_u32(handle, "rf_timeout", rfScanTimeout);
  nvs_set_u32(handle, "default_timeout", defaultAttackTimeout);

  nvs_set_u16(handle, "max_results", maxResults);
  nvs_set_u16(handle, "max_wifi", maxWiFiNetworks);
  nvs_set_u16(handle, "max_ble", maxBLEDevices);

  nvs_set_i8(handle, "rssi_threshold", rssiThreshold);
  nvs_set_u8(handle, "scan_mode", (uint8_t)scanMode);
  nvs_set_u8(handle, "logging", enableLogging ? 1 : 0);
  nvs_set_u8(handle, "auto_export", enableAutoExport ? 1 : 0);
  nvs_set_u8(handle, "export_html", exportHTML ? 1 : 0);
  nvs_set_u8(handle, "export_json", exportJSON ? 1 : 0);
  nvs_set_u8(handle, "export_csv", exportCSV ? 1 : 0);
  nvs_set_u8(handle, "perf_monitor", enablePerformanceMonitoring ? 1 : 0);
  nvs_set_u8(handle, "dup_detect", enableDuplicateDetection ? 1 : 0);
  nvs_set_u8(handle, "hop_enabled", enableFrequencyHopping ? 1 : 0);
  nvs_set_u16(handle, "hop_interval", hopInterval);
  nvs_set_u8(handle, "stealth", stealthMode ? 1 : 0);
  nvs_set_u16(handle, "stealth_delay", stealthDelay);
  nvs_set_u8(handle, "max_concurrent", maxConcurrentAttacks);

  nvs_commit(handle);
  nvs_close(handle);
  DebugLogger::println("[AuditParams] Saved to NVS");
}

void AuditParameters::resetToDefaults() {
  wifiScanTimeout = 15000;
  bleScanTimeout = 15000;
  rfScanTimeout = 20000;
  defaultAttackTimeout = 30000;

  maxResults = 100;
  maxWiFiNetworks = 50;
  maxBLEDevices = 50;

  rssiThreshold = -100;
  signalStrengthMin = 0;

  scanMode = NORMAL;

  enableLogging = true;
  enableAutoExport = false;

  exportHTML = true;
  exportJSON = true;
  exportCSV = false;

  enablePerformanceMonitoring = false;
  enableDuplicateDetection = true;
  enableFrequencyHopping = false;
  hopInterval = 500;

  stealthMode = false;
  stealthDelay = 100;

  maxConcurrentAttacks = 2;

  saveToNVS();
  DebugLogger::println("[AuditParams] Reset to defaults");
}

void AuditParameters::printParameters() const {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║       AUDIT PARAMETERS REPORT          ║");
  Serial.println("╚════════════════════════════════════════╝");

  Serial.printf("⏱️  WiFi Scan Timeout:      %u ms\n", wifiScanTimeout);
  Serial.printf("⏱️  BLE Scan Timeout:       %u ms\n", bleScanTimeout);
  Serial.printf("⏱️  RF Scan Timeout:        %u ms\n", rfScanTimeout);
  Serial.printf("⏱️  Default Attack Timeout: %u ms\n", defaultAttackTimeout);

  Serial.printf("\n📊 Result Limits:\n");
  Serial.printf("   Max Results:    %u\n", maxResults);
  Serial.printf("   Max WiFi:       %u networks\n", maxWiFiNetworks);
  Serial.printf("   Max BLE:        %u devices\n", maxBLEDevices);

  Serial.printf("\n📶 Signal Filtering:\n");
  Serial.printf("   RSSI Threshold: %d dBm\n", rssiThreshold);
  Serial.printf("   Strength Min:   %u\n", signalStrengthMin);

  Serial.printf("\n🔄 Scan Mode: %s\n", getScanModeString());

  Serial.printf("\n📝 Logging & Export:\n");
  Serial.printf("   Logging:       %s\n", enableLogging ? "ON" : "OFF");
  Serial.printf("   Auto-Export:   %s\n", enableAutoExport ? "ON" : "OFF");
  Serial.printf("   Export HTML:   %s\n", exportHTML ? "ON" : "OFF");
  Serial.printf("   Export JSON:   %s\n", exportJSON ? "ON" : "OFF");
  Serial.printf("   Export CSV:    %s\n", exportCSV ? "ON" : "OFF");

  Serial.printf("\n⚙️  Advanced:\n");
  Serial.printf("   Perf Monitor:  %s\n", enablePerformanceMonitoring ? "ON" : "OFF");
  Serial.printf("   Dup Detection: %s\n", enableDuplicateDetection ? "ON" : "OFF");
  Serial.printf("   Freq Hopping:  %s (%u ms)\n", enableFrequencyHopping ? "ON" : "OFF", hopInterval);
  Serial.printf("   Stealth Mode:  %s (%u ms delay)\n", stealthMode ? "ON" : "OFF", stealthDelay);
  Serial.printf("   Max Concurrent: %u attacks\n", maxConcurrentAttacks);

  Serial.println("\n");
}
