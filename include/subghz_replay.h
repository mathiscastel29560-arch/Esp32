#pragma once
#include <Arduino.h>

namespace SubGhzReplay {

struct CaptureResult {
    bool success;
    uint32_t frequency;    // MHz (433, 868, etc.)
    uint16_t duration;     // ms
    uint32_t bitrate;      // bps
    uint16_t capturedBits; // nombre de bits capturés
    String error;
};

struct ReplayResult {
    bool success;
    uint8_t repeatCount;   // nombre de fois rejoué
    uint32_t totalDurationMs;
    String message;
    String error;
};

// Enregistre un signal sub-GHz (433/868 MHz) pour 5 secondes
CaptureResult capture(uint32_t frequencyMhz = 433);

// Rejoue un signal capturé N fois avec délai entre chaque
ReplayResult replay(uint8_t repeatCount = 10, uint16_t delayMs = 100);

// Analyse le pattern du signal (modulation, bitrate)
String analyzePattern(uint16_t samples);

} // namespace SubGhzReplay
