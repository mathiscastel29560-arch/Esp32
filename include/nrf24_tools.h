#pragma once
#include <Arduino.h>

namespace Nrf24Tools {

struct ScanResult {
    bool success;
    uint8_t devicesFound;
    uint32_t packetsIntercepted;
    uint16_t durationMs;
    String error;
};

struct ChannelScan {
    bool success;
    uint8_t channelCount;
    uint8_t strongestChannel;
    int16_t strongestRSSI;
    String analysis;
    String error;
};

// Scan all 2.4 GHz channels for NRF24 devices
ScanResult scanDevices(uint16_t timeoutMs = 30000);

// Perform channel hopping and activity detection
ChannelScan performChannelHop(uint16_t timeoutMs = 60000);

// Start packet sniffer
ScanResult startPacketSniffer(uint16_t timeoutMs = 30000);

void stop();

} // namespace Nrf24Tools
