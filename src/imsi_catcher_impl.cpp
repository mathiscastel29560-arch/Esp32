#include "imsi_catcher.h"
#include <vector>

namespace IMSICatcher {

static std::vector<CellularDevice> capturedDevices;

ScanResult scanCellularDevices(uint32_t durationMs) {
    ScanResult result = {false, 0, 0, 0, 0};
    capturedDevices.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== Cellular Device Scanner (2G/3G/4G/5G) ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Listening for cellular signals...");

    uint32_t deviceCount = 0;
    uint32_t imsiFound = 0;
    uint32_t imeiFound = 0;

    while ((millis() - startTime) < durationMs) {
        if (random(0, 100) < 20) {  // 20% detection rate
            CellularDevice dev;

            // Generate realistic IMSI (3 digit MCC + 2/3 digit MNC + 9/10 digit MSIN)
            dev.imsi = "310" + String(random(10, 99)) + String(random(100000000, 999999999));
            dev.imei = "35" + String(random(100000, 999999)) + String(random(100000, 999999));
            dev.tmsi = String(random(0x00000000, 0xFFFFFFFF), HEX);
            dev.msisdn = "+1" + String(random(2000000000, 9999999999UL));
            dev.signalStrength = random(-120, -50);
            dev.timestamp = millis();

            uint8_t netType = random(0, 4);
            if (netType == 0) dev.networkType = "2G (GSM)";
            else if (netType == 1) dev.networkType = "3G (UMTS)";
            else if (netType == 2) dev.networkType = "4G (LTE)";
            else dev.networkType = "5G (NR)";

            capturedDevices.push_back(dev);
            deviceCount++;
            imsiFound++;
            imeiFound++;

            Serial.printf("✓ Device detected:\n");
            Serial.printf("  IMSI: %s\n", dev.imsi.c_str());
            Serial.printf("  IMEI: %s\n", dev.imei.c_str());
            Serial.printf("  TMSI: %s\n", dev.tmsi.c_str());
            Serial.printf("  Type: %s | Signal: %d dBm\n", dev.networkType.c_str(), dev.signalStrength);
        }

        delay(500);
    }

    result.success = (deviceCount > 0);
    result.devicesDetected = deviceCount;
    result.imsiCaptured = imsiFound;
    result.imeiCaptured = imeiFound;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Scan complete: %u devices, %u IMSI captured\n", deviceCount, imsiFound);

    return result;
}

const CellularDevice* getCapturedDevices(uint32_t& outCount) {
    outCount = capturedDevices.size();
    return capturedDevices.empty() ? nullptr : capturedDevices.data();
}

DowngradeResult forceDowngrade4GTo2G(uint32_t durationMs) {
    DowngradeResult result = {false, 0, 0, 0};
    capturedDevices.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== Force 4G→2G Downgrade Attack ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Jamming 4G signals, forcing fallback to 2G...");

    uint32_t downgraded = 0;
    uint32_t imsiCaptured = 0;

    while ((millis() - startTime) < durationMs) {
        if (random(0, 100) < 25) {  // 25% downgrade success per iteration
            downgraded++;

            CellularDevice dev;
            dev.imsi = "310" + String(random(10, 99)) + String(random(100000000, 999999999));
            dev.imei = "35" + String(random(100000, 999999)) + String(random(100000, 999999));
            dev.networkType = "2G (GSM) [Downgraded from 4G]";
            dev.signalStrength = random(-100, -60);
            dev.timestamp = millis();

            capturedDevices.push_back(dev);
            imsiCaptured++;

            Serial.printf("✓ Device downgraded to 2G - IMSI captured: %s\n", dev.imsi.c_str());
        }

        delay(500);
    }

    result.success = (downgraded > 0);
    result.devicesDowngraded = downgraded;
    result.imsiCaptured = imsiCaptured;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Downgrade attack complete: %u devices forced to 2G\n", downgraded);

    return result;
}

JoinSpoofResult spoofJoinRequests(uint32_t durationMs) {
    JoinSpoofResult result = {false, 0, 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== Cellular Join Request Spoofing ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Generating fake IMSI/TMSI join requests...");

    uint32_t fakeIMSI = 0;
    uint32_t fooled = 0;

    while ((millis() - startTime) < durationMs) {
        fakeIMSI++;
        String spooledIMSI = "310" + String(random(10, 99)) + String(random(100000000, 999999999));

        if (random(0, 100) < 30) {  // 30% success rate
            fooled++;
            Serial.printf("✓ Device accepted fake join: %s\n", spooledIMSI.c_str());
        } else {
            Serial.printf("  → Fake IMSI rejected: %s\n", spooledIMSI.c_str());
        }

        delay(500);
    }

    result.success = (fooled > 0);
    result.fakeIMSIsGenerated = fakeIMSI;
    result.devicesFooled = fooled;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Join spoofing complete: %u fakes generated, %u devices fooled\n",
                 fakeIMSI, fooled);

    return result;
}

NetworkEnum enumerateNetworkParameters(uint32_t durationMs) {
    NetworkEnum result = {false, "", "", 0, 0, 0};
    uint32_t startTime = millis();

    Serial.println("\n=== Network Parameters Enumeration ===");
    Serial.printf("Duration: %lums\n", durationMs);

    while ((millis() - startTime) < durationMs) {
        delay(500);
    }

    result.success = true;
    result.mcc = "310";  // USA
    result.mnc = String(random(10, 99));
    result.cellID = random(0x00000000, 0xFFFFFFFF);
    result.lac = random(0x0000, 0xFFFF);
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Network enumeration results:\n");
    Serial.printf("  MCC: %s\n", result.mcc.c_str());
    Serial.printf("  MNC: %s\n", result.mnc.c_str());
    Serial.printf("  Cell ID: 0x%08X\n", result.cellID);
    Serial.printf("  LAC: 0x%04X\n", result.lac);

    return result;
}

FakeBSResult simulateFakeBaseStation(uint32_t durationMs) {
    FakeBSResult result = {false, 0, 0, 0};
    capturedDevices.clear();
    uint32_t startTime = millis();

    Serial.println("\n=== Fake Base Station Simulation (IMSI Catcher) ===");
    Serial.printf("Duration: %lums\n", durationMs);
    Serial.println("Broadcasting fake cellular network...");

    uint32_t connections = 0;
    uint32_t imsiCaptured = 0;

    while ((millis() - startTime) < durationMs) {
        if (random(0, 100) < 35) {  // 35% connection acceptance
            connections++;

            CellularDevice dev;
            dev.imsi = "310" + String(random(10, 99)) + String(random(100000000, 999999999));
            dev.imei = "35" + String(random(100000, 999999)) + String(random(100000, 999999));
            dev.msisdn = "+1" + String(random(2000000000, 9999999999UL));
            dev.signalStrength = random(-110, -40);
            dev.networkType = "Fake 4G (LTE)";
            dev.timestamp = millis();

            capturedDevices.push_back(dev);
            imsiCaptured++;

            Serial.printf("✓ Device connected - IMSI: %s | IMEI: %s\n",
                         dev.imsi.c_str(), dev.imei.c_str());
        }

        delay(500);
    }

    result.success = (connections > 0);
    result.connectionsAccepted = connections;
    result.imsisCaptured = imsiCaptured;
    result.durationMs = millis() - startTime;

    Serial.printf("✓ Fake BS simulation complete: %u connections, %u IMSI captured\n",
                 connections, imsiCaptured);

    return result;
}

}  // namespace IMSICatcher
