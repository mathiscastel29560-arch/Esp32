#pragma once
#include <Arduino.h>

namespace GPIODriver {

// Button events
enum ButtonEvent {
    BUTTON_NONE,
    BUTTON_PRESSED,
    BUTTON_RELEASED,
    BUTTON_LONG_PRESS
};

// Initialize all GPIO
bool init();

// ---- BUTTONS ----
ButtonEvent getButtonState(uint8_t btnPin);
bool isButtonPressed(uint8_t btnPin);

// ---- BUZZER ----
void buzzerBeep(uint32_t durationMs);
void buzzerTone(uint16_t frequency, uint32_t durationMs);
void buzzerPattern(const uint16_t* durations, uint8_t count);

// ---- IR ----
void irLedOn(uint16_t frequency = 38000);  // Send IR carrier
void irLedOff();

// ---- BATTERY ----
uint8_t getBatteryPercent();
float getBatteryVoltage();
bool isBatteryLow();

}  // namespace GPIODriver
