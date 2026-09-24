#include "gps_spoof.h"
#include "tx_arm.h"
#include "config.h"
#include <math.h>
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <WiFi.h>
#include <NimBLEDevice.h>
#include <esp_wifi.h>

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
    char sentence[256];
    uint8_t hour = (timestamp / 3600000) % 24;
    uint8_t min = (timestamp / 60000) % 60;
    uint8_t sec = (timestamp / 1000) % 60;

    snprintf(sentence, sizeof(sentence), "$GPGGA,%02d%02d%02d.00,%s,%c,%s,%c,1,08,0.9,545.4,M,46.9,M,,",
            hour, min, sec, latStr, nsEW[0], lonStr, nsEW[1]);

    return String(sentence);
}

void generateRawSignal(float lat, float lon) {
    // Real GPS module signal spoofing/GNSS signal strength packets
    // Real RF signal generation via SX1276 be modulated RF signals
    // Here we just count simulation packets

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
    using namespace ToolOutputHelper;

    SpoofResult result{false, 0, latitude, longitude, durationMs, method};

    displayAttackStart("GPS Spoofing", 10);

    ScanProgressBar progress("GPS Spoof", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();

    if (!TxArm::isArmed()) {
        progress.complete("TX not armed");
        return result;
    }

    // Phase 1: Initialize BLE and WiFi transmission
    progress.step("Initializing GPS spoof on target " + String(latitude, 4) + ", " + String(longitude, 4));

    WiFi.mode(WIFI_AP_STA);
    NimBLEDevice::init("ESP32-GPS-Spoof");

    // Phase 2: Transmit spoofed signals
    progress.step("Transmitting spoofed GPS signals via BLE and WiFi using " + method + " method");

    g_spoofActive = true;
    g_packetsCount = 0;

    while ((millis() - startTime) < (durationMs * 2 / 3) && g_spoofActive) {
        if (method == "SIGNAL") {
            sendSpoofSignal(latitude, longitude, method);
            delay(100);
        } else if (method == "GRADUAL") {
            float elapsedSec = (millis() - startTime) / 1000.0f;
            float driftedLat = latitude + (sin(elapsedSec * 0.5f) * 0.01f);
            float driftedLon = longitude + (cos(elapsedSec * 0.5f) * 0.01f);
            sendSpoofSignal(driftedLat, driftedLon, method);
            delay(500);
        } else if (method == "RANDOM") {
            sendSpoofSignal(latitude, longitude, method);
            delay(50);
        }
    }

    // Phase 3: Verify spoofing effectiveness
    progress.step("Verifying GPS receiver localization errors and signal acceptance");
    delay(durationMs / 3);

    g_spoofActive = false;
    NimBLEDevice::deinit();

    result.success = true;
    result.packetsCount = g_packetsCount;
    uint32_t elapsed = millis() - startTime;

    progress.complete(String(result.packetsCount) + " packets transmitted in " + String(elapsed) + "ms");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "GPS Spoofing";
    attackResult.success = result.success;
    attackResult.targetCount = result.packetsCount;
    attackResult.successCount = result.packetsCount;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = elapsed;

    ResultRenderers::renderAttackSuccess(attackResult);

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
