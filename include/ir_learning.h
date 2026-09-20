#pragma once
#include <Arduino.h>

namespace IRLearning {
struct LearnResult { bool success; String irCode; uint16_t pulseCount; String error; };
struct ReplayResult { bool success; uint8_t repeats; uint32_t durationMs; };
LearnResult learn(uint16_t timeoutMs = 10000);  // Record IR command
ReplayResult replay(const String &irCode, uint8_t repeats = 5);
String listLearned();
} // namespace IRLearning
