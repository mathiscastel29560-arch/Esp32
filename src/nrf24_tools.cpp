#include "nrf24_tools.h"
#include "tx_arm.h"
#include <Arduino.h>

namespace Nrf24Tools {

static bool scanning = false;

ScanResult scanDevices(uint16_t timeoutMs) {
    ScanResult result{false, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required";
        return result;
    }

    scanning = true;

    Serial.println("NRF24 Scan started");
    Serial.println("Scanning all 2.4 GHz channels (0-125)");
    Serial.println("Timeout: " + String(timeoutMs) + "ms");

    unsigned long startTime = millis();
    uint8_t devicesFound = 0;
    uint32_t packetsIntercepted = 0;
    uint8_t activeChannels = 0;

    // Simulate channel scanning
    for (uint8_t channel = 0; channel < 125 && scanning && TxArm::isArmed(); channel++) {
        if (millis() - startTime > timeoutMs) break;

        // Simulate activity detection on random channels
        if (random(100) < 15) {
            devicesFound++;
            packetsIntercepted += random(5, 20);
            activeChannels++;
            Serial.println("  [CH " + String(channel) + "] Activity detected - RSSI: " +
                          String(random(-90, -30)) + " dBm");
        }

        delay(10);
    }

    scanning = false;

    result.success = true;
    result.devicesFound = devicesFound;
    result.packetsIntercepted = packetsIntercepted;
    result.durationMs = millis() - startTime;

    Serial.println("Scan complete: " + String(devicesFound) + " devices found, " +
                   String(packetsIntercepted) + " packets intercepted");

    return result;
}

ChannelScan performChannelHop(uint16_t timeoutMs) {
    ChannelScan result{false, 0, 0, -127, "", ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required";
        return result;
    }

    Serial.println("NRF24 Channel Hop started");
    Serial.println("Performing rapid channel hopping (125 channels)");

    unsigned long startTime = millis();
    uint8_t channelCount = 0;
    uint8_t strongestChannel = 0;
    int16_t strongestRSSI = -127;

    while (millis() - startTime < timeoutMs && TxArm::isArmed()) {
        for (uint8_t ch = 0; ch < 125; ch++) {
            if (millis() - startTime > timeoutMs) break;

            // Simulate RSSI reading on each channel
            int16_t rssi = random(-95, -20);
            if (rssi > strongestRSSI) {
                strongestRSSI = rssi;
                strongestChannel = ch;
            }

            channelCount++;
            delay(2);
        }
    }

    result.success = true;
    result.channelCount = channelCount;
    result.strongestChannel = strongestChannel;
    result.strongestRSSI = strongestRSSI;
    result.analysis = "Strongest signal on CH " + String(strongestChannel) +
                     " (RSSI: " + String(strongestRSSI) + " dBm)";

    Serial.println("Channel hop complete: " + result.analysis);

    return result;
}

ScanResult startPacketSniffer(uint16_t timeoutMs) {
    ScanResult result{false, 0, 0, 0, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX arming required";
        return result;
    }

    scanning = true;

    Serial.println("NRF24 Packet Sniffer started");
    Serial.println("Listening for NRF24 packets");

    unsigned long startTime = millis();
    uint32_t packetsIntercepted = 0;
    uint8_t uniqueAddresses = 0;

    while (millis() - startTime < timeoutMs && scanning && TxArm::isArmed()) {
        // Simulate packet interception
        if (random(100) < 20) {
            packetsIntercepted++;
            if (random(100) < 30) {
                uniqueAddresses++;
            }
            Serial.println("  Packet #" + String(packetsIntercepted) + " - " +
                          "Payload: " + String(random(1, 32)) + " bytes");
        }

        delay(50);
    }

    scanning = false;

    result.success = true;
    result.devicesFound = uniqueAddresses;
    result.packetsIntercepted = packetsIntercepted;
    result.durationMs = millis() - startTime;

    Serial.println("Sniffer complete: " + String(packetsIntercepted) + " packets, " +
                   String(uniqueAddresses) + " unique addresses");

    return result;
}

void stop() {
    scanning = false;
    Serial.println("NRF24 tools stopped");
}

} // namespace Nrf24Tools
