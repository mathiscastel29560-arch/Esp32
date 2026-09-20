#include "ble_gatt_audit.h"
#include <NimBLEDevice.h>

namespace {
// Device Information Service (0x180A) characteristics worth flagging if
// they're readable — serial numbers and firmware versions are useful to
// an attacker for targeting known vulnerabilities.
struct DevInfoChar {
    uint16_t uuid;
    const char *label;
};
const DevInfoChar DEV_INFO_CHARS[] = {
    {0x2A29, "Manufacturer Name"},
    {0x2A24, "Model Number"},
    {0x2A25, "Serial Number"},
    {0x2A26, "Firmware Revision"},
    {0x2A27, "Hardware Revision"},
};
constexpr uint16_t DEVICE_INFO_SERVICE = 0x180A;
}

namespace BleGattAudit {

AuditReport audit(const String &address, uint32_t scanTimeoutSeconds) {
    AuditReport report;

    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(true);
    NimBLEScanResults results = pScan->start(scanTimeoutSeconds, false);

    NimBLEAdvertisedDevice *target = nullptr;
    for (auto it = results.begin(); it != results.end(); ++it) {
        if (String((*it)->getAddress().toString().c_str()).equalsIgnoreCase(address)) {
            target = *it;
            break;
        }
    }
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

    // --- Pass 1: enumerate + probe every characteristic, unauthenticated ---
    auto *services = client->getServices(true);
    for (auto *svc : *services) {
        String svcUuid = String(svc->getUUID().toString().c_str());
        auto *chars = svc->getCharacteristics(true);
        for (auto *chr : *chars) {
            CharFinding f;
            f.serviceUuid = svcUuid;
            f.charUuid = String(chr->getUUID().toString().c_str());
            f.readable = chr->canRead();
            f.writable = chr->canWrite() || chr->canWriteNoResponse();
            f.notifiable = chr->canNotify();

            if (f.readable) {
                NimBLEAttValue val = chr->readValue();
                f.readableWithoutPairing = val.length() > 0;

                // Harmless self-test: write the exact same bytes back, only
                // if it's also writable, to see if the write is accepted
                // without ever having paired.
                if (f.writable && f.readableWithoutPairing) {
                    f.writableWithoutAuth = chr->writeValue(val.data(), val.length(), true);
                }
            }
            report.findings.push_back(f);
        }
    }

    // --- Device Information Service leaks ---
    NimBLERemoteService *devInfo = client->getService(NimBLEUUID(DEVICE_INFO_SERVICE));
    if (devInfo) {
        for (auto &dc : DEV_INFO_CHARS) {
            NimBLERemoteCharacteristic *chr = devInfo->getCharacteristic(NimBLEUUID(dc.uuid));
            if (chr && chr->canRead()) {
                NimBLEAttValue val = chr->readValue();
                if (val.length() > 0) {
                    report.deviceInfoLeaks.push_back(String(dc.label) + ": " + String(val.c_str()));
                }
            }
        }
    }

    // --- Pass 2: attempt pairing, check whether it was "Just Works" ---
    report.pairingAttempted = true;
    client->secureConnection();
    NimBLEConnInfo info = client->getConnInfo();
    report.bonded = info.isBonded();
    report.encrypted = info.isEncrypted();
    report.authenticated = info.isAuthenticated();

    client->disconnect();
    NimBLEDevice::deleteClient(client);
    pScan->clearResults();
    return report;
}

} // namespace BleGattAudit
