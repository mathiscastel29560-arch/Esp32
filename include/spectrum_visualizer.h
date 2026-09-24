#pragma once
#include <Arduino.h>
#include <vector>
#include <LittleFS.h>

namespace SpectrumVisualizer {

struct FrequencyBin {
    float frequency;
    uint8_t rssi;
    uint32_t activity;
};

struct SpectrumResult {
    bool success;
    uint32_t binsScanned;
    float peakFrequency;
    int8_t peakRSSI;
    String spectrumFile;
};

struct InterferencePattern {
    float centerFreq;
    float bandwidth;
    int8_t power;
    String description;
};

SpectrumResult scanSpectrum(float startFreq, float endFreq, float stepMHz = 0.1f, uint32_t durationMs = 30000);
void displaySpectrumASCII();
bool exportSpectrumCSV(const char* filename);
const FrequencyBin* getSpectrum(uint32_t& count);
std::vector<InterferencePattern> detectInterference();

} // namespace SpectrumVisualizer
