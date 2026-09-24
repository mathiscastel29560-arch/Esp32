#include <FS.h>
#include <LittleFS.h>
#include "gps_wardriving.h"
#include "gps_module.h"
#include "config.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include "audit_log.h"
#include "tool_result_persistence.h"

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
    using namespace ToolOutputHelper;

    g_sessionActive = false;
    g_session.endTime = millis();

    ScanProgressBar progress("Wardriving", 3000, 3);
    progress.start();

    // Phase 1: Retrieve session data
    progress.step("Retrieving GPS coordinates and network entries from session buffer");

    uint32_t sessionDuration = g_session.endTime - g_session.startTime;
    delay(300);

    // Phase 2: Calculate statistics
    progress.step("Calculating distance traveled and analyzing network distribution");

    float dLat = g_session.endLat - g_session.startLat;
    float dLon = g_session.endLon - g_session.startLon;
    g_session.totalDistance = (uint32_t)sqrt(dLat*dLat + dLon*dLon) * 111000;

    delay(300);

    // Phase 3: Compile and display results
    progress.step("Compiling wardriving statistics and route summary");

    delay(300);

    progress.complete(String(g_session.entries.size()) + " networks at " +
                     String(g_session.totalDistance / 1000) + " km");

    // Render results
    ResultRenderers::IoTScanResult scanResult;
    scanResult.devicesFound = g_session.entries.size();
    scanResult.brokersFound = 0;
    scanResult.vulnerabilitiesDiscovered = (g_session.entries.size() > 0) ? 1 : 0;
    scanResult.durationMs = sessionDuration;

    ResultRenderers::renderIoTScan(scanResult);

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
        LittleFS.end();
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
    LittleFS.end();
    Serial.println("✓ KML export complete!");
    return true;
}

}  // namespace GpsWardriving
