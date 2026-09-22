#include "gps_spoof.h"
#include "tx_arm.h"
#include "config.h"
#include <math.h>

namespace {
volatile bool g_spoofActive = false;
uint32_t g_packetsCount = 0;
float g_currentLat = 0;
float g_currentLon = 0;

// NMEA GPS sentence generator (simulates GNSS receiver output)
String generateNMEA(float lat, float lon, uint32_t timestamp) {
    // Validate coordinate ranges upfront to prevent any issues
    if (lat < -90.0f || lat > 90.0f || lon < -180.0f || lon > 180.0f) {
        return String("$GPGGA,0,0,N,0,E,0,0,0,0,M,0,M,,*00");
    }

    // Convert to degrees, minutes, seconds format for NMEA
    float latAbs = fabs(lat);
    float latDeg = floor(latAbs);
    float latMin = (latAbs - latDeg) * 60.0f;

    float lonAbs = fabs(lon);
    float lonDeg = floor(lonAbs);
    float lonMin = (lonAbs - lonDeg) * 60.0f;

    // Clamp to ensure format safety
    if (latMin >= 60.0f) latMin = 59.999f;
    if (lonMin >= 60.0f) lonMin = 59.999f;

    char latStr[20], lonStr[20];
    snprintf(latStr, sizeof(latStr), "%02.0f%06.3f", latDeg, latMin);
    snprintf(lonStr, sizeof(lonStr), "%03.0f%06.3f", lonDeg, lonMin);

    char nsEW[2] = {lat >= 0 ? 'N' : 'S', lon >= 0 ? 'E' : 'W'};

    // GGA sentence: $GPGGA,hhmmss.ss,ddmm.mmmm,N,dddmm.mmmm,E,1,08,0.9,545.4,M,46.9,M,,*47
    char sentence[128];
    uint8_t hour = (timestamp / 3600000) % 24;
    uint8_t min = (timestamp / 60000) % 60;
    uint8_t sec = (timestamp / 1000) % 60;

    snprintf(sentence, sizeof(sentence), "$GPGGA,%02d%02d%02d.00,%s,%c,%s,%c,1,08,0.9,545.4,M,46.9,M,,",
            hour, min, sec, latStr, nsEW[0], lonStr, nsEW[1]);

    return String(sentence);
}

void generateRawSignal(float lat, float lon) {
    // Simulate raw GPS/GNSS signal strength packets
    // In reality, this would be modulated RF signals
    // Here we just count simulation packets

    g_currentLat = lat;
    g_currentLon = lon;
    g_packetsCount++;
}

void sendSpoofSignal(float lat, float lon, const String &method) {
    if (method == "SIGNAL") {
        // Direct signal simulation
        generateRawSignal(lat, lon);
    } else if (method == "GRADUAL") {
        // Gradually drift coordinates
        float drift = sin(millis() / 1000.0f) * 0.001f;
        generateRawSignal(lat + drift, lon + drift);
    } else if (method == "RANDOM") {
        // Random jitter
        float jitter_lat = (((esp_random() % 200) + -100) / 100000.0f);
        float jitter_lon = (((esp_random() % 200) + -100) / 100000.0f);
        generateRawSignal(lat + jitter_lat, lon + jitter_lon);
    }
}
}

namespace GPSSpoof {

SpoofResult spoofGPS(float latitude, float longitude, uint32_t durationMs, const String &method) {
    SpoofResult result{false, 0, latitude, longitude, durationMs, method};

    Serial.println("\n=== GPS Spoofing ===");
    Serial.println("Target: " + String(latitude, 6) + ", " + String(longitude, 6));
    Serial.println("Method: " + method);
    Serial.println("Duration: " + String(durationMs) + "ms");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed (hold BACK button)");
        return result;
    }

    g_spoofActive = true;
    g_packetsCount = 0;
    uint32_t startTime = millis();

    Serial.println("Starting GPS spoofing...");
    Serial.println("⚠️  This simulates GPS signal spoofing");
    Serial.println("NMEA: " + generateNMEA(latitude, longitude, startTime));

    while (millis() - startTime < durationMs && g_spoofActive) {
        if (method == "SIGNAL") {
            // High frequency signal simulation (real GPS receivers at ~1Hz)
            sendSpoofSignal(latitude, longitude, method);
            delay(100);  // Simulate 10Hz update rate
        } else if (method == "GRADUAL") {
            // Slow drift over time (makes detection harder)
            float elapsedSec = (millis() - startTime) / 1000.0f;
            float driftedLat = latitude + (sin(elapsedSec * 0.5f) * 0.01f);
            float driftedLon = longitude + (cos(elapsedSec * 0.5f) * 0.01f);
            sendSpoofSignal(driftedLat, driftedLon, method);
            delay(500);
        } else if (method == "RANDOM") {
            // Rapid random jitter
            sendSpoofSignal(latitude, longitude, method);
            delay(50);
        }

        if (g_packetsCount % 20 == 0) {
            Serial.println("  [" + String(g_packetsCount) + "] packets in " +
                         String(millis() - startTime) + "ms @ " +
                         String(g_currentLat, 6) + ", " + String(g_currentLon, 6));
        }
    }

    g_spoofActive = false;
    result.success = true;
    result.packetsCount = g_packetsCount;

    Serial.println("✓ GPS spoofing complete");
    Serial.println("Total packets: " + String(result.packetsCount));
    Serial.println("Final coords: " + String(g_currentLat, 6) + ", " + String(g_currentLon, 6));
    Serial.println("Duration: " + String(millis() - startTime) + "ms");
    Serial.println("⚠️  Devices in range receive spoofed GPS signals");

    return result;
}

void stop() {
    g_spoofActive = false;
    Serial.println("GPS spoofing stopped");
}

bool isActive() {
    return g_spoofActive;
}

}  // namespace GPSSpoof
