#pragma once
#include <Arduino.h>

namespace SubghzJammerSuite {

struct JamResult {
    bool success;
    uint32_t jamPacketsCount;
    uint32_t durationMs;
};

JamResult jamSubghzDevices(uint32_t durationMs = 10000);
void stop();
bool isActive();

}  // namespace SubghzJammerSuite
