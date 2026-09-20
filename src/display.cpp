#include "display.h"
#include "config.h"
#include "mascot.h"
#include "ui/theme.h"
#include <TFT_eSPI.h>

namespace {
TFT_eSPI tft = TFT_eSPI(); // pins/driver set via platformio.ini build_flags

// TFT_eSPI's drawXBitmap draws a 1bpp bitmap at native size (like Adafruit's
// drawBitmap); it has no scale factor, so scaling is done by hand here,
// one filled block per set bit. Fine for a tiny 32x16 mascot. Only used by
// the transient "booting..." placeholder below — ui.cpp has its own copy
// for the real (sprite-based) splash/home screens.
void drawBitmapScaled(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h,
                       uint8_t scale, uint16_t color) {
    uint8_t bytesPerRow = (w + 7) / 8;
    for (uint8_t row = 0; row < h; row++) {
        for (uint8_t col = 0; col < w; col++) {
            uint8_t b = pgm_read_byte(&bitmap[row * bytesPerRow + col / 8]);
            if (b & (0x80 >> (col % 8))) {
                tft.fillRect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}
}

namespace Display {

TFT_eSPI &raw() { return tft; }

void begin() {
    tft.init();
    tft.setRotation(Theme::ROTATION); // single source of truth for orientation, see ui/theme.h
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // Transient placeholder until Ui::begin() + Ui::showSplash() take over
    // a few lines into setup() — drawn directly (no sprite yet), gone
    // within well under a second.
    drawBitmapScaled((tft.width() - Mascot::WIDTH * 3) / 2, 10, Mascot::SHARK_BITMAP,
                      Mascot::WIDTH, Mascot::HEIGHT, 3, TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, Mascot::HEIGHT * 3 + 20);
    tft.println("ESP32 Audit Tool");
    tft.println("booting...");
}

} // namespace Display
