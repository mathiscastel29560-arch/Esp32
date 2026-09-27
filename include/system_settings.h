#pragma once

#include <stdint.h>
#include <string>

// ============= SYSTEM SETTINGS MANAGER =============
class SystemSettings {
public:
  static SystemSettings& getInstance() {
    static SystemSettings instance;
    return instance;
  }

  // Brightness Control (0-100%)
  uint8_t getBrightness() const { return brightness; }
  void setBrightness(uint8_t level) { brightness = level; }

  // Volume Control (0-100%)
  uint8_t getVolume() const { return volume; }
  void setVolume(uint8_t level) { volume = level; }

  // Battery Saving Modes
  enum BatterySavingMode {
    NORMAL = 0,         // Performance normal
    POWER_SAVING = 1,   // Réduit CPU/WiFi
    ULTRA_POWER_SAVING = 2  // Minimum vital
  };

  BatterySavingMode getBatterySavingMode() const { return batterySavingMode; }
  void setBatterySavingMode(BatterySavingMode mode) { batterySavingMode = mode; }
  const char* getBatterySavingModeString() const;

  // Screen Timeout (0 = jamais, autres en secondes)
  uint16_t getScreenTimeout() const { return screenTimeout; }
  void setScreenTimeout(uint16_t seconds) { screenTimeout = seconds; }

  // WiFi Power Saving
  bool isWiFiPowerSavingEnabled() const { return wifiPowerSaving; }
  void setWiFiPowerSaving(bool enable) { wifiPowerSaving = enable; }

  // CPU Frequency Scaling
  enum CPUFrequency {
    FREQ_80MHZ = 0,     // Basse consommation
    FREQ_160MHZ = 1,    // Équilibré
    FREQ_240MHZ = 2     // Haute performance
  };

  CPUFrequency getCPUFrequency() const { return cpuFrequency; }
  void setCPUFrequency(CPUFrequency freq) { cpuFrequency = freq; }
  const char* getCPUFrequencyString() const;
  uint32_t getCPUFrequencyHz() const;

  // Display Color Inversion
  bool isColorInversionEnabled() const { return colorInversion; }
  void setColorInversion(bool enable) { colorInversion = enable; }

  // Display Contrast (50-150%)
  uint8_t getContrast() const { return contrast; }
  void setContrast(uint8_t level) { contrast = level; }

  // Sleep/Standby Mode
  bool isSleepModeEnabled() const { return sleepMode; }
  void setSleepMode(bool enable) { sleepMode = enable; }

  uint16_t getSleepTimeout() const { return sleepTimeout; }
  void setSleepTimeout(uint16_t seconds) { sleepTimeout = seconds; }

  // USB Charging Detection
  bool isUSBChargingEnabled() const { return usbCharging; }
  void setUSBCharging(bool enable) { usbCharging = enable; }

  // Bluetooth Toggle
  bool isBluetoothEnabled() const { return bluetoothEnabled; }
  void setBluetooth(bool enable) { bluetoothEnabled = enable; }

  // DEBUG Mode
  bool isDebugEnabled() const { return debugMode; }
  void setDebugMode(bool enable) { debugMode = enable; }

  // Load & Save
  void loadFromNVS();
  void saveToNVS();
  void resetToDefaults();
  void printSettings() const;

private:
  SystemSettings();

  // Display
  uint8_t brightness = 80;
  uint8_t volume = 70;
  uint16_t screenTimeout = 300;  // 5 minutes
  bool colorInversion = false;
  uint8_t contrast = 100;

  // Power Management
  BatterySavingMode batterySavingMode = NORMAL;
  bool wifiPowerSaving = false;
  bool sleepMode = false;
  uint16_t sleepTimeout = 600;  // 10 minutes
  CPUFrequency cpuFrequency = FREQ_160MHZ;

  // Connectivity
  bool bluetoothEnabled = true;
  bool usbCharging = true;

  // Debug
  bool debugMode = false;
};

#endif // SYSTEM_SETTINGS_H
