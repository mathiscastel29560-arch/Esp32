#pragma once

#include <string>
#include <vector>
#include <Arduino.h>

namespace NfcRelayAttack {

struct RelayConfig {
    uint32_t durationMs;
    bool capturePayload;
    bool modifyPayload;
    bool replayPayload;
};

struct RelayResult {
    bool success;
    uint32_t transactionsRelayed;
    uint32_t payloadsCaptured;
    String logFile;
};

class NfcRelay {
public:
    NfcRelay();
    RelayResult performRelay(const RelayConfig& config);
    void stop();

private:
    bool isRunning_;
};

}  // namespace NfcRelayAttack
