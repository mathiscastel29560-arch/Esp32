#pragma once
#include <Arduino.h>
#include <vector>

// Sub-GHz (433/868MHz) bruteforce for common wireless devices
// Supports: Door locks, garage openers, car locks, smart home devices
namespace SubghzBruteforce {

struct BruteResult {
    bool success;
    String codeFound;
    uint32_t attemptsCount;
    String deviceType;
    String frequency;  // e.g., "433 MHz"
    String modulation; // e.g., "OOK"
};

// Bruteforce common Sub-GHz commands for target device type
// Supported: "LOCK", "GARAGE", "ALARM", "GENERIC"
// Returns attempt count and success indicator
BruteResult bruteForce(const String &deviceType = "GENERIC", uint16_t timeoutMs = 30000);

// Get common Sub-GHz codes for a device type
std::vector<uint32_t> getCommonCodes(const String &deviceType);

} // namespace SubghzBruteforce
