#include "display.h"
#include "config.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "tx_arm.h"
#include "battery.h"
#include "mascot.h"
#include "ui/theme.h"
#include <TFT_eSPI.h>
#include <WiFi.h>

namespace {
TFT_eSPI tft = TFT_eSPI(); // pins/driver set via platformio.ini build_flags

// Shark "swim-by" cameo on the idle status screen: mostly off, occasionally
// crosses the bottom row. INT16_MIN means "not currently swimming".
int16_t g_sharkX = INT16_MIN;
uint16_t g_ticksUntilSwim = 20; // ~20 status refreshes (~20s) between cameos
constexpr uint8_t SHARK_SCALE = 2;

// TFT_eSPI's drawXBitmap draws a 1bpp bitmap at native size (like Adafruit's
// drawBitmap); it has no scale factor, so scaling is done by hand here,
// one filled block per set bit. Fine for a tiny 32x16 mascot.
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

    drawBitmapScaled(g_sharkX, tft.height() - Mascot::HEIGHT * SHARK_SCALE, Mascot::SHARK_BITMAP,
                      Mascot::WIDTH, Mascot::HEIGHT, SHARK_SCALE, TFT_WHITE);
    g_sharkX += 24;
    if (g_sharkX > tft.width()) {
        g_sharkX = INT16_MIN;
        g_ticksUntilSwim = 20;
    }
    return true;
}
}

namespace Display {

TFT_eSPI &raw() { return tft; }

void begin() {
    tft.init();
    tft.setRotation(Theme::ROTATION); // single source of truth for orientation, see ui/theme.h
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    splash("ESP32 Audit Tool", "booting...");
}

void splash(const String &line1, const String &line2) {
    tft.fillScreen(TFT_BLACK);
    drawBitmapScaled((tft.width() - Mascot::WIDTH * 3) / 2, 10, Mascot::SHARK_BITMAP,
                      Mascot::WIDTH, Mascot::HEIGHT, 3, TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, Mascot::HEIGHT * 3 + 20);
    tft.println(line1);
    tft.println(line2);
}

void update(const String &lastAction) {
    tft.fillScreen(TFT_BLACK);
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

    tft.print("Batt: ");
    tft.print(Battery::voltage(), 2);
    tft.print("V ");
    tft.print(Battery::percent());
    tft.println("%");

    tft.print("TX arm (BACK): ");
    tft.println(TxArm::isArmed() ? "HELD" : "off");

    tft.println("--------------------------------");

    if (!advanceSharkAnimation()) {
        tft.setTextSize(1);
        tft.println(lastAction);
    }
}

void showList(const String &title, const std::vector<String> &items, int selectedIndex) {
    tft.fillScreen(TFT_BLACK);
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
        tft.println(items[i].substring(0, 36)); // fits the 240px-wide portrait screen at size(1)
    }
}

void showText(const String &title, const String &body) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.println(title);
    tft.println("--------------------------------");
    tft.setTextSize(1);
    tft.println(body);
}

} // namespace Display
