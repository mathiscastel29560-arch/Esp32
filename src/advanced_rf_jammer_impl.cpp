#include "advanced_rf_jammer.h"
#include "tx_arm.h"
#include "config.h"
#include <RadioLib.h>

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);
volatile bool g_jamActive = false;
uint32_t g_jamPacketsCount = 0;

void sendJamSignal(float freq) {
    radio.begin(freq);
    radio.setOOK(true);
    radio.transmitDirectAsync();

    for (int i = 0; i < 32; i++) {
        digitalWrite(PIN_CC1101_GDO0, i % 2);
        delayMicroseconds(50);
    }
    g_jamPacketsCount++;
}

void sweepFrequencies() {
    for (float f = 433.05f; f <= 434.79f; f += 0.05f) {
        sendJamSignal(f);
        delayMicroseconds(200);
    }
}
}

namespace AdvancedRFJammer {

JamResult jamRFSignals(const String &frequency, uint32_t durationMs, const String &method) {
    JamResult result{false, 0, durationMs, method, frequency};

    Serial.println("\n=== Advanced RF Jammer ===");
    Serial.println("Frequency: " + frequency);
    Serial.println("Method: " + method);
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    g_jamActive = true;
    g_jamPacketsCount = 0;
    uint32_t startTime = millis();

    Serial.println("Starting RF jamming...");

    while (millis() - startTime < durationMs && g_jamActive) {
        if (method == "NOISE") {
            sendJamSignal(433.92f);
            delay(10);
        } else if (method == "SWEEP") {
            sweepFrequencies();
        } else if (method == "FOLLOW") {
            for (float f = 433.05f; f <= 434.79f; f += 0.1f) {
                sendJamSignal(f);
            }
        }

        if (g_jamPacketsCount % 50 == 0) {
            Serial.println("  [" + String(g_jamPacketsCount) + "] packets");
        }
    }

    g_jamActive = false;
    result.success = true;
    result.jamPacketsCount = g_jamPacketsCount;

    Serial.println("✓ RF jamming complete: " + String(result.jamPacketsCount) + " packets");
    return result;
}

void stop() {
    g_jamActive = false;
    Serial.println("RF jammer stopped");
}

bool isActive() {
    return g_jamActive;
}

}  // namespace AdvancedRFJammer
