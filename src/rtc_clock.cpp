#include "rtc_clock.h"
#include "config.h"
#include <Wire.h>

namespace {
RTC_DS3231 rtc;
bool available = false;
}

namespace RtcClock {

bool begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    available = rtc.begin();
    if (available && rtc.lostPower()) {
        // Keep counting from build time rather than 2000-01-01 until the
        // user sets it properly (web UI) or a GPS fix arrives.
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    return available;
}

bool isRunning() { return available; }

DateTime now() {
    if (!available) return DateTime((uint32_t)0);
    return rtc.now();
}

String isoTimestamp() {
    DateTime dt = now();
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
    return String(buf);
}

String fileTimestamp() {
    DateTime dt = now();
    char buf[16];
    snprintf(buf, sizeof(buf), "%04d%02d%02d_%02d%02d%02d",
             dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
    return String(buf);
}

void adjust(const DateTime &dt) {
    if (available) rtc.adjust(dt);
}

} // namespace RtcClock
