#include "ui/scan_visualizations.h"
#include "ui/advanced_widgets.h"
#include "display.h"
#include "buttons.h"
#include <TFT_eSPI.h>
#include <cmath>
#include <algorithm>

namespace ScanVisualizations {

namespace {
TFT_eSPI &tft = Display::raw();
TFT_eSprite canvas(&tft);
}

// ---- Helper: Convert RSSI to intensity ----
uint8_t rssiToIntensity(int32_t rssiDbm) {
    // Map dBm (-100 to -30) to intensity (0 to 255)
    // -100 dBm = 0 (very weak)
    // -30 dBm = 255 (very strong)
    if (rssiDbm <= -100) return 0;
    if (rssiDbm >= -30) return 255;

    // Linear interpolation: (-100 to -30 = 70 dBm range)
    int32_t normalized = rssiDbm + 100;  // 0 to 70
    return (uint8_t)((normalized * 255) / 70);
}

// ---- Helper: Create histogram from RSSI values ----
std::vector<uint16_t> createRssiHistogram(const std::vector<int> &rssiValues) {
    // Create 3 bins: weak (-100 to -67), fair (-67 to -50), strong (-50 to -30)
    std::vector<uint16_t> bins(3, 0);

    for (int rssi : rssiValues) {
        if (rssi <= -67) bins[0]++;      // Weak
        else if (rssi <= -50) bins[1]++; // Fair
        else bins[2]++;                   // Strong
    }

    return bins;
}

// ---- WiFi Visualization ----
void showWifiVisualization(const WifiScanData &data) {
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("WiFi visualization requires TFT display");
        return;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate WiFi visualization canvas");
        return;
    }

    Serial.println("[WiFi Viz] Showing channel distribution and signal strength");

    bool done = false;
    while (!done) {
        canvas.fillSprite(Theme::COLOR_BG);

        // Title
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("WiFi NETWORKS - Channel Distribution", Theme::SPACE_2, Theme::SPACE_2 + 10);
        canvas.unloadFont();

        // Histogram: channel activity
        std::vector<String> channelLabels;
        for (int i = 0; i < 14; i++) {
            channelLabels.push_back(String(i + 1));
        }

        AdvancedWidgets::histogram(canvas, Theme::SPACE_1, Theme::SPACE_3 + 10,
                                   canvas.width() - Theme::SPACE_2, 100,
                                   data.channelCounts, channelLabels);

        // Gauge: Strongest signal
        int gaugeX = canvas.width() / 2;
        int gaugeY = Theme::SPACE_3 + 125;
        float fraction = (data.strongestRssi + 100) / 70.0f;  // -100 to -30 dBm scale
        if (fraction < 0) fraction = 0;
        if (fraction > 1) fraction = 1;

        AdvancedWidgets::gauge(canvas, gaugeX, gaugeY, 25, fraction, "Signal");

        // Info text
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        String infoStr = "Networks: " + String(data.totalNetworks) + " | Strongest: " +
                        data.strongestSsid + " (" + String(data.strongestRssi) + " dBm)";
        canvas.drawString(infoStr, Theme::SPACE_2, canvas.height() - 30);

        canvas.drawString("Press any button to continue...", Theme::SPACE_2, canvas.height() - 8);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);

        if (Buttons::poll() != Buttons::NONE) done = true;
        delay(30);
    }

    canvas.deleteSprite();
}

// ---- BLE Visualization ----
void showBleVisualization(const BleScanData &data) {
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("BLE visualization requires TFT display");
        return;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate BLE visualization canvas");
        return;
    }

    Serial.println("[BLE Viz] Showing device distribution and signal strength");

    bool done = false;
    while (!done) {
        canvas.fillSprite(Theme::COLOR_BG);

        // Title
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("BLE DEVICES - Signal Strength Distribution", Theme::SPACE_2, Theme::SPACE_2 + 10);
        canvas.unloadFont();

        // Histogram: RSSI distribution (weak/fair/strong)
        auto bins = createRssiHistogram(data.rssiValues);
        std::vector<String> labels = {"Weak\n(-100 to -67)", "Fair\n(-67 to -50)", "Strong\n(-50 to -30)"};

        AdvancedWidgets::histogram(canvas, Theme::SPACE_1, Theme::SPACE_3 + 10,
                                   canvas.width() - Theme::SPACE_2, 100,
                                   bins, labels);

        // Gauge: Strongest signal
        int gaugeX = canvas.width() / 2;
        int gaugeY = Theme::SPACE_3 + 125;
        float fraction = (data.strongestRssi + 100) / 70.0f;
        if (fraction < 0) fraction = 0;
        if (fraction > 1) fraction = 1;

        AdvancedWidgets::gauge(canvas, gaugeX, gaugeY, 25, fraction, "Best");

        // Pulsing indicator if scanning
        if (data.scanning) {
            AdvancedWidgets::pulsingIndicator(canvas, 60, gaugeY, 6, millis(), Theme::COLOR_ACCENT);
        }

        // Info text
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        String infoStr = "Devices: " + String(data.totalDevices) + " | Strongest: " +
                        data.strongestDevice + " (" + String(data.strongestRssi) + " dBm)";
        canvas.drawString(infoStr, Theme::SPACE_2, canvas.height() - 30);

        String scanStatus = data.scanning ? "Scanning active..." : "Scan complete";
        canvas.drawString(scanStatus, Theme::SPACE_2, canvas.height() - 18);

        canvas.drawString("Press any button to continue...", Theme::SPACE_2, canvas.height() - 8);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);

        if (Buttons::poll() != Buttons::NONE) done = true;
        delay(30);
    }

    canvas.deleteSprite();
}

// ---- Spectrum Visualization ----
void showSpectrumVisualization(const SpectrumData &data) {
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("Spectrum visualization requires TFT display");
        return;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate spectrum visualization canvas");
        return;
    }

    Serial.println("[Spectrum Viz] Showing frequency spectrum with waterfall");

    bool done = false;
    while (!done) {
        canvas.fillSprite(Theme::COLOR_BG);

        // Title
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("SPECTRUM ANALYSIS - Frequency Activity", Theme::SPACE_2, Theme::SPACE_2 + 10);
        canvas.unloadFont();

        // Waterfall: Frequency spectrum
        AdvancedWidgets::waterfall(canvas, Theme::SPACE_2, Theme::SPACE_3 + 10,
                                   canvas.width() - Theme::SPACE_2 * 2, 110,
                                   data.frequencyBins, 32);

        // Gauge: Dominant amplitude
        int gaugeX = canvas.width() / 2;
        int gaugeY = Theme::SPACE_3 + 135;
        float fraction = (data.dominantAmplitude + 100) / 100.0f;  // Normalize to 0-1
        if (fraction < 0) fraction = 0;
        if (fraction > 1) fraction = 1;

        AdvancedWidgets::gauge(canvas, gaugeX, gaugeY, 20, fraction, "Amp");

        // Info text
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        String freqStr = String(data.dominantFrequency, 1) + " MHz";
        String infoStr = "Peaks: " + String(data.peaksFound) + " | Dominant: " + freqStr +
                        " @ " + String(data.dominantAmplitude) + " dBm";
        canvas.drawString(infoStr, Theme::SPACE_2, canvas.height() - 30);

        canvas.drawString("Color: Blue→Red = Weak→Strong signal", Theme::SPACE_2, canvas.height() - 18);
        canvas.drawString("Press any button to continue...", Theme::SPACE_2, canvas.height() - 8);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);

        if (Buttons::poll() != Buttons::NONE) done = true;
        delay(30);
    }

    canvas.deleteSprite();
}

} // namespace ScanVisualizations
