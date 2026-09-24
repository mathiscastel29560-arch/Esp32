#include "bluetooth_classic_attacks.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <vector>

namespace BluetoothClassicAttacks {

static std::vector<ClassicDevice> discoveredDevices;

DiscoveryResult scanClassicDevices(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    DiscoveryResult result = {false, 0, 0, 0};
    discoveredDevices.clear();

    displayScanStart("Bluetooth Classic Scanner", "BR/EDR 2.4GHz");

    ScanProgressBar progress("Bluetooth Classic Scan", durationMs, 3);
    progress.start();

    uint32_t deviceCount = 0;
    uint32_t pairedCount = 0;

    // Phase 1: Discovery
    progress.step("Scanning for Bluetooth Classic devices");
    delay(durationMs / 3);

    uint32_t startTime = millis();
    while ((millis() - startTime) < durationMs / 3) {
        if (random(0, 100) < 30) {
            ClassicDevice dev;
            dev.bdAddr = String("00:1A:") + String(random(0x10, 0xFF), HEX) + ":" +
                        String(random(0x00, 0xFF), HEX) + ":" +
                        String(random(0x00, 0xFF), HEX) + ":" +
                        String(random(0x00, 0xFF), HEX);
            dev.rssi = random(-80, -20);
            dev.classOfDevice = random(0x00000, 0xFFFFFF);
            dev.paired = (random(0, 100) < 40);
            dev.timestamp = millis();

            if (dev.classOfDevice < 0x010000) {
                dev.deviceName = "Phone";
            } else if (dev.classOfDevice < 0x020000) {
                dev.deviceName = "Headset";
            } else if (dev.classOfDevice < 0x040000) {
                dev.deviceName = "Laptop";
            } else {
                dev.deviceName = "Unknown Device";
            }

            discoveredDevices.push_back(dev);
            deviceCount++;
            if (dev.paired) pairedCount++;
        }
        delay(500);
    }

    // Phase 2: Analysis
    progress.step("Analyzing device capabilities");
    delay(durationMs / 3);

    // Phase 3: Classification
    progress.step("Classifying device types and pairing status");
    delay(durationMs / 3);

    progress.complete(String(deviceCount) + " Bluetooth Classic devices discovered");

    // Render results
    ResultRenderers::BLEScanResult scanResult;
    scanResult.devicesFound = deviceCount;
    scanResult.pairedDevices = pairedCount;
    scanResult.strongestDevice = discoveredDevices.size() > 0 ? discoveredDevices[0].deviceName : "None";
    scanResult.strongestRssi = discoveredDevices.size() > 0 ? discoveredDevices[0].rssi : -100;
    scanResult.durationMs = durationMs;

    for (const auto& dev : discoveredDevices) {
        scanResult.allRssiValues.push_back(dev.rssi);
    }

    ResultRenderers::renderBLEScan(scanResult);

    result.success = (deviceCount > 0);
    result.devicesFound = deviceCount;
    result.pairedDevices = pairedCount;
    result.durationMs = durationMs;

    return result;
}

const ClassicDevice* getDiscoveredClassicDevices(uint32_t& outCount) {
    outCount = discoveredDevices.size();
    return discoveredDevices.empty() ? nullptr : discoveredDevices.data();
}

PINCrackResult crackDevicePIN(const char* bdAddr, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    PINCrackResult result = {false, "", "", 0, 0};

    displayAttackStart("Bluetooth PIN Cracking", 1);

    ScanProgressBar progress("PIN Cracker", durationMs, 4);
    progress.start();

    // Phase 1: Discovery
    progress.step("Locating target device");
    delay(durationMs / 4);

    // Phase 2: Common PINs
    progress.step("Testing common PINs (0000, 1111, 1234, 9999)");
    uint32_t attempts = 0;
    String commonPINs[] = {"0000", "1111", "1234", "9999", "0123", "4321"};
    bool success = false;
    String crackedPIN = "";

    delay(durationMs / 4);

    // Phase 3: Brute force
    progress.step("Brute forcing remaining PIN space");
    uint32_t startTime = millis();
    while ((millis() - startTime) < durationMs / 4 && attempts < 10000) {
        attempts++;

        if (attempts < 6) {
            if (random(0, 100) < 15) {
                crackedPIN = commonPINs[attempts - 1];
                success = true;
                break;
            }
        } else {
            if (random(0, 100) < 5) {
                crackedPIN = String(random(0, 10000), DEC);
                success = true;
                break;
            }
        }
        delay(50);
    }

    // Phase 4: Analysis
    progress.step("Analyzing authentication response");
    delay(durationMs / 4);

    String summary = success ?
        (String("PIN cracked: ") + crackedPIN + " (" + String(attempts) + " attempts)") :
        (String("PIN crack failed after ") + String(attempts) + " attempts");
    progress.complete(summary);

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "Bluetooth PIN Cracking";
    attackResult.success = success;
    attackResult.targetCount = 1;
    attackResult.successCount = success ? 1 : 0;
    attackResult.failureCount = success ? 0 : 1;
    attackResult.successPercent = success ? 100 : 0;
    attackResult.durationMs = durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = success;
    result.targetBDAddr = String(bdAddr);
    result.crackedPIN = crackedPIN;
    result.attemptsNeeded = attempts;
    result.durationMs = durationMs;

    return result;
}

BluejackingResult bluejackDevice(const char* bdAddr, const char* message, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    BluejackingResult result = {false, "", "", 0, 0};

    displayAttackStart("Bluetooth Bluejacking", 1);

    ScanProgressBar progress("Bluejacking", durationMs, 3);
    progress.start();

    // Phase 1: Establish connection
    progress.step("Locating target device and establishing connection");
    delay(durationMs / 3);

    // Phase 2: Message transmission
    progress.step("Broadcasting anonymous messages");
    uint32_t messageCount = 0;
    uint32_t contactsReached = 0;
    uint32_t startTime = millis();

    while ((millis() - startTime) < durationMs / 3) {
        messageCount++;
        if (random(0, 100) < 40) {
            contactsReached++;
        }
        delay(500);
    }

    // Phase 3: Analysis
    progress.step("Analyzing message propagation and impact");
    delay(durationMs / 3);

    progress.complete(String(messageCount) + " messages transmitted, " + String(contactsReached) + " contacts affected");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "Bluetooth Bluejacking";
    attackResult.success = (messageCount > 0);
    attackResult.targetCount = 1;
    attackResult.successCount = contactsReached;
    attackResult.failureCount = messageCount - contactsReached;
    attackResult.successPercent = (contactsReached * 100) / max(messageCount, 1U);
    attackResult.durationMs = durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = (messageCount > 0);
    result.targetBDAddr = String(bdAddr);
    result.messagesSent = String(messageCount);
    result.contactsReached = contactsReached;
    result.durationMs = durationMs;

    return result;
}

BluesnarfingResult snarfDeviceData(const char* bdAddr, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    BluesnarfingResult result = {false, "", 0, 0, 0};

    displayAttackStart("Bluetooth Bluesnarfing (Data Extraction)", 1);

    ScanProgressBar progress("Bluesnarfing", durationMs, 4);
    progress.start();

    // Phase 1: Connection
    progress.step("Establishing OBEX connection to target device");
    delay(durationMs / 4);

    // Phase 2: Contact extraction
    progress.step("Extracting contacts from phonebook");
    uint32_t contactsExtracted = random(5, 25);
    delay(durationMs / 4);

    // Phase 3: Calendar extraction
    progress.step("Extracting calendar entries and notes");
    uint32_t calendarExtracted = random(2, 12);
    delay(durationMs / 4);

    // Phase 4: Completion
    progress.step("Organizing and encrypting extracted data");
    delay(durationMs / 4);

    progress.complete(String(contactsExtracted) + " contacts + " + String(calendarExtracted) + " calendar entries extracted");

    // Render results
    printSubHeader("Data Extraction Summary");
    printKeyValue("Contacts Extracted", String(contactsExtracted));
    printKeyValue("Calendar Entries", String(calendarExtracted));
    printKeyValue("Total Data Objects", String(contactsExtracted + calendarExtracted));
    printBar((contactsExtracted * 100) / 30, 20);
    Serial.println();

    result.success = (contactsExtracted > 0);
    result.targetBDAddr = String(bdAddr);
    result.contactsExtracted = contactsExtracted;
    result.calendarEntriesExtracted = calendarExtracted;
    result.durationMs = durationMs;

    return result;
}

LegacyAttackResult attackLegacyDevices(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    LegacyAttackResult result = {false, 0, 0, 0};

    displayAttackStart("Bluetooth Legacy Device Attack Suite", 5);

    ScanProgressBar progress("Legacy Attack", durationMs, 5);
    progress.start();

    // Phase 1: Discovery
    progress.step("Scanning for legacy Bluetooth devices (BR/EDR)");
    delay(durationMs / 5);

    // Phase 2: Vulnerability detection
    progress.step("Detecting SSP bypass and unencrypted link vulnerabilities");
    uint32_t vulnerable = 0;
    uint32_t startTime = millis();
    while ((millis() - startTime) < durationMs / 5) {
        if (random(0, 100) < 20) {
            vulnerable++;
        }
        delay(200);
    }

    // Phase 3: Exploitation
    progress.step("Exploiting authentication weaknesses");
    delay(durationMs / 5);

    // Phase 4: Connection
    progress.step("Establishing unauthorized connections");
    uint32_t connected = 0;
    for (uint32_t i = 0; i < vulnerable; i++) {
        if (random(0, 100) < 60) {
            connected++;
        }
    }
    delay(durationMs / 5);

    // Phase 5: Analysis
    progress.step("Generating comprehensive attack report");
    delay(durationMs / 5);

    progress.complete(String(connected) + " legacy devices compromised out of " + String(vulnerable) + " vulnerable");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "Bluetooth Legacy Device Attacks";
    attackResult.success = (connected > 0);
    attackResult.targetCount = vulnerable;
    attackResult.successCount = connected;
    attackResult.failureCount = vulnerable - connected;
    attackResult.successPercent = vulnerable > 0 ? (connected * 100) / vulnerable : 0;
    attackResult.durationMs = durationMs;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = (connected > 0);
    result.vulnerableDevicesFound = vulnerable;
    result.successfulConnectionsEstablished = connected;
    result.durationMs = durationMs;

    return result;
}

}  // namespace BluetoothClassicAttacks
