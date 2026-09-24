#pragma once
#include <Arduino.h>

namespace BluetoothClassicAttacks {

struct ClassicDevice {
    String bdAddr;
    String deviceName;
    int8_t rssi;
    uint32_t classOfDevice;
    bool paired;
    uint32_t timestamp;
};

struct DiscoveryResult {
    bool success;
    uint32_t devicesFound;
    uint32_t pairedDevices;
    uint32_t durationMs;
};

// Scan for Bluetooth Classic devices
DiscoveryResult scanClassicDevices(uint32_t durationMs = 30000);

// Get discovered devices
const ClassicDevice* getDiscoveredClassicDevices(uint32_t& outCount);

// PIN cracking attack
struct PINCrackResult {
    bool success;
    String targetBDAddr;
    String crackedPIN;
    uint32_t attemptsNeeded;
    uint32_t durationMs;
};
PINCrackResult crackDevicePIN(const char* bdAddr, uint32_t durationMs = 60000);

// Bluejacking - send unsolicited messages
struct BluejackingResult {
    bool success;
    String targetBDAddr;
    String messagesSent;
    uint32_t contactsReached;
    uint32_t durationMs;
};
BluejackingResult bluejackDevice(const char* bdAddr, const char* message, uint32_t durationMs = 20000);

// Bluesnarfing - extract contacts/calendar
struct BluesnarfingResult {
    bool success;
    String targetBDAddr;
    uint32_t contactsExtracted;
    uint32_t calendarEntriesExtracted;
    uint32_t durationMs;
};
BluesnarfingResult snarfDeviceData(const char* bdAddr, uint32_t durationMs = 45000);

// Legacy device attack targeting
struct LegacyAttackResult {
    bool success;
    uint32_t vulnerableDevicesFound;
    uint32_t successfulConnectionsEstablished;
    uint32_t durationMs;
};
LegacyAttackResult attackLegacyDevices(uint32_t durationMs = 40000);

}  // namespace BluetoothClassicAttacks
