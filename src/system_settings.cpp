#include "system_settings.h"
#include "debug_logger.h"
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_pm.h>
#include <driver/ledc.h>

SystemSettings::SystemSettings() {
  loadFromNVS();
  DebugLogger::println("[SystemSettings] Initialized");
}

const char* SystemSettings::getBatterySavingModeString() const {
  switch (batterySavingMode) {
    case NORMAL:                return "NORMAL";
    case POWER_SAVING:          return "POWER SAVING";
    case ULTRA_POWER_SAVING:    return "ULTRA POWER SAVING";
    default:                    return "UNKNOWN";
  }
}

const char* SystemSettings::getCPUFrequencyString() const {
  switch (cpuFrequency) {
    case FREQ_80MHZ:   return "80MHz (Low)";
    case FREQ_160MHZ:  return "160MHz (Normal)";
    case FREQ_240MHZ:  return "240MHz (High)";
    default:           return "UNKNOWN";
  }
}

uint32_t SystemSettings::getCPUFrequencyHz() const {
  switch (cpuFrequency) {
    case FREQ_80MHZ:   return 80000000;
    case FREQ_160MHZ:  return 160000000;
    case FREQ_240MHZ:  return 240000000;
    default:           return 160000000;
  }
}

void SystemSettings::loadFromNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("system_settings", NVS_READONLY, &handle);

  if (err != ESP_OK) {
    DebugLogger::println("[SystemSettings] NVS not found, using defaults");
    return;
  }

  uint8_t value;

  nvs_get_u8(handle, "brightness", &brightness);
  nvs_get_u8(handle, "volume", &volume);
  nvs_get_u16(handle, "screen_timeout", &screenTimeout);

  if (nvs_get_u8(handle, "color_inv", &value) == ESP_OK) {
    colorInversion = (value != 0);
  }

  nvs_get_u8(handle, "contrast", &contrast);

  if (nvs_get_u8(handle, "battery_mode", &value) == ESP_OK) {
    batterySavingMode = (BatterySavingMode)value;
  }

  if (nvs_get_u8(handle, "wifi_psave", &value) == ESP_OK) {
    wifiPowerSaving = (value != 0);
  }

  if (nvs_get_u8(handle, "sleep_mode", &value) == ESP_OK) {
    sleepMode = (value != 0);
  }

  nvs_get_u16(handle, "sleep_timeout", &sleepTimeout);

  if (nvs_get_u8(handle, "cpu_freq", &value) == ESP_OK) {
    cpuFrequency = (CPUFrequency)value;
  }

  if (nvs_get_u8(handle, "bluetooth", &value) == ESP_OK) {
    bluetoothEnabled = (value != 0);
  }

  if (nvs_get_u8(handle, "usb_charging", &value) == ESP_OK) {
    usbCharging = (value != 0);
  }

  if (nvs_get_u8(handle, "debug", &value) == ESP_OK) {
    debugMode = (value != 0);
  }

  nvs_close(handle);
  DebugLogger::println("[SystemSettings] Loaded from NVS");
}

void SystemSettings::saveToNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("system_settings", NVS_READWRITE, &handle);

  if (err != ESP_OK) {
    DebugLogger::println("[SystemSettings] Error opening NVS");
    return;
  }

  nvs_set_u8(handle, "brightness", brightness);
  nvs_set_u8(handle, "volume", volume);
  nvs_set_u16(handle, "screen_timeout", screenTimeout);
  nvs_set_u8(handle, "color_inv", colorInversion ? 1 : 0);
  nvs_set_u8(handle, "contrast", contrast);
  nvs_set_u8(handle, "battery_mode", (uint8_t)batterySavingMode);
  nvs_set_u8(handle, "wifi_psave", wifiPowerSaving ? 1 : 0);
  nvs_set_u8(handle, "sleep_mode", sleepMode ? 1 : 0);
  nvs_set_u16(handle, "sleep_timeout", sleepTimeout);
  nvs_set_u8(handle, "cpu_freq", (uint8_t)cpuFrequency);
  nvs_set_u8(handle, "bluetooth", bluetoothEnabled ? 1 : 0);
  nvs_set_u8(handle, "usb_charging", usbCharging ? 1 : 0);
  nvs_set_u8(handle, "debug", debugMode ? 1 : 0);

  nvs_commit(handle);
  nvs_close(handle);
  DebugLogger::println("[SystemSettings] Saved to NVS");
}

void SystemSettings::resetToDefaults() {
  brightness = 80;
  volume = 70;
  screenTimeout = 300;
  colorInversion = false;
  contrast = 100;
  batterySavingMode = NORMAL;
  wifiPowerSaving = false;
  sleepMode = false;
  sleepTimeout = 600;
  cpuFrequency = FREQ_160MHZ;
  bluetoothEnabled = true;
  usbCharging = true;
  debugMode = false;

  saveToNVS();
  DebugLogger::println("[SystemSettings] Reset to defaults");
}

void SystemSettings::printSettings() const {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║       SYSTEM SETTINGS REPORT           ║");
  Serial.println("╚════════════════════════════════════════╝");

  Serial.printf("🖥️  Affichage:\n");
  Serial.printf("   Luminosité:       %u%%\n", brightness);
  Serial.printf("   Contraste:        %u%%\n", contrast);
  Serial.printf("   Inversion couleur:%s\n", colorInversion ? " ON" : " OFF");
  Serial.printf("   Timeout écran:    %u s\n", screenTimeout);

  Serial.printf("\n🔊 Audio & Retour:\n");
  Serial.printf("   Volume:           %u%%\n", volume);

  Serial.printf("\n⚡ Gestion Énergie:\n");
  Serial.printf("   Mode batterie:    %s\n", getBatterySavingModeString());
  Serial.printf("   WiFi power save:  %s\n", wifiPowerSaving ? "ON" : "OFF");
  Serial.printf("   Fréquence CPU:    %s\n", getCPUFrequencyString());
  Serial.printf("   Mode veille:      %s (%u s)\n", sleepMode ? "ON" : "OFF", sleepTimeout);

  Serial.printf("\n🔗 Connectivité:\n");
  Serial.printf("   Bluetooth:        %s\n", bluetoothEnabled ? "ON" : "OFF");
  Serial.printf("   Chargement USB:   %s\n", usbCharging ? "ON" : "OFF");

  Serial.printf("\n🐛 Diagnostic:\n");
  Serial.printf("   Mode DEBUG:       %s\n", debugMode ? "ON" : "OFF");

  Serial.println("\n");
}
