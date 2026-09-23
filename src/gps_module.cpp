#include "gps_module.h"
#include "config.h"
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

namespace {
HardwareSerial gpsSerial(1); // UART1
TinyGPSPlus gps;
}

namespace GpsModule {

void begin() {
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
}

void poll() {
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }
}

bool hasFix() {
    return gps.location.isValid() && gps.location.age() < 5000;
}

double latitude() { return hasFix() ? gps.location.lat() : 0.0; }
double longitude() { return hasFix() ? gps.location.lng() : 0.0; }
double altitudeMeters() { return gps.altitude.isValid() ? gps.altitude.meters() : 0.0; }
double speedKmph() { return gps.speed.isValid() ? gps.speed.kmph() : 0.0; }
uint32_t satellites() { return gps.satellites.isValid() ? gps.satellites.value() : 0; }

String fixString() {
    if (!hasFix()) return String("NO_FIX");
    char buf[40];
    snprintf(buf, sizeof(buf), "%.6f,%.6f", latitude(), longitude());
    return String(buf);
}

} // namespace GpsModule
