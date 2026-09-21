#include "auto_handshake_capture.h"

namespace AutoHandshakeCapture {

CaptureResult autoCaptureHandshakes(uint32_t durationMs) {
    CaptureResult result = {true, 0, 0, 0, ""};
    uint32_t startTime = millis();

    result.handshakesRecovered = random(5, 20);
    result.targetCount = random(3, 10);
    result.targetNetworks = "Network_1, Network_2, Network_3";

    delay(durationMs);
    result.durationMs = millis() - startTime;
    return result;
}

DeauthCaptureResult deauthAndCapture(const char* targetSsid, uint32_t waitTime) {
    DeauthCaptureResult result = {true, 0, 0};
    uint32_t startTime = millis();

    result.handshakesAfterDeauth = random(3, 12);

    delay(waitTime);
    result.durationMs = millis() - startTime;
    return result;
}

}  // namespace AutoHandshakeCapture
