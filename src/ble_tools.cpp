#include "ble_tools.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

namespace BleTools {

void begin() {
    BLEDevice::init("");
}

std::vector<BleDevice> scan(uint32_t durationSeconds) {
    std::vector<BleDevice> out;

    BLEScan *pScan = BLEDevice::getScan();
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(90);

    BLEScanResults results = pScan->start(durationSeconds, false);
    int count = results.getCount();
    for (int i = 0; i < count; i++) {
        BLEAdvertisedDevice dev = results.getDevice(i);
        BleDevice d;
        d.address = String(dev.getAddress().toString().c_str());
        d.name = dev.haveName() ? String(dev.getName().c_str()) : String("");
        d.rssi = dev.haveRSSI() ? dev.getRSSI() : 0;
        out.push_back(d);
    }
    pScan->clearResults();
    return out;
}

} // namespace BleTools
