#pragma once
#include <Arduino.h>
#include <cstdint>

namespace TimeoutUtils {

// Safe timeout check that handles millis() wraparound (occurs every ~49 days)
// Usage: while (isWithinTimeout(startTime, timeoutMs)) { ... }
// This avoids the unsafe pattern: while (millis() - start < timeout)
inline bool isWithinTimeout(uint32_t startTime, uint32_t timeoutMs) {
    uint32_t deadline = startTime + timeoutMs;
    return (int32_t)(millis() - deadline) < 0;
}

// Check if a specific time has been reached (safe for wraparound)
// Usage: if (hasReached(targetTime)) { ... }
inline bool hasReached(uint32_t targetTime) {
    return (int32_t)(millis() - targetTime) >= 0;
}

// Get elapsed time since a start point (safe for wraparound)
inline uint32_t elapsedSince(uint32_t startTime) {
    return millis() - startTime;
}

}  // namespace TimeoutUtils
