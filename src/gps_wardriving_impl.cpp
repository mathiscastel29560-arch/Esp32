#include "gps_wardriving.h"
#include "gps_module.h"
#include "config.h"
#include "results_display.h"
#include <LittleFS.h>

namespace GpsWardriving {

static struct CurrentSession {
    uint32_t startTime;
    uint32_t endTime;
    std::vector<WardriveEntry> entries;
    float startLat, startLon;
    float endLat, endLon;
    uint32_t totalDistance;
} g_session{0, 0, {}, 0, 0, 0, 0, 0};

static bool g_sessionActive = false;

void beginSession() {
    g_session = {};
    g_session.startTime = millis();
    g_sessionActive = true;
    
    Serial.println("\n=== GPS Wardriving Session Started ===");
    Serial.println("Waiting for GPS fix...");
}

void logWiFiNetwork(const String &ssid, const String &bssid, int8_t rssi) {
    if (!g_sessionActive || !GpsModule::hasFix()) {
        Serial.println("✗ No GPS fix or session not active");
        return;
    }
    
    WardriveEntry entry{
        (float)GpsModule::latitude(),
        (float)GpsModule::longitude(),
        millis(),
        ssid,
        rssi,
        bssid,
        "WiFi"
    };
    
    g_session.entries.push_back(entry);
    g_session.endLat = entry.latitude;
    g_session.endLon = entry.longitude;
    
    Serial.println("  Logged WiFi: " + ssid + " @ (" + String(entry.latitude, 4) + 
                  ", " + String(entry.longitude, 4) + ") RSSI:" + String(rssi));
}

void logBLEDevice(const String &name, const String &address, int8_t rssi) {
    if (!g_sessionActive || !GpsModule::hasFix()) {
        Serial.println("✗ No GPS fix or session not active");
        return;
    }
    
    WardriveEntry entry{
        (float)GpsModule::latitude(),
        (float)GpsModule::longitude(),
        millis(),
        name,
        rssi,
        address,
        "BLE"
    };
    
    g_session.entries.push_back(entry);
    g_session.endLat = entry.latitude;
    g_session.endLon = entry.longitude;
    
    Serial.println("  Logged BLE: " + name + " @ (" + String(entry.latitude, 4) + 
                  ", " + String(entry.longitude, 4) + ") RSSI:" + String(rssi));
}

WardriveSession endSession() {
    g_sessionActive = false;
    g_session.endTime = millis();
    
    // Calculate rough distance (simplified)
    float dLat = g_session.endLat - g_session.startLat;
    float dLon = g_session.endLon - g_session.startLon;
    g_session.totalDistance = (uint32_t)sqrt(dLat*dLat + dLon*dLon) * 111000;
    
    Serial.println("\n=== Wardriving Session Ended ===");
    Serial.println("Duration: " + String(g_session.endTime - g_session.startTime) + "ms");
    Serial.println("Networks logged: " + String(g_session.entries.size()));
    Serial.println("Distance: ~" + String(g_session.totalDistance / 1000) + " km");

    std::vector<String> displayLines;
    displayLines.push_back(String(g_session.entries.size()) + " network(s) logged");
    displayLines.push_back("Distance: ~" + String(g_session.totalDistance / 1000) + " km");
    displayLines.push_back("Start: " + String(g_session.startLat, 4) + ", " + String(g_session.startLon, 4));
    displayLines.push_back("End: " + String(g_session.endLat, 4) + ", " + String(g_session.endLon, 4));
    displayLines.push_back("Duration: " + String((g_session.endTime - g_session.startTime) / 1000) + "s");

    ResultsDisplay::showResult("Wardriving", {
        "Wardriving Session",
        String(g_session.entries.size()) + " network(s)",
        100,
        displayLines,
        g_session.entries.size() > 0 ? ResultsDisplay::ResultType::SUCCESS : ResultsDisplay::ResultType::INFO
    });

    return {g_session.startTime, g_session.endTime, g_session.entries,
            g_session.startLat, g_session.startLon, g_session.endLat, g_session.endLon, g_session.totalDistance};
}

bool hasGPSFix() {
    return GpsModule::hasFix();
}

bool exportToKML(const String &filename, const WardriveSession &session) {
    Serial.println("Exporting to KML: " + filename);
    
    if (!LittleFS.begin()) {
        Serial.println("✗ LittleFS mount failed");
        return false;
    }
    
    File f = LittleFS.open(filename, "w");
    if (!f) {
        Serial.println("✗ Cannot open file");
        return false;
    }
    
    f.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    f.println("<kml xmlns=\"http://www.opengis.net/kml/2.2\">");
    f.println("<Document>");
    f.println("  <name>Wardriving Session</name>");
    f.println("  <Folder>");
    f.println("    <name>Networks</name>");
    
    for (const auto &entry : session.entries) {
        f.println("    <Placemark>");
        f.println("      <name>" + entry.ssid + "</name>");
        f.println("      <description>Type: " + entry.type + " | RSSI: " + String(entry.rssi) + "dBm</description>");
        f.println("      <Point>");
        f.println("        <coordinates>" + String(entry.longitude, 6) + "," + String(entry.latitude, 6) + ",0</coordinates>");
        f.println("      </Point>");
        f.println("    </Placemark>");
    }
    
    f.println("  </Folder>");
    f.println("</Document>");
    f.println("</kml>");
    
    f.close();
    Serial.println("✓ KML export complete!");
    return true;
}

}  // namespace GpsWardriving
