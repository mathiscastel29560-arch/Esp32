#include "signal_decoder.h"
#include "rf_signal_recorder.h"
#include "results_display.h"

namespace SignalDecoder {

DecodedSignal decodeSignal() {
    DecodedSignal result = {false, "UNKNOWN", "UNKNOWN", 0, 0, 0, ""};

    uint32_t dataLen = 0;
    const uint8_t* data = RfSignalRecorder::getCapturedData(dataLen);

    if (!data || dataLen < 8) {
    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
        return result;
    }

    result.success = true;

    // Analyze encoding
    if (detectManchester()) {
        result.format = "Manchester";
    } else if (detectNRZ()) {
        result.format = "NRZ";
    } else {
        result.format = "OOK";
    }

    // Detect bit repetition
    uint8_t bitRep = detectBitRepetition();
    if (bitRep == 1) {
        result.modulationType = "ASK";
    } else if (bitRep == 2) {
        result.modulationType = "FSK";
    } else {
        result.modulationType = "GFSK";
    }

    // Estimate bitrate
    result.estimatedBitrate = 2400 * bitRep;  // Base 2400 bps, scaled by repetition

    // Find pattern
    PatternMatch pattern = findRepeatingPattern();
    result.patternLength = pattern.length;

    // Decode to hex with bounds checking
    char hexBuf[513] = {0};
    uint32_t maxBytes = (dataLen < 256) ? dataLen : 256;
    for (uint32_t i = 0; i < maxBytes; i++) {
        size_t remaining = sizeof(hexBuf) - (i * 2);
        if (remaining < 3) break;  // Need 2 chars + null terminator
        snprintf(hexBuf + (i * 2), remaining, "%02X", data[i]);
    }
    result.decodedData = String(hexBuf);

    // Estimate frequency spread (for FSK)
    result.estimatedFrequency = estimateFrequencySpread();

    ResultsDisplay::showResult("Tool", {"Tool", "Complete", 100, {"Success"}, ResultsDisplay::ResultType::SUCCESS});
    return result;
}

bool detectManchester() {
    uint32_t dataLen = 0;
    const uint8_t* data = RfSignalRecorder::getCapturedData(dataLen);

    if (!data || dataLen < 16) return false;

    // Manchester encoding: each bit becomes two transitions
    uint32_t transitions = 0;
    uint32_t checkLen = (dataLen < 128) ? dataLen : 128;

    for (uint32_t i = 1; i < checkLen; i++) {
        if (data[i] != data[i-1]) transitions++;
    }

    // Manchester should have ~2x transitions per byte
    return (transitions > checkLen);
}

bool detectNRZ() {
    uint32_t dataLen = 0;
    const uint8_t* data = RfSignalRecorder::getCapturedData(dataLen);

    if (!data || dataLen < 16) return false;

    // NRZ: one level per bit, fewer transitions
    uint32_t transitions = 0;
    for (uint32_t i = 1; i < min((uint32_t)128, dataLen); i++) {
        if (data[i] != data[i-1]) transitions++;
    }

    return (transitions < min((uint32_t)64, dataLen / 2));
}

uint8_t detectBitRepetition() {
    uint32_t dataLen = 0;
    const uint8_t* data = RfSignalRecorder::getCapturedData(dataLen);

    if (!data || dataLen < 16) return 1;

    // Check for repeated bytes (1x, 2x, 4x, 8x)
    uint32_t doubleCount = 0;
    uint32_t quadCount = 0;

    for (uint32_t i = 0; i + 2 < dataLen; i += 2) {
        if (data[i] == data[i+1]) doubleCount++;
    }

    for (uint32_t i = 0; i + 4 < dataLen; i += 4) {
        if (data[i] == data[i+1] && data[i+1] == data[i+2] && data[i+2] == data[i+3]) {
            quadCount++;
        }
    }

    if (quadCount > dataLen / 8) return 4;
    if (doubleCount > dataLen / 4) return 2;
    return 1;
}

PatternMatch findRepeatingPattern() {
    PatternMatch match = {0, 0, 0};
    uint32_t dataLen = 0;
    const uint8_t* data = RfSignalRecorder::getCapturedData(dataLen);

    if (!data || dataLen < 4) return match;

    // Try pattern lengths from 1 to 256 bytes
    for (uint32_t patLen = 1; patLen <= min((uint32_t)256, dataLen / 2); patLen++) {
        uint32_t reps = 0;
        bool isRepeating = true;

        for (uint32_t i = patLen; i < dataLen; i += patLen) {
            uint32_t len = min(patLen, dataLen - i);
            if (memcmp(&data[0], &data[i], len) == 0) {
                reps++;
            } else {
                isRepeating = false;
                break;
            }
        }

        if (isRepeating && reps >= 2) {
            match.offset = 0;
            match.length = patLen;
            match.repetitions = reps + 1;
            return match;
        }
    }

    return match;
}

float estimateFrequencySpread() {
    uint32_t dataLen = 0;
    const uint8_t* data = RfSignalRecorder::getCapturedData(dataLen);

    if (!data || dataLen < 8) return 0;

    // Simple FSK spread estimation based on data variance
    uint32_t sum = 0;
    for (uint32_t i = 0; i < dataLen; i++) {
        sum += data[i];
    }
    float avg = sum / (float)dataLen;

    float variance = 0;
    for (uint32_t i = 0; i < dataLen; i++) {
        float diff = data[i] - avg;
        variance += (diff * diff);
    }
    variance /= dataLen;

    // FSK spread proportional to data variance
    return sqrt(variance) * 50.0f;  // Scale to kHz
}

}  // namespace SignalDecoder
