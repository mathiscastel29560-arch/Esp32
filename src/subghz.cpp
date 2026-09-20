#include "subghz.h"
#include "config.h"
#include "tx_arm.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <LittleFS.h>

namespace {
constexpr size_t MAX_PULSES = 1024;
volatile uint16_t g_pulseBuf[MAX_PULSES];
volatile size_t g_pulseCount = 0;
volatile uint32_t g_lastEdgeUs = 0;
volatile bool g_capturing = false;

void IRAM_ATTR onEdge() {
    uint32_t now = micros();
    uint32_t dt = now - g_lastEdgeUs;
    g_lastEdgeUs = now;
    if (!g_capturing) return;
    if (g_pulseCount < MAX_PULSES) {
        g_pulseBuf[g_pulseCount++] = (dt > 0xFFFF) ? 0xFFFF : (uint16_t)dt;
    }
}

// PKTCTRL0 = 0x08. Value 0x32 selects asynchronous serial mode: GDO0
// becomes the raw demodulated bitstream in RX and the direct modulator
// input in TX, which is what makes edge-timing capture/replay possible.
constexpr uint8_t REG_PKTCTRL0 = 0x08;
constexpr uint8_t PKTCTRL0_ASYNC_SERIAL = 0x32;
}

namespace SubGhz {

void begin() {
    ELECHOUSE_cc1101.setSpiPin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_CC1101_CS);
    ELECHOUSE_cc1101.setGDO(PIN_CC1101_GDO0, PIN_CC1101_GDO2);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setModulation(2); // ASK/OOK
    ELECHOUSE_cc1101.setMHZ(433.92);
    ELECHOUSE_cc1101.SpiWriteReg(REG_PKTCTRL0, PKTCTRL0_ASYNC_SERIAL);
    ELECHOUSE_cc1101.SetRx();
}

int8_t rssiAt(float freqMHz) {
    ELECHOUSE_cc1101.setMHZ(freqMHz);
    ELECHOUSE_cc1101.SetRx();
    delay(5);
    return ELECHOUSE_cc1101.getRssi();
}

Capture record(float freqMHz, uint32_t timeoutMs) {
    Capture cap;
    cap.freqMHz = freqMHz;

    ELECHOUSE_cc1101.setMHZ(freqMHz);
    ELECHOUSE_cc1101.SetRx();

    g_pulseCount = 0;
    g_lastEdgeUs = micros();
    g_capturing = true;
    pinMode(PIN_CC1101_GDO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_CC1101_GDO0), onEdge, CHANGE);

    uint32_t start = millis();
    while (millis() - start < timeoutMs && g_pulseCount < MAX_PULSES) {
        delay(1);
    }

    detachInterrupt(digitalPinToInterrupt(PIN_CC1101_GDO0));
    g_capturing = false;

    cap.pulsesUs.assign((const uint16_t *)g_pulseBuf, (const uint16_t *)g_pulseBuf + g_pulseCount);
    return cap;
}

bool replay(const Capture &capture) {
    if (!TxArm::isArmed()) return false;
    if (capture.pulsesUs.empty()) return false;

    ELECHOUSE_cc1101.setMHZ(capture.freqMHz);
    ELECHOUSE_cc1101.SetTx();
    pinMode(PIN_CC1101_GDO0, OUTPUT);

    bool level = HIGH;
    for (uint16_t d : capture.pulsesUs) {
        digitalWrite(PIN_CC1101_GDO0, level);
        delayMicroseconds(d);
        level = !level;
    }
    digitalWrite(PIN_CC1101_GDO0, LOW);

    ELECHOUSE_cc1101.SetRx();
    return true;
}

bool saveCapture(const Capture &capture, const String &filePath) {
    if (!LittleFS.exists(SUBGHZ_CAPTURE_DIR)) LittleFS.mkdir(SUBGHZ_CAPTURE_DIR);
    File f = LittleFS.open(filePath, FILE_WRITE);
    if (!f) return false;
    f.println(capture.freqMHz, 3);
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
