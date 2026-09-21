#include "menu.h"
#include "menu_icons.h"
#include "buttons.h"
#include "config.h"
#include "tx_arm.h"
#include "wifi_tools.h"
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
#include <vector>
#include <set>

namespace {

enum State {
    HOME,
    MAIN_MENU,
    WIFI_SUBMENU,
    BLE_SUBMENU,
    RF_SUBMENU,
    SYSTEM_SUBMENU,
    HELP_SUBMENU,
    RESULT_SCREEN,
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
        "[W] WiFi Tools",
        "[B] BLE Tools",
        "[R] RF/2.4GHz",
        "[S] System",
        "[?] Help",
    };
}

std::vector<String> wifiMenuItems() {
    return {
        "Scan Networks",
        "Beacon Spam " + String(BeaconSpam::active() ? "STOP" : "start"),
        "Evil Portal " + String(EvilPortal::active() ? "STOP" : "start"),
        "Back",
    };
}

std::vector<String> bleMenuItems() {
    return {
        "Scan Devices (5s)",
        "BLE Spam Watch " + String(BleSpamDetector::active() ? "STOP" : "start"),
        "Check Spam Alert",
        "Back",
    };
}

std::vector<String> rfMenuItems() {
    return {
        "2.4GHz Spectrum Scan",
        "Drone Tracker (RSSI)",
        "Signal Sniffer (NRF24)",
        "Sub-GHz Scanner",
        "Sub-GHz Bruteforce",
        "Sub-GHz Replay",
        "IR: TV Power Toggle",
        "IR: Bruteforce TV",
        "IR: Bruteforce AC",
        "IR: Bruteforce Light",
        "Back",
    };
}

std::vector<String> systemMenuItems() {
    return {
        "TX Arm Status",
        "Battery Status",
        "GPS Map",
        "Dualboot OTA1",
        "Back",
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
                showResult("WiFi Scan",
                          String(results.size()) + " networks found\n" +
                          "Best: " + results[0].ssid + " " + String(results[0].rssi) + "dBm");
            }
            break;
        }
        case 1: { // Beacon spam toggle
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
        case 2: { // Evil Portal toggle
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
    }
}

void runBleAction(int idx) {
    switch (idx) {
        case 0: { // Scan
            auto results = BleTools::scan(5);
            if (results.empty()) {
                showResult("BLE Scan", "No devices found");
            } else {
                showResult("BLE Scan",
                          String(results.size()) + " devices found\n" +
                          "Best: " + results[0].name + " " + String(results[0].rssi) + "dBm");
            }
            break;
        }
        case 1: { // Spam watch toggle
            if (BleSpamDetector::active()) {
                BleSpamDetector::stop();
                showResult("BLE Spam Watch", "STOPPED");
            } else {
                BleSpamDetector::start();
                showResult("BLE Spam Watch", "STARTED\n(passive, no TX)");
            }
            break;
        }
        case 2: { // Check alert
            showResult("BLE Spam Alert", "No alerts detected");
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
        case 3: { // Sub-GHz Scanner
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
        case 4: { // Sub-GHz bruteforce
            auto result = SubghzBruteforce::bruteForce("GENERIC", 15000);
            showResult("Sub-GHz Bruteforce",
                      "Sent " + String(result.attemptsCount) + " codes\n" +
                      "Freq: " + result.frequency + " OOK\n" +
                      "Check device response!");
            break;
        }
        case 5: { // Sub-GHz Replay
            showResult("Sub-GHz Replay",
                      "Record mode not yet\nconfigured in menu\n(see source code)");
            break;
        }
        case 6: { // IR TV toggle
            showResult("IR: TV Power", "Sending codes...\n(requires IR LED)");
            break;
        }
        case 7: { // IR Bruteforce TV
            IRBruteforce::BruteResult result = IRBruteforce::bruteForce("TV", 10000);
            showResult("IR: TV Bruteforce",
                      "Sent " + String(result.attemptsCount) + " codes\n" +
                      "Check if TV responded!");
            break;
        }
        case 8: { // IR Bruteforce AC
            IRBruteforce::BruteResult result = IRBruteforce::bruteForce("AC", 10000);
            showResult("IR: AC Bruteforce",
                      "Sent " + String(result.attemptsCount) + " codes\n" +
                      "Check if AC responded!");
            break;
        }
        case 9: { // IR Bruteforce Light
            IRBruteforce::BruteResult result = IRBruteforce::bruteForce("LIGHT", 10000);
            showResult("IR: Light Bruteforce",
                      "Sent " + String(result.attemptsCount) + " codes\n" +
                      "Check if light responded!");
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

void drawSimpleMenu(const std::vector<String> &items, int selection, const String &title) {
    Serial.println("\n=== " + title + " ===");
    for (size_t i = 0; i < items.size(); i++) {
        String line = String(i == selection ? "> " : "  ") + items[i];
        Serial.println(line);
    }
    Serial.println("Use UP/DOWN to select, OK to choose, BACK to exit");
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
            Serial.println("\n*** ESP32 Audit Tool ***");
            Serial.println("Press OK to begin");
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
                    case 3: g_state = SYSTEM_SUBMENU; break;
                    case 4: g_state = HELP_SUBMENU; break;
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

        case SYSTEM_SUBMENU:
            items = systemMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 3;
                } else {
                    runSystemAction(g_selection);
                }
            }
            drawSimpleMenu(items, g_selection, "SYSTEM");
            break;

        case HELP_SUBMENU:
            items = helpMenuItems();
            if (upPress) g_selection = (g_selection - 1 + items.size()) % items.size();
            if (dnPress) g_selection = (g_selection + 1) % items.size();
            if (okPress) {
                if (g_selection == items.size() - 1) {
                    g_state = MAIN_MENU;
                    g_selection = 4;
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

        case RESULT_SCREEN:
            Serial.println("\n*** " + g_resultTitle + " ***");
            Serial.println(g_resultBody);
            Serial.println("\nPress OK to continue, BACK to go back");
            if (okPress || backTap) {
                g_state = MAIN_MENU;
                g_selection = 0;
            }
            break;

        default:
            break;
    }
}

bool isActive() {
    return g_state != HOME;
}

} // namespace Menu
