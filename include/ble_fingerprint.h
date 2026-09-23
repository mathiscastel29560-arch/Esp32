#pragma once
#include <Arduino.h>
#include <vector>
namespace BLEFingerprint {
struct FingerprintResult { String deviceName; String macAddr; String manufacturer; int rssi; String deviceType; };
std::vector<FingerprintResult> scan(uint16_t durationMs = 10000);
String identifyDevice(const String &macAddr);
} // namespace BLEFingerprint
