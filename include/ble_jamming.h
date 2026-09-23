#pragma once
#include <Arduino.h>
namespace BLEJamming {
struct JamResult { bool success; uint32_t durationMs; uint8_t powerLevel; String error; };
JamResult startJamming(uint32_t durationMs = 30000, uint8_t powerLevel = 20);
void stopJamming();
bool isJamming();
} // namespace BLEJamming
