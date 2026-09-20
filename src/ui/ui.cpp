#include "ui/ui.h"
#include "ui/theme.h"
#include "display.h"
#include "mascot.h"
#include <TFT_eSPI.h>

namespace {
TFT_eSPI &tft = Display::raw();
TFT_eSprite canvas(&tft);
bool canvasReady = false;

// Selection-highlight slide animation state (main menu).
int g_animFromY = 0;
int g_animToY = 0;
uint32_t g_animStart = 0;
bool g_animating = false;
int g_lastSelectedIndex = -1;

void ensureCanvas() {
    if (canvasReady) return;
    canvas.setColorDepth(16);
    canvas.createSprite(tft.width(), tft.height()); // auto-allocates in PSRAM (see Sprite.cpp)
    canvasReady = true;
}

// Draws the mascot bitmap scaled up, straight into the sprite buffer —
// same bit-per-pixel format as mascot.cpp's own helper, duplicated here
// since it's three lines and not worth a shared header for.
void drawMascot(int16_t x, int16_t y, uint8_t scale, uint16_t color) {
    uint8_t bytesPerRow = (Mascot::WIDTH + 7) / 8;
    for (uint8_t row = 0; row < Mascot::HEIGHT; row++) {
        for (uint8_t col = 0; col < Mascot::WIDTH; col++) {
            uint8_t b = pgm_read_byte(&Mascot::SHARK_BITMAP[row * bytesPerRow + col / 8]);
            if (b & (0x80 >> (col % 8))) {
                canvas.fillRect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

void drawStatusBar(const Ui::StatusInfo &status) {
    canvas.fillRect(0, 0, canvas.width(), Theme::STATUS_BAR_H, Theme::COLOR_SURFACE);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_SURFACE);
    canvas.drawString(status.time, Theme::SPACE_1, Theme::STATUS_BAR_H / 2);

    canvas.setTextDatum(MR_DATUM);
    uint16_t battColor = status.battPercent < 20   ? Theme::COLOR_DANGER
                          : status.battPercent < 50 ? Theme::COLOR_WARN
                                                     : Theme::COLOR_OK;
    canvas.setTextColor(battColor, Theme::COLOR_SURFACE);
    canvas.drawString(String(status.battPercent) + "%", canvas.width() - Theme::SPACE_1,
                       Theme::STATUS_BAR_H / 2);
    canvas.unloadFont();

    canvas.fillCircle(canvas.width() - 70, Theme::STATUS_BAR_H / 2, 4,
                       status.gpsFix ? Theme::COLOR_OK : Theme::COLOR_TEXT_DIM);
    if (status.radioActive) {
        canvas.fillCircle(canvas.width() - 90, Theme::STATUS_BAR_H / 2, 4, Theme::COLOR_DANGER);
    }
}
} // namespace

namespace Ui {

void begin() {
    ensureCanvas();
}

void showSplash(const String &title, const String &subtitle) {
    ensureCanvas();

    uint32_t start = millis();
    while (millis() - start < Theme::SPLASH_DURATION_MS) {
        float t = (float)(millis() - start) / Theme::SPLASH_DURATION_MS;
        float ease = 1.0f - (1.0f - t) * (1.0f - t); // ease-out quad
        uint8_t scale = 1 + (uint8_t)(ease * 2.0f);  // grows 1x -> 3x

        canvas.fillSprite(Theme::COLOR_BG);

        int16_t mw = Mascot::WIDTH * scale;
        int16_t mh = Mascot::HEIGHT * scale;
        drawMascot((canvas.width() - mw) / 2, canvas.height() / 2 - mh - 10, scale, Theme::COLOR_ACCENT);

        canvas.loadFont(FONT_TITLE);
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString(title, canvas.width() / 2, canvas.height() / 2 + 40);
        canvas.unloadFont();

        canvas.loadFont(FONT_BODY);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString(subtitle, canvas.width() / 2, canvas.height() / 2 + 75);
        canvas.unloadFont();

        canvas.pushSprite(0, 0);
        delay(30);
    }
}

void showMainMenu(const StatusInfo &status, const String &title,
                   const std::vector<String> &items, int selectedIndex) {
    ensureCanvas();

    if (selectedIndex != g_lastSelectedIndex) {
        g_animFromY = (g_lastSelectedIndex < 0 ? selectedIndex : g_lastSelectedIndex) * Theme::ROW_H;
        g_animToY = selectedIndex * Theme::ROW_H;
        g_animStart = millis();
        g_animating = true;
        g_lastSelectedIndex = selectedIndex;
    }

    int highlightY = g_animToY;
    if (g_animating) {
        float t = (float)(millis() - g_animStart) / Theme::ANIM_SELECTION_MS;
        if (t >= 1.0f) {
            g_animating = false;
            highlightY = g_animToY;
        } else {
            highlightY = g_animFromY + (int)((g_animToY - g_animFromY) * t);
        }
    }

    canvas.fillSprite(Theme::COLOR_BG);
    drawStatusBar(status);

    int listTop = Theme::STATUS_BAR_H + Theme::SPACE_1;

    canvas.fillRoundRect(Theme::SPACE_1, listTop + highlightY, canvas.width() - Theme::SPACE_2,
                          Theme::ROW_H - 4, Theme::RADIUS_MD, Theme::COLOR_SURFACE);
    canvas.drawRoundRect(Theme::SPACE_1, listTop + highlightY, canvas.width() - Theme::SPACE_2,
                          Theme::ROW_H - 4, Theme::RADIUS_MD, Theme::COLOR_ACCENT);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    for (size_t i = 0; i < items.size(); i++) {
        int rowY = listTop + i * Theme::ROW_H;
        if (rowY > canvas.height()) break;
        bool selected = (int)i == selectedIndex;
        uint16_t bg = selected ? Theme::COLOR_SURFACE : Theme::COLOR_BG;
        canvas.setTextColor(selected ? Theme::COLOR_TEXT : Theme::COLOR_TEXT_DIM, bg);
        canvas.drawString(items[i], Theme::SPACE_2, rowY + Theme::ROW_H / 2 - 2);
    }
    canvas.unloadFont();

    canvas.pushSprite(0, 0);
    (void)title; // reserved: header/breadcrumb row, added when list/detail screens are wired
}

} // namespace Ui
