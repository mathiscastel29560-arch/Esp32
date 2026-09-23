#pragma once

#include <string>
#include <Arduino.h>

namespace RfidNfcEmulation {

struct EmulationConfig {
    uint32_t durationMs;
    String tagType;
};

struct EmulationResult {
    bool success;
    uint32_t readsDetected;
    uint32_t emulationTime;
    String logFile;
};

class RfidEmulator {
public:
    RfidEmulator();
    EmulationResult emulateNfcTag(const EmulationConfig& config);
    EmulationResult replayCapture(const uint8_t* data, uint32_t len);
    void stop();

private:
    bool isRunning_;
};

}  // namespace RfidNfcEmulation
