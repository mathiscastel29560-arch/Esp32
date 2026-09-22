#include "advanced_rf_jammer.h"
#include "tx_arm.h"
#include "config.h"
#include <RadioLib.h>

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);
volatile bool g_jamActive = false;
uint32_t g_jamPacketsCount = 0;
bool g_radioInitialized = false;

int initCC1101Advanced(float freq) {
    if (!g_radioInitialized) {
        int state = radio.begin(freq);
        if (state != RADIOLIB_ERR_NONE) {
            return state;
        }

        radio.setOOK(true);
        radio.setRxBandwidth(58.0f);
        radio.setBitRate(4.8f);
        radio.setOutputPower(10);  // 10 dBm
        g_radioInitialized = true;
    }

    radio.setFrequency(freq);
    return RADIOLIB_ERR_NONE;
}

void sendCarrierOnFreq(float freq, uint32_t durationMs) {
    if (initCC1101Advanced(freq) != RADIOLIB_ERR_NONE) return;

    radio.transmitDirect();
    delay(durationMs);
    radio.standby();
    g_jamPacketsCount++;
}

void frequencySweep(float startFreq, float endFreq, uint32_t sweepTimeMs) {
    uint32_t sweepSteps = (uint32_t)((endFreq - startFreq) * 20);
    if (sweepSteps == 0) sweepSteps = 1;

    float stepSize = (endFreq - startFreq) / sweepSteps;
    uint32_t delayPerStep = sweepTimeMs / sweepSteps;

    for (uint32_t i = 0; i < sweepSteps; i++) {
        float freq = startFreq + (stepSize * i);

        if (initCC1101Advanced(freq) != RADIOLIB_ERR_NONE) break;

        radio.transmitDirect();
        if (delayPerStep >= 1000) {
            delay(delayPerStep / 1000);
        }
        delayMicroseconds((delayPerStep % 1000) * 1000);
        radio.standby();

        g_jamPacketsCount++;

        if (!TxArm::isArmed()) break;
    }
}

void adaptiveFrequencyFollow() {
    const float followFreqs[] = {433.05f, 433.50f, 433.92f, 434.50f};
    const uint8_t freqCount = 4;

    for (int i = 0; i < freqCount; i++) {
        sendCarrierOnFreq(followFreqs[i], 100);
        delayMicroseconds(500);
    }
}
}

namespace AdvancedRFJammer {

JamResult jamRFSignals(const String &frequency, uint32_t durationMs, const String &method) {
    JamResult result{false, 0, durationMs, method, frequency};

    Serial.println("\n=== Advanced RF Jammer (REAL CC1101 Multi-Method) ===");
    Serial.println("Target Frequency: " + frequency);
    Serial.println("Method: " + method);
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Modulation: OOK | Power: Max");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    g_radioInitialized = false;
    g_jamActive = true;
    g_jamPacketsCount = 0;
    uint32_t startTime = millis();

    float targetFreq = frequency.toFloat();
    if (targetFreq == 0.0f) targetFreq = 433.92f;

    Serial.println("Initializing CC1101...");
    if (initCC1101Advanced(targetFreq) != RADIOLIB_ERR_NONE) {
        Serial.println("✗ CC1101 initialization failed");
        g_jamActive = false;
        return result;
    }

    Serial.println("Transmitting advanced RF jamming...");

    while (millis() - startTime < durationMs && g_jamActive && TxArm::isArmed()) {
        if (method == "NOISE") {
            sendCarrierOnFreq(targetFreq, 5);
            delay(2);
        }
        else if (method == "SWEEP") {
            float sweepStart = targetFreq - 0.5f;
            float sweepEnd = targetFreq + 0.5f;
            frequencySweep(sweepStart, sweepEnd, 100);
        }
        else if (method == "FOLLOW") {
            adaptiveFrequencyFollow();
        }

        if (g_jamPacketsCount % 50 == 0) {
            Serial.printf("  [%u] RF transmissions in %lums\n",
                         g_jamPacketsCount, millis() - startTime);
        }
    }

    g_jamActive = false;
    radio.standby();
    result.success = true;
    result.jamPacketsCount = g_jamPacketsCount;

    uint32_t elapsed = millis() - startTime;
    Serial.printf("✓ Advanced RF jamming complete\n");
    Serial.printf("  Method: %s | Transmissions: %u | Duration: %lums (%.1f tx/sec)\n",
                 method.c_str(), result.jamPacketsCount, elapsed,
                 (result.jamPacketsCount * 1000.0f) / elapsed);
    Serial.printf("⚠️  RF spectrum around %.2f MHz jammed\n", targetFreq);

    return result;
}

void stop() {
    g_jamActive = false;
    if (g_radioInitialized) {
        radio.standby();
    }
    Serial.println("Advanced RF jammer stopped");
}

bool isActive() {
    return g_jamActive;
}

}  // namespace AdvancedRFJammer
