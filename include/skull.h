#pragma once
#include <Arduino.h>
#include <math.h>

// Small pixel-art skull, spun in place (flat, in-plane rotation, like a
// coin) for the boot splash -- see ui/ui.cpp and ui/oled_ui.cpp. One
// bitmap + one rotation routine shared by both the TFT and OLED splash so
// they stay visually consistent instead of hand-animating two versions.
namespace Skull {

extern const uint8_t BITMAP[] PROGMEM;
constexpr uint8_t WIDTH = 24;
constexpr uint8_t HEIGHT = 24;

inline bool pixelSet(uint8_t col, uint8_t row) {
    constexpr uint8_t bytesPerRow = (WIDTH + 7) / 8;
    uint8_t b = pgm_read_byte(&BITMAP[row * bytesPerRow + col / 8]);
    return b & (0x80 >> (col % 8));
}

// Draws the skull rotated by `angleRad` around its own center, scaled by
// `scale`, centered on-screen at (cx, cy). Walks the destination pixels
// and samples back into the source bitmap via the inverse rotation
// (nearest-neighbor) so the spin has no gaps, then calls `plot(x, y)` for
// every pixel that should be lit -- the caller decides how to actually
// set that pixel (TFT sprite vs. OLED framebuffer).
template <typename PlotFn>
void drawRotated(int16_t cx, int16_t cy, float angleRad, uint8_t scale, PlotFn plot) {
    float c = cosf(angleRad);
    float s = sinf(angleRad);
    float halfW = WIDTH / 2.0f;
    float halfH = HEIGHT / 2.0f;
    int16_t destHalf = (int16_t)(sqrtf(halfW * halfW + halfH * halfH) * scale) + 1;

    for (int16_t dy = -destHalf; dy <= destHalf; dy++) {
        for (int16_t dx = -destHalf; dx <= destHalf; dx++) {
            float sx = (c * dx + s * dy) / scale + halfW;
            float sy = (-s * dx + c * dy) / scale + halfH;
            int16_t col = (int16_t)floorf(sx);
            int16_t row = (int16_t)floorf(sy);
            if (col < 0 || col >= WIDTH || row < 0 || row >= HEIGHT) continue;
            if (pixelSet((uint8_t)col, (uint8_t)row)) plot(cx + dx, cy + dy);
        }
    }
}

} // namespace Skull
