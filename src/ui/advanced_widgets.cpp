#include "ui/advanced_widgets.h"
#include "ui/theme.h"
#include <cmath>

namespace AdvancedWidgets {

namespace {
// Helper: Blend two RGB565 colors
uint8_t r565(uint16_t c) { return (c >> 11) & 0x1F; }
uint8_t g565(uint16_t c) { return (c >> 5) & 0x3F; }
uint8_t b565(uint16_t c) { return c & 0x1F; }

uint16_t blend565(uint16_t a, uint16_t b, float t) {
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    uint8_t r = r565(a) + (uint8_t)((r565(b) - (int)r565(a)) * t);
    uint8_t g = g565(a) + (uint8_t)((g565(b) - (int)g565(a)) * t);
    uint8_t bl = b565(a) + (uint8_t)((b565(b) - (int)b565(a)) * t);
    return (r << 11) | (g << 5) | bl;
}
} // namespace

// ---- Intensity to Color: Blue → Cyan → Green → Yellow → Red ----
uint16_t intensityToColor(uint8_t intensity) {
    float t = intensity / 255.0f;

    // 5-point gradient:
    // 0.0 (blue) → 0.25 (cyan) → 0.5 (green) → 0.75 (yellow) → 1.0 (red)

    if (t < 0.25f) {
        // Blue → Cyan
        float local = t / 0.25f;
        return blend565(0x0019, 0x07FF, local);  // #00001F → #00FFFF
    } else if (t < 0.5f) {
        // Cyan → Green
        float local = (t - 0.25f) / 0.25f;
        return blend565(0x07FF, Theme::COLOR_OK, local);  // #00FFFF → #2BD420
    } else if (t < 0.75f) {
        // Green → Yellow
        float local = (t - 0.5f) / 0.25f;
        return blend565(Theme::COLOR_OK, 0xFFA0, local);  // #2BD420 → #FFFF00
    } else {
        // Yellow → Red
        float local = (t - 0.75f) / 0.25f;
        return blend565(0xFFA0, Theme::COLOR_DANGER, local);  // #FFFF00 → #FF4008
    }
}

// ---- 1. WATERFALL GRAPH ----
void waterfall(TFT_eSprite &canvas, int16_t x, int16_t y, int16_t w, int16_t h,
               const std::vector<uint8_t> &frequencyData, uint8_t numBins) {
    if (frequencyData.empty() || numBins == 0) return;

    // Draw background border
    canvas.drawRect(x, y, w, h, Theme::COLOR_TEXT_DIM);

    // Calculate pixel width per bin
    int16_t pixelsPerBin = w / numBins;
    if (pixelsPerBin < 1) pixelsPerBin = 1;

    // Draw frequency data as columns (simulating scrolling waterfall effect)
    // Each column = one frequency bin, height = intensity
    for (size_t bin = 0; bin < frequencyData.size() && bin < numBins; bin++) {
        uint8_t intensity = frequencyData[bin];
        uint16_t color = intensityToColor(intensity);

        int16_t barX = x + (bin * pixelsPerBin);
        int16_t barW = pixelsPerBin;
        int16_t barH = (int16_t)(h * intensity / 255.0f);

        // Fill from bottom up
        canvas.fillRect(barX, y + h - barH, barW, barH, color);
    }

    // Top border line to separate from other content
    canvas.drawLine(x, y, x + w, y, Theme::COLOR_ACCENT);
}

// ---- 2. HISTOGRAM ----
void histogram(TFT_eSprite &canvas, int16_t x, int16_t y, int16_t w, int16_t h,
               const std::vector<uint16_t> &values, const std::vector<String> &labels) {
    if (values.empty()) return;

    // Find max value for scaling
    uint16_t maxValue = *std::max_element(values.begin(), values.end());
    if (maxValue == 0) maxValue = 1;

    // Draw background border
    canvas.drawRect(x, y, w, h, Theme::COLOR_TEXT_DIM);
    canvas.drawLine(x, y + h - 1, x + w - 1, y + h - 1, Theme::COLOR_SURFACE_ALT);  // Baseline

    // Calculate pixel width per bar
    int16_t pixelsPerBar = w / values.size();
    if (pixelsPerBar < 2) pixelsPerBar = 2;

    // Draw bars
    for (size_t i = 0; i < values.size(); i++) {
        uint16_t value = values[i];

        // Scale height to fit in box
        int16_t barH = (int16_t)(h * value / (float)maxValue);
        if (barH < 1 && value > 0) barH = 1;

        int16_t barX = x + (i * pixelsPerBar) + 1;
        int16_t barW = pixelsPerBar - 2;
        int16_t barY = y + h - barH;

        // Use semantic color based on height ratio
        float ratio = value / (float)maxValue;
        uint16_t color = Theme::COLOR_DANGER;
        if (ratio < 0.33f) color = Theme::COLOR_DANGER;
        else if (ratio < 0.66f) color = Theme::COLOR_WARN;
        else color = Theme::COLOR_OK;

        // Draw bar with rounded top
        canvas.fillRect(barX, barY, barW, barH, color);

        // Optional: Draw label below bar
        if (!labels.empty() && i < labels.size()) {
            canvas.loadFont(FONT_BODY);
            canvas.setTextDatum(MC_DATUM);
            canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
            canvas.drawString(labels[i], barX + barW / 2, y + h + 12);
            canvas.unloadFont();
        }
    }
}

// ---- 3. HEATMAP ----
void heatmap(TFT_eSprite &canvas, int16_t x, int16_t y, int16_t w, int16_t h,
             const std::vector<std::vector<uint8_t>> &intensityGrid) {
    if (intensityGrid.empty()) return;

    size_t rows = intensityGrid.size();
    size_t cols = intensityGrid[0].size();
    if (cols == 0) return;

    // Calculate cell size
    int16_t cellW = w / cols;
    int16_t cellH = h / rows;

    if (cellW < 1) cellW = 1;
    if (cellH < 1) cellH = 1;

    // Draw grid
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++) {
            if (c >= intensityGrid[r].size()) continue;

            uint8_t intensity = intensityGrid[r][c];
            uint16_t color = intensityToColor(intensity);

            int16_t cellX = x + (c * cellW);
            int16_t cellY = y + (r * cellH);

            canvas.fillRect(cellX, cellY, cellW, cellH, color);
            canvas.drawRect(cellX, cellY, cellW, cellH, blend565(color, Theme::COLOR_BG, 0.5f));
        }
    }

    // Border
    canvas.drawRect(x, y, w, h, Theme::COLOR_TEXT_DIM);
}

// ---- 4. GAUGE ----
void gauge(TFT_eSprite &canvas, int16_t cx, int16_t cy, int16_t radius,
           float fraction, const String &label) {
    if (fraction < 0) fraction = 0;
    if (fraction > 1) fraction = 1;

    // Draw background circle (empty)
    canvas.drawCircle(cx, cy, radius, Theme::COLOR_TEXT_DIM);

    // Draw filled arc based on fraction
    int16_t startAngle = -90;  // Start at top
    int16_t endAngle = startAngle + (int16_t)(360 * fraction);

    // Draw arc in semantic color
    uint16_t arcColor = Theme::COLOR_DANGER;
    if (fraction < 0.33f) arcColor = Theme::COLOR_DANGER;
    else if (fraction < 0.66f) arcColor = Theme::COLOR_WARN;
    else arcColor = Theme::COLOR_OK;

    canvas.drawArc(cx, cy, radius, radius - 3, startAngle, endAngle, arcColor, Theme::COLOR_BG, true);

    // Draw center circle and percentage text
    canvas.fillCircle(cx, cy, radius - 6, Theme::COLOR_SURFACE);
    canvas.drawCircle(cx, cy, radius - 6, arcColor);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(MC_DATUM);
    canvas.setTextColor(arcColor, Theme::COLOR_SURFACE);
    String percentStr = String((int)(fraction * 100)) + "%";
    canvas.drawString(percentStr, cx, cy - 6);

    if (label.length()) {
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_SURFACE);
        canvas.drawString(label, cx, cy + 10);
    }
    canvas.unloadFont();
}

// ---- 5. PULSING INDICATOR ----
void pulsingIndicator(TFT_eSprite &canvas, int16_t x, int16_t y, int16_t radius,
                      uint32_t nowMs, uint16_t activeColor) {
    // Gentle pulse: oscillates radius between radius and radius*1.3 every 400ms
    float phase = (nowMs % Theme::ANIM_PULSE_MS) / (float)Theme::ANIM_PULSE_MS;

    // Sine wave for smooth pulse
    float pulse = 0.5f + 0.5f * cosf(phase * 2.0f * PI);  // 0.5 to 1.5
    int16_t pulseRadius = (int16_t)(radius * (1.0f + pulse * 0.3f));

    // Draw pulsing dot
    canvas.fillCircle(x, y, pulseRadius, activeColor);

    // Optional: glow effect (faint outer ring)
    canvas.drawCircle(x, y, pulseRadius + 1, blend565(activeColor, Theme::COLOR_BG, 0.6f));
}

} // namespace AdvancedWidgets
