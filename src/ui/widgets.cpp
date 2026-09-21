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
    uint16_t textColor = selected ? Theme::COLOR_TEXT : Theme::COLOR_TEXT_DIM;

    int16_t rightEdge = x + w - Theme::SPACE_2;

    // Arrange right-side widgets with proper spacing
    if (hasRssi) {
        int16_t barW = 48; // Slightly wider for better visibility
        rightEdge -= barW;
        rssiBar(c, rightEdge, y + (h - 10) / 2, barW, 10, rssi);
        rightEdge -= Theme::SPACE_1;
    }
    if (badgeText.length()) {
        int16_t badgeW = badgeText.length() * 7 + 14;
        rightEdge -= badgeW;
        badge(c, rightEdge, y + (h - 16) / 2, badgeText, Theme::COLOR_ACCENT_LIGHT);
        rightEdge -= Theme::SPACE_2;
    }

    c.loadFont(FONT_BODY);
    c.setTextDatum(ML_DATUM);
    c.setTextColor(textColor, Theme::COLOR_BG);

    // Clip label to fit available space with ellipsis
    String clipped = label;
    int16_t maxW = rightEdge - (x + Theme::SPACE_2);
    while (clipped.length() > 2 && c.textWidth(clipped) > maxW) {
        clipped = clipped.substring(0, clipped.length() - 1);
    }
    if (clipped.length() < label.length()) {
        clipped = clipped + ".";
    }
    c.drawString(clipped, x + Theme::SPACE_2, y + h / 2);
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
    // Map dBm to 0..1: -100dBm = 0 (weak), -40dBm = 1 (strong)
    float t = (float)(rssi + 100) / 60.0f;
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    // Background: subtle surface color with rounded corners
    c.fillRoundRect(x, y, w, h, Theme::RADIUS_SM, Theme::COLOR_SURFACE_ALT);
    c.drawRoundRect(x, y, w, h, Theme::RADIUS_SM, Theme::COLOR_TEXT_DIM);

    // Foreground: semantic color based on strength
    int16_t fillW = (int16_t)(w * t);
    if (fillW > Theme::RADIUS_SM) {
        uint16_t barColor = signalColor(t);
        c.fillRoundRect(x, y, fillW, h, Theme::RADIUS_SM, barColor);
    }
}

void badge(TFT_eSprite &c, int16_t x, int16_t y, const String &text, uint16_t color) {
    c.loadFont(FONT_BODY);
    int16_t tw = c.textWidth(text);
    int16_t w = tw + 14;
    int16_t h = 20;

    // Filled background with subtle color
    c.fillRoundRect(x, y, w, h, Theme::RADIUS_SM,
                    blend565(color, Theme::COLOR_BG, 0.85f)); // Very faint background

    // Border for definition
    c.drawRoundRect(x, y, w, h, Theme::RADIUS_SM, color);

    // Text centered
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
