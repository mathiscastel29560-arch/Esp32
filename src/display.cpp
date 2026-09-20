#include "display.h"
#include "config.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "tx_arm.h"
#include "mascot.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <WiFi.h>

namespace {
Adafruit_ILI9341 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST); // uses the shared SPI bus set up in main.cpp

// Shark "swim-by" cameo on the idle status screen: mostly off, occasionally
// crosses the bottom row. INT16_MIN means "not currently swimming".
int16_t g_sharkX = INT16_MIN;
uint16_t g_ticksUntilSwim = 20; // ~20 status refreshes (~20s) between cameos
constexpr uint8_t SHARK_SCALE = 2;

// Adafruit_GFX's drawBitmap has no scale factor, so this draws each set bit
// as an NxN block. Fine for a tiny 32x16 mascot; not meant for big images.
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

bool advanceSharkAnimation() {
    const int16_t sharkW = Mascot::WIDTH * SHARK_SCALE;
    if (g_sharkX == INT16_MIN) {
        if (g_ticksUntilSwim > 0) {
            g_ticksUntilSwim--;
            return false;
        }
        g_sharkX = -sharkW;
    }

    drawBitmapScaled(g_sharkX, TFT_HEIGHT - Mascot::HEIGHT * SHARK_SCALE, Mascot::SHARK_BITMAP,
                      Mascot::WIDTH, Mascot::HEIGHT, SHARK_SCALE, ILI9341_WHITE);
    g_sharkX += 24;
    if (g_sharkX > TFT_WIDTH) {
        g_sharkX = INT16_MIN;
        g_ticksUntilSwim = 20;
    }
    return true;
}
}

namespace Display {

void begin() {
    tft.begin();
    tft.setRotation(1); // landscape 320x240; use 3 instead if the image is upside down
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    splash("ESP32 Audit Tool", "booting...");
}

void splash(const String &line1, const String &line2) {
    tft.fillScreen(ILI9341_BLACK);
    drawBitmapScaled((TFT_WIDTH - Mascot::WIDTH * 3) / 2, 10, Mascot::SHARK_BITMAP,
                      Mascot::WIDTH, Mascot::HEIGHT, 3, ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, Mascot::HEIGHT * 3 + 20);
    tft.println(line1);
    tft.println(line2);
}

void update(const String &lastAction) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 0);

    tft.println(RtcClock::isoTimestamp());

    tft.print("GPS: ");
    if (GpsModule::hasFix()) {
        tft.print("FIX sats=");
        tft.println(GpsModule::satellites());
    } else {
        tft.println("no fix");
    }

    tft.print("AP clients: ");
    tft.println(WiFi.softAPgetStationNum());

    tft.print("TX arm (BACK): ");
    tft.println(TxArm::isArmed() ? "HELD" : "off");

    tft.println("--------------------------------");

    if (!advanceSharkAnimation()) {
        tft.setTextSize(1);
        tft.println(lastAction);
    }
}

void showList(const String &title, const std::vector<String> &items, int selectedIndex) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.println(title);
    tft.println("--------------------------------");

    tft.setTextSize(1);
    const int visibleRows = 20;
    int start = selectedIndex - visibleRows / 2;
    if (start < 0) start = 0;
    if ((int)items.size() > visibleRows && start > (int)items.size() - visibleRows) {
        start = items.size() - visibleRows;
    }

    for (int i = start; i < (int)items.size() && i < start + visibleRows; i++) {
        tft.print(i == selectedIndex ? "> " : "  ");
        tft.println(items[i].substring(0, 50));
    }
}

void showText(const String &title, const String &body) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.println(title);
    tft.println("--------------------------------");
    tft.setTextSize(1);
    tft.println(body);
}

} // namespace Display
