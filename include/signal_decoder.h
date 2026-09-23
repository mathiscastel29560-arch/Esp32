#pragma once
#include <Arduino.h>

namespace SignalDecoder {

struct DecodedSignal {
    bool success;
    String format;           // "OOK", "FSK", "PSK", "UNKNOWN"
    String modulationType;   // "ASK", "FSK", "PSK", "GFSK"
    uint32_t estimatedBitrate;  // bits per second
    uint32_t patternLength;  // detected repeating pattern length
    float estimatedFrequency;    // estimated center frequency
    String decodedData;      // hex string of decoded bytes
};

// Analyze captured signal data for modulation type, bitrate, patterns
DecodedSignal decodeSignal();

// Detect Manchester encoding
bool detectManchester();

// Detect NRZ encoding
bool detectNRZ();

// Detect bit repetition (1x, 2x, 4x, 8x)
uint8_t detectBitRepetition();

// Find repeating patterns in captured data
struct PatternMatch {
    uint32_t offset;
    uint32_t length;
    uint32_t repetitions;
};
PatternMatch findRepeatingPattern();

// Estimate signal frequency spread (frequency shift keying spread)
float estimateFrequencySpread();

}  // namespace SignalDecoder
