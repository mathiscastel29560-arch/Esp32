#include "wardriving.h"
#include "gps_module.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <time.h>

namespace Wardriving {

static File wardrivingLog;
static bool logging = false;

void start() {
    if (!LittleFS.exists("/logs")) {
        LittleFS.mkdir("/logs");
    }

    wardrivingLog = LittleFS.open("/logs/wardriving.csv", "a");
    if (!wardrivingLog) {
        Serial.println("Failed to open wardriving log");
        return;
    }

    wardrivingLog.println("timestamp,ssid,bssid,channel,rssi,encryption,lat,lon");
    logging = true;
    Serial.println("Wardriving started - logging to /logs/wardriving.csv");
}

void snapshot() {
    if (!logging) return;

    int networks = WiFi.scanNetworks(false, false, false);

    for (int i = 0; i < networks; i++) {
        String line = String(time(nullptr)) + ",";
        line += WiFi.SSID(i) + ",";
        line += WiFi.BSSIDstr(i) + ",";
        line += String(WiFi.channel(i)) + ",";
        line += String(WiFi.RSSI(i)) + ",";

        uint8_t auth = WiFi.encryptionType(i);
        if (auth == WIFI_AUTH_OPEN) line += "OPEN";
        else if (auth == WIFI_AUTH_WEP) line += "WEP";
        else if (auth == WIFI_AUTH_WPA_PSK) line += "WPA";
        else if (auth == WIFI_AUTH_WPA2_PSK) line += "WPA2";
        else if (auth == WIFI_AUTH_WPA_WPA2_PSK) line += "WPA/WPA2";
        else line += "OTHER";

        line += ",";
        line += String(GpsModule::getLatitude(), 6) + ",";
        line += String(GpsModule::getLongitude(), 6);

        wardrivingLog.println(line);
    }

    wardrivingLog.flush();
}

void stop() {
    if (logging) {
        wardrivingLog.close();
        logging = false;
        Serial.println("Wardriving stopped");
    }
}

} // namespace Wardriving
