#include "display.h"
#include "config.h"
#include "mascot.h"
#include "ui/theme.h"
#include <TFT_eSPI.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>

namespace {
TFT_eSPI tft = TFT_eSPI(); // pins/driver set via platformio.ini build_flags
Adafruit_SSD1306 oledDisplay(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
Display::ScreenKind g_kind = Display::ScreenKind::NONE;

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

bool i2cDevicePresent(uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

// Reads the ILI9341's own RDID4 self-ID register over SPI (byte at index
// 2 is the fixed value 0x93 for every real ILI9341 panel — this is
// TFT_eSPI's own documented self-test pattern, see its TFT_Read_Reg
// example). A floating/absent MISO line reads back 0x00 or 0xFF instead,
// so this doubles as a "is a real panel actually wired" check, not just
// "did tft.init() crash" -- it can't, since this is a plain SPI
// transaction, not a re-init.
bool probeIli9341() { return tft.readcommand8(ILI9341_RDID4, 2) == 0x93; }
} // namespace

namespace Display {

ScreenKind kind() { return g_kind; }
TFT_eSPI &raw() { return tft; }
Adafruit_SSD1306 &oled() { return oledDisplay; }

void begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    if (i2cDevicePresent(OLED_I2C_ADDR) && oledDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        g_kind = ScreenKind::OLED;
        oledDisplay.clearDisplay();
        oledDisplay.setTextColor(SSD1306_WHITE);
        oledDisplay.setTextSize(1);
        oledDisplay.setCursor(0, 0);
        oledDisplay.println("ESP32 Audit Tool");
        oledDisplay.println("booting...");
        oledDisplay.display();

        // No TFT_eSPI init in this branch, so nothing else has called
        // SPI.begin() yet -- CC1101/NRF24 still need the shared SPI bus.
        SPI.begin(PIN_SPI_CLK, PIN_SPI_MISO, PIN_SPI_MOSI);
        return;
    }

    // No OLED answering on I2C -- try the TFT next. Deliberately NOT
    // calling SPI.begin() ourselves first: tft.init() already calls it
    // internally on the same shared global SPI object, and calling it
    // twice was the likely cause of a StoreProhibited crash in
    // begin_tft_write() when no physical panel was attached (TFT_eSPI's
    // `spi` reference resolves to the same global `SPI` instance on this
    // board/driver config).
    tft.init();

    if (probeIli9341()) {
        g_kind = ScreenKind::TFT;
        tft.setRotation(Theme::ROTATION); // single source of truth for orientation, see ui/theme.h
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        // Transient placeholder until Ui::begin() + Ui::showSplash() take
        // over a few lines into setup() — drawn directly (no sprite yet).
        drawBitmapScaled((tft.width() - Mascot::WIDTH * 3) / 2, 10, Mascot::SHARK_BITMAP,
                          Mascot::WIDTH, Mascot::HEIGHT, 3, TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(0, Mascot::HEIGHT * 3 + 20);
        tft.println("ESP32 Audit Tool");
        tft.println("booting...");
        return;
    }

    // Neither screen responds -- headless. tft.init() above already
    // called SPI.begin() on the shared bus once, so CC1101/NRF24 are
    // still covered even though the TFT itself isn't there.
    g_kind = ScreenKind::NONE;
    Serial.println("[display] no screen detected (TFT or OLED) -- running headless");
}

} // namespace Display
