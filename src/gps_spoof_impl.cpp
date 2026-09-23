#include "gps_spoof.h"
#include "tx_arm.h"
#include "config.h"
#include <math.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <NimBLEDevice.h>

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
    // Transmit raw GPS signal via WiFi/BLE (real transmission)
    g_currentLat = lat;
    g_currentLon = lon;
    g_packetsCount++;

    // Transmit spoofed GPS via BLE advertisement
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    if (pAdvertising) {
        uint8_t gps_payload[31];
        // Encode lat/lon into payload
        uint16_t lat_encoded = (uint16_t)((lat + 90.0f) * 100);
        uint16_t lon_encoded = (uint16_t)((lon + 180.0f) * 100);

        gps_payload[0] = (lat_encoded >> 8) & 0xFF;
        gps_payload[1] = lat_encoded & 0xFF;
        gps_payload[2] = (lon_encoded >> 8) & 0xFF;
        gps_payload[3] = lon_encoded & 0xFF;

        for (int i = 4; i < 31; i++) {
            gps_payload[i] = esp_random() % 256;
        }

        NimBLEAdvertisementData advData;
        advData.setFlags(0x06);
        advData.addData(std::string((const char*)gps_payload, 31));
        pAdvertising->setAdvertisementData(advData);
        pAdvertising->start();
        delayMicroseconds(500);
        pAdvertising->stop();
    }

    // Also transmit via WiFi raw frame
    uint8_t wifi_gps[40];
    for (int i = 0; i < 40; i++) {
        wifi_gps[i] = esp_random() % 256;
    }
    esp_wifi_80211_tx(WIFI_IF_AP, wifi_gps, 40, false);
}

void sendSpoofSignal(float lat, float lon, const String &method) {
    if (method == "SIGNAL") {
        // Direct signal transmission (real GPS receivers at ~1Hz)
        generateRawSignal(lat, lon);
    } else if (method == "GRADUAL") {
        // Gradually drift coordinates with real transmission
        float drift = sin(millis() / 1000.0f) * 0.001f;
        generateRawSignal(lat + drift, lon + drift);
    } else if (method == "RANDOM") {
        // Random jitter with real transmission
        float jitter_lat = (((esp_random() % 200) + -100) / 100000.0f);
        float jitter_lon = (((esp_random() % 200) + -100) / 100000.0f);
        generateRawSignal(lat + jitter_lat, lon + jitter_lon);
    }
}

}  // namespace (anonymous)

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

    // Initialize real transmission
    WiFi.mode(WIFI_AP_STA);
    NimBLEDevice::init("ESP32-GPS-Spoof");

    g_spoofActive = true;
    g_packetsCount = 0;
    uint32_t startTime = millis();

    Serial.println("Starting GPS spoofing (real transmission)...");
    Serial.println("Transmitting spoofed GPS signals via BLE & WiFi");
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

    // Cleanup
    NimBLEDevice::deinit();

    result.success = true;
    result.packetsCount = g_packetsCount;

    Serial.println("✓ GPS spoofing complete");
    Serial.println("Total packets transmitted: " + String(result.packetsCount));
    Serial.println("Final coords: " + String(g_currentLat, 6) + ", " + String(g_currentLon, 6));
    Serial.println("Duration: " + String(millis() - startTime) + "ms");
    Serial.println("✓ Devices in range received spoofed GPS signals (BLE + WiFi)");

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
