#include "display.h"
#include "config.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "safety_switch.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

namespace {
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &SPI, PIN_OLED_DC, PIN_OLED_RST, PIN_OLED_CS);
}

namespace Display {

void begin() {
    oled.begin(SSD1306_SWITCHCAPVCC);
    oled.setTextColor(SSD1306_WHITE);
    oled.cp437(true);
    splash("ESP32 Audit Tool", "booting...");
}

void splash(const String &line1, const String &line2) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(line1);
    oled.println(line2);
    oled.display();
}

void update(const String &lastAction) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);

    oled.println(RtcClock::isoTimestamp());

    oled.print("GPS: ");
    if (GpsModule::hasFix()) {
        oled.print("FIX sats=");
        oled.println(GpsModule::satellites());
    } else {
        oled.println("no fix");
    }

    oled.print("AP clients: ");
    oled.println(WiFi.softAPgetStationNum());

    oled.print("TX safety: ");
    oled.println(SafetySwitch::isArmed() ? "ARMED" : "SAFE");

    oled.println("----------------");
    // last action line, truncated to fit 21 chars at text size 1 on a 128px panel
    oled.println(lastAction.substring(0, 21));

    oled.display();
}

void showList(const String &title, const std::vector<String> &items, int selectedIndex) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(title);
    oled.println("----------------");

    const int visibleRows = 5;
    int start = selectedIndex - visibleRows / 2;
    if (start < 0) start = 0;
    if ((int)items.size() > visibleRows && start > (int)items.size() - visibleRows) {
        start = items.size() - visibleRows;
    }

    for (int i = start; i < (int)items.size() && i < start + visibleRows; i++) {
        oled.print(i == selectedIndex ? "> " : "  ");
        oled.println(items[i].substring(0, 19));
    }
    oled.display();
}

void showText(const String &title, const String &body) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println(title);
    oled.println("----------------");
    oled.println(body);
    oled.display();
}

} // namespace Display
