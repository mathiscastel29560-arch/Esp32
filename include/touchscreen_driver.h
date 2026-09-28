#pragma once
#include <Arduino.h>
#include "buttons.h"

namespace Touchscreen {

// Initialize XPT2046 touchscreen controller
// Must be called after Display::begin() and Buttons::begin()
void begin();

// Poll touch input and convert to button events
// Returns a button action (UP, DOWN, SELECT, BACK) or NONE
// Call every main loop iteration for responsive touch
Buttons::Button poll();

// Check if screen is currently being touched (raw level, no debounce)
// Returns true if finger is down
bool isTouched();

// Get raw calibrated touch coordinates (after being touched)
// X: 0-2047 (left to right), Y: 0-2047 (top to bottom)
void getRawCoordinates(uint16_t &x, uint16_t &y);

// Calibrate touchscreen — must be run once after boot
// Displays 4 crosshairs on screen for calibration
// User must tap each corner precisely
void calibrate();

// Load calibration data from persistent storage
bool loadCalibration();

// Save calibration data to persistent storage
bool saveCalibration();

// Get screen dimensions for touch mapping
uint16_t getScreenWidth();
uint16_t getScreenHeight();

} // namespace Touchscreen
