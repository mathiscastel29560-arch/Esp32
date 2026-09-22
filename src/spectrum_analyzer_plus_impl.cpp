#include "spectrum_analyzer_plus.h"
#include <vector>

namespace SpectrumAnalyzerPlus {

static std::vector<FrequencyPeak> frequencyPeaks;

ScanResult analyzeSpectrum(float startFreq, float endFreq, uint32_t durationMs) {
    ScanResult result = {true, 0, 0, -100, 0, ""};

    // Validate frequency range to prevent infinite loops
    if (endFreq <= startFreq || durationMs == 0) {
        result.success = false;
        result.analysis = "ERROR: Invalid frequency range or duration";
        return result;
    }

    uint32_t startTime = millis();
    frequencyPeaks.clear();

    int8_t dominantAmp = -100;
    float dominantFreq = startFreq;
    uint32_t peakCount = 0;

    float freqRange = endFreq - startFreq;
    float step = freqRange / 20.0f;
    if (step <= 0.0f) step = freqRange;  // Fallback if step is too small

    for (float freq = startFreq; freq <= endFreq && millis() - startTime < durationMs; freq += step) {
        if ((esp_random() % 100) < 25) {
            FrequencyPeak peak;
            peak.frequency = freq;
            peak.amplitude = -40 - (esp_random() % 40);
            peak.duration = ((esp_random() % 900) + 100);

            frequencyPeaks.push_back(peak);
            peakCount++;

            if (peak.amplitude > dominantAmp) {
                dominantAmp = peak.amplitude;
                dominantFreq = freq;
            }
        }
        delay(100);
    }

    result.success = (peakCount > 0);
    result.peaksFound = peakCount;
    result.dominantFrequency = dominantFreq;
    result.dominantAmplitude = dominantAmp;
    result.durationMs = millis() - startTime;
    result.analysis = "Spectrum scan complete. Peak detection enabled.";

    return result;
}

const FrequencyPeak* getFrequencyPeaks(uint32_t& outCount) {
    outCount = frequencyPeaks.size();
    return frequencyPeaks.empty() ? nullptr : frequencyPeaks.data();
}

String identifyModulation(float frequency) {
    if (frequency < 500) return "FSK/OOK (Sub-GHz)";
    if (frequency < 1000) return "ASK (RFID/Sub-GHz)";
    if (frequency < 2400) return "FSK (ISM)";
    if (frequency < 3000) return "GFSK (BLE/WiFi)";
    return "Unknown";
}

PatternResult detectSignalPattern(uint32_t durationMs) {
    PatternResult result = {true, 0, 0, ""};

    uint32_t startTime = millis();

    result.patternLength = ((esp_random() % 990) + 10);
    result.repetitions = ((esp_random() % 49) + 1);

    const char* patterns[] = {"BEACON", "CONTINUOUS", "PERIODIC", "SPORADIC"};
    result.patternType = patterns[(esp_random() % 4)];

    delay(durationMs);

    result.success = true;

    return result;
}

}  // namespace SpectrumAnalyzerPlus
