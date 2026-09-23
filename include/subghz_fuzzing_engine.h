#pragma once

#include <string>
#include <vector>
#include <Arduino.h>

namespace SubGhzFuzzing {

struct FuzzConfig {
    uint32_t baseFreq;
    uint32_t durationMs;
    bool randomizeFreq;
    uint32_t modulation;
    bool varyModulation;
};

struct FuzzResult {
    bool success;
    uint32_t packetsSent;
    uint32_t mutationsApplied;
    uint32_t elapsedMs;
    String logFile;
};

class FuzzingEngine {
public:
    FuzzingEngine();
    FuzzResult fuzzSubGhz(const FuzzConfig& config);
    void generateFuzzVector(std::vector<uint8_t>& payload);
    void stop();

private:
    bool isRunning_;
    uint32_t startTime_;
};

}  // namespace SubGhzFuzzing
