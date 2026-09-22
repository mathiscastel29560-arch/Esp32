#pragma once

#include <Arduino.h>
#include <vector>

namespace ZigbeeSniffer {

// Zigbee frame types
enum ZigbeeFrameType {
    BEACON,          // Zigbee beacon
    DATA,            // Data frame
    ACK,             // Acknowledgement
    MAC_COMMAND,     // MAC command
};

// IEEE 802.15.4 frame (Zigbee uses this)
struct ZigbeeFrame {
    uint32_t timestamp;
    uint8_t frameType;
    uint16_t panId;
    uint16_t sourceAddr;
    uint16_t destAddr;
    uint8_t sequence;
    uint8_t length;
    uint8_t payload[128];
    int8_t rssi;
};

struct SnifferConfig {
    uint8_t channel;           // Zigbee channel (11-26, default 15 or 20)
    uint32_t durationMs;       // Sniffing duration
    bool capturePayloads;      // Save full payloads
    bool autoJoin;             // Try to join network
    bool attemptDecryption;    // Try default keys
};

struct SnifferResult {
    bool success;
    uint32_t framesCapured;
    uint32_t beaconsFound;
    uint32_t networkDevices;
    std::vector<ZigbeeFrame> frames;
    std::vector<String> discoveredNetworks;  // Network SSIDs/names
    String captureFile;
    String error;
};

// Sniff Zigbee network traffic
SnifferResult sniff(const SnifferConfig &config);

// Discover Zigbee networks (beacon scan)
std::vector<String> discoverNetworks(uint32_t durationMs);

// Try to join discovered network
bool joinNetwork(const String &networkName, const String &key);

// Get captured frames
std::vector<ZigbeeFrame> getCapturedFrames();

// Save capture to PCAP format
bool savePcap(const String &filename, const std::vector<ZigbeeFrame> &frames);

} // namespace ZigbeeSniffer
