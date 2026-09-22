#include "ui/advanced_demo.h"
#include "ui/theme.h"
#include "ui/advanced_widgets.h"
#include "display.h"
#include "buttons.h"
#include <TFT_eSPI.h>

namespace AdvancedDemo {

namespace {
TFT_eSPI &tft = Display::raw();
TFT_eSprite canvas(&tft);

void drawWaterfallDemo() {
    canvas.fillSprite(Theme::COLOR_BG);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("WATERFALL GRAPH (Spectrum)", Theme::SPACE_2, Theme::SPACE_2 + 10);
    canvas.unloadFont();

    // Simulate RF spectrum data (32 frequency bins, 0-255 intensity)
    std::vector<uint8_t> spectrum;
    for (int i = 0; i < 32; i++) {
        // Create peaks at different frequencies
        uint8_t intensity = 40;
        if (i >= 8 && i <= 12) intensity = 200;  // Peak 1
        if (i >= 18 && i <= 22) intensity = 150; // Peak 2
        if (i >= 26 && i <= 28) intensity = 100; // Peak 3
        // Add some noise
        intensity += ((esp_random() % 30) - 15);
        if (intensity < 0) intensity = 0;
        if (intensity > 255) intensity = 255;
        spectrum.push_back(intensity);
    }

    AdvancedWidgets::waterfall(canvas, Theme::SPACE_2, Theme::SPACE_3 + 10,
                               canvas.width() - Theme::SPACE_2 * 2, 140, spectrum, 32);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString("Frequency bins (left to right) | Intensity (bottom to top)",
                      Theme::SPACE_2, canvas.height() - 20);
    canvas.drawString("Press any button to continue...", Theme::SPACE_2, canvas.height() - 8);
    canvas.unloadFont();

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}

void drawHistogramDemo() {
    canvas.fillSprite(Theme::COLOR_BG);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("HISTOGRAM (WiFi Channels)", Theme::SPACE_2, Theme::SPACE_2 + 10);
    canvas.unloadFont();

    // Simulate WiFi channel distribution (14 channels)
    std::vector<uint16_t> channelCounts = {45, 120, 80, 35, 60, 95, 25, 110, 70, 50, 40, 65, 30, 15};
    std::vector<String> channelLabels = {"1", "6", "11", "14", "", "", "", "", "", "", "", "", "", ""};

    AdvancedWidgets::histogram(canvas, Theme::SPACE_1, Theme::SPACE_3 + 10,
                               canvas.width() - Theme::SPACE_2, 120,
                               channelCounts, channelLabels);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString("Bar color: RED=weak, AMBER=fair, GREEN=strong",
                      Theme::SPACE_2, canvas.height() - 20);
    canvas.drawString("Press any button to continue...", Theme::SPACE_2, canvas.height() - 8);
    canvas.unloadFont();

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}

void drawHeatmapDemo() {
    canvas.fillSprite(Theme::COLOR_BG);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("HEATMAP (RF Power over Time)", Theme::SPACE_2, Theme::SPACE_2 + 10);
    canvas.unloadFont();

    // Simulate time/frequency matrix (16 time slices x 16 frequency bins)
    std::vector<std::vector<uint8_t>> heatmapData;
    for (int t = 0; t < 16; t++) {
        std::vector<uint8_t> row;
        for (int f = 0; f < 16; f++) {
            uint8_t intensity = 50;
            // Add activity pattern
            if (t > 4 && t < 12 && f > 6 && f < 10) {
                intensity = 200 - (abs(t - 8) * 15) - (abs(f - 8) * 10);
            }
            if (intensity < 0) intensity = 0;
            if (intensity > 255) intensity = 255;
            row.push_back(intensity);
        }
        heatmapData.push_back(row);
    }

    AdvancedWidgets::heatmap(canvas, Theme::SPACE_2, Theme::SPACE_3 + 10,
                             canvas.width() - Theme::SPACE_2 * 2, 120, heatmapData);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString("Color: BLUE=weak, GREEN=medium, RED=strong",
                      Theme::SPACE_2, canvas.height() - 20);
    canvas.drawString("Press any button to continue...", Theme::SPACE_2, canvas.height() - 8);
    canvas.unloadFont();

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}

void drawGaugeDemo() {
    canvas.fillSprite(Theme::COLOR_BG);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("GAUGE (Signal Quality Indicators)", Theme::SPACE_2, Theme::SPACE_2 + 10);
    canvas.unloadFont();

    // Draw 4 gauges at different fill levels
    int gaugeY = Theme::SPACE_3 + 40;

    // Poor (red)
    AdvancedWidgets::gauge(canvas, 50, gaugeY, 30, 0.25f, "Poor");

    // Fair (amber)
    AdvancedWidgets::gauge(canvas, 120, gaugeY, 30, 0.50f, "Fair");

    // Good (green)
    AdvancedWidgets::gauge(canvas, 190, gaugeY, 30, 0.75f, "Good");

    // Excellent (bright green)
    AdvancedWidgets::gauge(canvas, 260, gaugeY, 30, 1.00f, "Excellent");

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(MC_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString("Circular progress indicator with semantic coloring",
                      canvas.width() / 2, canvas.height() - 40);
    canvas.drawString("Press any button to continue...", canvas.width() / 2, canvas.height() - 8);
    canvas.unloadFont();

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}

void drawPulsingIndicatorDemo() {
    uint32_t startTime = millis();
    uint32_t demoDuration = 5000;  // Show for 5 seconds

    while (millis() - startTime < demoDuration) {
        canvas.fillSprite(Theme::COLOR_BG);

        uint32_t nowMs = millis();

        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString("PULSING INDICATOR (Activity Animation)", Theme::SPACE_2, Theme::SPACE_2 + 10);
        canvas.unloadFont();

        // Title labels
        int labelY = Theme::SPACE_3 + 20;
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString("Scanning", 60, labelY);
        canvas.drawString("Transmitting", 160, labelY);
        canvas.drawString("Receiving", 260, labelY);
        canvas.unloadFont();

        // Draw pulsing indicators
        int indicatorY = labelY + 50;
        AdvancedWidgets::pulsingIndicator(canvas, 60, indicatorY, 8, nowMs, Theme::COLOR_ACCENT);
        AdvancedWidgets::pulsingIndicator(canvas, 160, indicatorY, 8, nowMs, Theme::COLOR_DANGER);
        AdvancedWidgets::pulsingIndicator(canvas, 260, indicatorY, 8, nowMs, Theme::COLOR_OK);

        // Help text
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString("Animated dots show active operations", canvas.width() / 2, canvas.height() - 40);

        int elapsed = nowMs - startTime;
        int remaining = demoDuration - elapsed;
        canvas.drawString("Auto-advance in " + String(remaining / 1000 + 1) + "s, or press any button",
                          canvas.width() / 2, canvas.height() - 8);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);

        // Allow early exit
        if (Buttons::poll() != Buttons::NONE) break;
        delay(30);  // ~33fps
    }
}

} // namespace

void show() {
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("Advanced demo requires TFT display");
        return;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate advanced demo canvas");
        return;
    }

    Serial.println("\n[AdvancedDemo] Showing advanced widget demonstration...");
    Serial.println("  Press any button to advance between demos");

    drawWaterfallDemo();
    drawHistogramDemo();
    drawHeatmapDemo();
    drawGaugeDemo();
    drawPulsingIndicatorDemo();

    Serial.println("[AdvancedDemo] Advanced widget demo complete!");
}

} // namespace AdvancedDemo
