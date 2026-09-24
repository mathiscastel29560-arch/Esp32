#pragma once
#include <Arduino.h>

namespace SubGhzProtocolDecoder {

struct DecodedSignal {
    String protocol;         // "PT2260", "Keeloq", "Secplus", etc
    String manufacturer;
    String deviceType;       // "Door Lock", "Gate", "Light", etc
    uint32_t frequency;      // 433920000
    String rawData;
    int signalStrength;
    bool rollingCodeDetected;
    uint32_t timestamp;
};

struct DecodeResult {
    bool success;
    uint32_t signalsDecoded;
    uint32_t uniqueProtocols;
    String mostCommonProtocol;
    uint32_t durationMs;
};

// Decode 433MHz signals
DecodeResult decodeProtocols(uint32_t durationMs = 30000);
const DecodedSignal* getDecodedSignals(uint32_t& outCount);

// Analyze specific signal
struct SignalAnalysis {
    bool success;
    String encodingType;     // "OOK", "FSK", "ASK"
    uint32_t bitRate;
    String modulation;
    bool potentialVulnerability;
};
SignalAnalysis analyzeSignal(const char* rawData);

// Rolling code detection
struct RollingCodeAnalysis {
    bool success;
    uint32_t sequenceLength;
    bool incrementalPattern;
    uint32_t predictedNextCode;
};
RollingCodeAnalysis detectRollingCode(const DecodedSignal& signal, uint32_t durationMs = 60000);

}  // namespace SubGhzProtocolDecoder
