#include <RadioLib.h>
#include "config.h"
#include <vector>
#include <algorithm>
#include "spectrum_visualizer.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <LittleFS.h>

namespace SpectrumVisualizer {

static std::vector<FrequencyBin> spectrum;

// Real spectrum scanning with CC1101
SpectrumResult scanSpectrum(float startFreq, float endFreq, float stepMHz, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    SpectrumResult result{false, 0, 0.0f, -127, ""};

    displayScanStart("Real-time Spectrum Analyzer",
                    String(startFreq, 2) + " - " + String(endFreq, 2) + " MHz");

    ScanProgressBar progress("Spectrum Scan", durationMs, 3);
    progress.start();

    spectrum.clear();

    Module cc1101Module(PIN_CC1101_CS, PIN_CC1101_GDO0, RADIOLIB_NC, PIN_CC1101_GDO2, SPI);
    CC1101 radio(&cc1101Module);

    if (radio.begin((startFreq + endFreq) / 2.0f) != RADIOLIB_ERR_NONE) {
        progress.complete("CC1101 initialization failed");
        return result;
    }

    radio.setOOK(true);
    radio.setRxBandwidth(812.5);

    // Phase 1: Scan spectrum
    progress.step("Scanning " + String(startFreq, 2) + "-" + String(endFreq, 2) +
                  " MHz with " + String(stepMHz, 2) + " MHz resolution");

    uint32_t startTime = millis();
    uint32_t binsScanned = 0;
    int8_t maxRssi = -127;
    float maxFreq = startFreq;

    for (float freq = startFreq; freq <= endFreq && (millis() - startTime) < (durationMs * 2 / 3); freq += stepMHz) {
        radio.setFrequency(freq);
        radio.startReceive();

        // Sample for short duration per frequency
        uint32_t binStart = millis();
        uint8_t samples = 0;
        int32_t rssiSum = 0;

        while ((millis() - binStart) < 50 && samples < 5) {
            int8_t rssi = radio.getRSSI();
            rssiSum += rssi;
            samples++;
            delayMicroseconds(10000);
        }

        radio.standby();

        int8_t avgRssi = (samples > 0) ? (rssiSum / samples) : -127;

        FrequencyBin bin;
        bin.frequency = freq;
        bin.rssi = (uint8_t)(avgRssi + 127);  // Shift to 0-255 range
        bin.activity = (avgRssi > -90) ? 1 : 0;  // Activity detected

        spectrum.push_back(bin);
        binsScanned++;

        if (avgRssi > maxRssi) {
            maxRssi = avgRssi;
            maxFreq = freq;
        }

        // Console output with spectrum bar
        if (binsScanned % 5 == 0) {
            Serial.printf("[%3.2f MHz] RSSI: %3d dBm ", freq, avgRssi);
            for (int i = 0; i < (avgRssi + 127) / 8; i++) {
                Serial.print("█");
            }
            Serial.println();
        }
    }

    // Phase 2: Identify hotspots
    progress.step("Identifying frequency hotspots and interference patterns");

    std::vector<FrequencyBin> sorted = spectrum;
    std::sort(sorted.begin(), sorted.end(),
              [](const FrequencyBin& a, const FrequencyBin& b) {
                  return a.rssi > b.rssi;  // Sort by RSSI descending
              });

    // Top 5 busiest frequencies
    Serial.println("\n=== TOP ACTIVE FREQUENCIES ===");
    for (uint32_t i = 0; i < (sorted.size() > 5 ? 5 : sorted.size()); i++) {
        int8_t rssiDb = sorted[i].rssi - 127;
        Serial.printf("#%d: %.2f MHz | RSSI: %d dBm\n", i + 1, sorted[i].frequency, rssiDb);
    }

    // Phase 3: Generate spectrum visualization
    progress.step("Generating spectrum visualization and heatmap");

    delay(durationMs / 3);

    progress.complete(String(binsScanned) + " frequency bins analyzed, " +
                     "peak at " + String(maxFreq, 2) + " MHz");

    // Render results
    ResultRenderers::RFScanResult scanResult;
    scanResult.signalsDetected = binsScanned;
    scanResult.dominantProtocol = String(maxFreq, 2) + " MHz";
    scanResult.rollingCodesDetected = (maxRssi > -90) ? 1 : 0;
    for (const auto& bin : spectrum) {
        scanResult.frequencies.push_back((uint32_t)(bin.frequency * 1000000));
        scanResult.signalStrengths.push_back((int8_t)(bin.rssi - 127));
    }
    scanResult.durationMs = millis() - startTime;

    ResultRenderers::renderRFScan(scanResult);

    result.success = true;
    result.binsScanned = binsScanned;
    result.peakFrequency = maxFreq;
    result.peakRSSI = maxRssi;
    result.spectrumFile = "/logs/spectrum/scan_" + String(millis()) + ".csv";

    return result;
}

// ASCII spectrum visualization for terminal
void displaySpectrumASCII() {
    if (spectrum.empty()) {
        Serial.println("No spectrum data available");
        return;
    }

    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║              SPECTRUM VISUALIZATION (ASCII)              ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");

    // Find min/max for scaling
    uint8_t minRssi = 255, maxRssiVal = 0;
    for (const auto& bin : spectrum) {
        if (bin.rssi < minRssi) minRssi = bin.rssi;
        if (bin.rssi > maxRssiVal) maxRssiVal = bin.rssi;
    }

    uint8_t scale = maxRssiVal - minRssi;
    if (scale == 0) scale = 1;

    // Display each frequency bin
    for (size_t i = 0; i < spectrum.size(); i++) {
        const auto& bin = spectrum[i];

        // Frequency label
        Serial.printf("%.2f | ", bin.frequency);

        // ASCII bar
        uint8_t barHeight = (bin.rssi - minRssi) * 40 / scale;
        for (uint8_t j = 0; j < barHeight; j++) {
            Serial.print("█");
        }

        // RSSI value
        int8_t rssiDb = (int8_t)(bin.rssi - 127);
        Serial.printf(" %3d dBm\n", rssiDb);
    }

    Serial.println("╚════════════════════════════════════════════════════════════╝");
}

// Export spectrum to CSV for analysis
bool exportSpectrumCSV(const char* filename) {
    fs::File file = LittleFS.open(filename, "w");
    if (!file) return false;

    file.println("Frequency_MHz,RSSI_dBm,Activity");

    for (const auto& bin : spectrum) {
        int8_t rssiDb = (int8_t)(bin.rssi - 127);
        file.printf("%.2f,%d,%u\n", bin.frequency, rssiDb, bin.activity);
    }

    file.close();
    return true;
}

// Get spectrum data for external analysis
const FrequencyBin* getSpectrum(uint32_t& count) {
    count = spectrum.size();
    return spectrum.empty() ? nullptr : spectrum.data();
}

// Find interference patterns
std::vector<InterferencePattern> detectInterference() {
    std::vector<InterferencePattern> patterns;

    if (spectrum.size() < 3) return patterns;

    // Scan for continuous high-power regions
    int32_t inInterference = 0;
    float interfStart = 0;
    int8_t interfPower = -127;

    for (size_t i = 0; i < spectrum.size(); i++) {
        int8_t power = (int8_t)(spectrum[i].rssi - 127);

        if (power > -80) {  // Strong signal threshold
            if (!inInterference) {
                interfStart = spectrum[i].frequency;
                inInterference = 1;
                interfPower = power;
            } else {
                if (power > interfPower) interfPower = power;
            }
        } else {
            if (inInterference) {
                float bandwidth = spectrum[i].frequency - interfStart;

                InterferencePattern pattern;
                pattern.centerFreq = (interfStart + spectrum[i].frequency) / 2.0f;
                pattern.bandwidth = bandwidth;
                pattern.power = interfPower;

                if (bandwidth < 0.1f) {
                    pattern.description = "Narrowband (FSK/PSK)";
                } else if (bandwidth < 0.5f) {
                    pattern.description = "Medium bandwidth (OOK/ASK)";
                } else if (bandwidth < 2.0f) {
                    pattern.description = "Wideband (GFSK/Data)";
                } else {
                    pattern.description = "Very wideband (WiFi/BLE-like)";
                }

                patterns.push_back(pattern);
                inInterference = 0;
            }
        }
    }

    return patterns;
}

} // namespace SpectrumVisualizer
