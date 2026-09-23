#include "ui/style_guide.h"
#include "ui/theme.h"
#include "ui/widgets.h"
#include "display.h"
#include "buttons.h"
#include <TFT_eSPI.h>

namespace StyleGuide {

namespace {
TFT_eSPI &tft = Display::raw();
TFT_eSprite canvas(&tft);

struct ColorSwatch {
    String name;
    uint16_t color;
};

void drawColorPalette() {
    std::vector<ColorSwatch> colors = {
        {"BG", Theme::COLOR_BG},
        {"SURFACE", Theme::COLOR_SURFACE},
        {"SURFACE_ALT", Theme::COLOR_SURFACE_ALT},
        {"TEXT", Theme::COLOR_TEXT},
        {"TEXT_DIM", Theme::COLOR_TEXT_DIM},
        {"ACCENT", Theme::COLOR_ACCENT},
        {"OK", Theme::COLOR_OK},
        {"WARN", Theme::COLOR_WARN},
        {"DANGER", Theme::COLOR_DANGER},
    };

    canvas.fillSprite(Theme::COLOR_BG);

    // Title
    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("COLOR PALETTE", Theme::SPACE_2, Theme::SPACE_2 + 10);
    canvas.unloadFont();

    int y = Theme::SPACE_3 + Theme::SPACE_2;
    int swatchW = (canvas.width() - Theme::SPACE_2 * 2) / 3;
    int swatchH = 28;
    int col = 0;

    for (auto &swatch : colors) {
        int x = Theme::SPACE_2 + (col % 3) * (swatchW + Theme::SPACE_1);
        if (col % 3 == 0 && col > 0) {
            y += swatchH + Theme::SPACE_2;
        }

        // Draw color swatch
        canvas.fillRoundRect(x, y, swatchW, swatchH, Theme::RADIUS_MD, swatch.color);
        canvas.drawRoundRect(x, y, swatchW, swatchH, Theme::RADIUS_MD, Theme::COLOR_TEXT_DIM);

        // Draw label
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(MC_DATUM);
        uint16_t textCol = (swatch.color == Theme::COLOR_BG || swatch.color == Theme::COLOR_SURFACE)
                           ? Theme::COLOR_TEXT : Theme::COLOR_TEXT;
        canvas.setTextColor(textCol, swatch.color);
        canvas.drawString(swatch.name, x + swatchW / 2, y + swatchH / 2);
        canvas.unloadFont();

        col++;
    }

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}

void drawTypography() {
    canvas.fillSprite(Theme::COLOR_BG);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("TYPOGRAPHY", Theme::SPACE_2, Theme::SPACE_2 + 10);
    canvas.unloadFont();

    int y = Theme::SPACE_3 + Theme::SPACE_2;

    // Title font
    canvas.loadFont(FONT_TITLE);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_ACCENT, Theme::COLOR_BG);
    canvas.drawString("BIG TITLE", Theme::SPACE_2, y);
    y += 50;
    canvas.unloadFont();

    // Body font regular
    canvas.loadFont(FONT_BODY);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("BODY TEXT - Main content", Theme::SPACE_2, y);
    y += Theme::SPACE_3;

    // Body font dim
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString("DIM TEXT - Secondary info", Theme::SPACE_2, y);
    y += Theme::SPACE_3;

    // Mono font
    canvas.loadFont(FONT_MONO);
    canvas.setTextColor(Theme::COLOR_ACCENT, Theme::COLOR_BG);
    canvas.drawString("192.168.1.1", Theme::SPACE_2, y);
    canvas.unloadFont();

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}

void drawWidgets() {
    canvas.fillSprite(Theme::COLOR_BG);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("WIDGETS & COMPONENTS", Theme::SPACE_2, Theme::SPACE_2 + 10);
    canvas.unloadFont();

    int y = Theme::SPACE_3 + Theme::SPACE_2;

    // Title: RSSI Bars
    canvas.loadFont(FONT_BODY);
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString("Signal Strength Bars:", Theme::SPACE_2, y);
    y += Theme::SPACE_2;
    canvas.unloadFont();

    // Show RSSI bars at different strengths
    int rssiValues[] = {-95, -75, -55, -35};
    const char *labels[] = {"Weak", "Fair", "Good", "Strong"};
    for (int i = 0; i < 4; i++) {
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString(labels[i], Theme::SPACE_2, y + 6);
        Widgets::rssiBar(canvas, Theme::SPACE_2 + 50, y, 120, 12, rssiValues[i]);
        canvas.unloadFont();
        y += Theme::SPACE_2 + 6;
    }

    y += Theme::SPACE_1;

    // Title: Badges
    canvas.loadFont(FONT_BODY);
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString("Badges:", Theme::SPACE_2, y);
    y += Theme::SPACE_2;
    canvas.unloadFont();

    Widgets::badge(canvas, Theme::SPACE_2, y, "WiFi", Theme::COLOR_ACCENT);
    Widgets::badge(canvas, Theme::SPACE_2 + 60, y, "BLE", Theme::COLOR_ACCENT);
    Widgets::badge(canvas, Theme::SPACE_2 + 110, y, "OK", Theme::COLOR_OK);
    y += Theme::SPACE_3;

    // Danger and Warn
    Widgets::badge(canvas, Theme::SPACE_2, y, "WARN", Theme::COLOR_WARN);
    Widgets::badge(canvas, Theme::SPACE_2 + 70, y, "DANGER", Theme::COLOR_DANGER);

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}

void drawSpacing() {
    canvas.fillSprite(Theme::COLOR_BG);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
    canvas.drawString("SPACING & GRID (8px base)", Theme::SPACE_2, Theme::SPACE_2 + 10);
    canvas.unloadFont();

    int y = Theme::SPACE_3 + Theme::SPACE_2;

    struct Spacing {
        const char *name;
        uint8_t value;
    };

    Spacing spacings[] = {
        {"SPACE_1", Theme::SPACE_1},
        {"SPACE_2", Theme::SPACE_2},
        {"SPACE_3", Theme::SPACE_3},
        {"SPACE_4", Theme::SPACE_4},
    };

    for (auto &s : spacings) {
        canvas.loadFont(FONT_BODY);
        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
        canvas.drawString(s.name, Theme::SPACE_2, y + 8);

        // Draw spacing box
        canvas.fillRect(Theme::SPACE_2 + 80, y, s.value, 16, Theme::COLOR_ACCENT);
        canvas.drawRect(Theme::SPACE_2 + 80, y, s.value, 16, Theme::COLOR_TEXT_DIM);

        canvas.setTextDatum(ML_DATUM);
        canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_BG);
        canvas.drawString(String(s.value) + "px", Theme::SPACE_2 + 90 + s.value, y + 6);
        canvas.unloadFont();

        y += Theme::SPACE_3;
    }

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}

void drawSampleList() {
    canvas.fillSprite(Theme::COLOR_BG);

    // Fake status bar
    canvas.fillRect(0, 0, canvas.width(), Theme::STATUS_BAR_H, Theme::COLOR_SURFACE);
    canvas.drawLine(0, Theme::STATUS_BAR_H - 1, canvas.width(), Theme::STATUS_BAR_H - 1, Theme::COLOR_SURFACE_ALT);

    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT, Theme::COLOR_SURFACE);
    canvas.drawString("14:32", Theme::SPACE_2, Theme::STATUS_BAR_H / 2);
    canvas.setTextDatum(MR_DATUM);
    canvas.setTextColor(Theme::COLOR_OK, Theme::COLOR_SURFACE);
    canvas.drawString("85%", canvas.width() - Theme::SPACE_2, Theme::STATUS_BAR_H / 2);
    canvas.unloadFont();

    // Sample list items
    int y = Theme::STATUS_BAR_H + Theme::SPACE_1;
    int rowW = canvas.width() - Theme::SPACE_2;
    int highlightY = y;

    // Selection capsule
    canvas.fillRoundRect(Theme::SPACE_1, highlightY, rowW, Theme::ROW_H - 4,
                         Theme::RADIUS_MD, Theme::COLOR_SURFACE);
    canvas.drawRoundRect(Theme::SPACE_1, highlightY, rowW, Theme::ROW_H - 4,
                         Theme::RADIUS_MD, Theme::COLOR_ACCENT);

    // Sample list items with data
    struct SampleItem {
        const char *label;
        bool hasRssi;
        int rssi;
        const char *badge;
    } items[] = {
        {"WiFi Network Alpha", true, -45, "Active"},
        {"BLE Device Beta", true, -65, nullptr},
        {"ZigBee Node Gamma", true, -85, nullptr},
    };

    for (int i = 0; i < 3; i++) {
        bool selected = (i == 0);
        String badgeStr = items[i].badge ? String(items[i].badge) : "";
        Widgets::listRow(canvas, Theme::SPACE_1, y + i * Theme::ROW_H, rowW, Theme::ROW_H - 4,
                         items[i].label, selected, items[i].hasRssi, items[i].rssi, badgeStr);
    }

    // Help text
    canvas.loadFont(FONT_BODY);
    canvas.setTextDatum(MC_DATUM);
    canvas.setTextColor(Theme::COLOR_TEXT_DIM, Theme::COLOR_BG);
    canvas.drawString("Press any button to continue...", canvas.width() / 2, canvas.height() - Theme::SPACE_2 - 10);
    canvas.unloadFont();

    canvas.pushSprite(0, 0);
    while (Buttons::poll() == Buttons::NONE) delay(10);
}
} // namespace

void show() {
    // Ensure display is initialized
    if (Display::kind() != Display::ScreenKind::TFT) {
        Serial.println("Style guide requires TFT display");
        return;
    }

    canvas.setColorDepth(16);
    if (!canvas.createSprite(tft.width(), tft.height())) {
        Serial.println("Failed to allocate style guide canvas");
        return;
    }

    Serial.println("\n[StyleGuide] Showing design validation screens...");
    Serial.println("  Press any button to advance between screens");

    drawColorPalette();
    drawTypography();
    drawWidgets();
    drawSpacing();
    drawSampleList();

    Serial.println("[StyleGuide] Design validation complete!");
}

} // namespace StyleGuide
