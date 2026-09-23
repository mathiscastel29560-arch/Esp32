#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "battery.h"

enum class PowerMode {
    PERFORMANCE = 0,  // Full speed, all radios active
    BALANCED = 1,     // Medium CPU, selective radio
    ECO = 2,          // Low CPU, only essentials
    SLEEP = 3,        // Deep sleep, minimal power
};

class PowerManager {
public:
    static PowerManager& instance() {
        static PowerManager pm;
        return pm;
    }

    void begin();
    
    // Set power mode manually or auto-select based on battery
    void setPowerMode(PowerMode mode);
    PowerMode getPowerMode() const { return currentMode_; }
    
    // Auto-select mode based on battery percentage
    void updateModeBattery();
    
    // Sleep for duration, waking on button or timeout
    void sleepDeep(uint32_t seconds);
    
    // Throttle CPU frequency based on mode
    void throttleCPU(uint32_t freq_mhz);
    
    // Control radio power
    void enableWiFi(bool enable);
    void enableBLE(bool enable);
    void enableRF(bool enable);
    
    // Get current power stats
    uint32_t getCurrentFreq() const { return cpuFreq_; }
    float getCurrentConsumption() const { return estimatedMA_; }
    
    // Power budget for operation (in mA-minutes)
    uint32_t getRemainingBudget() const;

private:
    PowerManager();
    
    PowerMode currentMode_;
    uint32_t cpuFreq_;
    float estimatedMA_;
    
    // Mode configurations (mA consumption estimates)
    struct ModeConfig {
        uint32_t cpu_mhz;
        float current_ma;
        bool wifi_enabled;
        bool ble_enabled;
        bool rf_enabled;
    } modes_[4];
};

#endif
