#include "wardriving.h"
#include "config.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "wifi_tools.h"
#include "ble_tools.h"
#include <LittleFS.h>

namespace {
size_t g_rowCount = 0;

void ensureLog() {
    if (!LittleFS.exists(LOG_DIR)) LittleFS.mkdir(LOG_DIR);
    if (!LittleFS.exists(WARDRIVE_LOG_FILE)) {
        File f = LittleFS.open(WARDRIVE_LOG_FILE, FILE_WRITE);
        if (f) {
            f.println("timestamp,gps,type,identifier,name_or_ssid,rssi,extra");
            f.close();
        }
    }
}

void appendRow(const String &type, const String &identifier, const String &nameOrSsid,
               int rssi, const String &extra) {
    File f = LittleFS.open(WARDRIVE_LOG_FILE, FILE_APPEND);
    if (!f) return;
    f.printf("%s,%s,%s,%s,%s,%d,%s\n",
             RtcClock::isoTimestamp().c_str(), GpsModule::fixString().c_str(),
             type.c_str(), identifier.c_str(), nameOrSsid.c_str(), rssi, extra.c_str());
    f.close();
    g_rowCount++;
}
}

namespace Wardriving {

void begin() {
    ensureLog();
}

size_t captureSnapshot(uint32_t bleScanSeconds) {
    size_t before = g_rowCount;

    for (auto &ap : WifiTools::scan()) {
        String extra = "ch=" + String(ap.channel) + ";enc=" + ap.enc;
        appendRow("WIFI", ap.bssid, ap.ssid, ap.rssi, extra);
    }

    for (auto &dev : BleTools::scan(bleScanSeconds)) {
        appendRow("BLE", dev.address, dev.name, dev.rssi, "");
    }

    return g_rowCount - before;
}

size_t rowCount() { return g_rowCount; }

} // namespace Wardriving
