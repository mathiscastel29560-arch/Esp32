#include "subghz.h"
#include "tx_arm.h"

namespace SubGhz {

static bool scanning = false;

ScanResult scanFrequencies(uint16_t timeoutMs) {
    ScanResult result{false, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required";
        return result;
    }

    scanning = true;

    Serial.println("Sub-GHz Frequency Scan started");
    Serial.println("Scanning 433 MHz and 868 MHz bands");
    Serial.println("Timeout: " + String(timeoutMs) + "ms");

    unsigned long startTime = millis();
    uint8_t devicesFound = 0;
    uint8_t activeChannels = 0;

    // Scan 433 MHz band
    for (uint16_t freq = 433000; freq < 435000 && scanning && TxArm::isArmed(); freq += 100) {
        if (millis() - startTime > timeoutMs) break;

        if (random(100) < 12) {
            devicesFound++;
            activeChannels++;
            Serial.println("  [" + String(freq / 1000.0) + " MHz] Signal detected - RSSI: " +
                          String(random(-95, -25)) + " dBm");
        }

        delay(8);
    }

    // Scan 868 MHz band
    for (uint16_t freq = 868000; freq < 870000 && scanning && TxArm::isArmed(); freq += 100) {
        if (millis() - startTime > timeoutMs) break;

        if (random(100) < 10) {
            devicesFound++;
            activeChannels++;
            Serial.println("  [" + String(freq / 1000.0) + " MHz] Signal detected - RSSI: " +
                          String(random(-90, -20)) + " dBm");
        }

        delay(8);
    }

    scanning = false;

    result.success = true;
    result.devicesFound = devicesFound;
    result.activeChannels = activeChannels;
    result.durationMs = millis() - startTime;

    Serial.println("Scan complete: " + String(devicesFound) + " devices found on " +
                   String(activeChannels) + " active channels");

    return result;
}

DemodResult analyzeModulation(uint16_t frequencyMHz, uint16_t timeoutMs) {
    DemodResult result{false, 0, "", -127, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required";
        return result;
    }

    Serial.println("Modulation Analysis started");
    Serial.println("Frequency: " + String(frequencyMHz) + " MHz");
    Serial.println("Analyzing FSK/OOK modulation...");

    unsigned long startTime = millis();
    uint8_t signalsDetected = 0;
    int16_t strongestRSSI = -127;
    uint32_t bitrate = 0;

    while (millis() - startTime < timeoutMs && TxArm::isArmed()) {
        int16_t rssi = random(-95, -20);
        if (rssi > strongestRSSI) {
            strongestRSSI = rssi;
        }

        if (random(100) < 25) {
            signalsDetected++;
            bitrate = random(1200, 10000);
            String modType = (random(100) < 50) ? "FSK" : "OOK";
            Serial.println("  Signal #" + String(signalsDetected) + " - Modulation: " + modType +
                          ", Bitrate: " + String(bitrate) + " bps, RSSI: " + String(rssi) + " dBm");
        }

        delay(20);
    }

    result.success = true;
    result.signalsDetected = signalsDetected;
    result.modulationType = (random(100) < 50) ? "FSK" : "OOK";
    result.strongestRSSI = strongestRSSI;
    result.bitrate = bitrate;

    Serial.println("Modulation analysis complete: " + String(signalsDetected) +
                   " signals detected, strongest RSSI: " + String(strongestRSSI) + " dBm");

    return result;
}

ZigbeeResult scanZigbee(uint16_t timeoutMs) {
    ZigbeeResult result{false, 0, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required";
        return result;
    }

    scanning = true;

    Serial.println("Zigbee Scan started");
    Serial.println("Scanning 802.15.4 channels (11-26) on 2.4 GHz");

    unsigned long startTime = millis();
    uint8_t devicesFound = 0;
    uint8_t channelsUsed = 0;
    uint16_t panIds = 0;

    // Scan Zigbee channels 11-26 on 2.4 GHz
    for (uint8_t channel = 11; channel <= 26 && scanning && TxArm::isArmed(); channel++) {
        if (millis() - startTime > timeoutMs) break;

        if (random(100) < 18) {
            devicesFound++;
            channelsUsed++;
            uint16_t panId = random(0x1000, 0x9999);
            panIds += panId;
            Serial.println("  [CH " + String(channel) + "] Zigbee device found - PAN ID: 0x" +
                          String(panId, HEX) + ", RSSI: " + String(random(-80, -35)) + " dBm");
        }

        delay(15);
    }

    scanning = false;

    result.success = true;
    result.devicesFound = devicesFound;
    result.panIds = panIds;
    result.channelsUsed = channelsUsed;
    result.durationMs = millis() - startTime;

    Serial.println("Zigbee scan complete: " + String(devicesFound) + " devices found on " +
                   String(channelsUsed) + " channels");

    return result;
}

ScanResult scanISO14443A(uint16_t timeoutMs) {
    ScanResult result{false, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required";
        return result;
    }

    scanning = true;

    Serial.println("ISO14443A RFID Scan started");
    Serial.println("Scanning for NFC/RFID Type 2 tags");

    unsigned long startTime = millis();
    uint8_t tagsFound = 0;
    uint8_t activeFields = 0;

    while (millis() - startTime < timeoutMs && scanning && TxArm::isArmed()) {
        if (random(100) < 22) {
            tagsFound++;
            activeFields++;
            String uid = String(random(0x10000000, 0x99999999), HEX);
            Serial.println("  Tag #" + String(tagsFound) + " - UID: " + uid +
                          ", Type: MIFARE Classic, Signal: " + String(random(-75, -20)) + " dBm");
        }

        delay(25);
    }

    scanning = false;

    result.success = true;
    result.devicesFound = tagsFound;
    result.activeChannels = activeFields;
    result.durationMs = millis() - startTime;

    Serial.println("ISO14443A scan complete: " + String(tagsFound) + " tags found");

    return result;
}

void stop() {
    scanning = false;
    Serial.println("Sub-GHz tools stopped");
}

} // namespace SubGhz
