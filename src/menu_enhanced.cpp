#include "menu_enhanced.h"

namespace MenuEnhanced {

void drawTopBorder() {
    Serial.print(THEME_MAIN);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print(RESET);
}

void drawBottomBorder() {
    Serial.print(THEME_MAIN);
    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(RESET);
}

void drawSeparator() {
    Serial.print(THEME_INFO);
    Serial.println("├─────────────────────────────────────────┤");
    Serial.print(RESET);
}

void drawSignalStrength(int rssi) {
    String bars = "";
    if (rssi > -50) {
        bars = "████████░░";  // Excellent
    } else if (rssi > -60) {
        bars = "██████░░░░";  // Good
    } else if (rssi > -70) {
        bars = "████░░░░░░";  // Fair
    } else if (rssi > -80) {
        bars = "██░░░░░░░░";  // Poor
    } else {
        bars = "░░░░░░░░░░";  // Weak
    }

    Serial.print(THEME_INFO);
    Serial.print("│ Signal: [" + bars + "] " + String(rssi) + "dBm");
    Serial.println(String(29 - bars.length(), ' ') + "│");
    Serial.print(RESET);
}

void drawProgressBar(uint8_t percent) {
    uint8_t filled = percent / 10;
    String bar = "[";

    for (uint8_t i = 0; i < 10; i++) {
        if (i < filled) {
            if (percent > 75) Serial.print(THEME_ACCENT);
            else if (percent > 50) Serial.print(THEME_WARNING);
            else Serial.print(THEME_ALERT);
            bar += "█";
        } else {
            bar += "░";
        }
    }
    bar += "]";

    Serial.print(THEME_INFO);
    Serial.print("│ ");
    Serial.print(bar);
    Serial.print(" " + String(percent) + "%");
    Serial.println(String(28 - String(percent).length(), ' ') + "│");
    Serial.print(RESET);
}

void drawHeader(const String &title, const String &subtitle) {
    drawTopBorder();

    Serial.print(THEME_ACCENT);
    Serial.print("║  ✦ ");
    Serial.print(title);
    Serial.print(THEME_MAIN);
    Serial.println(String(35 - title.length(), ' ') + "║");

    if (subtitle.length() > 0) {
        Serial.print(THEME_INFO);
        Serial.print("║  ");
        Serial.print(subtitle);
        Serial.print(THEME_MAIN);
        Serial.println(String(35 - subtitle.length(), ' ') + "║");
    }

    drawSeparator();
}

void drawLoadingBar(uint8_t duration) {
    for (uint8_t i = 0; i <= duration; i++) {
        uint8_t percent = (i * 100) / duration;
        drawProgressBar(percent);
        delay(100);
    }
}

void drawMenuAnimation() {
    Serial.println();
    for (int i = 0; i < 3; i++) {
        Serial.print(THEME_WARNING);
        Serial.print("▓");
        delay(100);
        Serial.print(THEME_ALERT);
        Serial.print("▒");
        delay(100);
        Serial.print(THEME_ACCENT);
        Serial.print("░");
        delay(100);
    }
    Serial.println(RESET);
}

}  // namespace MenuEnhanced
