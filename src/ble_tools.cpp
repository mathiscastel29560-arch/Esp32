#include "ble_tools.h"
#include <NimBLEDevice.h>
#include <algorithm>

namespace {
// A handful of common Bluetooth SIG company IDs (little-endian in the
// manufacturer-data field). Not exhaustive — see the Bluetooth SIG's
// assigned-numbers list for the rest.
String manufacturerName(uint16_t id) {
    switch (id) {
        case 0x004C: return "Apple";
        case 0x0075: return "Samsung";
        case 0x00E0: return "Google";
        case 0x0006: return "Microsoft";
        case 0x000F: return "Broadcom";
        case 0x0059: return "Nordic Semiconductor";
        case 0x0157: return "Xiaomi";
        default: return "";
    }
}
}

namespace BleTools {

void begin() {
    NimBLEDevice::init("");
}

std::vector<BleDevice> scan(uint32_t durationSeconds) {
    std::vector<BleDevice> out;

    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(90);

    NimBLEScanResults results = pScan->start(durationSeconds, false);
    int count = results.getCount();
    for (int i = 0; i < count; i++) {
        NimBLEAdvertisedDevice dev = results.getDevice(i);
        BleDevice d;
        d.address = String(dev.getAddress().toString().c_str());
        d.randomAddress = dev.getAddress().getType() != 0; // 0 = BLE_ADDR_PUBLIC
        d.name = dev.haveName() ? String(dev.getName().c_str()) : String("");
        d.rssi = dev.haveRSSI() ? dev.getRSSI() : 0;

        if (dev.haveManufacturerData()) {
            std::string data = dev.getManufacturerData();
            if (data.size() >= 2) {
                uint16_t id = (uint8_t)data[0] | ((uint8_t)data[1] << 8);
                d.manufacturerId = id;
                d.manufacturerName = manufacturerName(id);
            }
        }

        for (uint8_t u = 0; u < dev.getServiceUUIDCount(); u++) {
            d.serviceUuids.push_back(String(dev.getServiceUUID(u).toString().c_str()));
        }

        out.push_back(d);
    }
    pScan->clearResults();

    std::sort(out.begin(), out.end(),
              [](const BleDevice &a, const BleDevice &b) { return a.rssi > b.rssi; });
    return out;
}

} // namespace BleTools
