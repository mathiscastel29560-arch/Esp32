#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

// Advanced reusable widgets for premium visualization on TFT display.
// Each widget is a pure drawing function: takes data and draws into a sprite.
// No state, no side effects. All colors/spacing come from theme.h.
namespace AdvancedWidgets {

// ---- 1. WATERFALL GRAPH ----
// Time-series visualization: vertical axis = time (scrolling), each row = signal snapshot.
// Used for: Frequency spectrograms, RF activity over time, signal monitoring.
//
// Example usage:
//   std::vector<uint8_t> spectrum = {50, 65, 80, 72, 55, 40, 35, 30, ...};  // 0-255 scale
//   AdvancedWidgets::waterfall(sprite, 10, 50, 300, 150, spectrum, 32);
void waterfall(TFT_eSprite &canvas, int16_t x, int16_t y, int16_t w, int16_t h,
               const std::vector<uint8_t> &frequencyData,  // 0-255 intensity per frequency bin
               uint8_t numBins);                            // number of frequency bins

// ---- 2. HISTOGRAM / SPECTRUM GRAPH ----
// Vertical bar chart: one bar per category, height = value.
// Used for: Channel occupancy, frequency distribution, signal strength by device type.
//
// Example usage:
//   std::vector<uint16_t> channels = {45, 120, 80, 35, 60, 90, 25};  // Count per channel
//   std::vector<String> labels = {"1", "6", "11", "14", ...};
//   AdvancedWidgets::histogram(sprite, 20, 60, 280, 120, channels, labels);
void histogram(TFT_eSprite &canvas, int16_t x, int16_t y, int16_t w, int16_t h,
               const std::vector<uint16_t> &values,        // Bar heights (auto-scaled)
               const std::vector<String> &labels = {});    // Optional bar labels

// ---- 3. HEATMAP ----
// 2D intensity grid: rows = time, columns = frequency, color = intensity.
// Used for: RF power over time/frequency, network activity matrix, signal patterns.
//
// Example usage:
//   std::vector<std::vector<uint8_t>> matrix = { {10, 50, 90, 40}, ... };  // rows x cols
//   AdvancedWidgets::heatmap(sprite, 20, 80, 280, 100, matrix);
void heatmap(TFT_eSprite &canvas, int16_t x, int16_t y, int16_t w, int16_t h,
             const std::vector<std::vector<uint8_t>> &intensityGrid);  // 0-255 per cell

// ---- 4. GAUGE ----
// Circular progress indicator: arc fills from 0° to current value.
// Used for: Signal quality, battery, link quality, operation progress.
//
// Example usage:
//   AdvancedWidgets::gauge(sprite, 160, 120, 60, 75, "RSSI");  // Center at 160,120; radius 60; 75% full
void gauge(TFT_eSprite &canvas, int16_t cx, int16_t cy, int16_t radius,
           float fraction,      // 0.0 (empty) to 1.0 (full)
           const String &label = "");  // Optional label in center

// ---- 5. PULSING INDICATOR ----
// Animated dot with gentle pulse (for "scanning" / "transmitting" states).
// Used for: Active tool indication, real-time status, operation in progress.
//
// Example usage:
//   uint32_t nowMs = millis();
//   AdvancedWidgets::pulsingIndicator(sprite, 280, 40, 8, nowMs, Theme::COLOR_DANGER);
void pulsingIndicator(TFT_eSprite &canvas, int16_t x, int16_t y, int16_t radius,
                      uint32_t nowMs,           // Current time for animation
                      uint16_t activeColor);    // Color when pulsing

// ---- HELPER: Intensity to Color Mapping ----
// Converts 0-255 intensity to semantic color using blue→cyan→green→yellow→red gradient.
// Used internally by waterfall/heatmap, but exposed for custom visualizations.
//
// Example usage:
//   uint16_t color = intensityToColor(128);  // Half intensity → cyan
uint16_t intensityToColor(uint8_t intensity);  // 0-255 input

} // namespace AdvancedWidgets
