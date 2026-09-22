#include "ble_fuzzer.h"
#include <NimBLEDevice.h>

namespace {
NimBLEAdvertisedDevice *findDevice(NimBLEScan *pScan, NimBLEScanResults &results, const String &address) {
    for (auto it = results.begin(); it != results.end(); ++it) {
        if (String((*it)->getAddress().toString().c_str()).equalsIgnoreCase(address)) {
            return *it;
        }
    }
    return nullptr;
}
} // namespace

namespace BleFuzzer {

FuzzReport fuzz(const String &address, uint32_t scanTimeoutSeconds) {
    FuzzReport report;

    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(true);
    NimBLEScanResults results = pScan->start(scanTimeoutSeconds, false);

    NimBLEAdvertisedDevice *target = findDevice(pScan, results, address);
    if (!target) {
        pScan->clearResults();
        return report;
    }

    NimBLEClient *client = NimBLEDevice::createClient();
    report.connected = client->connect(target);
    if (!report.connected) {
        NimBLEDevice::deleteClient(client);
        pScan->clearResults();
        return report;
    }

    // --- Oversized writes + writes to read-only characteristics ---
    static uint8_t oversized[512];
    memset(oversized, 0xFF, sizeof(oversized));

    auto *services = client->getServices(true);
    if (!services) {
        client->disconnect();
        NimBLEDevice::deleteClient(client);
        pScan->clearResults();
        return report;
    }

    for (auto *svc : *services) {
        auto *chars = svc->getCharacteristics(true);
        if (!chars) continue;
        for (auto *chr : *chars) {
            if (chr->canWrite() || chr->canWriteNoResponse()) {
                report.oversizedWritesAttempted++;
                if (chr->writeValue(oversized, sizeof(oversized), true)) {
                    report.oversizedWritesAccepted++;
                }
            } else if (chr->canRead()) {
                // read-only characteristic: try writing to it anyway
                report.readOnlyWritesAttempted++;
                uint8_t probe[4] = {0xDE, 0xAD, 0xBE, 0xEF};
                if (chr->writeValue(probe, sizeof(probe), true)) {
                    report.readOnlyWritesAccepted++;
                }
            }
        }
    }

    // --- Rapid connect/disconnect cycles ---
    constexpr int CYCLES = 10;
    for (int i = 0; i < CYCLES; i++) {
        report.reconnectCyclesAttempted++;
        client->disconnect();
        delay(50);
        if (!client->connect(target)) {
            report.reconnectCyclesFailed++;
        }
    }

    report.deviceUnresponsiveAtEnd = !client->isConnected();

    client->disconnect();
    NimBLEDevice::deleteClient(client);
    pScan->clearResults();
    return report;
}

} // namespace BleFuzzer
