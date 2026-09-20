#include "ble_spam_detector.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <vector>
#include <algorithm>
#include <Arduino.h>

namespace {
struct Sighting {
    uint32_t ts;
    String mac;
    String type;
    int rssi;
};

std::vector<Sighting> g_sightings;
bool g_active = false;
} // namespace

namespace BleSpamDetector {

void begin() {
    BLEDevice::init("");
    BLEScan *pScan = BLEDevice::getScan();
    pScan->setActiveScan(false);
    pScan->setInterval(100);
    pScan->setWindow(100);
    Serial.println("BLE Spam Detector initialized");
}

void start() {
    g_active = true;
    Serial.println("BLE Spam Detector started");
}

void stop() {
    g_active = false;
    Serial.println("BLE Spam Detector stopped");
}

bool active() { return g_active; }

Alert checkAlert() {
    Alert alert;
    uint32_t now = millis();

    for (int i = g_sightings.size() - 1; i >= 0; i--) {
        if (now - g_sightings[i].ts > BLE_SPAM_WINDOW_MS) {
            g_sightings.erase(g_sightings.begin() + i);
        }
    }

    const char *types[] = {"Apple Continuity", "Fast Pair", "Swift Pair"};
    for (const char *t : types) {
        std::vector<String> macs;
        int strongest = -127;
        for (auto &s : g_sightings) {
            if (s.type != t) continue;
            bool seen = false;
            for (auto &m : macs)
                if (m == s.mac) { seen = true; break; }
            if (!seen) macs.push_back(s.mac);
            if (s.rssi > strongest) strongest = s.rssi;
        }
        if ((int)macs.size() >= BLE_SPAM_DISTINCT_MAC_THRESHOLD) {
            alert.type = t;
            alert.distinctMacs = macs.size();
            alert.strongestRssi = strongest;
            return alert;
        }
    }
    return alert;
}

} // namespace BleSpamDetector
