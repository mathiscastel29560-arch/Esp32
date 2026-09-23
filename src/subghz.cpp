#include "subghz.h"
#include "config.h"
#include "tx_arm.h"
#include <RadioLib.h>
#include <LittleFS.h>

namespace {
Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
CC1101 radio(&cc1101Module);

constexpr size_t MAX_PULSES = 1024;
constexpr uint16_t MAX_PULSE_WIDTH = 0xFFFF;
constexpr uint32_t RSSI_MEASUREMENT_DELAY_MS = 5;
constexpr uint32_t CAPTURE_POLL_DELAY_MS = 1;
constexpr int FREQ_DECIMAL_PLACES = 3;
volatile uint16_t g_pulseBuf[MAX_PULSES];
volatile size_t g_pulseCount = 0;
volatile uint32_t g_lastEdgeUs = 0;
volatile bool g_capturing = false;

portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onEdge() {
    uint32_t now = micros();
    uint32_t dt = now - g_lastEdgeUs;
    g_lastEdgeUs = now;
    if (!g_capturing) return;

    // Critical section: protect buffer modification
    portENTER_CRITICAL_ISR(&spinlock);
    if (g_pulseCount < MAX_PULSES) {
        g_pulseBuf[g_pulseCount++] = (dt > MAX_PULSE_WIDTH) ? MAX_PULSE_WIDTH : (uint16_t)dt;
    }
    portEXIT_CRITICAL_ISR(&spinlock);
}
}

namespace SubGhz {

void begin() {
    int initResult = radio.begin(CC1101_FREQ_MHZ);
    if (initResult != RADIOLIB_ERR_NONE) {
        Serial.println("✗ CC1101 initialization failed (code: " + String(initResult) + ")");
        return;
    }
    radio.setOOK(true);
    radio.receiveDirectAsync(); // GDO0 becomes the raw demodulated bitstream
    Serial.println("✓ CC1101 initialized at " + String(CC1101_FREQ_MHZ, 2) + " MHz");
}

int8_t rssiAt(float freqMHz) {
    radio.setFrequency(freqMHz);
    radio.receiveDirectAsync();
    delay(RSSI_MEASUREMENT_DELAY_MS);
    return (int8_t)radio.getRSSI();
}

Capture record(float freqMHz, uint32_t timeoutMs) {
    Capture cap;
    cap.freqMHz = freqMHz;

    radio.setFrequency(freqMHz);
    radio.receiveDirectAsync();

    g_pulseCount = 0;
    g_lastEdgeUs = micros();
    g_capturing = true;
    pinMode(PIN_CC1101_GDO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_CC1101_GDO0), onEdge, CHANGE);

    uint32_t start = millis();
    uint32_t deadline = start + timeoutMs;

    // Capture with timeout (handles millis() wraparound safely)
    while ((int32_t)(millis() - deadline) < 0 && g_pulseCount < MAX_PULSES) {
        delay(CAPTURE_POLL_DELAY_MS);
    }

    detachInterrupt(digitalPinToInterrupt(PIN_CC1101_GDO0));
    g_capturing = false;

    // Safe copy of capture data with synchronization
    portENTER_CRITICAL(&spinlock);
    size_t capturedPulses = g_pulseCount;
    uint16_t localBuf[MAX_PULSES];
    memcpy(localBuf, (void *)g_pulseBuf, capturedPulses * sizeof(uint16_t));
    portEXIT_CRITICAL(&spinlock);

    cap.pulsesUs.assign(localBuf, localBuf + capturedPulses);
    return cap;
}

bool replay(const Capture &capture) {
    if (!TxArm::isArmed()) return false;
    if (capture.pulsesUs.empty()) return false;

    radio.setFrequency(capture.freqMHz);
    radio.transmitDirectAsync(); // GDO0 becomes the direct modulator input
    pinMode(PIN_CC1101_GDO0, OUTPUT);

    bool level = HIGH;
    for (uint16_t d : capture.pulsesUs) {
        digitalWrite(PIN_CC1101_GDO0, level);
        delayMicroseconds(d);
        level = !level;
    }
    digitalWrite(PIN_CC1101_GDO0, LOW);

    radio.packetMode();
    radio.receiveDirectAsync();
    return true;
}

bool saveCapture(const Capture &capture, const String &filePath) {
    if (!LittleFS.exists(SUBGHZ_CAPTURE_DIR)) LittleFS.mkdir(SUBGHZ_CAPTURE_DIR);
    File f = LittleFS.open(filePath, FILE_WRITE);
    if (!f) return false;
    f.println(capture.freqMHz, FREQ_DECIMAL_PLACES);
    for (uint16_t d : capture.pulsesUs) f.printf("%u\n", d);
    f.close();
    return true;
}

Capture loadCapture(const String &filePath) {
    Capture cap;
    File f = LittleFS.open(filePath, FILE_READ);
    if (!f) return cap;
    cap.freqMHz = f.readStringUntil('\n').toFloat();
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length()) cap.pulsesUs.push_back((uint16_t)line.toInt());
    }
    f.close();
    return cap;
}

} // namespace SubGhz
