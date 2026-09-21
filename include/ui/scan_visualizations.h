#pragma once
#include <Arduino.h>
#include <vector>
#include "ui/theme.h"

// Enhanced scan visualization screens using advanced widgets.
// Displays live scan data (WiFi, BLE, Spectrum) with histograms, gauges,
// and other visualizations instead of plain text lists.
namespace ScanVisualizations {

// ---- WiFi Scan Visualization ----
// Display WiFi scan results as:
//   - Histogram: Channel distribution (which channels have activity)
//   - Gauge: Strongest signal RSSI
//   - Info: Network count, strongest SSID
struct WifiScanData {
    std::vector<uint16_t> channelCounts;  // Count per channel (1-14 for 2.4GHz)
    int32_t strongestRssi;                 // dBm value (-100 to -30)
    String strongestSsid;
    uint16_t totalNetworks;
};

// Render WiFi histogram + gauge visualization
void showWifiVisualization(const WifiScanData &data);

// ---- BLE Scan Visualization ----
// Display BLE scan results as:
//   - Histogram: Device count by RSSI strength bands
//   - Gauge: Strongest signal RSSI
//   - Pulsing indicator: Scanning active
struct BleScanData {
    std::vector<int> rssiValues;  // All observed RSSI values
    int32_t strongestRssi;
    String strongestDevice;
    uint16_t totalDevices;
    bool scanning;
};

// Render BLE histogram + gauge visualization
void showBleVisualization(const BleScanData &data);

// ---- Spectrum Analysis Visualization ----
// Display spectrum data as:
//   - Waterfall: Frequency spectrum graph
//   - Heatmap: Frequency activity over time (if historical data available)
//   - Gauge: Dominant amplitude
struct SpectrumData {
    std::vector<uint8_t> frequencyBins;    // 0-255 intensity per bin (32 bins typical)
    int8_t dominantAmplitude;              // Dominant signal amplitude
    float dominantFrequency;               // Dominant frequency in MHz
    uint32_t peaksFound;
};

// Render spectrum waterfall visualization
void showSpectrumVisualization(const SpectrumData &data);

// ---- Helper: Convert RSSI to intensity (0-255) ----
// Maps dBm RSSI (-100 to -30) to 0-255 intensity scale for visualizations
uint8_t rssiToIntensity(int32_t rssiDbm);

// ---- Helper: Create histogram bins from RSSI values ----
// Groups RSSI values into strength bands (weak/fair/strong)
std::vector<uint16_t> createRssiHistogram(const std::vector<int> &rssiValues);

} // namespace ScanVisualizations
