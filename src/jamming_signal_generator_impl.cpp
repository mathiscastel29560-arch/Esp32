#include "jamming_signal_generator.h"
#include "tx_arm.h"
#include "config.h"
#include <RadioLib.h>

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);
volatile bool g_genActive = false;
uint32_t g_signalsCount = 0;

void generateWhiteNoise() {
    radio.begin(433.92f);
    radio.setOOK(true);
    radio.transmitDirectAsync();

    for (int i = 0; i < 256; i++) {
        bool bit = (esp_random() % 2);
        digitalWrite(PIN_CC1101_GDO0, bit);
        delayMicroseconds(100);
    }
    g_signalsCount++;
}

void generatePinkNoise() {
    // Simplified pink noise (1/f)
    radio.begin(433.92f);
    radio.setOOK(true);
    radio.transmitDirectAsync();

    for (int i = 0; i < 512; i++) {
        int level = (i * i) % 256;
        digitalWrite(PIN_CC1101_GDO0, level > 128 ? 1 : 0);
        delayMicroseconds(50);
    }
    g_signalsCount++;
}

void generateSweep() {
    for (float f = 433.05f; f <= 434.79f; f += 0.01f) {
        radio.begin(f);
        radio.setOOK(true);
        radio.transmitDirectAsync();

        for (int i = 0; i < 100; i++) {
            digitalWrite(PIN_CC1101_GDO0, 1);
            delayMicroseconds(50);
            digitalWrite(PIN_CC1101_GDO0, 0);
            delayMicroseconds(50);
        }
    }
    g_signalsCount++;
}
}

namespace JammingSignalGenerator {

JamResult generateJammingSignal(uint32_t durationMs, const String &noiseType) {
    JamResult result{false, 0, durationMs, noiseType};

    Serial.println("\n=== Jamming Signal Generator ===");
    Serial.println("Noise type: " + noiseType);
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }

    g_genActive = true;
    g_signalsCount = 0;
    uint32_t startTime = millis();

    while (millis() - startTime < durationMs && g_genActive) {
        if (noiseType == "WHITE") {
            generateWhiteNoise();
        } else if (noiseType == "PINK") {
            generatePinkNoise();
        } else if (noiseType == "SWEEP") {
            generateSweep();
        }

        if (g_signalsCount % 10 == 0) {
            Serial.println("  [" + String(g_signalsCount) + "] signals");
        }
    }

    g_genActive = false;
    result.success = true;
    result.signalsGenerated = g_signalsCount;

    Serial.println("✓ Complete: " + String(g_signalsCount) + " signals generated");
    return result;
}

void stop() {
    g_genActive = false;
}

bool isActive() {
    return g_genActive;
}

}  // namespace JammingSignalGenerator
