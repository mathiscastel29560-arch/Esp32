#include "ui/advanced_scanning.h"
#include "ui/advanced_widgets.h"
#include "ui/widgets.h"
#include "display.h"
#include "buttons.h"
#include <TFT_eSPI.h>
#include <algorithm>
#include <cmath>

namespace AdvancedScanning {

namespace {
TFT_eSPI &tft = Display::raw();
TFT_eSprite canvas(&tft);
}

// ---- Live Scanning Progress ----
bool showScanProgress(const ScanProgress &progress) {
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("Scan progress requires TFT display");
        return false;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate scan progress canvas");
        return false;
    }

    uint32_t startMs = millis();
    bool buttonPressed = false;

    while (millis() - startMs < progress.totalDurationMs && !buttonPressed) {
        canvas.fillSprite(Theme::COLOR_BG);

        // Title
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString(progress.title, canvas.width() / 2, Theme::SPACE_2 + 10);
        canvas.unloadFont();

        // Progress bar
        uint32_t elapsed = millis() - startMs;
        float progressFraction = (float)elapsed / progress.totalDurationMs;
        if (progressFraction > 1.0f) progressFraction = 1.0f;

        int barX = Theme::SPACE_2;
        int barY = 80;
        int barW = canvas.width() - Theme::SPACE_2 * 2;
        int barH = 20;

        // Background bar
        canvas.drawRoundRect(barX, barY, barW, barH, Theme::RADIUS_SM, Theme::COLOR_TEXT_DIM);
        // Progress fill
        int fillW = (int)(barW * progressFraction);
        canvas.fillRoundRect(barX, barY, fillW, barH, Theme::RADIUS_SM, Theme::COLOR_ACCENT);

        // Pulsing indicator
        int indicatorX = barX + barW / 2;
        int indicatorY = barY + barH / 2 + 40;
        AdvancedWidgets::pulsingIndicator(canvas, indicatorX, indicatorY, 8, millis(), Theme::COLOR_ACCENT);

        // Info text
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString("Devices: " + String(progress.devicesFound), canvas.width() / 2, 140);
        canvas.drawString(String((int)(progressFraction * 100)) + "% complete", canvas.width() / 2, 160);
        canvas.drawString("Press any button to skip", canvas.width() / 2, canvas.height() - 8);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);

        if (Buttons::poll() != Buttons::NONE) buttonPressed = true;
        delay(30);
    }

    canvas.deleteSprite();
    return buttonPressed;
}

// ---- Network Devices Visualization ----
void showNetworkDevices(const NetworkDeviceData &data) {
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("Network visualization requires TFT display");
        return;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate network canvas");
        return;
    }

    bool done = false;
    while (!done) {
        canvas.fillSprite(Theme::COLOR_BG);

        // Title
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString(data.title + " - Distribution", Theme::SPACE_2, Theme::SPACE_2 + 10);
        canvas.unloadFont();

        // Histogram of device types
        if (!data.devicesByType.empty() && !data.typeLabels.empty()) {
            size_t maxBars = std::min(data.devicesByType.size(), data.typeLabels.size());
            std::vector<uint16_t> bars(data.devicesByType.begin(),
                                       data.devicesByType.begin() + maxBars);
            std::vector<String> labels(data.typeLabels.begin(),
                                       data.typeLabels.begin() + maxBars);

            AdvancedWidgets::histogram(canvas, Theme::SPACE_1, Theme::SPACE_3 + 10,
                                      canvas.width() - Theme::SPACE_2, 100,
                                      bars, labels);
        }

        // Gauge: Strongest signal
        int gaugeX = canvas.width() / 2;
        int gaugeY = Theme::SPACE_3 + 125;
        float fraction = (data.strongestSignal + 100) / 70.0f;
        if (fraction < 0) fraction = 0;
        if (fraction > 1) fraction = 1;

        AdvancedWidgets::gauge(canvas, gaugeX, gaugeY, 25, fraction, "Signal");

        // Info
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        String infoStr = "Total: " + String(data.totalDevices) + " | Strongest: " +
                        data.strongestDeviceName + " (" + String(data.strongestSignal) + " dBm)";
        canvas.drawString(infoStr, Theme::SPACE_2, canvas.height() - 30);
        canvas.drawString("Press any button to continue...", Theme::SPACE_2, canvas.height() - 8);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);

        if (Buttons::poll() != Buttons::NONE) done = true;
        delay(30);
    }

    canvas.deleteSprite();
}

// ---- Signal Timeline ----
void showSignalTimeline(const SignalTimeline &data) {
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("Timeline visualization requires TFT display");
        return;
    }

    if (data.rssiHistory.empty()) {
        Serial.println("No signal history to display");
        return;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate timeline canvas");
        return;
    }

    bool done = false;
    while (!done) {
        canvas.fillSprite(Theme::COLOR_BG);

        // Title
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("SIGNAL QUALITY - " + data.deviceName, Theme::SPACE_2, Theme::SPACE_2 + 10);
        canvas.unloadFont();

        // Simple bar graph: RSSI values as vertical bars
        int graphX = Theme::SPACE_2;
        int graphY = Theme::SPACE_3 + 10;
        int graphW = canvas.width() - Theme::SPACE_2 * 2;
        int graphH = 120;

        // Border
        canvas.drawRect(graphX, graphY, graphW, graphH, Theme::COLOR_TEXT_DIM);

        // Find RSSI range
        int32_t minRssi = *std::min_element(data.rssiHistory.begin(), data.rssiHistory.end());
        int32_t maxRssi = *std::max_element(data.rssiHistory.begin(), data.rssiHistory.end());
        if (minRssi == maxRssi) minRssi = -100;  // Default range if all values same

        int rssiRange = maxRssi - minRssi;
        if (rssiRange == 0) rssiRange = 1;

        // Draw RSSI bars
        size_t maxBars = std::min(data.rssiHistory.size(), (size_t)(graphW / 4));
        int barSpacing = graphW / (maxBars > 0 ? maxBars : 1);

        for (size_t i = 0; i < maxBars; i++) {
            int32_t rssi = data.rssiHistory[i];
            uint8_t intensity = (uint8_t)((rssi - minRssi) * 255 / rssiRange);
            uint16_t color = AdvancedWidgets::intensityToColor(intensity);

            int barX = graphX + (i * barSpacing) + 2;
            int barHeight = (graphH - 4) * intensity / 255;
            int barY = graphY + graphH - barHeight - 2;

            canvas.fillRect(barX, barY, barSpacing - 2, barHeight, color);
        }

        // Info
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString("RSSI: " + String(minRssi) + " to " + String(maxRssi) + " dBm | Samples: " +
                         String(data.rssiHistory.size()), Theme::SPACE_2, canvas.height() - 30);
        canvas.drawString("Press any button to continue...", Theme::SPACE_2, canvas.height() - 8);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);

        if (Buttons::poll() != Buttons::NONE) done = true;
        delay(30);
    }

    canvas.deleteSprite();
}

// ---- Concurrent Scans ----
void showConcurrentScans(const ConcurrentScanProgress &data) {
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("Concurrent scans view requires TFT display");
        return;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate concurrent scans canvas");
        return;
    }

    uint32_t startMs = millis();
    bool done = false;

    while (!done && millis() - startMs < 30000) {  // Show for max 30 seconds
        canvas.fillSprite(Theme::COLOR_BG);

        // Title
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("NETWORK SCANNING", canvas.width() / 2, Theme::SPACE_2 + 10);
        canvas.unloadFont();

        // Three scan progress sections
        int sectionY = Theme::SPACE_3 + 10;
        int sectionH = 40;

        // WiFi
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("WiFi", Theme::SPACE_2, sectionY + 5);
        canvas.unloadFont();

        int barX = 80;
        int barW = canvas.width() - barX - Theme::SPACE_2;
        int barY = sectionY + 3;
        int barH = 12;

        canvas.drawRect(barX, barY, barW, barH, Theme::COLOR_TEXT_DIM);
        int fillW = (int)(barW * data.wifiProgress);
        canvas.fillRect(barX, barY, fillW, barH, Theme::COLOR_ACCENT);

        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MR_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString(data.wifiStatus, canvas.width() - Theme::SPACE_2, sectionY + 5);
        canvas.unloadFont();

        // BLE
        sectionY += sectionH;
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("BLE", Theme::SPACE_2, sectionY + 5);
        canvas.unloadFont();

        barY = sectionY + 3;
        canvas.drawRect(barX, barY, barW, barH, Theme::COLOR_TEXT_DIM);
        fillW = (int)(barW * data.bleProgress);
        canvas.fillRect(barX, barY, fillW, barH, Theme::COLOR_OK);

        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MR_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString(data.bleStatus, canvas.width() - Theme::SPACE_2, sectionY + 5);
        canvas.unloadFont();

        // RF/Spectrum
        sectionY += sectionH;
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("Spectrum", Theme::SPACE_2, sectionY + 5);
        canvas.unloadFont();

        barY = sectionY + 3;
        canvas.drawRect(barX, barY, barW, barH, Theme::COLOR_TEXT_DIM);
        fillW = (int)(barW * data.rfProgress);
        canvas.fillRect(barX, barY, fillW, barH, Theme::COLOR_WARN);

        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MR_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString(data.rfStatus, canvas.width() - Theme::SPACE_2, sectionY + 5);
        canvas.unloadFont();

        // Help text
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString("Press any button to continue...", canvas.width() / 2, canvas.height() - 8);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);

        if (Buttons::poll() != Buttons::NONE) done = true;
        delay(30);
    }

    canvas.deleteSprite();
}

// ---- Scan Indicator ----
void drawScanIndicator(int x, int y, ScanState state, uint32_t nowMs) {
    switch (state) {
        case SCAN_IDLE:
            // Static circle
            canvas.drawCircle(x, y, 4, Theme::COLOR_TEXT_DIM);
            break;

        case SCAN_ACTIVE: {
            // Pulsing indicator using AdvancedWidgets
            AdvancedWidgets::pulsingIndicator(canvas, x, y, 4, nowMs, Theme::COLOR_ACCENT);
            break;
        }

        case SCAN_COMPLETE:
            // Filled circle with checkmark effect
            canvas.fillCircle(x, y, 4, Theme::COLOR_OK);
            canvas.drawCircle(x, y, 4, Theme::COLOR_ACCENT);
            break;

        case SCAN_ERROR:
            // Red indicator
            canvas.fillCircle(x, y, 4, Theme::COLOR_DANGER);
            canvas.drawCircle(x, y, 4, Theme::COLOR_ACCENT);
            break;
    }
}

// ---- Detailed RSSI Histogram ----
std::vector<uint16_t> createDetailedRssiHistogram(const std::vector<int32_t> &rssiValues) {
    // 5 bins: excellent / good / fair / weak / very weak
    // Excellent: -30 to -50
    // Good: -50 to -67
    // Fair: -67 to -80
    // Weak: -80 to -95
    // Very weak: < -95
    std::vector<uint16_t> bins(5, 0);

    for (int32_t rssi : rssiValues) {
        if (rssi >= -50) bins[0]++;           // Excellent
        else if (rssi >= -67) bins[1]++;      // Good
        else if (rssi >= -80) bins[2]++;      // Fair
        else if (rssi >= -95) bins[3]++;      // Weak
        else bins[4]++;                        // Very weak
    }

    return bins;
}

} // namespace AdvancedScanning
