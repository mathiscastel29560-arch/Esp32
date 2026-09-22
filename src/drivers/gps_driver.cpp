#include "drivers/gps_driver.h"
#include "hw_config.h"

namespace GPSDriver {

static HardwareSerial* gpsSerial = nullptr;
static Location currentLocation = {0, 0, 0, 0, 0, false};
static bool initialized = false;

// Parse NMEA GPGGA sentence
void parseGPGGA(const String& sentence) {
    // Format: $GPGGA,time,lat,N/S,lon,E/W,fix,sat,HDOP,alt,M,geoid,M
    if (!sentence.startsWith("$GPGGA")) return;

    int lastComma = 0;
    int commaCount = 0;
    double latVal = 0, lonVal = 0;

    for (int i = 0; i < sentence.length(); i++) {
        if (sentence[i] == ',') {
            String field = sentence.substring(lastComma + 1, i);

            switch (commaCount) {
                case 1: {  // UTC time
                    break;
                }
                case 2: {  // Latitude
                    if (field.length() > 0) {
                        latVal = field.toFloat();
                        currentLocation.latitude = latVal / 100.0 + (int)(latVal / 100) / 100.0;
                    }
                    break;
                }
                case 3: {  // N/S
                    if (field == "S") currentLocation.latitude = -currentLocation.latitude;
                    break;
                }
                case 4: {  // Longitude
                    if (field.length() > 0) {
                        lonVal = field.toFloat();
                        currentLocation.longitude = lonVal / 100.0 + (int)(lonVal / 100) / 100.0;
                    }
                    break;
                }
                case 5: {  // E/W
                    if (field == "W") currentLocation.longitude = -currentLocation.longitude;
                    break;
                }
                case 6: {  // Fix quality (0=none, 1=GPS)
                    currentLocation.hasFix = (field.toInt() > 0);
                    break;
                }
                case 7: {  // Number of satellites
                    currentLocation.satellites = field.toInt();
                    break;
                }
                case 9: {  // Altitude
                    currentLocation.altitude = field.toFloat();
                    break;
                }
            }

            lastComma = i;
            commaCount++;
        }
    }
}

bool init() {
    if (initialized) return true;

    // Guard: Check if GpsModule (existing) is already using UART1
    extern HardwareSerial Serial0;  // UART0 (console)
    extern HardwareSerial Serial1;  // UART1 (potentially GpsModule)

    // Check if UART1 is already in use by detecting if Serial1 has been initialized
    if (Serial1.baudRate() > 0) {
        Serial.println("[GPS] ⚠️  UART1 already in use (GpsModule active)");
        Serial.println("[GPS] Cannot initialize GPSDriver - use existing GpsModule instead");
        return false;
    }

    Serial.println("[GPS] Initializing UART1...");

    gpsSerial = new HardwareSerial(1);  // UART1 (pin 18 RX, 17 TX)
    gpsSerial->begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

    delay(500);
    initialized = true;
    Serial.println("[GPS] ✓ Initialized");
    return true;
}

void deinit() {
    if (!initialized) return;

    if (gpsSerial) {
        gpsSerial->end();
        delete gpsSerial;
        gpsSerial = nullptr;
    }

    initialized = false;
}

void update() {
    if (!initialized || !gpsSerial) return;

    static String buffer = "";

    while (gpsSerial->available()) {
        char c = gpsSerial->read();

        if (c == '\n') {
            parseGPGGA(buffer);
            buffer = "";
        } else if (c != '\r') {
            buffer += c;
        }
    }
}

Location getLocation() {
    return currentLocation;
}

bool hasFix() {
    return currentLocation.hasFix;
}

uint8_t getSatelliteCount() {
    return currentLocation.satellites;
}

}  // namespace GPSDriver
