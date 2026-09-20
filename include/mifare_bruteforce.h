#pragma once
#include <Arduino.h>
#include <vector>
namespace MifareBruteforce {
struct BruteResult { bool found; String keyFound; uint16_t attemptsCount; uint32_t durationMs; String error; };
BruteResult bruteForceKeys(uint8_t sector = 0, uint16_t timeoutMs = 60000);
std::vector<String> getCommonKeys();
} // namespace MifareBruteforce
