#pragma once
#include <Arduino.h>

namespace AutoHandshakeCapture {

struct CaptureResult {
    bool success;
    uint32_t handshakesRecovered;
    uint32_t targetCount;
    uint32_t durationMs;
    String targetNetworks;
};

// Automatically capture WiFi handshakes and BLE pairing attempts
CaptureResult autoCaptureHandshakes(uint32_t durationMs = 60000);

// Trigger deauth and wait for reconnection handshakes
struct DeauthCaptureResult {
    bool success;
    uint32_t handshakesAfterDeauth;
    uint32_t durationMs;
};
DeauthCaptureResult deauthAndCapture(const char* targetSsid, uint32_t waitTime = 30000);

}  // namespace AutoHandshakeCapture
