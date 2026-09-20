#pragma once
#include <Arduino.h>

// NEO-6M GPS reader (TinyGPS++ over UART1). Call GpsModule::poll() often
// (every loop()) to keep feeding the NMEA parser; the getters below give the
// most recently decoded fix.
namespace GpsModule {

void begin();
void poll();

bool hasFix();
double latitude();
double longitude();
double altitudeMeters();
double speedKmph();
uint32_t satellites();

// "lat,lon" or "NO_FIX" — used directly as a CSV field by the wardriving logger
String fixString();

} // namespace GpsModule
