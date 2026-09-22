#pragma once
#include <Arduino.h>
#include <vector>

namespace CANBusSniffer {

// CAN 2.0B frame structure (extended: 29-bit ID, standard: 11-bit)
// Standard CAN: 2.412 Mbps, ISO 11898
// Typical automotive: 500 kbps (OBD-II) or 250 kbps (luxury)

struct CANFrame {
    uint32_t id;              // CAN ID (11-bit or 29-bit)
    bool extended;            // True if 29-bit extended ID
    uint8_t dlc;              // Data Length Code (0-8 bytes)
    uint8_t data[8];          // Payload
    uint32_t timestamp;       // Capture timestamp
};

struct SnifferConfig {
    uint32_t durationMs;      // Capture duration
    uint32_t baudrate;        // CAN bus speed (250000, 500000, 1000000)
    uint8_t filterMode;       // 0=all, 1=engine, 2=brake, 3=transmission
    bool logToSD;             // Save to SD card
};

struct SnifferResult {
    bool success;
    uint32_t frameCount;
    uint32_t durationMs;
    std::vector<CANFrame> frames;
    std::vector<uint32_t> uniqueIDs;
    String dominantID;
    String error;
};

// Passive sniffer: Monitor CAN bus traffic
// Requires: MCP2515 or SPI CAN controller (GPIO: CS=5, CLK=18, MOSI=23, MISO=19)
// Captures all frames, analyzes patterns, detects ECU communication
SnifferResult sniffCANBus(const SnifferConfig& config);

// Analyze captured frames for pattern/anomalies
// Detects: engine parameters, brake commands, acceleration, steering, transmission
SnifferResult analyzeCANFrames(const std::vector<CANFrame>& frames);

// Identify ECU by CAN ID signature
// Maps: Engine (0x100), Brake (0x200), Transmission (0x300), etc.
String identifyECU(uint32_t canId);

// Extract readable values from CAN payload
// Decodes: RPM, speed, throttle position, brake pressure, temperatures
String decodeCANPayload(uint32_t canId, const uint8_t data[8]);

}  // namespace CANBusSniffer
