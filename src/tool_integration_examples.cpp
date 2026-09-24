// ============================================================================
// TOOL INTEGRATION EXAMPLES - Refactored tools with beautiful output
// ============================================================================
// Copy these patterns to update ALL tools for professional output
// ============================================================================

#include "tool_output_helper.h"
#include "result_renderers.h"
#include "zigbee_scanner.h"
#include "mqtt_hijacker.h"

namespace ToolIntegrationExamples {

// ============================================================================
// EXAMPLE 1: WiFi Packet Sniffer (Refactored)
// ============================================================================
void wifi_packet_sniffer_beautiful_example(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    // Create progress bar
    ScanProgressBar progress("WiFi Packet Sniffer", durationMs, 3);
    progress.start();

    // Step 1: Enable promiscuous mode
    delay(100);
    progress.step("Enabling promiscuous mode");

    // Step 2: Capture packets (simulated)
    std::vector<uint8_t> channelDist = {0, 5, 8, 3, 6, 12, 4, 2, 7, 1, 4, 9, 2, 0};
    uint32_t totalPackets = 0;
    for (uint8_t d : channelDist) totalPackets += d;

    delay(durationMs / 2);
    progress.step("Capturing packets on channels 1-14");

    // Step 3: Analyze results
    progress.step("Analyzing " + String(totalPackets) + " packets");
    delay(200);

    // Complete and render
    progress.complete(String(totalPackets) + " packets analyzed");

    // Render formatted results
    ResultRenderers::WiFiScanResult result;
    result.devicesFound = totalPackets;
    result.strongestRssi = -35;
    result.strongestSSID = "YourNetwork";
    result.channelDistribution = channelDist;
    for (uint8_t d : channelDist) {
        for (uint8_t i = 0; i < d; i++) {
            result.allRssiValues.push_back(random(-85, -25));
        }
    }
    result.durationMs = durationMs;

    ResultRenderers::renderWiFiScan(result);
}

// ============================================================================
// EXAMPLE 2: Bluetooth Classic Attack (Refactored)
// ============================================================================
void bluetooth_classic_attack_beautiful_example(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    ScanProgressBar progress("Bluetooth PIN Cracking", durationMs, 4);
    progress.start();

    // Phase 1: Discovery
    progress.step("Scanning for Bluetooth Classic devices");
    delay(durationMs / 4);

    // Phase 2: Testing common PINs
    progress.step("Testing common PINs (0000, 1111, 1234, 9999)");
    delay(durationMs / 4);

    // Phase 3: Brute force
    progress.step("Brute forcing remaining PIN space");
    delay(durationMs / 4);

    // Phase 4: Analysis
    progress.step("Generating attack report");
    delay(durationMs / 4);

    progress.complete("PIN cracking attack complete");

    // Display results
    printSubHeader("Attack Summary");
    printKeyValue("Targets Scanned", "12");
    printKeyValue("Successful Cracks", "8");
    printKeyValue("Failed Attempts", "4");
    printKeyValue("Success Rate", "67%");
    printBar(67, 20);

    // Render with formatter
    ResultRenderers::AttackSuccessResult result;
    result.attackName = "Bluetooth PIN Cracking";
    result.success = true;
    result.targetCount = 12;
    result.successCount = 8;
    result.failureCount = 4;
    result.successPercent = 67;
    result.durationMs = durationMs;

    ResultRenderers::renderAttackSuccess(result);
}

// ============================================================================
// EXAMPLE 3: Zigbee Scanner (Refactored)
// ============================================================================
void zigbee_scanner_beautiful_example(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    displayScanStart("Zigbee Device Scanner", "Channels 11-26 @ 2.4GHz");

    ScanProgressBar progress("Zigbee Scanner", durationMs, 3);
    progress.start();

    // Scan phase
    progress.step("Scanning Zigbee channels 11-26");
    delay(durationMs / 3);

    // Analysis phase
    progress.step("Analyzing network topology");
    delay(durationMs / 3);

    // Vulnerability phase
    progress.step("Checking for vulnerabilities");
    delay(durationMs / 3);

    progress.complete("Zigbee scan complete");

    // Render results
    ResultRenderers::RFScanResult result;
    result.signalsDetected = 8;
    result.dominantProtocol = "Zigbee (802.15.4)";
    result.rollingCodesDetected = 3;
    result.frequencies = {2405, 2410, 2415, 2420, 2425, 2430, 2435, 2440};
    result.signalStrengths = {-45, -52, -48, -60, -55, -50, -58, -63};
    result.durationMs = durationMs;

    ResultRenderers::renderRFScan(result);
}

// ============================================================================
// EXAMPLE 4: MQTT Hijacker (Refactored)
// ============================================================================
void mqtt_hijacker_beautiful_example(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    displayScanStart("MQTT Broker Hijacking Attack", "Scanning ports 1883/8883");

    ScanProgressBar progress("MQTT Hijacker", durationMs, 5);
    progress.start();

    progress.step("Scanning for open MQTT brokers");
    delay(durationMs / 5);

    progress.step("Connecting to broker at 192.168.1.100:1883");
    delay(durationMs / 5);

    progress.step("Testing default credentials");
    delay(durationMs / 5);

    progress.step("Publishing malicious payloads");
    delay(durationMs / 5);

    progress.step("Analyzing broker traffic");
    delay(durationMs / 5);

    progress.complete("MQTT hijacking complete");

    // Render IoT results
    ResultRenderers::IoTScanResult result;
    result.devicesFound = 3;
    result.brokersFound = 2;
    result.vulnerabilitiesDiscovered = 5;
    result.protocols = {"MQTT", "CoAP", "MQTT"};
    result.ports = {1883, 5683, 8883};
    result.durationMs = durationMs;

    ResultRenderers::renderIoTScan(result);
}

// ============================================================================
// EXAMPLE 5: Cellular IMSI Catcher (Refactored)
// ============================================================================
void imsi_catcher_beautiful_example(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    displayScanStart("IMSI Catcher - Fake Base Station", "Broadcasting fake 4G network");

    ScanProgressBar progress("IMSI Catcher", durationMs / 1000, 4);  // Convert to seconds
    progress.start();

    progress.step("Setting up fake base station");
    delay(durationMs / 4);

    progress.step("Broadcasting fake 4G network signal");
    delay(durationMs / 4);

    progress.step("Accepting device connections");
    delay(durationMs / 4);

    progress.step("Capturing IMSI/IMEI identifiers");
    delay(durationMs / 4);

    progress.complete("IMSI capture complete");

    // Render cellular results
    ResultRenderers::CellularScanResult result;
    result.devicesDetected = 15;
    result.imsiCaptured = 12;
    result.networkTypes = {"4G (LTE)", "3G (UMTS)", "2G (GSM)"};
    result.signalStrengths = {-95, -88, -92, -85, -90, -87, -93, -86, -91, -89, -94, -84};
    result.downgradeAttacks = 8;
    result.durationMs = durationMs;

    ResultRenderers::renderCellularScan(result);
}

// ============================================================================
// EXAMPLE 6: NFC Relay Attack (Refactored)
// ============================================================================
void nfc_relay_attack_beautiful_example(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    displayScanStart("NFC Relay Attack", "Man-in-the-Middle via PN532");

    ScanProgressBar progress("NFC Relay", durationMs, 4);
    progress.start();

    progress.step("Listening for NFC communications");
    delay(durationMs / 4);

    progress.step("Capturing card signals");
    delay(durationMs / 4);

    progress.step("Relaying to local emulator");
    delay(durationMs / 4);

    progress.step("Executing transactions");
    delay(durationMs / 4);

    progress.complete("NFC relay attack complete");

    // Render NFC results
    ResultRenderers::NFCScanResult result;
    result.cardsDetected = 7;
    result.relayedSuccessfully = 6;
    result.cardTypes = {"Mifare Classic", "ISO14443A Type 4", "ISO15693"};
    result.signalStrengths = {-40, -55, -50, -48, -60, -45, -52};
    result.vulnerabilitiesFound = 4;
    result.durationMs = durationMs;

    ResultRenderers::renderNFCScan(result);
}

// ============================================================================
// EXAMPLE 7: Comprehensive Tool Output Template
// ============================================================================
void generic_tool_beautiful_template(
    const String &toolName,
    const String &icon,
    uint32_t durationMs,
    uint8_t expectedSteps) {

    using namespace ToolOutputHelper;

    // Standard header
    printHeader(toolName, icon);
    Serial.println();

    // Create progress bar
    ScanProgressBar progress(toolName, durationMs, expectedSteps);
    progress.start();

    // Execute scanning/attack phases
    for (uint8_t step = 1; step <= expectedSteps; step++) {
        String message = "Executing step " + String(step) + "/" + String(expectedSteps);
        progress.step(message);
        delay(durationMs / expectedSteps);
    }

    // Complete and display summary
    progress.complete("Tool execution complete");

    // Add custom results here using ResultRenderers
}

}  // namespace ToolIntegrationExamples

// ============================================================================
// HOW TO USE IN YOUR TOOLS:
// ============================================================================
//
// 1. Add includes:
//    #include "tool_output_helper.h"
//    #include "result_renderers.h"
//
// 2. Replace old Serial.printf output with:
//    using namespace ToolOutputHelper;
//    ScanProgressBar progress("Tool Name", 30000, 3);  // 30sec, 3 steps
//    progress.start();
//    progress.step("Message");
//    progress.complete("Summary");
//
// 3. Add result rendering at the end:
//    ResultRenderers::RFScanResult result {...};
//    ResultRenderers::renderRFScan(result);
//
// 4. Optional: Use helper functions:
//    printHeader("Title", "🔍");
//    printSuccess("Found device!");
//    printKeyValue("Signal", "-45 dBm");
//    printBar(75, 20);
//
// Result: Beautiful, professional output with progress bars! ✨
// ============================================================================
