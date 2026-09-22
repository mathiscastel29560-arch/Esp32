#include "drivers/rtc_driver.h"
#include "hw_config.h"
#include <Wire.h>

namespace RTCDriver {

#define DS3231_ADDRESS 0x68

static TwoWire* wire = nullptr;
static bool initialized = false;

// Convert BCD to decimal
uint8_t bcdToDec(uint8_t bcd) {
    return (bcd / 16 * 10) + (bcd % 16);
}

// Convert decimal to BCD
uint8_t decToBcd(uint8_t dec) {
    return ((dec / 10) * 16) + (dec % 10);
}

bool init() {
    if (initialized) return true;

    Serial.println("[RTC] Initializing I2C...");

    wire = new TwoWire(0);
    wire->begin(RTC_I2C_SDA, RTC_I2C_SCL, 100000);

    delay(100);

    // Test communication by reading register 0x0F
    wire->beginTransmission(DS3231_ADDRESS);
    wire->write(0x0F);
    if (wire->endTransmission() != 0) {
        Serial.println("[RTC] Communication failed");
        return false;
    }

    initialized = true;
    Serial.println("[RTC] ✓ Initialized");
    return true;
}

void deinit() {
    if (!initialized) return;

    if (wire) {
        wire->end();
        delete wire;
        wire = nullptr;
    }

    initialized = false;
}

DateTime getDateTime() {
    DateTime dt = {2026, 1, 1, 0, 0, 0};

    if (!initialized) return dt;

    wire->beginTransmission(DS3231_ADDRESS);
    wire->write(0x00);  // Start at seconds register
    if (wire->endTransmission() != 0) return dt;

    wire->requestFrom(DS3231_ADDRESS, 7);

    uint8_t seconds = bcdToDec(wire->read());
    uint8_t minutes = bcdToDec(wire->read());
    uint8_t hours = bcdToDec(wire->read() & 0x3F);
    uint8_t dayOfWeek = wire->read();
    uint8_t day = bcdToDec(wire->read());
    uint8_t month = bcdToDec(wire->read() & 0x1F);
    uint8_t year = bcdToDec(wire->read());

    dt.second = seconds;
    dt.minute = minutes;
    dt.hour = hours;
    dt.day = day;
    dt.month = month;
    dt.year = 2000 + year;

    return dt;
}

bool setDateTime(const DateTime& dt) {
    if (!initialized) return false;

    wire->beginTransmission(DS3231_ADDRESS);
    wire->write(0x00);
    wire->write(decToBcd(dt.second));
    wire->write(decToBcd(dt.minute));
    wire->write(decToBcd(dt.hour));
    wire->write(1);  // Day of week
    wire->write(decToBcd(dt.day));
    wire->write(decToBcd(dt.month));
    wire->write(decToBcd(dt.year % 100));

    if (wire->endTransmission() != 0) {
        Serial.println("[RTC] Write failed");
        return false;
    }

    Serial.printf("[RTC] Set to %04d-%02d-%02d %02d:%02d:%02d\n",
                  dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    return true;
}

float getTemperature() {
    if (!initialized) return 0.0f;

    wire->beginTransmission(DS3231_ADDRESS);
    wire->write(0x11);  // Temperature registers start at 0x11
    if (wire->endTransmission() != 0) return 0.0f;

    wire->requestFrom(DS3231_ADDRESS, 2);

    uint8_t tempH = wire->read();
    uint8_t tempL = wire->read();

    float temp = tempH + ((tempL >> 6) * 0.25f);
    return temp;
}

bool isBatteryLow() {
    if (!initialized) return false;

    wire->beginTransmission(DS3231_ADDRESS);
    wire->write(0x0F);  // Status register
    if (wire->endTransmission() != 0) return false;

    wire->requestFrom(DS3231_ADDRESS, 1);
    uint8_t status = wire->read();

    // Bit 4 is OSF (oscillator stop flag)
    return (status & 0x10) != 0;
}

}  // namespace RTCDriver
