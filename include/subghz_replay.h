#pragma once
#include <Arduino.h>
#include <vector>

namespace SubghzReplay {

struct SignalCapture {
    float frequency;
    std::vector<uint16_t> pulsesUs;  // Timing of pulses in microseconds
    uint32_t captureTimeMs;
    uint32_t timestamp;
};

struct ReplayResult {
    bool success;
    uint32_t pulsesCount;
    uint32_t replayCount;
    float frequency;
};

// Record Sub-GHz signal for specified duration
SignalCapture recordSignal(float freqMHz = 433.92f, uint32_t durationMs = 5000);

// Replay previously recorded signal
ReplayResult replaySignal(const SignalCapture &capture, uint8_t repeatCount = 1);

// Get current RSSI at frequency
int8_t getRSSI(float freqMHz = 433.92f);

}  // namespace SubghzReplay
