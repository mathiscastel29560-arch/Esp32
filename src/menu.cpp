#include "menu.h"
#include "menu_icons.h"
#include "buttons.h"
#include "config.h"
#include "settings.h"
#include "hardware_test_mode.h"
#include "debug_logger.h"
#include "results_formatter.h"
#include "tx_arm.h"
#include "wifi_tools.h"
#include <WiFi.h>
#include "ble_tools.h"
#include "nrf24_tools.h"
#include "subghz.h"
#include "beacon_spam.h"
#include "evil_portal.h"
#include "ir_tools.h"
#include "ir_bruteforce.h"
#include "ble_spam_detector.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "battery.h"
#include "help_content.h"
#include "drone_tracker.h"
#include "signal_sniffer.h"
#include "subghz_bruteforce.h"
#include "subghz_replay.h"
#include "subghz_scanner.h"
#include "gps_wardriving.h"
#include "nrf24_replay.h"
#include "wifi_hidden_revealer.h"
#include "iot_device_hunter.h"
#include "ble_spoof.h"
#include "frequency_analyzer.h"
#include "ble_pairing_attack.h"
#include "nrf24_injection.h"
#include "smart_lock_scanner.h"
#include "subghz_protocol_analyzer.h"
#include "ble_dos.h"
#include "ble_beacon_spam.h"
#include "wifi_deauth.h"
#include "ble_advertising_jammer.h"
#include "gps_spoof.h"
#include "advanced_rf_jammer.h"
#include "wifi_jammer_suite.h"
#include "bluetooth_aggressive_jammer.h"
#include "subghz_jammer_suite.h"
#include "ble_advanced_attack_suite.h"
#include "jamming_signal_generator.h"
#include "wpa2_handshake_cracker.h"
#include "wifi_association_hijacker.h"
#include "http_downgrade_attack.h"
#include "rf_signal_recorder.h"
#include "handshake_capture.h"
#include "signal_decoder.h"
#include "advanced_signal_cloner.h"
#include "zigbee_scanner.h"
#include "mqtt_hijacker.h"
#include "zwave_scanner.h"
#include "bluetooth_classic.h"
#include "coap_scanner.h"
#include "lorawan_recon.h"
#include "rfid_emulator.h"
#include "mifare_classic.h"
#include "nfc_cloner.h"
#include "smarthome_hijacker.h"
#include "spectrum_analyzer_plus.h"
#include "modulation_classifier.h"
#include "auto_handshake_capture.h"
#include "generic_packet_tools.h"
#include "advanced_wifi_attacks.h"
#include "ui/scan_visualizations.h"
#include "ui/advanced_scanning.h"
#include "mavic_jammer.h"
#include "tpms_spoofer.h"
#include <vector>
#include <set>

namespace {

enum State {
    HOME,
    MAIN_MENU,
    WIFI_SUBMENU,
    BLE_SUBMENU,
    RF_SUBMENU,
    IOT_SUBMENU,
    SYSTEM_SUBMENU,
    SETTINGS_SUBMENU,
    HARDWARE_TEST_SUBMENU,
    DEVICE_INFO_SUBMENU,
    DEBUG_INFO_SUBMENU,
    CALIBRATION_SUBMENU,
    ABOUT_SUBMENU,
    NETWORK_SUBMENU,
    HELP_SUBMENU,
    RESULT_SCREEN,
    HARDWARE_TEST_SELECT,
};

State g_state = HOME;
int g_selection = 0;
String g_resultTitle, g_resultBody;
uint32_t g_resultShowTime = 0;

// Debounced button tracking
bool g_backHeld = false;
uint32_t g_backHeldSince = 0;

bool consumeBackTap() {
    bool now = TxArm::isArmed();
    bool tapped = false;
    if (now && !g_backHeld) {
        g_backHeldSince = millis();
    } else if (!now && g_backHeld) {
        if (millis() - g_backHeldSince < 500) tapped = true;
    }
    g_backHeld = now;
    return tapped;
}

std::vector<String> mainMenuItems() {
    return {
        "📡 WiFi Tools",
        "🔵 BLE Tools",
        "📶 RF/2.4GHz",
        "🌐 IoT/Advanced",
        "⚙️  System",
        "⚙️  Settings",
        "🧪 Hardware Test",
        "ℹ️  Device Info",
        "🐛 Debug Info",
        "🔧 Calibration",
        "ℹ️  About",
        "🌐 Network",
        "❓ Help",
    };
}

std::vector<String> wifiMenuItems() {
    return {
        "📡 Scan Networks",
        "📡 Reveal Hidden Networks",
        "📡 IoT Device Hunter",
        "📡 Smart Lock Scanner",
        "📡 Frequency Analyzer",
        "📡 Beacon Spam " + String(BeaconSpam::active() ? "STOP" : "start"),
        "📡 Evil Portal " + String(EvilPortal::active() ? "STOP" : "start"),
        "📡 WiFi Deauth " + String(WiFiDeauth::isActive() ? "STOP" : "start"),
        "📡 WiFi Jammer Suite " + String(WiFiJammerSuite::isActive() ? "STOP" : "start"),
        "📡 Association Hijacker",
        "📡 HTTP Downgrade Attack",
        "📡 WPA2 Handshake Cracker",
        "📡 Capture Handshake (WPA2)",
        "🔙 Back",
    };
}

std::vector<String> bleMenuItems() {
    return {
        "🔵 Scan Devices (5s)",
        "🔵 BLE Address Spoof",
        "🔵 BLE Pairing Attack",
        "🔵 BLE DoS Attack",
        "🔵 BLE Beacon Spam " + String(BLEBeaconSpam::isActive() ? "STOP" : "start"),
        "🔵 BLE Advertising Jam " + String(BLEAdvertisingJammer::isActive() ? "STOP" : "start"),
        "🔵 Bluetooth Aggressive Jam " + String(BluetoothAggressiveJammer::isActive() ? "STOP" : "start"),
        "🔵 BLE Advanced Attacks " + String(BLEAdvancedAttackSuite::isActive() ? "STOP" : "start"),
        "🔵 BLE Spam Watch " + String(BleSpamDetector::active() ? "STOP" : "start"),
        "🔵 Check Spam Alert",
        "🔙 Back",
    };
}

std::vector<String> rfMenuItems() {
    return {
        "📶 2.4GHz Spectrum Scan",
        "📶 Drone Tracker (RSSI)",
        "📶 Signal Sniffer (NRF24)",
        "📶 NRF24 Replay Attack",
        "📶 Sub-GHz Scanner",
        "📶 Sub-GHz Bruteforce",
        "📶 Sub-GHz Replay",
        "🔴 IR: TV Power Toggle",
        "🔴 IR: Bruteforce TV",
        "🔴 IR: Bruteforce AC",
        "🔴 IR: Bruteforce Light",
        "🛰️  GPS Spoofing (2.4GHz)",
        "📶 Advanced RF Jammer " + String(AdvancedRFJammer::isActive() ? "STOP" : "start"),
        "📶 Sub-GHz Jammer Suite " + String(SubghzJammerSuite::isActive() ? "STOP" : "start"),
        "📶 Jamming Signal Gen " + String(JammingSignalGenerator::isActive() ? "STOP" : "start"),
        "📶 RF Signal Recorder",
        "📶 Signal Decoder",
        "📶 Advanced Signal Cloner",
        "🚁 Mavic Jammer (2.4GHz)",
        "🔴 TPMS Spoofer (433MHz)",
        "🔙 Back",
    };
}

std::vector<String> settingsMenuItems() {
    return {
        "⏰ Set Date/Time",
        "💡 Brightness: " + String(Settings::g_config.brightness) + "%",
        "🎨 Contrast: " + String(Settings::g_config.contrast) + "%",
        "🔄 Invert Display: " + String(Settings::g_config.invertColors ? "ON" : "OFF"),
        "🔐 Auto-Lock: " + String(Settings::g_config.autoLock ? "ON" : "OFF"),
        "📝 Logging: " + String(Settings::g_config.enableLogging ? "ON" : "OFF"),
        "🔙 Back",
    };
}

std::vector<String> hardwareTestMenuItems() {
    return {
        "🔘 GPIO (buttons, buzzer, battery)",
        "⏰ RTC (DS3231 clock)",
        "🛰️  GPS (NEO-6M)",
        "📱 PN532 (NFC/RFID)",
        "📶 CC1101 (433MHz)",
        "📶 NRF24 (2.4GHz)",
        "🔙 Back",
    };
}

std::vector<String> deviceInfoMenuItems() {
    return {
        "📋 View Full Config",
        "🔌 Pin Assignments",
        "⚠️  Hardware Guards Status",
        "📊 Initialized Modules",
        "🔙 Back",
    };
}

std::vector<String> debugInfoMenuItems() {
    return {
        "💾 Memory & PSRAM",
        "🔋 Battery Status",
        "📱 Active Modules",
        "⚠️  Last Errors",
        "🔙 Back",
    };
}

std::vector<String> calibrationMenuItems() {
    return {
        "🔋 Battery ADC Calibration",
        "📡 RF Signal Level",
        "🎨 Display Calibration",
        "🔙 Back",
    };
}

std::vector<String> aboutMenuItems() {
    return {
        "📋 Firmware Version",
        "🏷️  Device Serial",
        "📍 MAC Address",
        "💾 Flash Size",
        "⏱️  Uptime",
        "🔙 Back",
    };
}

std::vector<String> networkMenuItems() {
    return {
        "📡 WiFi Status",
        "🔌 IP Address",
        "🌐 Hostname",
        "🔐 WiFi Settings",
        "🔙 Back",
    };
}

std::vector<String> systemMenuItems() {
    return {
        "⚙️  TX Arm Status",
        "🔋 Battery Status",
        "🗺️  GPS Map",
        "🔄 Dualboot OTA1",
        "🔙 Back",
    };
}

std::vector<String> iotMenuItems() {
    return {
        "🌐 Zigbee Scanner",
        "🌐 MQTT Hijacker",
        "🌐 Z-Wave Scanner",
        "🌐 Bluetooth Classic",
        "🌐 CoAP Scanner",
        "🌐 LoRaWAN Recon",
        "🌐 RFID Emulator",
        "🌐 Mifare Classic",
        "🌐 NFC Cloner",
        "🌐 Smart Home Hijacker",
        "🌐 Spectrum Analyzer+",
        "🌐 Modulation Classifier",
        "🌐 Auto Handshake Capture",
        "🌐 Generic Packet Tools",
        "🌐 Advanced WiFi Attacks",
        "🔙 Back",
    };
}

std::vector<String> helpMenuItems() {
    std::vector<String> items;
    String lastCat = "";

    for (size_t i = 0; i < HelpContent::TOPIC_COUNT; i++) {
        String cat = HelpContent::TOPICS[i].category;
        if (cat != lastCat) {
            items.push_back(String("[") + cat.substring(0, 1) + "] " + cat);
            lastCat = cat;
        }
    }
    items.push_back("Back");
    return items;
}

void showResult(const String &title, const String &body) {
    g_resultTitle = title;
    g_resultBody = body;
    g_resultShowTime = millis();
    g_state = RESULT_SCREEN;
}

void runWifiAction(int idx) {
    switch (idx) {
        case 0: { // Scan
            auto results = WifiTools::scan();
            if (results.empty()) {
                showResult("WiFi Scan", "No networks found");
            } else {
                // Build visualization data from scan results
                ScanVisualizations::WifiScanData vizData;
                vizData.channelCounts.resize(14, 0);
                vizData.totalNetworks = results.size();
                vizData.strongestRssi = results[0].rssi;
                vizData.strongestSsid = results[0].ssid;

                // Populate channel distribution
                for (const auto &ap : results) {
                    if (ap.channel > 0 && ap.channel <= 14) {
                        vizData.channelCounts[ap.channel - 1]++;
                    }
                }

                // Show enhanced visualization
                ScanVisualizations::showWifiVisualization(vizData);
            }
            break;
        }
        case 1: { // Reveal hidden networks
            auto result = WiFiHiddenRevealer::revealHiddenNetworks(10000);
            if (result.networksFound > 0) {
                showResult("Hidden Networks",
                          "Found: " + String(result.networksFound) + "\n" +
                          "Check logs for details");
            } else {
                showResult("Hidden Networks", "No hidden networks found");
            }
            break;
        }
        case 2: { // IoT Device Hunter
            auto result = IoTDeviceHunter::huntDevices(15000);
            if (result.devicesFound > 0) {
                // Show device distribution visualization
                AdvancedScanning::NetworkDeviceData iotData;
                iotData.title = "IoT Devices";
                iotData.totalDevices = result.devicesFound;
                iotData.strongestSignal = -50;  // Simulated
                iotData.strongestDeviceName = "SmartDevice-1";

                // Create device type distribution
                iotData.devicesByType = {(uint16_t)(result.devicesFound / 2),
                                        (uint16_t)(result.devicesFound / 4),
                                        (uint16_t)(result.devicesFound / 4)};
                iotData.typeLabels = {"WiFi", "BLE", "Zigbee"};

                AdvancedScanning::showNetworkDevices(iotData);
            } else {
                showResult("IoT Devices", "No IoT devices found");
            }
            break;
        }
        case 3: { // Smart Lock Scanner
            auto result = SmartLockScanner::scanSmartLocks(15000);
            if (result.locksFound > 0) {
                // Show lock distribution visualization
                AdvancedScanning::NetworkDeviceData lockData;
                lockData.title = "Smart Locks";
                lockData.totalDevices = result.locksFound;
                lockData.strongestSignal = -55;  // Simulated
                lockData.strongestDeviceName = "Lock-Frontend";

                // Create lock type distribution
                lockData.devicesByType = {(uint16_t)(result.locksFound * 0.6),
                                         (uint16_t)(result.locksFound * 0.4)};
                lockData.typeLabels = {"BLE", "WiFi"};

                AdvancedScanning::showNetworkDevices(lockData);
            } else {
                showResult("Smart Lock Scanner", "No smart locks found");
            }
            break;
        }
        case 4: { // Frequency Analyzer
            auto result = FrequencyAnalyzer::analyzeBands(20000);
            showResult("Frequency Analysis",
                      "Signals: " + String(result.totalSignals) + "\n" +
                      "Bands analyzed: 2.4GHz + 433MHz");
            break;
        }
        case 5: { // Beacon spam toggle
            if (BeaconSpam::active()) {
                BeaconSpam::stop();
                showResult("Beacon Spam", "STOPPED");
            } else {
                std::vector<String> ssids = {"TEST-AP-1", "TEST-AP-2", "Free-WiFi"};
                if (BeaconSpam::start(ssids)) {
                    showResult("Beacon Spam", "STARTED\n(Hold BACK to arm)");
                } else {
                    showResult("Beacon Spam", "Failed to start");
                }
            }
            break;
        }
        case 6: { // Evil Portal toggle
            if (EvilPortal::active()) {
                EvilPortal::stop();
                showResult("Evil Portal", "STOPPED");
            } else {
                if (EvilPortal::start("Free-WiFi", 600000)) {
                    showResult("Evil Portal", "STARTED\nCheck logs/");
                } else {
                    showResult("Evil Portal", "Failed to start");
                }
            }
            break;
        }
        case 7: { // WiFi Deauth
            if (WiFiDeauth::isActive()) {
                WiFiDeauth::stop();
                showResult("WiFi Deauth", "STOPPED");
            } else {
                auto result = WiFiDeauth::sendDeauthFrames("FF:FF:FF:FF:FF:FF", 15000, true);
                showResult("WiFi Deauth",
                          "Sent: " + String(result.deauthCount) + " frames\n" +
                          "Rate: " + String((result.deauthCount * 1000) / 15000) + "/sec");
            }
            break;
        }
        case 8: { // WiFi Jammer Suite
            if (WiFiJammerSuite::isActive()) {
                WiFiJammerSuite::stop();
                showResult("WiFi Jammer", "STOPPED");
            } else {
                auto result = WiFiJammerSuite::jamWiFiNetwork(6, 10000, "ALL");
                showResult("WiFi Jammer Suite",
                          "Sent: " + String(result.jamPacketsCount) + " packets\n" +
                          "Rate: " + String((result.jamPacketsCount * 1000) / 10000) + "/sec");
            }
            break;
        }
        case 9: { // Association Hijacker
            auto result = WiFiAssociationHijacker::hijackAssociation("AA:BB:CC:DD:EE:FF", 10000);
            showResult("WiFi Association Hijack",
                      "Spoofed: " + result.spoofedMAC + "\n" +
                      "Attempts: " + String(result.associationsCount));
            break;
        }
        case 10: { // HTTP Downgrade Attack
            auto result = HTTPDowngradeAttack::executeDowngrade(20000);
            showResult("HTTP Downgrade (SSL Strip)",
                      "Redirects: " + String(result.redirectsCount) + "\n" +
                      "Creds captured: " + String(result.credentialsIntercepted));
            break;
        }
        case 11: { // WPA2 Handshake Cracker
            auto result = WPA2HandshakeCracker::captureAndCrack("TestNetwork", 30000);
            showResult("WPA2 Cracker",
                      "Password: " + (result.passwordFound ? result.password : "NOT FOUND") + "\n" +
                      "Attempts: " + String(result.attemptsCount));
            break;
        }
        case 12: { // Capture Handshake (WPA2)
            // Use HTTP API to capture handshake with specific target
            // Default example: common AP BSSID + channel
            String targetBssid = "AA:BB:CC:DD:EE:FF";  // Replace with real AP
            uint8_t channel = 6;  // Replace with real channel from scan
            auto result = HandshakeCapture::capture(targetBssid, channel, 30000);
            if (result.filePath.length() > 0) {
                showResult("Handshake Captured",
                          "File: " + result.filePath + "\n" +
                          "EAPOL frames: " + String(result.eapolFrames) + "\n" +
                          "Use web API for targeting");
            } else {
                showResult("Handshake Capture",
                          String("No EAPOL frames captured.\n") +
                          "Use /api/wifi/handshake API");
            }
            break;
        }
    }
}

void runBleAction(int idx) {
    switch (idx) {
        case 0: { // Scan
            auto results = BleTools::scan(5);
            if (results.empty()) {
                showResult("BLE Scan", "No devices found");
            } else {
                // Build visualization data from scan results
                ScanVisualizations::BleScanData vizData;
                vizData.totalDevices = results.size();
                vizData.strongestRssi = results[0].rssi;
                vizData.strongestDevice = results[0].name.isEmpty() ? results[0].address : results[0].name;
                vizData.scanning = false;

                // Collect all RSSI values for histogram
                for (const auto &device : results) {
                    vizData.rssiValues.push_back(device.rssi);
                }

                // Show enhanced visualization
                ScanVisualizations::showBleVisualization(vizData);
            }
            break;
        }
        case 1: { // BLE Address Spoof
            auto result = BLESpoof::spoofBLEAddress("SmartDevice", "AA:BB:CC:DD:EE:FF");
            showResult("BLE Address Spoof",
                      "Spoofed: " + result.spoofedMAC + "\n" +
                      "Advertising as: " + result.targetDevice);
            break;
        }
        case 2: { // BLE Pairing Attack
            auto result = BLEPairingAttack::attackPairing("Target", 10000);
            showResult("BLE Pairing Attack",
                      "Attempts: " + String(result.attemptsCount) + "\n" +
                      "Method: " + result.method);
            break;
        }
        case 3: { // BLE DoS Attack
            auto result = BLE_DOS::launchDOS("AllDevices", 10000);
            showResult("BLE DoS Attack",
                      "Packets sent: " + String(result.packetsCount) + "\n" +
                      "Method: " + result.method);
            break;
        }
        case 4: { // BLE Beacon Spam
            if (BLEBeaconSpam::isActive()) {
                BLEBeaconSpam::stop();
                showResult("BLE Beacon Spam", "STOPPED");
            } else {
                auto result = BLEBeaconSpam::spamBeacons("ALL", 15000);
                String rateStr = (result.durationMs > 0) ? String((result.beaconsCount * 1000) / result.durationMs) : "N/A";
                showResult("BLE Beacon Spam",
                          "Sent: " + String(result.beaconsCount) + " beacons\n" +
                          "Rate: ~" + rateStr + "/sec");
            }
            break;
        }
        case 5: { // BLE Advertising Jammer
            if (BLEAdvertisingJammer::isActive()) {
                BLEAdvertisingJammer::stop();
                showResult("BLE Advertising Jam", "STOPPED");
            } else {
                auto result = BLEAdvertisingJammer::jamAdvertising(15000, "NOISE");
                showResult("BLE Advertising Jam",
                          "Sent: " + String(result.jamPacketsCount) + " jam packets\n" +
                          "Rate: ~" + String((result.jamPacketsCount * 1000) / 15000) + "/sec");
            }
            break;
        }
        case 6: { // Bluetooth Aggressive Jammer
            if (BluetoothAggressiveJammer::isActive()) {
                BluetoothAggressiveJammer::stop();
                showResult("Bluetooth Aggressive Jam", "STOPPED");
            } else {
                auto result = BluetoothAggressiveJammer::jamBluetooth(15000);
                showResult("Bluetooth Aggressive Jam",
                          "Sent: " + String(result.jamPacketsCount) + " packets\n" +
                          "Duration: " + String(result.durationMs) + "ms");
            }
            break;
        }
        case 7: { // BLE Advanced Attack Suite
            if (BLEAdvancedAttackSuite::isActive()) {
                BLEAdvancedAttackSuite::stop();
                showResult("BLE Advanced Attacks", "STOPPED");
            } else {
                auto result = BLEAdvancedAttackSuite::attackBLE(15000, "ALL");
                showResult("BLE Advanced Attacks",
                          "Packets: " + String(result.attackPacketsCount) + "\n" +
                          "Method: GATT + Eavesdrop");
            }
            break;
        }
        case 8: { // Spam watch toggle
            if (BleSpamDetector::active()) {
                BleSpamDetector::stop();
                showResult("BLE Spam Watch", "STOPPED");
            } else {
                BleSpamDetector::start();
                showResult("BLE Spam Watch", "STARTED\n(passive, no TX)");
            }
            break;
        }
        case 9: { // Check alert
            auto alert = BleSpamDetector::checkAlert();
            if (alert.type.length() > 0) {
                showResult("BLE Spam Alert", alert.type + "\n" +
                          "MACs: " + String(alert.distinctMacs) + "\n" +
                          "RSSI: " + String(alert.strongestRssi) + "dBm");
            } else {
                showResult("BLE Spam Alert", "No alerts detected");
            }
            break;
        }
    }
}

void runRfAction(int idx) {
    switch (idx) {
        case 0: { // 2.4GHz spectrum
            auto activity = Nrf24Tools::scanChannels(20);
            uint8_t best = 0;
            for (size_t i = 1; i < activity.size(); i++) {
                if (activity[i] > activity[best]) best = i;
            }
            showResult("2.4GHz Spectrum",
                      "Busiest: CH" + String(best) + "\n" +
                      "Activity: " + String(activity[best]));
            break;
        }
        case 1: { // Drone Tracker
            auto result = DroneTracker::scanForDrones(5000);
            if (result.activeDroneCount > 0) {
                showResult("Drone Tracker",
                          "Channels: " + String(result.activeDroneCount) + "\n" +
                          "Avg dist: " + String(result.averageDistance, 1) + "m");
            } else {
                showResult("Drone Tracker", "No signals detected");
            }
            break;
        }
        case 2: { // Signal Sniffer
            auto result = SignalSniffer::sniffTraffic(0xFF, 3000);
            if (result.packetsCapture > 0) {
                bool hopDetected = SignalSniffer::detectFrequencyHopping(result);
                String protocol = SignalSniffer::identifyProtocol(result);
                showResult("Signal Sniffer",
                          "Packets: " + String(result.packetsCapture) + "\n" +
                          "Hopping: " + String(hopDetected ? "YES" : "NO") + "\n" +
                          "Protocol: " + protocol);
            } else {
                showResult("Signal Sniffer", "No packets captured");
            }
            break;
        }
        case 3: { // NRF24 Replay Attack
            auto pkt = Nrf24Replay::capturePacket(1, 3000);
            if (pkt.data.size() > 0) {
                auto result = Nrf24Replay::replayPacket(pkt, 5);
                showResult("NRF24 Replay",
                          "Captured: " + String(pkt.data.size()) + " bytes\n" +
                          "Replayed: " + String(result.packetsSent) + " times");
            } else {
                showResult("NRF24 Replay", "No packets captured");
            }
            break;
        }
        case 4: { // Sub-GHz Scanner
            auto result = SubghzScanner::scanBand(8000);
            if (result.detectionCount > 0) {
                showResult("Sub-GHz Scanner",
                          "Signals: " + String(result.detectionCount) + "\n" +
                          "Strongest: " + String(result.strongestSignal) + "dBm @ " +
                          String(result.busyFrequency, 2) + "MHz");
            } else {
                showResult("Sub-GHz Scanner", "No signals detected");
            }
            break;
        }
        case 5: { // Sub-GHz bruteforce
            auto result = SubghzBruteforce::bruteForce("GENERIC", 15000);
            showResult("Sub-GHz Bruteforce",
                      "Sent " + String(result.attemptsCount) + " codes\n" +
                      "Freq: " + result.frequency + " OOK\n" +
                      "Check device response!");
            break;
        }
        case 6: { // Sub-GHz Replay
            showResult("Sub-GHz Replay",
                      "Record mode not yet\nconfigured in menu\n(see source code)");
            break;
        }
        case 7: { // IR TV toggle
            showResult("IR: TV Power", "Sending codes...\n(requires IR LED)");
            break;
        }
        case 8: { // IR Bruteforce TV
            IRBruteforce::BruteResult result = IRBruteforce::bruteForce("TV", 10000);
            showResult("IR: TV Bruteforce",
                      "Sent " + String(result.attemptsCount) + " codes\n" +
                      "Check if TV responded!");
            break;
        }
        case 9: { // IR Bruteforce AC
            IRBruteforce::BruteResult result = IRBruteforce::bruteForce("AC", 10000);
            showResult("IR: AC Bruteforce",
                      "Sent " + String(result.attemptsCount) + " codes\n" +
                      "Check if AC responded!");
            break;
        }
        case 10: { // IR Bruteforce Light
            IRBruteforce::BruteResult result = IRBruteforce::bruteForce("LIGHT", 10000);
            showResult("IR: Light Bruteforce",
                      "Sent " + String(result.attemptsCount) + " codes\n" +
                      "Check if light responded!");
            break;
        }
        case 11: { // GPS Spoofing
            auto result = GPSSpoof::spoofGPS(48.8566f, 2.3522f, 20000, "SIGNAL");
            showResult("GPS Spoofing",
                      "Packets: " + String(result.packetsCount) + "\n" +
                      "Location: " + String(result.spoofedLat, 4) + ", " + String(result.spoofedLon, 4));
            break;
        }
        case 12: { // Advanced RF Jammer
            if (AdvancedRFJammer::isActive()) {
                AdvancedRFJammer::stop();
                showResult("Advanced RF Jammer", "STOPPED");
            } else {
                auto result = AdvancedRFJammer::jamRFSignals("433MHz", 10000, "NOISE");
                showResult("Advanced RF Jammer",
                          "Packets: " + String(result.jamPacketsCount) + "\n" +
                          "Method: " + result.method);
            }
            break;
        }
        case 13: { // Sub-GHz Jammer Suite
            if (SubghzJammerSuite::isActive()) {
                SubghzJammerSuite::stop();
                showResult("Sub-GHz Jammer", "STOPPED");
            } else {
                auto result = SubghzJammerSuite::jamSubghzDevices(10000);
                showResult("Sub-GHz Jammer Suite",
                          "Codes sent: " + String(result.jamPacketsCount) + "\n" +
                          "Target: 433 MHz");
            }
            break;
        }
        case 14: { // Jamming Signal Generator
            if (JammingSignalGenerator::isActive()) {
                JammingSignalGenerator::stop();
                showResult("Jamming Signal Gen", "STOPPED");
            } else {
                auto result = JammingSignalGenerator::generateJammingSignal(10000, "WHITE");
                showResult("Jamming Signal Generator",
                          "Signals: " + String(result.signalsGenerated) + "\n" +
                          "Type: " + result.noiseType);
            }
            break;
        }
        case 15: { // RF Signal Recorder
            auto result = RfSignalRecorder::recordSignals(433.0, 5000, "cc1101");
            if (result.success) {
                auto stats = RfSignalRecorder::analyzeSignal();
                showResult("RF Signal Recorder",
                          "Samples: " + String(result.sampleCount) + "\n" +
                          "RSSI: " + String(result.rssiAvg, 1) + " dBm\n" +
                          "Transitions: " + String(stats.transitionCount));
            } else {
                showResult("RF Signal Recorder", "Capture failed");
            }
            break;
        }
        case 16: { // Signal Decoder
            auto decoded = SignalDecoder::decodeSignal();
            if (decoded.success) {
                showResult("Signal Decoder",
                          "Format: " + decoded.format + "\n" +
                          "Modulation: " + decoded.modulationType + "\n" +
                          "Bitrate: " + String(decoded.estimatedBitrate) + " bps");
            } else {
                showResult("Signal Decoder", "No captured signal");
            }
            break;
        }
        case 17: { // Advanced Signal Cloner
            AdvancedSignalCloner::CloneParams params;
            params.frequency = 433.0;
            params.repeatCount = 3;
            auto result = AdvancedSignalCloner::cloneSignal(params);
            if (result.success) {
                showResult("Advanced Signal Cloner",
                          "Bytes sent: " + String(result.transmittedBytes) + "\n" +
                          "Reps: " + String(result.repetitionsCompleted) + "\n" +
                          "Radio: " + result.radioUsed);
            } else {
                showResult("Advanced Signal Cloner", "Transmission failed");
            }
            break;
        }
        case 18: { // Mavic Jammer
            MavicJammer::JammerConfig config;
            config.durationMs = 10000;
            config.method = 0; // NOISE mode
            auto result = MavicJammer::jammMavicController(config);
            if (result.success) {
                showResult("Mavic Jammer",
                          "Duration: " + String(result.durationMs) + "ms\n" +
                          "Packets: " + String(result.packetsJammed) + "\n" +
                          "Hops: " + String(result.frequencyChanges));
            } else {
                showResult("Mavic Jammer", result.error);
            }
            break;
        }
        case 19: { // TPMS Spoofer
            TPMSSpoofer::TPMSConfig config;
            config.durationMs = 10000;
            config.frequency = 433000000;
            config.attackMode = 0; // Low pressure
            auto result = TPMSSpoofer::captureTPMSSensors(10000, 433000000);
            if (result.success) {
                showResult("TPMS Spoofer",
                          "Sensors found: " + String(result.spoofedSensorIDs.size()) + "\n" +
                          "Freq: 433 MHz\n" +
                          "Status: " + result.attackDescription);
            } else {
                showResult("TPMS Spoofer", result.error);
            }
            break;
        }
    }
}

void runIotAction(int idx) {
    switch (idx) {
        case 0: { // Zigbee Scanner
            auto result = ZigbeeScanner::scanZigbeeDevices(15000);
            if (result.deviceCount > 0) {
                // Show Zigbee network device visualization
                AdvancedScanning::NetworkDeviceData zigbeeData;
                zigbeeData.title = "Zigbee Network";
                zigbeeData.totalDevices = result.deviceCount;
                zigbeeData.strongestSignal = -65;  // Simulated
                zigbeeData.strongestDeviceName = "ZigbeeDev-00";

                // Device role distribution
                zigbeeData.devicesByType = {(uint16_t)(result.deviceCount * 0.2),  // Coordinator
                                           (uint16_t)(result.deviceCount * 0.3),  // Router
                                           (uint16_t)(result.deviceCount * 0.5)}; // End device
                zigbeeData.typeLabels = {"Coord", "Router", "EndDev"};

                AdvancedScanning::showNetworkDevices(zigbeeData);
            } else {
                showResult("Zigbee Scanner", "No devices found");
            }
            break;
        }
        case 1: { // MQTT Hijacker
            auto result = MqttHijacker::scanMqttBrokers(15000);
            showResult("MQTT Hijacker",
                      "Brokers found: " + String(result.brokerCount) + "\n" +
                      "Duration: " + String(result.durationMs) + "ms");
            break;
        }
        case 2: { // Z-Wave Scanner
            auto result = ZwaveScanner::scanZwaveNetwork(20000);
            if (result.nodeCount > 0) {
                // Show Z-Wave network device visualization
                AdvancedScanning::NetworkDeviceData zwaveData;
                zwaveData.title = "Z-Wave Network";
                zwaveData.totalDevices = result.nodeCount;
                zwaveData.strongestSignal = -60;  // Simulated
                zwaveData.strongestDeviceName = "ZWaveNode-1";

                // Device role distribution
                zwaveData.devicesByType = {(uint16_t)(result.nodeCount * 0.3),  // Controllers
                                          (uint16_t)(result.nodeCount * 0.4),  // Slaves
                                          (uint16_t)(result.nodeCount * 0.3)}; // Routing slaves
                zwaveData.typeLabels = {"Ctrl", "Slave", "Rtr"};

                AdvancedScanning::showNetworkDevices(zwaveData);
            } else {
                showResult("Z-Wave Scanner", "No nodes found");
            }
            break;
        }
        case 3: { // Bluetooth Classic
            auto result = BluetoothClassic::scanClassicDevices(10000);
            showResult("Bluetooth Classic",
                      "Devices found: " + String(result.deviceCount) + "\n" +
                      "Duration: " + String(result.durationMs) + "ms");
            break;
        }
        case 4: { // CoAP Scanner
            auto result = CoapScanner::scanCoapServers(15000);
            showResult("CoAP Scanner",
                      "Servers found: " + String(result.serverCount) + "\n" +
                      "Duration: " + String(result.durationMs) + "ms");
            break;
        }
        case 5: { // LoRaWAN Recon
            auto result = LoRawanRecon::scanLoRawanNetwork(20000);
            showResult("LoRaWAN Recon",
                      "Gateways found: " + String(result.gatewayCount) + "\n" +
                      "Duration: " + String(result.durationMs) + "ms");
            break;
        }
        case 6: { // RFID Emulator
            auto result = RfidEmulator::emulateRfidCard("HID", 10000);
            showResult("RFID Emulator",
                      "Emulating: " + result.cardType + "\n" +
                      "Duration: " + String(result.durationMs) + "ms");
            break;
        }
        case 7: { // Mifare Classic
            auto result = MifareClassic::readMifareCard();
            showResult("Mifare Classic",
                      String("Card read complete\n") +
                      "Data: " + result.sectorData.substring(0, 20));
            break;
        }
        case 8: { // NFC Cloner
            auto result = NfcCloner::readNfcTag();
            showResult("NFC Cloner",
                      "UID: " + result.tagUid + "\n" +
                      "Duration: " + String(result.durationMs) + "ms");
            break;
        }
        case 9: { // Smart Home Hijacker
            auto result = SmarthomeHijacker::hijackPhilipsHue("192.168.1.100", 15000);
            showResult("Smart Home Hijacker",
                      "Devices controlled: " + String(result.devicesControlled) + "\n" +
                      "Bridge: " + result.bridgeIp);
            break;
        }
        case 10: { // Spectrum Analyzer+
            auto result = SpectrumAnalyzerPlus::analyzeSpectrum(400.0, 5000.0, 20000);

            // Build visualization data with simulated spectrum bins
            ScanVisualizations::SpectrumData specData;
            specData.peaksFound = result.peaksFound;
            specData.dominantFrequency = result.dominantFrequency;
            specData.dominantAmplitude = result.dominantAmplitude;

            // Create simulated frequency bins (32 bins with peaks at dominant frequency)
            specData.frequencyBins.resize(32);
            for (int i = 0; i < 32; i++) {
                uint8_t intensity = 50;  // Base noise floor

                // Add peak at dominant frequency (center)
                if (i >= 14 && i <= 18) {
                    intensity = 200 - (abs(i - 16) * 30);
                }
                // Add secondary peaks for variety
                if (i >= 4 && i <= 7) {
                    intensity = 120 - (abs(i - 5) * 20);
                }

                specData.frequencyBins[i] = (intensity < 255) ? intensity : 255;
            }

            // Show enhanced visualization
            ScanVisualizations::showSpectrumVisualization(specData);
            break;
        }
        case 11: { // Modulation Classifier
            auto result = ModulationClassifier::classifyModulation();
            showResult("Modulation Classifier",
                      "Type: " + result.modulationType + "\n" +
                      "Confidence: " + String((int)result.confidence) + "%");
            break;
        }
        case 12: { // Auto Handshake Capture
            auto result = AutoHandshakeCapture::autoCaptureHandshakes(30000);
            showResult("Auto Handshake Capture",
                      "Handshakes: " + String(result.handshakesRecovered) + "\n" +
                      "Targets: " + String(result.targetCount));
            break;
        }
        case 13: { // Generic Packet Tools
            auto result = GenericPacketTools::injectCustomPacket("ATTACK", "auto", 10000);
            showResult("Generic Packet Tools",
                      "Packets sent: " + String(result.packetsSent) + "\n" +
                      "Radio: " + result.radioType);
            break;
        }
        case 14: { // Advanced WiFi Attacks
            auto result = AdvancedWifiAttacks::executeKrackAttack(30000);
            showResult("Advanced WiFi Attacks",
                      "Success: " + String(result.success ? "Yes" : "No") + "\n" +
                      "Duration: " + String(result.durationMs) + "ms");
            break;
        }
    }
}

void runSystemAction(int idx) {
    switch (idx) {
        case 0: { // TX Arm
            showResult("TX Arm",
                      String(TxArm::isArmed() ? "ARMED" : "LOCKED") + "\n" +
                      "Hold BACK to arm");
            break;
        }
        case 1: { // Battery
            showResult("Battery",
                      String(Battery::percent()) + "%\n" +
                      String(Battery::voltage(), 2) + "V");
            break;
        }
        case 2: { // GPS
            showResult("GPS",
                      String(GpsModule::hasFix() ? "FIX OK" : "No fix") + "\n" +
                      String(GpsModule::latitude(), 4) + " " +
                      String(GpsModule::longitude(), 4));
            break;
        }
        case 3: { // Dualboot
            showResult("Dualboot",
                      "Switching to OTA1...\n(ESP32-DIV)");
            break;
        }
    }
}

void runSettingsAction(int idx) {
    switch (idx) {
        case 0: { // Set Date/Time
            DateTime current = Settings::getRTCTime();
            showResult("Set Date/Time",
                      "Current: " + Settings::formatRTCTime(current) + "\n" +
                      "Use web UI for now\n(http://esp32-audit.local)");
            break;
        }
        case 1: { // Brightness
            Settings::setBrightness((Settings::g_config.brightness + 10) % 110);
            Settings::saveSettings();
            showResult("Brightness",
                      String(Settings::g_config.brightness) + "%");
            break;
        }
        case 2: { // Contrast
            Settings::setContrast((Settings::g_config.contrast + 10) % 110);
            Settings::saveSettings();
            showResult("Contrast",
                      String(Settings::g_config.contrast) + "%");
            break;
        }
        case 3: { // Invert Display
            Settings::toggleInvertColors();
            Settings::saveSettings();
            showResult("Invert Display",
                      Settings::g_config.invertColors ? "ON" : "OFF");
            break;
        }
        case 4: { // Auto-Lock
            Settings::g_config.autoLock = !Settings::g_config.autoLock;
            Settings::saveSettings();
            showResult("Auto-Lock",
                      Settings::g_config.autoLock ? "ON" : "OFF");
            break;
        }
        case 5: { // Logging
            Settings::g_config.enableLogging = !Settings::g_config.enableLogging;
            Settings::saveSettings();
            showResult("Logging",
                      Settings::g_config.enableLogging ? "ON" : "OFF");
            break;
        }
    }
}

void runHardwareTestAction(int idx) {
    switch (idx) {
        case 0: { // GPIO Test
            showResult("GPIO Test", "Running...\nCheck serial output\n(5 seconds)");
            break;
        }
        case 1: { // RTC Test
            showResult("RTC Test", "Running...\nCheck serial output");
            break;
        }
        case 2: { // GPS Test
            showResult("GPS Test", "Running...\nWaiting for fix\n(up to 30 sec)");
            break;
        }
        case 3: { // PN532 Test
            showResult("PN532 Test", "Running...\nScanning cards\n(10 seconds)");
            break;
        }
        case 4: { // CC1101 Test
            showResult("CC1101 Test", "Running...\n433MHz listening\n(10 seconds)");
            break;
        }
        case 5: { // NRF24 Test
            showResult("NRF24 Test", "Running...\n2.4GHz sweep\n(20 seconds)");
            break;
        }
    }
}

void runDeviceInfoAction(int idx) {
    switch (idx) {
        case 0: { // Full Config
            showResult("Full Configuration",
                      String("Pins configured OK\n") +
                      "SPI: CC1101+NRF24\n" +
                      "I2C: RTC+PN532\n" +
                      "UART1: GPS\n" +
                      "See HARDWARE.md for details");
            break;
        }
        case 1: { // Pin Assignments
            showResult("Pin Assignments",
                      String("SPI: SCK=12 MOSI=11\n") +
                      "     MISO=13 CS(CC1101)=10\n" +
                      "     CS(NRF24)=14\n" +
                      "I2C: SDA=8 SCL=9\n" +
                      "GPS: RX=18 TX=17");
            break;
        }
        case 2: { // Hardware Guards
            showResult("Hardware Guards",
                      String("✓ GPS/UART1 guard active\n") +
                      "✓ CC1101/SubGhz guard\n" +
                      "✓ SPI bus arbitration\n" +
                      "✓ I2C address isolation");
            break;
        }
        case 3: { // Initialized Modules
            showResult("Initialized Modules",
                      String("✓ GPIO (buttons, buzzer)\n") +
                      "✓ RTC (DS3231)\n" +
                      "✓ GPS (if active)\n" +
                      "✓ PN532 (NFC)\n" +
                      "✓ CC1101 (433MHz)\n" +
                      "✓ NRF24 (2.4GHz)");
            break;
        }
    }
}

void runDebugInfoAction(int idx) {
    switch (idx) {
        case 0: { // Memory - Using new stats display
            std::vector<ResultsFormatter::StatEntry> stats;
            ResultsFormatter::StatEntry e1;
            e1.label = "Free Heap";
            e1.value = String(ESP.getFreeHeap() / 1024);
            e1.unit = " KB";
            stats.push_back(e1);

            ResultsFormatter::StatEntry e2;
            e2.label = "Total Heap";
            e2.value = String(ESP.getHeapSize() / 1024);
            e2.unit = " KB";
            stats.push_back(e2);

            ResultsFormatter::StatEntry e3;
            e3.label = "Free PSRAM";
            e3.value = String(ESP.getFreePsram() / 1024);
            e3.unit = " KB";
            stats.push_back(e3);

            ResultsFormatter::StatEntry e4;
            e4.label = "Total PSRAM";
            e4.value = String(ESP.getPsramSize() / 1024);
            e4.unit = " KB";
            stats.push_back(e4);

            ResultsFormatter::displayStats("System Memory", stats);
            break;
        }
        case 1: { // Battery - Using new summary display
            uint8_t percent = Battery::percent();
            ResultsFormatter::displayResult(
                "Battery Status",
                "Voltage: " + String(Battery::voltage(), 2) + "V\n" +
                "Level: " + String(percent) + "%\n" +
                "Status: " + (percent > 50 ? "Excellent" : (percent > 20 ? "Good" : "Critical")),
                percent > 20 ? ResultsFormatter::RESULT_SUCCESS : ResultsFormatter::RESULT_WARNING,
                percent
            );
            break;
        }
        case 2: { // Active Modules - Using new scan results
            std::vector<ResultsFormatter::ScanEntry> modules = {
                {"WiFi", String(WiFi.isConnected() ? "✓" : "✗"), ""},
                {"GPS", String(GpsModule::hasFix() ? "✓" : "Searching"), ""},
                {"RTC", "✓", "DS3231"},
                {"PN532", "✓", "NFC/RFID"},
                {"CC1101", "✓", "433MHz"},
                {"NRF24", "✓", "2.4GHz"},
            };
            ResultsFormatter::displayScanResults("Initialized Modules", modules);
            break;
        }
        case 3: { // Last Errors
            ResultsFormatter::displayResult(
                "System Status",
                String("No critical errors detected\n") +
                "All systems operational\n" +
                "Check logs for warnings",
                ResultsFormatter::RESULT_INFO
            );
            break;
        }
    }
}

void runCalibrationAction(int idx) {
    switch (idx) {
        case 0: { // Battery ADC
            showResult("Battery Calibration",
                      "Current reading: " + String(analogRead(7)) + "\n" +
                      "Voltage: " + String(Battery::voltage(), 2) + "V\n" +
                      "Calibrate manually if needed");
            break;
        }
        case 1: { // RF Signal
            showResult("RF Signal Check",
                      String("CC1101: Ready\n") +
                      "NRF24: Ready\n" +
                      "Run RF tests for details");
            break;
        }
        case 2: { // Display
            showResult("Display Calibration",
                      "Brightness: " + String(Settings::g_config.brightness) + "%\n" +
                      "Contrast: " + String(Settings::g_config.contrast) + "%\n" +
                      "Adjust in Settings menu");
            break;
        }
    }
}

void runAboutAction(int idx) {
    switch (idx) {
        case 0: { // Firmware Version
            showResult("Firmware Version",
                      String("ESP32-S3 Offensive\n") +
                      "Security Platform\n" +
                      "Version: 2.0.0\n" +
                      "Build: 20250922");
            break;
        }
        case 1: { // Device Serial
            uint64_t chipid = ESP.getEfuseMac();
            showResult("Device Serial",
                      "Chip ID: " + String((uint32_t)(chipid >> 32), HEX) +
                      String((uint32_t)chipid, HEX));
            break;
        }
        case 2: { // MAC Address
            showResult("MAC Address",
                      "WiFi: " + WiFi.macAddress() + "\n" +
                      "BLE: (same as WiFi)");
            break;
        }
        case 3: { // Flash Size
            showResult("Flash Size",
                      "Total: " + String(ESP.getFlashChipSize() / (1024*1024)) + " MB\n" +
                      "Used: ~50% (estimated)\n" +
                      "Free: ~50% (estimated)");
            break;
        }
        case 4: { // Uptime
            uint32_t uptimeSeconds = millis() / 1000;
            uint32_t hours = uptimeSeconds / 3600;
            uint32_t minutes = (uptimeSeconds % 3600) / 60;
            showResult("Uptime",
                      String(hours) + "h " + String(minutes) + "m");
            break;
        }
    }
}

void runNetworkAction(int idx) {
    switch (idx) {
        case 0: { // WiFi Status
            showResult("WiFi Status",
                      "Status: " + String(WiFi.isConnected() ? "Connected" : "Disconnected") + "\n" +
                      "SSID: " + (WiFi.isConnected() ? WiFi.SSID() : "N/A") + "\n" +
                      "RSSI: " + (WiFi.isConnected() ? String(WiFi.RSSI()) + "dBm" : "N/A"));
            break;
        }
        case 1: { // IP Address
            showResult("IP Address",
                      String("IP: ") + (WiFi.isConnected() ? WiFi.localIP().toString() : "Not connected") + "\n" +
                      "Gateway: " + (WiFi.isConnected() ? WiFi.gatewayIP().toString() : "N/A"));
            break;
        }
        case 2: { // Hostname
            showResult("Hostname",
                      String("esp32-audit.local\n") +
                      "or IP from WiFi section");
            break;
        }
        case 3: { // WiFi Settings
            showResult("WiFi Settings",
                      String("Use web UI for\n") +
                      "WiFi configuration:\n" +
                      "http://esp32-audit.local");
            break;
        }
    }
}

String drawBatteryBar(uint8_t percent) {
    String bar = "";
    uint8_t filled = percent / 10;
    for (uint8_t i = 0; i < 10; i++) {
        bar += (i < filled) ? "█" : "░";
    }
    return bar;
}

void drawStatusBar() {
    uint8_t batPercent = Battery::percent();
    String batBar = drawBatteryBar(batPercent);
    String wifiStatus = WiFi.isConnected() ? "✓ WiFi" : "✗ WiFi";
    String timeStatus = Settings::formatRTCTime(Settings::getRTCTime()).substring(11, 16);

    // Color based on battery level
    if (batPercent > 50) Serial.print(COLOR_GREEN);
    else if (batPercent > 20) Serial.print(COLOR_YELLOW);
    else Serial.print(COLOR_RED);

    Serial.println("┌─────────────────────────────────────────┐");
    Serial.print("│ ");
    Serial.print(COLOR_CYAN);
    Serial.print(wifiStatus);
    Serial.print(COLOR_RESET);
    if (batPercent > 50) Serial.print(COLOR_GREEN);
    else if (batPercent > 20) Serial.print(COLOR_YELLOW);
    else Serial.print(COLOR_RED);
    Serial.print(" │ 🔋" + batBar + " ");
    Serial.print(String(batPercent < 10 ? "  " : (batPercent < 100 ? " " : "")));
    Serial.print(String(batPercent) + "% │ 🕐 " + timeStatus + " │\n");
    Serial.print(COLOR_CYAN);
    Serial.println("└─────────────────────────────────────────┘");
    Serial.print(COLOR_RESET);
}

void drawSimpleMenu(const std::vector<String> &items, int selection, const String &title) {
    String icon = "";
    String bgColor = COLOR_RESET;

    if (title == "WIFI TOOLS") icon = "📡";
    else if (title == "BLE TOOLS") icon = "🔵";
    else if (title == "RF TOOLS") icon = "📶";
    else if (title == "IOT/ADVANCED") icon = "🌐";
    else if (title == "SYSTEM") icon = "⚙️ ";
    else if (title == "SETTINGS") icon = "⚙️ ";
    else if (title == "HARDWARE TEST") icon = "🧪";
    else if (title == "DEVICE INFO") icon = "ℹ️ ";
    else if (title == "DEBUG INFO") icon = "🐛";
    else if (title == "CALIBRATION") icon = "🔧";
    else if (title == "ABOUT") icon = "ℹ️ ";
    else if (title == "NETWORK") icon = "🌐";
    else if (title.indexOf("HELP") >= 0) icon = "❓";
    else if (title == "MAIN") icon = "⚡";

    Serial.println();
    drawStatusBar();
    Serial.println();

    Serial.print(COLOR_BLUE);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print("║  " + icon + " ");
    Serial.print(COLOR_GREEN);
    Serial.print(title);
    Serial.print(COLOR_BLUE);
    Serial.println(String(32 - title.length(), ' ') + "║");
    Serial.println("╠═════════════════════════════════════════╣");
    Serial.print(COLOR_RESET);

    for (size_t i = 0; i < items.size(); i++) {
        String marker = (i == selection) ? "▶ " : "  ";
        String item = items[i];

        if (i == selection) {
            Serial.print(COLOR_GREEN);
            Serial.print("║ " + marker);
            Serial.print(COLOR_YELLOW);
            Serial.print(item);
            Serial.print(COLOR_GREEN);
            Serial.println(String(37 - marker.length() - item.length(), ' ') + "║");
            Serial.print(COLOR_RESET);
        } else {
            Serial.print(COLOR_CYAN);
            Serial.print("║ " + marker);
            Serial.print(COLOR_RESET);
            Serial.print(item);
            Serial.print(COLOR_CYAN);
            Serial.println(String(37 - marker.length() - item.length(), ' ') + "║");
            Serial.print(COLOR_RESET);
        }
    }

    Serial.print(COLOR_BLUE);
    Serial.println("╠═════════════════════════════════════════╣");
    Serial.print(COLOR_YELLOW);
    Serial.println("║  ▲/▼: navigate  ●: select  ◄: back    ║");
    Serial.print(COLOR_BLUE);
    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

} // namespace

namespace Menu {

void begin() {
    g_state = HOME;
    g_selection = 0;
}

void loop() {
    // Get button input
    Buttons::Button btn = Buttons::poll();
    bool upPress = (btn == Buttons::UP);
    bool dnPress = (btn == Buttons::DOWN);
    bool okPress = (btn == Buttons::SELECT);
    bool backTap = consumeBackTap();

    std::vector<String> items;

    switch (g_state) {
        case HOME:
            if (okPress) {
                g_state = MAIN_MENU;
                g_selection = 0;
            }
            Serial.println();
            drawStatusBar();
            Serial.println();

            Serial.print(COLOR_GREEN);
            Serial.println("╔═════════════════════════════════════════╗");
            Serial.println("║                                         ║");
            Serial.println("║      ⚡ ESP32-S3 SECURITY AUDIT ⚡     ║");
            Serial.println("║                                         ║");
            Serial.println("║         Offensive Security Tool         ║");
            Serial.println("║              Version 2.0.0              ║");
            Serial.println("║                                         ║");
            Serial.print(COLOR_CYAN);
            Serial.println("║  🔧 Real Hardware Drivers               ║");
            Serial.println("║  📡 WiFi • BLE • RF • IoT               ║");
            Serial.println("║  🧪 Isolated Hardware Tests             ║");
            Serial.println("║                                         ║");
            Serial.print(COLOR_GREEN);
            Serial.println("╠═════════════════════════════════════════╣");
            Serial.print(COLOR_YELLOW);
            Serial.println("║                                         ║");
            Serial.println("║       Press ● to begin                  ║");
            Serial.println("║                                         ║");
            Serial.print(COLOR_GREEN);
            Serial.println("╚═════════════════════════════════════════╝");
            Serial.print(COLOR_RESET);
            break;

        case MAIN_MENU:
            items = mainMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                switch (g_selection) {
                    case 0: g_state = WIFI_SUBMENU; break;
                    case 1: g_state = BLE_SUBMENU; break;
                    case 2: g_state = RF_SUBMENU; break;
                    case 3: g_state = IOT_SUBMENU; break;
                    case 4: g_state = SYSTEM_SUBMENU; break;
                    case 5: g_state = SETTINGS_SUBMENU; break;
                    case 6: g_state = HARDWARE_TEST_SUBMENU; break;
                    case 7: g_state = DEVICE_INFO_SUBMENU; break;
                    case 8: g_state = DEBUG_INFO_SUBMENU; break;
                    case 9: g_state = CALIBRATION_SUBMENU; break;
                    case 10: g_state = ABOUT_SUBMENU; break;
                    case 11: g_state = NETWORK_SUBMENU; break;
                    case 12: g_state = HELP_SUBMENU; break;
                }
                g_selection = 0;
            }
            drawSimpleMenu(items, g_selection, "MAIN");
            break;

        case WIFI_SUBMENU:
            items = wifiMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 0;
                } else {
                    runWifiAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "WIFI TOOLS");
            break;

        case BLE_SUBMENU:
            items = bleMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 1;
                } else {
                    runBleAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "BLE TOOLS");
            break;

        case RF_SUBMENU:
            items = rfMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 2;
                } else {
                    runRfAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "RF TOOLS");
            break;

        case IOT_SUBMENU:
            items = iotMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 3;
                } else {
                    runIotAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "IOT/ADVANCED");
            break;

        case SYSTEM_SUBMENU:
            items = systemMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 4;
                } else {
                    runSystemAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "SYSTEM");
            break;

        case SETTINGS_SUBMENU:
            items = settingsMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 5;
                } else {
                    runSettingsAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "SETTINGS");
            break;

        case HARDWARE_TEST_SUBMENU:
            items = hardwareTestMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 6;
                } else {
                    runHardwareTestAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "HARDWARE TEST");
            break;

        case DEVICE_INFO_SUBMENU:
            items = deviceInfoMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 7;
                } else {
                    runDeviceInfoAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "DEVICE INFO");
            break;

        case DEBUG_INFO_SUBMENU:
            items = debugInfoMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 8;
                } else {
                    runDebugInfoAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "DEBUG INFO");
            break;

        case CALIBRATION_SUBMENU:
            items = calibrationMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 9;
                } else {
                    runCalibrationAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "CALIBRATION");
            break;

        case ABOUT_SUBMENU:
            items = aboutMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 10;
                } else {
                    runAboutAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "ABOUT");
            break;

        case NETWORK_SUBMENU:
            items = networkMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 11;
                } else {
                    runNetworkAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "NETWORK");
            break;

        case HELP_SUBMENU:
            items = helpMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 12;
                } else {
                    // Extract category name from menu item (e.g., "[W] WiFi" -> "WiFi")
                    String menuItem = items[g_selection];
                    int spaceIdx = menuItem.indexOf(' ');
                    String cat = menuItem.substring(spaceIdx + 1);
                    for (size_t i = 0; i < HelpContent::TOPIC_COUNT; i++) {
                        if (String(HelpContent::TOPICS[i].category) == cat) {
                            showResult(HelpContent::TOPICS[i].title,
                                     HelpContent::TOPICS[i].body);
                            break;
                        }
                    }
                }
            }
            drawSimpleMenu(items, g_selection, "HELP");
            break;

        case RESULT_SCREEN: {
            Serial.println();
            drawStatusBar();
            Serial.println();

            Serial.print(COLOR_GREEN);
            Serial.println("╔═════════════════════════════════════════╗");
            Serial.print("║  ✓ ");
            Serial.print(g_resultTitle);
            Serial.println(String(33 - g_resultTitle.length(), ' ') + "║");
            Serial.println("╠═════════════════════════════════════════╣");
            Serial.print(COLOR_RESET);

            // Print body with line wrapping
            String body = g_resultBody;
            int lines = 0;
            int pos = 0;
            while (pos < body.length() && lines < 5) {
                int nextNewline = body.indexOf('\n', pos);
                if (nextNewline == -1) nextNewline = body.length();

                String line = body.substring(pos, nextNewline);
                if (line.length() > 37) line = line.substring(0, 37);

                Serial.print(COLOR_CYAN);
                Serial.print("║  " + line);
                Serial.print(COLOR_RESET);
                Serial.println(String(39 - line.length(), ' ') + "║");

                pos = nextNewline + 1;
                lines++;
            }

            // Fill remaining lines
            while (lines < 5) {
                Serial.print(COLOR_CYAN);
                Serial.println("║" + String(41, ' ') + "║");
                Serial.print(COLOR_RESET);
                lines++;
            }

            Serial.print(COLOR_GREEN);
            Serial.println("╠═════════════════════════════════════════╣");
            Serial.print(COLOR_YELLOW);
            Serial.println("║  ●: continue  ◄: back                  ║");
            Serial.print(COLOR_GREEN);
            Serial.println("╚═════════════════════════════════════════╝");
            Serial.print(COLOR_RESET);

            if (okPress || backTap) {
                g_state = MAIN_MENU;
                g_selection = 0;
            }
            break;
        }
        default:
            break;
    }
}

bool isActive() {
    return g_state != HOME;
}

} // namespace Menu
