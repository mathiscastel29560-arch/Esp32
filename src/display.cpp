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

} // namespace Display
