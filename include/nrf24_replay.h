#pragma once
#include <Arduino.h>
#include <vector>

namespace Nrf24Replay {

struct PacketCapture {
    uint8_t channel;
    std::vector<uint8_t> data;
    uint32_t timestamp;
    int8_t rssi;
};

struct ReplayResult {
    bool success;
    uint32_t replayCount;
    uint32_t packetsSent;
};

// Capture NRF24 packets
PacketCapture capturePacket(uint8_t channel, uint32_t timeoutMs = 3000);

// Replay captured packet
ReplayResult replayPacket(const PacketCapture &packet, uint8_t repeatCount = 5);

}  // namespace Nrf24Replay
