#include "ui/oled_ui.h"
#include "ui/theme.h"
#include "display.h"
#include "config.h"
#include "skull.h"
#include "france_outline.h"
#include <Adafruit_SSD1306.h>

// Default Adafruit_GFX font at textSize(1): 6px wide x 8px tall per
// character, so OLED_WIDTH/6 columns and OLED_HEIGHT/8 rows fit exactly
// (128x64 -> 21 columns x 8 rows; 128x32 -> 21 columns x 4 rows).
namespace {
constexpr int COLS = OLED_WIDTH / 6;
constexpr int ROWS = OLED_HEIGHT / 8;
// Row 0 is always the status bar; row 1 is the screen title. Whatever's
// left is content -- on a 128x32 module that's only 2 rows, so list/
// detail screens just show less at once rather than assuming 64px.
constexpr int CONTENT_TOP_ROW = 2;
constexpr int CONTENT_ROWS = ROWS > CONTENT_TOP_ROW ? ROWS - CONTENT_TOP_ROW : 0;

Adafruit_SSD1306 &oled() { return Display::oled(); }

String clip(const String &s, int maxLen) {
    if ((int)s.length() <= maxLen) return s;
    return s.substring(0, maxLen);
}

// Pads/clips to exactly `width` characters so left- and right-aligned
// pieces on the same line never overlap or leave stray characters from a
// previous, longer draw (no fillRect erase between frames here).
String padTo(const String &s, int width) {
    String c = clip(s, width);
    while ((int)c.length() < width) c += ' ';
    return c;
}

void drawStatusBar(const Ui::StatusInfo &status) {
    String left;
    left += status.gpsFix ? 'G' : ' ';
    left += status.radioActive ? '!' : ' ';
    String battText = String(status.battPercent) + "%";
    int padWidth = COLS - (int)left.length() - (int)battText.length();
    String line = left;
    for (int i = 0; i < padWidth; i++) line += ' ';
    line += battText;
    oled().setCursor(0, 0);
    oled().print(padTo(line, COLS));
}

void drawTitleRow(const String &title) {
    oled().setCursor(0, 8);
    oled().print(padTo(title, COLS));
}
} // namespace

namespace OledUi {

void showSplash(const String &title, const String &subtitle) {
    Adafruit_SSD1306 &d = oled();

    // Same skull bitmap and rotation math as the TFT splash (see skull.h)
    // -- spins in place for Theme::SPLASH_DURATION_MS, just plotted with
    // single-pixel drawPixel() calls instead of a TFT sprite.
    constexpr float ROTATIONS_PER_SEC = 1.5f;
    // 128x64 (ROWS>4): skull up top, title/subtitle below it. 128x32: no
    // room left for text once a 24px skull fits, so just center it.
    int16_t skullCx = OLED_WIDTH / 2;
    int16_t skullCy = ROWS > 4 ? (Skull::HEIGHT / 2) + 2 : OLED_HEIGHT / 2;

    uint32_t start = millis();
    while (millis() - start < Theme::SPLASH_DURATION_MS) {
        float elapsedSec = (millis() - start) / 1000.0f;
        float angle = elapsedSec * ROTATIONS_PER_SEC * 2.0f * PI;

        d.clearDisplay();
        d.setTextSize(1);
        d.setTextColor(SSD1306_WHITE);

        Skull::drawRotated(skullCx, skullCy, angle, 1,
                            [&](int16_t x, int16_t y) { d.drawPixel(x, y, SSD1306_WHITE); });

        if (ROWS > 4) {
            d.setCursor(0, Skull::HEIGHT + 4);
            d.print(clip(title, COLS));
            if (ROWS > 5) {
                d.setCursor(0, Skull::HEIGHT + 12);
                d.print(clip(subtitle, COLS));
            }
        }
        d.display();
    }
}

void showHome(const Ui::StatusInfo &status, const String &lastAction) {
    Adafruit_SSD1306 &d = oled();
    d.clearDisplay();
    d.setTextSize(1);
    d.setTextColor(SSD1306_WHITE);
    drawStatusBar(status);

    // status.time is "yyyy-mm-dd hh:mm:ss" -- fits on one row as-is.
    d.setCursor(0, 16);
    d.print(clip(status.time, COLS));

    if (lastAction.length() && ROWS > 4) {
        d.setCursor(0, 32);
        d.print(clip(lastAction, COLS));
    }
    d.display();
}

void showList(const Ui::StatusInfo &status, const String &title,
              const std::vector<Ui::ListItem> &items, int selectedIndex, bool scanning) {
    Adafruit_SSD1306 &d = oled();
    d.clearDisplay();
    d.setTextSize(1);
    d.setTextColor(SSD1306_WHITE);
    drawStatusBar(status);
    drawTitleRow(scanning ? title + " ..." : title);

    if (CONTENT_ROWS <= 0 || items.empty()) {
        d.display();
        return;
    }

    // Keep the selection visible: center it in the window when possible,
    // clamped so the window never scrolls past the list's start/end.
    int topIndex = selectedIndex - CONTENT_ROWS / 2;
    int maxTop = (int)items.size() - CONTENT_ROWS;
    if (topIndex > maxTop) topIndex = maxTop;
    if (topIndex < 0) topIndex = 0;

    for (int row = 0; row < CONTENT_ROWS; row++) {
        int idx = topIndex + row;
        if (idx >= (int)items.size()) break;
        const auto &item = items[idx];

        String line = (idx == selectedIndex) ? ">" : " ";
        line += item.label;
        // Room permitting, append a short RSSI/badge hint after the label.
        String suffix;
        if (item.hasRssi) suffix = " " + String(item.rssi);
        else if (item.badge.length()) suffix = " " + item.badge;
        if (suffix.length() && (int)(line.length() + suffix.length()) <= COLS) line += suffix;

        d.setCursor(0, (CONTENT_TOP_ROW + row) * 8);
        d.print(padTo(line, COLS));
    }
    d.display();
}

void showDetail(const Ui::StatusInfo &status, const String &title,
                 const std::vector<Ui::DetailRow> &rows) {
    Adafruit_SSD1306 &d = oled();
    d.clearDisplay();
    d.setTextSize(1);
    d.setTextColor(SSD1306_WHITE);
    drawStatusBar(status);
    drawTitleRow(title);

    for (int row = 0; row < CONTENT_ROWS && row < (int)rows.size(); row++) {
        const auto &r = rows[row];
        String line = r.label.length() ? (r.label + ":" + r.value) : r.value;
        d.setCursor(0, (CONTENT_TOP_ROW + row) * 8);
        d.print(padTo(line, COLS));
    }
    d.display();
}

void confirm(const String &title, const String &message) {
    Adafruit_SSD1306 &d = oled();
    d.clearDisplay();
    d.setTextSize(1);
    d.setTextColor(SSD1306_WHITE);
    d.setCursor(0, 0);
    d.println(clip(title, COLS));
    if (ROWS > 2) d.println(clip(message, COLS));
    if (ROWS > 3) {
        d.setCursor(0, (ROWS - 1) * 8);
        d.print(clip("SEL=OK  RET=annuler", COLS));
    }
    d.display();
}

void showGpsMap(const Ui::StatusInfo &status, bool hasFix, double lat, double lon) {
    Adafruit_SSD1306 &d = oled();
    d.clearDisplay();
    d.setTextSize(1);
    d.setTextColor(SSD1306_WHITE);
    drawStatusBar(status);

    constexpr int16_t mapY = 8; // row 0 is the status bar; map takes the rest
    int16_t mapH = OLED_HEIGHT - mapY;

    int16_t px, py, prevX, prevY;
    FranceOutline::project(FranceOutline::POINTS[0].lon, FranceOutline::POINTS[0].lat, 0, mapY, OLED_WIDTH,
                            mapH, prevX, prevY);
    for (size_t i = 1; i <= FranceOutline::POINT_COUNT; i++) {
        const auto &p = FranceOutline::POINTS[i % FranceOutline::POINT_COUNT];
        FranceOutline::project(p.lon, p.lat, 0, mapY, OLED_WIDTH, mapH, px, py);
        d.drawLine(prevX, prevY, px, py, SSD1306_WHITE);
        prevX = px;
        prevY = py;
    }

    if (hasFix) {
        FranceOutline::project((float)lon, (float)lat, 0, mapY, OLED_WIDTH, mapH, px, py);
        d.fillCircle(px, py, 2, SSD1306_WHITE);
    } else if (ROWS > 3) {
        d.setCursor(0, (ROWS - 1) * 8);
        d.print(clip("Pas de fix GPS", COLS));
    }
    d.display();
}

} // namespace OledUi
