#pragma once
#include <Arduino.h>

// LiPo 1S voltage via a resistor-divider into PIN_BATTERY_ADC. Adjust
// DIVIDER_RATIO in battery.cpp to match your actual resistor values.
namespace Battery {

void begin();
float voltage();  // volts, after divider correction
uint8_t percent(); // rough 0-100%, linear between EMPTY_V and FULL_V

} // namespace Battery
