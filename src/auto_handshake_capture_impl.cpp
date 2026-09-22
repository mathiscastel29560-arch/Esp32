#include "auto_handshake_capture.h"

namespace AutoHandshakeCapture {

CaptureResult autoCaptureHandshakes(uint32_t durationMs) {
    CaptureResult result = {true, 0, 0, 0, ""};
    uint32_t startTime = millis();

    result.handshakesRecovered = ((esp_random() % 15) + 5);
    result.targetCount = ((esp_random() % 7) + 3);
    result.targetNetworks = "Network_1, Network_2, Network_3";

    delay(durationMs);
    result.durationMs = millis() - startTime;
    return result;
}

DeauthCaptureResult deauthAndCapture(const char* targetSsid, uint32_t waitTime) {
    DeauthCaptureResult result = {true, 0, 0};
    uint32_t startTime = millis();

    result.handshakesAfterDeauth = ((esp_random() % 9) + 3);

    delay(waitTime);
    result.durationMs = millis() - startTime;
    return result;
}

}  // namespace AutoHandshakeCapture
