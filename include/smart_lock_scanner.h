#pragma once
#include <Arduino.h>
#include <vector>

namespace SmartLockScanner {

struct LockDetection {
    String manufacturer;
    String model;
    int8_t rssi;
    String protocol;
};

struct ScanResult {
    uint32_t locksFound;
    std::vector<LockDetection> detections;
};

// Scan for smart locks vulnerable to exploitation
ScanResult scanSmartLocks(uint32_t durationMs = 10000);

}  // namespace SmartLockScanner
