#pragma once
#include <Arduino.h>
#include <vector>

// CC1101 sub-GHz (record/replay) for testing your own fixed-code remotes
// (garage doors, gate openers, doorbells, sensors, etc.) that you are
// authorized to test. Replay is gated by the hardware safety switch.
namespace SubGhz {

struct Capture {
    float freqMHz = 433.92f;
    std::vector<uint16_t> pulsesUs; // alternating mark/space durations, starts with a mark
};

void begin();

// Tunes to freqMHz and returns the instantaneous RSSI in dBm — used to
// sweep a band and find where a remote is transmitting.
int8_t rssiAt(float freqMHz);

// Listens on freqMHz for up to timeoutMs waiting for OOK/ASK pulses (e.g.
// press the remote you're testing). Returns whatever was captured, which
// may be empty if nothing was seen.
Capture record(float freqMHz, uint32_t timeoutMs);

// Re-transmits a previously recorded capture. Returns false without doing
// anything if the hardware safety switch is off.
bool replay(const Capture &capture);

bool saveCapture(const Capture &capture, const String &filePath);
Capture loadCapture(const String &filePath);

} // namespace SubGhz
