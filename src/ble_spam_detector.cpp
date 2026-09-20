#include "ble_spam_detector.h"
#include "config.h"
#include <NimBLEDevice.h>
#include <vector>
#include <algorithm>

namespace {
struct Sighting {
    uint32_t ts;
    String mac;
    String type;
    int rssi;
};

std::vector<Sighting> g_sightings;
portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;

String classify(NimBLEAdvertisedDevice *dev) {
    if (dev->haveManufacturerData()) {
        std::string data = dev->getManufacturerData();
        if (data.size() >= 3 && (uint8_t)data[0] == 0x4C && (uint8_t)data[1] == 0x00) {
            return "Apple Continuity";
        }
        if (data.size() >= 3 && (uint8_t)data[0] == 0x06 && (uint8_t)data[1] == 0x00 &&
            (uint8_t)data[2] == 0x03) {
            return "Swift Pair";
        }
    }
    if (dev->isAdvertisingService(NimBLEUUID((uint16_t)0xFE2C))) {
        return "Fast Pair";
    }
    return "";
}

class SpamCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice *dev) override {
        if (dev->getAddress().getType() == 0) return; // public addresses aren't spam-beacon candidates
        String type = classify(dev);
        if (type.length() == 0) return;

        Sighting s;
        s.ts = millis();
        s.mac = String(dev->getAddress().toString().c_str());
        s.type = type;
        s.rssi = dev->haveRSSI() ? dev->getRSSI() : -100;

        portENTER_CRITICAL(&g_mux);
        g_sightings.push_back(s);
        portEXIT_CRITICAL(&g_mux);
    }
};

SpamCallbacks g_callbacks;
bool g_active = false;
} // namespace

namespace BleSpamDetector {

void begin() {
    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(&g_callbacks, true);
    pScan->setActiveScan(false); // passive: never solicit a scan response
    pScan->setInterval(100);
    pScan->setWindow(100);
    pScan->setDuplicateFilter(false);
}

void start() {
    NimBLEDevice::getScan()->start(0, (void (*)(NimBLEScanResults))nullptr, false);
    g_active = true;
}

void stop() {
    NimBLEDevice::getScan()->stop();
    g_active = false;
}

bool active() { return g_active; }

Alert checkAlert() {
    Alert alert;
    uint32_t now = millis();

    portENTER_CRITICAL(&g_mux);
    g_sightings.erase(std::remove_if(g_sightings.begin(), g_sightings.end(),
                                      [&](const Sighting &s) {
                                          return now - s.ts > BLE_SPAM_WINDOW_MS;
                                      }),
                       g_sightings.end());
    std::vector<Sighting> snapshot = g_sightings;
    portEXIT_CRITICAL(&g_mux);

    const char *types[] = {"Apple Continuity", "Fast Pair", "Swift Pair"};
    for (const char *t : types) {
        std::vector<String> macs;
        int strongest = -127;
        for (auto &s : snapshot) {
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
