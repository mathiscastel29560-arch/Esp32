#include "ble_passive_sniffer.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <vector>

namespace BLEPassiveSniffer {

static std::vector<BLEAdvertisement> discoveries;

SniffResult passiveSniff(uint32_t durationMs) {
    SniffResult result = {false, 0, 0, 0, 0};
    discoveries.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== BLE Passive Sniffer (REAL Non-Transmitting Scan) ===");
    Serial.printf("Duration: %lums (Passive - No TX)\n", durationMs);

    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    
    if (!pBLEScan) {
        Serial.println("✗ BLE Scan initialization failed");
        return result;
    }

    pBLEScan->setActiveScan(false);  // PASSIVE - no transmission
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    uint32_t deviceCount = 0;
    int32_t totalRssi = 0;
    
    while ((millis() - startTime) < durationMs) {
        BLEScanResults foundDevices = pBLEScan->start(2, false);  // 2 sec passive scan
        
        for (int i = 0; i < foundDevices.getCount(); i++) {
            BLEAdvertisedDevice device = foundDevices.getDevice(i);
            
            BLEAdvertisement adv;
            adv.macAddress = device.getAddress().toString().c_str();
            adv.deviceName = device.getName().c_str();
            adv.rssi = device.getRSSI();
            adv.timestamp = millis();
            adv.flags = "ADV_IND";
            
            if (adv.rssi != 0) {  // Valid RSSI
                discoveries.push_back(adv);
                deviceCount++;
                totalRssi += adv.rssi;
                
                Serial.printf("✓ Device: %s | RSSI: %d\n", 
                             adv.macAddress.c_str(), adv.rssi);
            }
        }

        pBLEScan->clearResults();
        delay(500);
    }

    BLEDevice::deinit(false);

    result.success = (deviceCount > 0);
    result.devicesDiscovered = deviceCount;
    result.packetsCapture = deviceCount;
    result.averageRssi = (deviceCount > 0) ? totalRssi / deviceCount : 0;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Passive scan complete: %u devices discovered\n", deviceCount);

    return result;
}

const BLEAdvertisement* getDiscoveredAdvertisements(uint32_t& outCount) {
    outCount = discoveries.size();
    return discoveries.empty() ? nullptr : discoveries.data();
}

DeviceAnalysis analyzeDevice(const char* macAddress, uint32_t durationMs) {
    DeviceAnalysis result = {false, 0, "", -100};
    uint32_t startTime = millis();

    Serial.println("\n=== BLE Device Analysis ===");
    Serial.printf("Target: %s\n", macAddress);

    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(false);

    uint32_t advertisementCount = 0;
    int strongestRssi = -100;

    while ((millis() - startTime) < durationMs) {
        BLEScanResults foundDevices = pBLEScan->start(2, false);
        
        for (int i = 0; i < foundDevices.getCount(); i++) {
            BLEAdvertisedDevice device = foundDevices.getDevice(i);

            std::string devAddr = device.getAddress().toString();
            if (strcmp(devAddr.c_str(), macAddress) == 0) {
                advertisementCount++;
                if (device.getRSSI() > strongestRssi) {
                    strongestRssi = device.getRSSI();
                }
            }
        }

        pBLEScan->clearResults();
        delay(500);
    }

    BLEDevice::deinit(false);

    result.success = (advertisementCount > 0);
    result.advertisementCount = advertisementCount;
    result.strongestRssi = strongestRssi;
    result.deviceVendor = "Unknown";

    Serial.printf("✓ Device seen %u times, strongest: %d dBm\n", 
                 advertisementCount, strongestRssi);

    return result;
}

}  // namespace BLEPassiveSniffer
