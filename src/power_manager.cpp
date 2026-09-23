#include "power_manager.h"
#include "audit_log.h"
#include <esp_pm.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <WiFi.h>

PowerManager::PowerManager() 
    : currentMode_(PowerMode::BALANCED), cpuFreq_(240), estimatedMA_(200.0f) {
    
    // Mode configurations [Performance, Balanced, Eco, Sleep]
    modes_[0] = {240, 180.0f, true, true, true};    // Performance: full speed
    modes_[1] = {160, 120.0f, true, true, false};   // Balanced: medium, no RF
    modes_[2] = {80, 80.0f, false, true, false};    // Eco: low CPU, BLE only
    modes_[3] = {10, 5.0f, false, false, false};    // Sleep: minimal
}

void PowerManager::begin() {
    // Configure dynamic frequency scaling
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 10,
        .light_sleep_enable = true,
    };
    esp_pm_configure(&pm_config);
    
    char details[96];
    snprintf(details, sizeof(details), "PowerManager initialized, mode=BALANCED");
    AuditLog::instance().log(AuditEventType::TOOL_START, "PowerManager", details);
}

void PowerManager::setPowerMode(PowerMode mode) {
    if (mode == currentMode_) return;
    
    currentMode_ = mode;
    ModeConfig &cfg = modes_[static_cast<int>(mode)];
    
    // Set CPU frequency
    throttleCPU(cfg.cpu_mhz);
    
    // Control radios
    enableWiFi(cfg.wifi_enabled);
    enableBLE(cfg.ble_enabled);
    enableRF(cfg.rf_enabled);
    
    estimatedMA_ = cfg.current_ma;
    
    const char* mode_str[] = {"PERFORMANCE", "BALANCED", "ECO", "SLEEP"};
    char details[96];
    snprintf(details, sizeof(details), "PowerMode=%s, cpu=%dMHz, current=%.0fmA", 
             mode_str[static_cast<int>(mode)], cfg.cpu_mhz, cfg.current_ma);
    AuditLog::instance().log(AuditEventType::CONFIG_CHANGED, "PowerManager", details);
}

void PowerManager::updateModeBattery() {
    uint8_t batt_pct = Battery::percent();
    
    if (Battery::isCritical()) {
        // < 5%: Emergency sleep
        setPowerMode(PowerMode::SLEEP);
    } else if (Battery::isLow()) {
        // 5-15%: Eco mode
        setPowerMode(PowerMode::ECO);
    } else if (batt_pct < 30) {
        // 15-30%: Balanced mode
        setPowerMode(PowerMode::BALANCED);
    } else {
        // 30%+: Performance mode
        setPowerMode(PowerMode::PERFORMANCE);
    }
}

void PowerManager::throttleCPU(uint32_t freq_mhz) {
    cpuFreq_ = freq_mhz;
    setCpuFrequencyMhz(freq_mhz);
}

void PowerManager::enableWiFi(bool enable) {
    if (enable) {
        WiFi.mode(WIFI_STA);
    } else {
        WiFi.mode(WIFI_OFF);
        esp_wifi_stop();
    }
}

void PowerManager::enableBLE(bool enable) {
    if (enable) {
        // BLE stays on (managed by NimBLE)
    } else {
        // Turn off BLE to save power
        esp_bt_controller_disable();
    }
}

void PowerManager::enableRF(bool enable) {
    // RF modules (CC1101, NRF24) managed via driver CS pins
    // enable/disable via GPIO control
}

void PowerManager::sleepDeep(uint32_t seconds) {
    char details[64];
    snprintf(details, sizeof(details), "Deep sleep for %d seconds", seconds);
    AuditLog::instance().log(AuditEventType::TOOL_STOP, "PowerManager", details);
    
    esp_sleep_enable_timer_wakeup(seconds * 1000000);
    esp_deep_sleep_start();
}

uint32_t PowerManager::getRemainingBudget() const {
    uint8_t batt_pct = Battery::percent();
    float batt_v = Battery::voltage();
    
    // Estimate: assume 2000mAh battery
    // At current consumption rate, how many minutes left?
    float capacity_mah = 2000.0f * (batt_pct / 100.0f);
    uint32_t minutes_remaining = (uint32_t)(capacity_mah / estimatedMA_ * 60);
    
    return minutes_remaining;
}
