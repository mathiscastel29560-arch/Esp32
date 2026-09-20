#pragma once
#include <Arduino.h>

namespace SubGhz {

struct ScanResult {
    bool success;
    uint8_t devicesFound;
    uint8_t activeChannels;
    uint16_t durationMs;
    String error;
};

struct DemodResult {
    bool success;
    uint8_t signalsDetected;
    String modulationType;
    int16_t strongestRSSI;
    uint32_t bitrate;
    String error;
};

struct ZigbeeResult {
    bool success;
    uint8_t devicesFound;
    uint16_t panIds;
    uint8_t channelsUsed;
    uint16_t durationMs;
    String error;
};

// Scan for Sub-GHz RF signals (433/868 MHz)
ScanResult scanFrequencies(uint16_t timeoutMs = 30000);

// Detect and analyze FSK/OOK modulated signals
DemodResult analyzeModulation(uint16_t frequencyMHz = 433, uint16_t timeoutMs = 20000);

// Scan for Zigbee devices on 802.15.4 channels (2.4 GHz)
ZigbeeResult scanZigbee(uint16_t timeoutMs = 30000);

// ISO14443A RFID tag detection
ScanResult scanISO14443A(uint16_t timeoutMs = 15000);

void stop();

} // namespace SubGhz
