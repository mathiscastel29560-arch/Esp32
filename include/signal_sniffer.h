#pragma once
#include <Arduino.h>
#include <vector>

// Signal Sniffer: Capture 2.4GHz NRF24 packets and export to PCAP format
// Compatible with Wireshark for analysis
namespace SignalSniffer {

struct RawPacket {
    uint32_t timestamp;      // millis()
    uint8_t channel;         // 0-125
    int16_t rssi;            // Signal strength
    std::vector<uint8_t> data;  // Raw packet bytes
};

struct SniffResult {
    uint32_t packetsCapture;
    uint32_t captureDurationMs;
    std::vector<RawPacket> packets;
};

// Sniff 2.4GHz NRF24 traffic
// Channels: specify which channel(s) to sniff (0-125)
// Format: 0 = single channel, 255 = hop all channels
SniffResult sniffTraffic(uint8_t channel = 0, uint32_t durationMs = 10000);

// Export captured packets to PCAP file (Wireshark-compatible)
// Format: Global header + packet headers + raw data
bool exportToPcap(const String &filename, const SniffResult &result);

// Export to CSV for analysis (simpler format)
bool exportToCsv(const String &filename, const SniffResult &result);

// Analyze captured packets for patterns
// Returns: protocol guesses, frequency patterns, timing analysis
void analyzePackets(const SniffResult &result);

// Frequency hop detection
// Identifies if traffic is using frequency hopping (characteristic of drones)
bool detectFrequencyHopping(const SniffResult &result);

// Protocol identifier
// Attempts to identify NRF24 protocol variant (ShockBurst, Enhanced ShockBurst, etc)
String identifyProtocol(const SniffResult &result);

}  // namespace SignalSniffer
