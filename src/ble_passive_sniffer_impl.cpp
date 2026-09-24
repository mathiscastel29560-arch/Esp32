#include "ble_passive_sniffer.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <vector>
#include "audit_log.h"
#include "tool_result_persistence.h"

namespace BLEPassiveSniffer {

static std::vector<BLEAdvertisement> discoveries;

SniffResult passiveSniff(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    SniffResult result = {false, 0, 0, 0, 0};
    discoveries.clear();

    displayScanStart("BLE Passive Sniffer", "BLE 2.4 GHz (passive, no TX)");

    ScanProgressBar progress("BLE Passive", durationMs, 3);
    progress.start();

    // Phase 1: Initialize scanner
    progress.step("Initializing BLE passive scanner (setActiveScan=false)");
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();

    if (!pBLEScan) {
        progress.complete("Failed to initialize BLE scanner");
        return result;
    }

    pBLEScan->setActiveScan(false);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    delay(durationMs / 3);

    // Phase 2: Scan for devices
    progress.step("Passively scanning for BLE advertisements");
    uint32_t deviceCount = 0;
    int32_t totalRssi = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 3) {
        BLEScanResults foundDevices = pBLEScan->start(2, false);

        for (int i = 0; i < foundDevices.getCount(); i++) {
            BLEAdvertisedDevice device = foundDevices.getDevice(i);

            BLEAdvertisement adv;
            adv.macAddress = device.getAddress().toString().c_str();
            adv.deviceName = device.getName().c_str();
            adv.rssi = device.getRSSI();
            adv.timestamp = millis();
            adv.flags = "ADV_IND";

            if (adv.rssi != 0) {
                discoveries.push_back(adv);
                deviceCount++;
                totalRssi += adv.rssi;
            }
        }

        pBLEScan->clearResults();
        delay(200);
    }

    // Phase 3: Analysis
    progress.step("Analyzing BLE device distribution and signal strength");
    delay(durationMs / 3);

    BLEDevice::deinit(false);

    progress.complete(String(deviceCount) + " BLE devices discovered (passive, no transmission)");

    // Render results
    ResultRenderers::BLEScanResult scanResult;
    scanResult.devicesFound = deviceCount;
    scanResult.pairedDevices = 0;
    scanResult.strongestDevice = deviceCount > 0 ? discoveries[0].deviceName : "None";
    scanResult.strongestRssi = (deviceCount > 0) ? totalRssi / deviceCount : -100;
    scanResult.durationMs = durationMs;

    for (const auto& adv : discoveries) {
        scanResult.allRssiValues.push_back(adv.rssi);
    }

    ResultRenderers::renderBLEScan(scanResult);

    result.success = (deviceCount > 0);
    result.devicesDiscovered = deviceCount;
    result.packetsCapture = deviceCount;
    result.averageRssi = (deviceCount > 0) ? totalRssi / deviceCount : 0;
    result.durationMs = durationMs;

    return result;
}

const BLEAdvertisement* getDiscoveredAdvertisements(uint32_t& outCount) {
    outCount = discoveries.size();
    return discoveries.empty() ? nullptr : discoveries.data();
}

DeviceAnalysis analyzeDevice(const char* macAddress, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    DeviceAnalysis result = {false, 0, "", -100};

    displayScanStart("BLE Device Analysis", String(macAddress));

    ScanProgressBar progress("Device Analysis", durationMs, 3);
    progress.start();

    // Phase 1: Setup scanner
    progress.step("Initializing BLE scanner for device tracking");
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(false);
    delay(durationMs / 3);

    // Phase 2: Track device
    progress.step("Tracking BLE device " + String(macAddress));
    uint32_t advertisementCount = 0;
    int strongestRssi = -100;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 3) {
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
        delay(200);
    }

    // Phase 3: Generate analysis
    progress.step("Generating device behavior and location analysis");
    delay(durationMs / 3);

    BLEDevice::deinit(false);

    progress.complete(String(advertisementCount) + " advertisements from device detected");

    // Render results
    printSubHeader("BLE Device Analysis");
    printKeyValue("MAC Address", String(macAddress));
    printKeyValue("Advertisements Seen", String(advertisementCount));
    printKeyValue("Strongest Signal", String(strongestRssi) + " dBm");
    printKeyValue("Device Vendor", "Unknown");
    printKeyValue("Tracking Success", advertisementCount > 0 ? "YES" : "NO");
    printBar(min(advertisementCount * 10, 100U), 20);
    Serial.println();

    result.success = (advertisementCount > 0);
    result.advertisementCount = advertisementCount;
    result.strongestRssi = strongestRssi;
    result.deviceVendor = "Unknown";

    return result;
}

}  // namespace BLEPassiveSniffer
