#pragma once
#include <Arduino.h>
namespace WiFiKRACK {
struct KrackResult { bool success; String apBssid; uint8_t channel; String status; String error; };
KrackResult simulateKRACKattack(const String &bssid, uint8_t channel, uint16_t durationMs = 30000);
String analyzeHandshakes();
} // namespace WiFiKRACK
