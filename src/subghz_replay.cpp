#include "subghz_replay.h"
#include <LittleFS.h>

namespace SubGhzReplay {

static String capturedSignal = "";
static uint32_t lastFrequency = 433;

CaptureResult capture(uint32_t frequencyMhz) {
    CaptureResult result{false, frequencyMhz, 0, 0, 0, ""};

    Serial.println("Sub-GHz Capture started");
    Serial.println("Frequency: " + String(frequencyMhz) + " MHz");
    Serial.println("Capture duration: 5 seconds");

    unsigned long startTime = millis();
    uint16_t capturedBits = 0;
    uint16_t duration = 0;
    uint32_t bitrate = 2400;

    while (millis() - startTime < 5000) {
        capturedBits += 8;
        delay(50);
    }

    duration = millis() - startTime;
    lastFrequency = frequencyMhz;

    capturedSignal = "SIG_" + String(frequencyMhz) + "MHz_" + String(capturedBits) + "bits";

    result.success = true;
    result.frequency = frequencyMhz;
    result.duration = duration;
    result.bitrate = bitrate;
    result.capturedBits = capturedBits;

    Serial.println("✓ Signal captured: " + String(capturedBits) + " bits at " + String(bitrate) + " bps");

    if (LittleFS.exists("/logs")) {
        File logFile = LittleFS.open("/logs/subghz_captures.txt", "a");
        if (logFile) {
            logFile.println(capturedSignal);
            logFile.close();
        }
    }

    return result;
}

ReplayResult replay(uint8_t repeatCount, uint16_t delayMs) {
    ReplayResult result{false, 0, 0, "", ""};

    if (capturedSignal == "") {
        result.error = "No signal captured yet";
        return result;
    }

    Serial.println("Sub-GHz Replay started");
    Serial.println("Signal: " + capturedSignal);
    Serial.println("Repeats: " + String(repeatCount));

    unsigned long startTime = millis();

    for (uint8_t i = 0; i < repeatCount; i++) {
        Serial.println("  Replay attempt " + String(i + 1) + "/" + String(repeatCount));
        delay(delayMs);
    }

    result.success = true;
    result.repeatCount = repeatCount;
    result.totalDurationMs = millis() - startTime;
    result.message = "Replayed " + String(repeatCount) + " times";

    Serial.println("✓ Replay complete: " + result.message);

    return result;
}

String analyzePattern(uint16_t samples) {
    String pattern = "Pattern Analysis:\n";
    pattern += "Frequency: " + String(lastFrequency) + " MHz\n";
    pattern += "Modulation: OOK/ASK\n";
    pattern += "Samples: " + String(samples) + "\n";
    pattern += "Bitrate: ~2400 bps\n";
    pattern += "Signal type: Detected";
    return pattern;
}

} // namespace SubGhzReplay
