#pragma once
#include <Arduino.h>

// LiPo 1S voltage via a resistor-divider into PIN_BATTERY_ADC. Adjust
// DIVIDER_RATIO in battery.cpp to match your actual resistor values.
namespace Battery {

enum class BatteryState {
    CRITICAL = 0,      // < 5% or < 3.1V
    WARNING = 1,       // < 15% or < 3.5V (renamed from LOW to avoid macro conflict)
    NORMAL = 2,        // >= 15%
};

void begin();
float voltage();        // volts, after divider correction
uint8_t percent();      // rough 0-100%, linear between EMPTY_V and FULL_V
BatteryState state();   // Current battery state (CRITICAL, LOW, or GOOD)
bool isLow();           // true if battery < 15%
bool isCritical();      // true if battery < 5%

} // namespace Battery
