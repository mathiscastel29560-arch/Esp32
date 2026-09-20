#include "ui/widgets.h"
#include "ui/theme.h"

namespace {
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

// Two-segment gradient: red -> amber -> green.
uint16_t signalColor(float t) {
    if (t < 0.5f) return blend565(Theme::COLOR_DANGER, Theme::COLOR_WARN, t / 0.5f);
    return blend565(Theme::COLOR_WARN, Theme::COLOR_OK, (t - 0.5f) / 0.5f);
}
} // namespace

namespace Widgets {

void listRow(TFT_eSprite &c, int16_t x, int16_t y, int16_t w, int16_t h, const String &label,
             bool selected, bool hasRssi, int rssi, const String &badgeText) {
    // Content only — no background box here. The selection capsule is a
    // single element that slides independently between rows (see
    // ui.cpp's showList), so drawing it per-row would fight that animation.
    uint16_t bg = selected ? Theme::COLOR_SURFACE : Theme::COLOR_BG;

    int16_t rightEdge = x + w - Theme::SPACE_1;
    if (hasRssi) {
        int16_t barW = 40;
        rightEdge -= barW;
        rssiBar(c, rightEdge, y + h / 2 - 4, barW, 8, rssi);
        rightEdge -= Theme::SPACE_1;
    }
    if (badgeText.length()) {
        rightEdge -= (badgeText.length() * 7 + 12);
        badge(c, rightEdge, y + h / 2 - 9, badgeText, Theme::COLOR_ACCENT);
        rightEdge -= Theme::SPACE_1;
    }

    c.loadFont(FONT_BODY);
    c.setTextDatum(ML_DATUM);
    c.setTextColor(selected ? Theme::COLOR_TEXT : Theme::COLOR_TEXT_DIM, bg);
    // Leave room for the widgets drawn on the right.
    String clipped = label;
    while (clipped.length() > 1 && c.textWidth(clipped) > (rightEdge - (x + Theme::SPACE_1))) {
        clipped = clipped.substring(0, clipped.length() - 1);
    }
    c.drawString(clipped, x + Theme::SPACE_1, y + h / 2);
    c.unloadFont();
}

void labelValue(TFT_eSprite &c, int16_t x, int16_t y, int16_t w, const String &label,
                const String &value) {
    c.loadFont(FONT_BODY);
    c.setTextDatum(ML_DATUM);
    c.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    c.drawString(label, x, y);

    c.setTextDatum(MR_DATUM);
    c.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    c.drawString(value, x + w, y);
    c.unloadFont();
}

void rssiBar(TFT_eSprite &c, int16_t x, int16_t y, int16_t w, int16_t h, int rssi) {
    float t = (float)(rssi + 100) / 60.0f; // -100dBm..-40dBm -> 0..1
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    c.fillRoundRect(x, y, w, h, h / 2, Theme::COLOR_SURFACE);
    int16_t fillW = (int16_t)(w * t);
    if (fillW > h) { // avoid a degenerate rounded rect thinner than its own radius
        c.fillRoundRect(x, y, fillW, h, h / 2, signalColor(t));
    }
}

void badge(TFT_eSprite &c, int16_t x, int16_t y, const String &text, uint16_t color) {
    c.loadFont(FONT_BODY);
    int16_t tw = c.textWidth(text);
    int16_t w = tw + 12;
    int16_t h = 18;
    c.drawRoundRect(x, y, w, h, h / 2, color);
    c.setTextDatum(MC_DATUM);
    c.setTextColor(color, Theme::COLOR_BG);
    c.drawString(text, x + w / 2, y + h / 2);
    c.unloadFont();
}

void spinner(TFT_eSprite &c, int16_t cx, int16_t cy, int16_t radius, uint32_t nowMs) {
    uint32_t angle = (nowMs / 3) % 360;
    c.drawArc(cx, cy, radius, radius - 4, angle, (angle + 90) % 360, Theme::COLOR_ACCENT,
              Theme::COLOR_BG, true);
}

void progressBar(TFT_eSprite &c, int16_t x, int16_t y, int16_t w, int16_t h, float fraction) {
    if (fraction < 0) fraction = 0;
    if (fraction > 1) fraction = 1;
    c.fillRoundRect(x, y, w, h, h / 2, Theme::COLOR_SURFACE);
    int16_t fillW = (int16_t)(w * fraction);
    if (fillW > h) {
        c.fillRoundRect(x, y, fillW, h, h / 2, Theme::COLOR_ACCENT);
    }
}

} // namespace Widgets
