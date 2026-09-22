#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace SubGhzRolling {

enum DeviceType {
    GARAGE_DOOR,     // Typical garage door opener (Sommer, Nice, etc)
    CAR_KEY_FOB,     // Car key fob (VW, Audi, Ford)
    GATE_REMOTE,     // Electric gate
    ALARM_REMOTE,    // Alarm remote control
};

struct RollingConfig {
    DeviceType deviceType;
    uint32_t startCounter;       // Initial counter value
    uint32_t endCounter;         // Final counter value
    uint32_t increment;          // Counter increment per transmission (usually 1 or 2)
    uint32_t frequency;          // Frequency in Hz (e.g., 433920000)
    uint32_t durationMs;         // Attack duration
    uint16_t transmitPower;      // TX power
    bool bruteforceCounter;      // Try all counters in range
    bool randomizeManufacturer;  // Randomize manufacturer ID
};

struct RollingResult {
    bool success;
    uint32_t codesGenerated;
    uint32_t transmissionsCompleted;
    uint32_t elapsedMs;
    uint32_t lastCounterSent;
    String error;
};

// Emulate rolling code garage door opener
RollingResult emulateGarageDoor(const RollingConfig &config);

// Emulate car key fob (more complex algorithm)
RollingResult emulateCarKeyfob(const RollingConfig &config);

// Generic rolling code with configurable algorithm
RollingResult emulateRollingCode(const RollingConfig &config);

// Brute force counter on captured rolling code
RollingResult bruteforceCounter(const RollingConfig &config);

} // namespace SubGhzRolling
