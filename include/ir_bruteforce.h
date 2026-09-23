#pragma once
#include <Arduino.h>
#include <vector>
namespace IRBruteforce {
struct BruteResult { bool success; String codeFound; uint32_t attemptsCount; String deviceType; };
BruteResult bruteForce(const String &deviceType = "TV", uint16_t timeoutMs = 30000);
std::vector<String> getCommonCodes(const String &deviceType);
} // namespace IRBruteforce
