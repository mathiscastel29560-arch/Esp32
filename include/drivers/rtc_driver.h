#pragma once
#include <Arduino.h>

namespace RTCDriver {

struct DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

// Initialize RTC (I2C)
bool init();
void deinit();

// Get current time from RTC
DateTime getDateTime();

// Set time on RTC
bool setDateTime(const DateTime& dt);

// Get temperature from RTC sensor (DS3231 has internal temp)
float getTemperature();

// Check if RTC battery is low
bool isBatteryLow();

}  // namespace RTCDriver
