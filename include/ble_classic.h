#pragma once

#include <Arduino.h>
#include <string>
#include <vector>

namespace BleClassic {

enum AttackType {
    PIN_BRUTEFORCE,      // 0000-9999 brute force
    FUZZING,             // Malformed packets
    SERVICE_ENUMERATION, // Discover all services
};

struct ClassicConfig {
    AttackType type;
    uint8_t targetAddr[6];   // Target Bluetooth address
    uint32_t timeoutMs;      // Attack duration
    uint16_t startPin;       // For bruteforce: start PIN
    uint16_t endPin;         // For bruteforce: end PIN
    bool aggressive;         // Parallel connection attempts
};

struct PairedDevice {
    uint8_t addr[6];
    String name;
    int8_t rssi;
    uint16_t classOfDevice;
};

struct AttackResult {
    bool success;
    uint32_t attemptsCompleted;
    String targetName;
    uint16_t validPin;       // Found PIN (if bruteforce succeeded)
    std::vector<PairedDevice> discoveredDevices;
    String error;
};

// Scan for BR/EDR devices in range
std::vector<PairedDevice> scanClassic(uint32_t durationMs);

// Brute force PIN on target device (0000-9999)
AttackResult bruteforcePin(const ClassicConfig &config);

// Fuzz target with malformed packets
AttackResult fuzz(const ClassicConfig &config);

// Enumerate services on connected device
std::vector<String> enumerateServices(const uint8_t *addr);

} // namespace BleClassic
