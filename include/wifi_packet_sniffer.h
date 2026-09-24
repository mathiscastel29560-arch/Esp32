#pragma once
#include <Arduino.h>

namespace WiFiPacketSniffer {

struct PacketInfo {
    String srcMac;
    String dstMac;
    String ssid;
    int rssi;
    uint8_t channel;
    String packetType;  // Beacon, Probe, Data, etc
    uint32_t timestamp;
};

struct SniffResult {
    bool success;
    uint32_t packetsCapture;
    uint32_t beaconsFound;
    uint32_t dataFrames;
    uint32_t durationMs;
};

SniffResult sniffPackets(uint32_t durationMs = 30000);
const PacketInfo* getCapturedPackets(uint32_t& outCount);
SniffResult analyzeChannelActivity(uint8_t channel, uint32_t durationMs = 10000);

}  // namespace WiFiPacketSniffer
