#include "subghz_replay.h"
#include "config.h"
#include "tx_arm.h"
#include <RadioLib.h>

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);

volatile uint16_t g_pulseBuf[1024];
volatile size_t g_pulseCount = 0;
volatile uint32_t g_lastEdgeUs = 0;
volatile bool g_capturing = false;

void IRAM_ATTR onEdge() {
    uint32_t now = micros();
    uint32_t dt = now - g_lastEdgeUs;
    g_lastEdgeUs = now;
    if (!g_capturing) return;
    if (g_pulseCount < 1024) {
        g_pulseBuf[g_pulseCount++] = (dt > 0xFFFF) ? 0xFFFF : (uint16_t)dt;
    }
}
}

namespace SubghzReplay {

SignalCapture recordSignal(float freqMHz, uint32_t durationMs) {
    SignalCapture cap{freqMHz, {}, durationMs, millis()};

    Serial.println("\n=== Sub-GHz Signal Recording ===");
    Serial.println("Frequency: " + String(freqMHz, 2) + " MHz");
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Waiting for signal...");

    if (radio.begin(freqMHz) != RADIOLIB_ERR_NONE) {
        Serial.println("✗ Radio init failed");
        return cap;
    }

    radio.setOOK(true);
    radio.receiveDirectAsync();

    g_pulseCount = 0;
    g_lastEdgeUs = micros();
    g_capturing = true;

    pinMode(PIN_CC1101_GDO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_CC1101_GDO0), onEdge, CHANGE);

    uint32_t start = millis();
    while (millis() - start < durationMs && g_pulseCount < 1024) {
        delay(10);
    }

    detachInterrupt(digitalPinToInterrupt(PIN_CC1101_GDO0));
    g_capturing = false;

    cap.pulsesUs.assign((const uint16_t *)g_pulseBuf, (const uint16_t *)g_pulseBuf + g_pulseCount);

    Serial.println("✓ Recorded " + String(cap.pulsesUs.size()) + " pulses");
    Serial.println("Duration: " + String(millis() - start) + "ms");

    return cap;
}

ReplayResult replaySignal(const SignalCapture &capture, uint8_t repeatCount) {
    ReplayResult result{false, 0, 0, capture.frequency};

    Serial.println("\n=== Sub-GHz Signal Replay ===");
    Serial.println("Frequency: " + String(capture.frequency, 2) + " MHz");
    Serial.println("Pulses: " + String(capture.pulsesUs.size()));
    Serial.println("Repeat: " + String(repeatCount) + "x");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    if (radio.begin(capture.frequency) != RADIOLIB_ERR_NONE) {
        Serial.println("✗ Radio init failed");
        return result;
    }

    radio.setOOK(true);
    radio.transmitDirectAsync();
    pinMode(PIN_CC1101_GDO0, OUTPUT);

    for (uint8_t rep = 0; rep < repeatCount; rep++) {
        bool level = HIGH;
        for (uint16_t pulse : capture.pulsesUs) {
            digitalWrite(PIN_CC1101_GDO0, level);
            delayMicroseconds(pulse);
            level = !level;
            result.pulsesCount++;
        }
        result.replayCount++;
        delay(200);
    }

    digitalWrite(PIN_CC1101_GDO0, LOW);
    result.success = true;

    Serial.println("✓ Replay complete: " + String(result.pulsesCount) + " pulses transmitted");

    return result;
}

int8_t getRSSI(float freqMHz) {
    if (radio.begin(freqMHz) != RADIOLIB_ERR_NONE) return -127;
    radio.setOOK(true);
    radio.receiveDirectAsync();
    delay(5);
    return (int8_t)radio.getRSSI();
}

}  // namespace SubghzReplay
