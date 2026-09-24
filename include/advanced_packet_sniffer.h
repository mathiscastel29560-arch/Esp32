#pragma once
#include <Arduino.h>
#include <vector>

namespace AdvancedPacketSniffer {

struct SniffResult {
    bool success;
    uint32_t totalPackets;
    uint32_t mgmtPackets;
    uint32_t dataPackets;
    String captureFile;
};

struct PacketStats {
    uint32_t totalPackets;
    uint32_t avgPacketSize;
    int8_t avgRssi;
    uint32_t uniqueDevices;
};

// WiFi packet sniffing
SniffResult startWiFiSniff(uint32_t durationMs, uint8_t channel = 6);
bool exportToPCAP(const char* filename);
uint32_t getPacketCount();
PacketStats analyzePackets();
void stop();

} // namespace AdvancedPacketSniffer
