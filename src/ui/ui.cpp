#include "ui/ui.h"
#include "ui/theme.h"
#include "ui/widgets.h"
#include "ui/oled_ui.h"
#include "display.h"
#include "mascot.h"
#include "buttons.h"
#include <TFT_eSPI.h>

namespace {
TFT_eSPI &tft = Display::raw();
TFT_eSprite canvas(&tft);         // current frame, always mirrors what's on screen after a push
TFT_eSprite prevSnapshot(&tft);   // last frame, only used while a transition plays
bool canvasReady = false;
bool g_displayOk = false; // false = no screen / sprite alloc failed: every draw call below no-ops

// Full-frame pushes are the slow part (a 320x240x16bpp push is tens of ms
// over SPI). menu.cpp calls showList()/showDetail() every single
// Menu::loop() iteration with no throttling of its own, so without a cap
// here a busy screen redraws as fast as the main loop spins — easily
// enough to starve WiFi/BLE housekeeping and trip the task watchdog.
// Screen-change transitions always draw immediately regardless.
constexpr uint32_t MIN_FRAME_INTERVAL_MS = 33; // ~30fps ceiling
uint32_t g_lastFrameMs = 0;

// Selection-highlight slide animation state (shared by every list screen —
// reset whenever the screen identity changes, see enterScreen()).
int g_animFromY = 0;
int g_animToY = 0;
uint32_t g_animStart = 0;
bool g_animating = false;
int g_lastSelectedIndex = -1;

// Screen-to-screen transition state.
String g_lastScreenId = "";
bool g_havePrevFrame = false;

// Idle-screen shark cameo (ported from the old display.cpp): mostly off,
// occasionally swims across the bottom row. INT16_MIN = not swimming.
int16_t g_sharkX = INT16_MIN;
uint16_t g_ticksUntilSwim = 20; // ~20 calls (~20s at the usual 1s refresh) between cameos
constexpr uint8_t SHARK_SCALE = 2;

void ensureCanvas() {
    if (canvasReady) return;
    canvasReady = true;
    if (Display::kind() != Display::ScreenKind::TFT) return; // g_displayOk stays false: no TFT, nothing to allocate
    canvas.setColorDepth(16);
    void *buf = canvas.createSprite(tft.width(), tft.height()); // auto-allocates in PSRAM (see Sprite.cpp)
    g_displayOk = (buf != nullptr);
    if (!g_displayOk) {
        Serial.println("[ui] display/sprite not available — running headless, UI calls are no-ops");
    }
}

void ensureSnapshot() {
    if (prevSnapshot.getPointer()) return;
    prevSnapshot.setColorDepth(16);
    prevSnapshot.createSprite(tft.width(), tft.height());
}

// Call once per screen function, before drawing into `canvas`. Returns
// true if this is a genuinely different screen than last time (so the
// caller should reset any screen-local animation state, e.g. the list
// selection highlight).
bool enterScreen(const String &screenId) {
    bool changed = g_lastScreenId.length() > 0 && screenId != g_lastScreenId;
    if (changed) {
        ensureSnapshot();
        size_t bytes = (size_t)tft.width() * tft.height() * 2;
        memcpy(prevSnapshot.getPointer(), canvas.getPointer(), bytes);
        g_havePrevFrame = true;
    }
    g_lastScreenId = screenId;
    return changed;
}

// Finishes a frame: either a plain push (same screen as last call) or a
// discreet slide transition (screen identity just changed).
void presentFrame(bool didChangeScreen) {
    if (!didChangeScreen || !g_havePrevFrame) {
        canvas.pushSprite(0, 0);
        return;
    }

    uint32_t start = millis();
    int16_t w = canvas.width();
    while (true) {
        float t = (float)(millis() - start) / Theme::ANIM_TRANSITION_MS;
        if (t >= 1.0f) break;
        float ease = 1.0f - (1.0f - t) * (1.0f - t); // ease-out quad
        int16_t offset = (int16_t)(w * (1.0f - ease));
        prevSnapshot.pushSprite(-offset, 0);
        canvas.pushSprite(w - offset, 0);
    }
    canvas.pushSprite(0, 0); // exact final frame, no rounding drift
    g_havePrevFrame = false;
}

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

void updateSelectionAnim(int selectedIndex, bool screenChanged) {
    if (screenChanged) {
        g_lastSelectedIndex = selectedIndex;
        g_animating = false;
        return;
    }
    if (selectedIndex != g_lastSelectedIndex) {
        g_animFromY = (g_lastSelectedIndex < 0 ? selectedIndex : g_lastSelectedIndex) * Theme::ROW_H;
        g_animToY = selectedIndex * Theme::ROW_H;
        g_animStart = millis();
        g_animating = true;
        g_lastSelectedIndex = selectedIndex;
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

    drawMascot(g_sharkX, canvas.height() - Mascot::HEIGHT * SHARK_SCALE, SHARK_SCALE, Theme::COLOR_ACCENT);
    g_sharkX += 24;
    if (g_sharkX > canvas.width()) {
        g_sharkX = INT16_MIN;
        g_ticksUntilSwim = 20;
    }
    return true;
}

int currentHighlightY(int selectedIndex) {
    int targetY = selectedIndex * Theme::ROW_H;
    if (!g_animating) return targetY;
    float t = (float)(millis() - g_animStart) / Theme::ANIM_SELECTION_MS;
    if (t >= 1.0f) {
        g_animating = false;
        return targetY;
    }
    return g_animFromY + (int)((g_animToY - g_animFromY) * t);
}
} // namespace

namespace Ui {

void begin() {
    ensureCanvas();
}

void showSplash(const String &title, const String &subtitle) {
    if (Display::kind() == Display::ScreenKind::OLED) {
        OledUi::showSplash(title, subtitle);
        return;
    }
    ensureCanvas();
    if (!g_displayOk) return;

    uint32_t start = millis();
    while (millis() - start < Theme::SPLASH_DURATION_MS) {
        float t = (float)(millis() - start) / Theme::SPLASH_DURATION_MS;
        float ease = 1.0f - (1.0f - t) * (1.0f - t);
        uint8_t scale = 1 + (uint8_t)(ease * 2.0f);

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
    g_lastScreenId = "__splash__";
}

void showHome(const StatusInfo &status, const String &lastAction) {
    if (Display::kind() == Display::ScreenKind::OLED) {
        OledUi::showHome(status, lastAction);
        return;
    }
    ensureCanvas();
    if (!g_displayOk) return;
    bool screenChanged = enterScreen("__home__");

    canvas.fillSprite(Theme::COLOR_BG);
    drawStatusBar(status);

    // status.time is "yyyy-mm-dd hh:mm:ss" — too wide for the title font as
    // a whole, so split: big clock, small date underneath.
    String datePart = status.time.length() >= 10 ? status.time.substring(0, 10) : "";
    String timePart = status.time.length() >= 19 ? status.time.substring(11) : status.time;

    canvas.loadFont(FONT_TITLE);
    canvas.setTextDatum(MC_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString(timePart, canvas.width() / 2, canvas.height() / 2 - 30);
    canvas.unloadFont();

    canvas.loadFont(FONT_BODY);
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString(datePart, canvas.width() / 2, canvas.height() / 2 + 5);
    canvas.unloadFont();

    if (!advanceSharkAnimation()) {
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MC_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString(lastAction, canvas.width() / 2, canvas.height() - 40);
        canvas.unloadFont();
    }

    presentFrame(screenChanged);
}

void showList(const StatusInfo &status, const String &title, const std::vector<ListItem> &items,
              int selectedIndex, bool scanning) {
    if (Display::kind() == Display::ScreenKind::OLED) {
        OledUi::showList(status, title, items, selectedIndex, scanning);
        return;
    }
    ensureCanvas();
    if (!g_displayOk) return;
    bool screenChanged = enterScreen(title);

    // Throttle: skip repaints of an unchanged screen faster than ~30fps so
    // an unthrottled caller (menu.cpp redraws every loop() iteration)
    // can't hog the CPU/SPI bus and starve WiFi/BLE or the watchdog.
    if (!screenChanged && millis() - g_lastFrameMs < MIN_FRAME_INTERVAL_MS) return;
    g_lastFrameMs = millis();

    updateSelectionAnim(selectedIndex, screenChanged);
    int highlightY = currentHighlightY(selectedIndex);

    canvas.fillSprite(Theme::COLOR_BG);
    drawStatusBar(status);
    if (scanning) Widgets::spinner(canvas, canvas.width() - 16, Theme::STATUS_BAR_H + 16, 8, millis());

    int listTop = Theme::STATUS_BAR_H + Theme::SPACE_1;
    int rowW = canvas.width() - Theme::SPACE_2;

    // One selection capsule, drawn first and slid independently of the
    // rows themselves (see currentHighlightY) — the row loop below just
    // draws text/badges on top of it, never its own background box.
    canvas.fillRoundRect(Theme::SPACE_1, listTop + highlightY, rowW, Theme::ROW_H - 4,
                          Theme::RADIUS_MD, Theme::COLOR_SURFACE);
    canvas.drawRoundRect(Theme::SPACE_1, listTop + highlightY, rowW, Theme::ROW_H - 4,
                          Theme::RADIUS_MD, Theme::COLOR_ACCENT);

    for (size_t i = 0; i < items.size(); i++) {
        int rowY = listTop + i * Theme::ROW_H;
        if (rowY > canvas.height()) break;
        // Bright text follows the capsule's current visual position, not
        // the logical selection, so text color never mismatches the
        // (opaque-erase) background it's drawn against mid-slide.
        bool underCapsule = rowY == listTop + highlightY;
        Widgets::listRow(canvas, Theme::SPACE_1, rowY, rowW, Theme::ROW_H - 4, items[i].label,
                          underCapsule, items[i].hasRssi, items[i].rssi, items[i].badge);
    }

    presentFrame(screenChanged);
}

void showList(const StatusInfo &status, const String &title, const std::vector<String> &items,
              int selectedIndex) {
    std::vector<ListItem> rich;
    rich.reserve(items.size());
    for (auto &s : items) rich.push_back(ListItem(s));
    showList(status, title, rich, selectedIndex, false);
}

void showDetail(const StatusInfo &status, const String &title, const std::vector<DetailRow> &rows,
                 const std::vector<Badge> &badges) {
    if (Display::kind() == Display::ScreenKind::OLED) {
        OledUi::showDetail(status, title, rows); // badges dropped -- monochrome, no room
        return;
    }
    ensureCanvas();
    if (!g_displayOk) return;
    bool screenChanged = enterScreen(title);

    if (!screenChanged && millis() - g_lastFrameMs < MIN_FRAME_INTERVAL_MS) return;
    g_lastFrameMs = millis();

    canvas.fillSprite(Theme::COLOR_BG);
    drawStatusBar(status);

    int y = Theme::STATUS_BAR_H + Theme::SPACE_2;
    int x = Theme::SPACE_1;

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString(title, x, y);
    canvas.unloadFont();
    y += Theme::SPACE_3;

    if (!badges.empty()) {
        int bx = x;
        for (auto &b : badges) {
            Widgets::badge(canvas, bx, y, b.text, b.color);
            bx += canvas.textWidth(b.text) + 12 + Theme::SPACE_1;
        }
        y += Theme::SPACE_3;
    }

    for (auto &row : rows) {
        if (y > canvas.height() - Theme::SPACE_1) break;
        Widgets::labelValue(canvas, x, y, canvas.width() - Theme::SPACE_2, row.label, row.value);
        y += Theme::SPACE_2 + 4;
    }

    presentFrame(screenChanged);
}

void showTextBlock(const StatusInfo &status, const String &title, const String &body) {
    std::vector<DetailRow> rows;
    int start = 0;
    while (start <= (int)body.length()) {
        int nl = body.indexOf('\n', start);
        if (nl < 0) nl = body.length();
        String line = body.substring(start, nl);
        if (line.length()) {
            int colon = line.indexOf(':');
            if (colon > 0 && colon < (int)line.length() - 1) {
                rows.push_back({line.substring(0, colon), line.substring(colon + 1)});
            } else {
                rows.push_back({"", line});
            }
        }
        start = nl + 1;
    }
    showDetail(status, title, rows);
}

bool confirm(const String &title, const String &message) {
    if (Display::kind() == Display::ScreenKind::OLED) {
        OledUi::confirm(title, message);
    } else {
        ensureCanvas();
        if (g_displayOk) {
            enterScreen("__confirm__" + title); // always a fresh screen: no lingering slide state

            canvas.fillSprite(Theme::COLOR_BG);
            canvas.loadFont(FONT_BODY);
            canvas.setTextDatum(MC_DATUM);
            canvas.setTextColor(Theme::COLOR_WARN, Theme::COLOR_BG);
            canvas.drawString(title, canvas.width() / 2, canvas.height() / 2 - 40);
            canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
            canvas.drawString(message, canvas.width() / 2, canvas.height() / 2 - 10);
            canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
            canvas.drawString("OK = confirmer   RETOUR = annuler", canvas.width() / 2, canvas.height() / 2 + 30);
            canvas.unloadFont();
            canvas.pushSprite(0, 0);
        }
    }
    // No screen to show the prompt on (ScreenKind::NONE): still block on
    // the buttons below rather than silently defaulting to yes/no, since
    // Buttons:: doesn't need the display to work.

    while (true) {
        Buttons::Button btn = Buttons::poll();
        if (btn == Buttons::SELECT) return true;
        if (btn == Buttons::BACK) return false;
        delay(10);
    }
}

} // namespace Ui
