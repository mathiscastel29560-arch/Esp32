#pragma once
#include <Arduino.h>
#include <vector>

namespace WiFiBruteforce {

struct BruteResult {
    bool passwordFound;
    String foundPassword;
    uint32_t attemptsCount;
    uint32_t durationMs;
    String targetSSID;
    String error;
};

// Brute-force WPA2 password against target SSID
// Uses common wordlist (top 1000 passwords)
BruteResult bruteForce(const String &targetSSID, uint16_t timeoutMs = 60000);

// Get common password wordlist
std::vector<String> getCommonPasswordList();

} // namespace WiFiBruteforce
