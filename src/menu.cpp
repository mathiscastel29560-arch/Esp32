#include "menu.h"
#include "buttons.h"
#include "display.h"
#include "config.h"
#include "safety_switch.h"
#include "wifi_tools.h"
#include "ble_tools.h"
#include "nrf24_tools.h"
#include "subghz.h"
#include "deauth.h"
#include "beacon_spam.h"
#include "evil_portal.h"
#include "wardriving.h"

#include <vector>

namespace {

enum State {
    HOME,
    MAIN,
    WIFI_RESULTS,
    WIFI_AP_ACTION,
    RESULT_MSG,
};

State g_state = HOME;
State g_resultReturnTo = MAIN;
int g_mainSel = 0;
int g_wifiSel = 0;
int g_apActionSel = 0;
std::vector<WifiTools::ApInfo> g_wifiResults;
SubGhz::Capture g_lastCapture;
bool g_haveCapture = false;
String g_resultTitle, g_resultBody;

std::vector<String> splitCsv(const String &s) {
    std::vector<String> out;
    int start = 0;
    while (start < (int)s.length()) {
        int comma = s.indexOf(',', start);
        if (comma < 0) comma = s.length();
        out.push_back(s.substring(start, comma));
        start = comma + 1;
    }
    return out;
}

std::vector<String> mainMenuItems() {
    return {
        "Wi-Fi Scan",
        "BLE Scan",
        "2.4GHz Scan",
        "Sub-GHz Scan",
        "Sub-GHz Record",
        "Sub-GHz Replay Last",
        String("Beacon Spam: ") + (BeaconSpam::active() ? "STOP" : "start"),
        String("Evil Portal: ") + (EvilPortal::active() ? "STOP" : "start"),
        "Wardrive Snapshot",
        "Safety switch status",
    };
}

void showResult(const String &title, const String &body, State returnTo) {
    g_resultTitle = title;
    g_resultBody = body;
    g_resultReturnTo = returnTo;
    g_state = RESULT_MSG;
}

void runMainAction(int idx) {
    switch (idx) {
        case 0: { // Wi-Fi Scan
            g_wifiResults = WifiTools::scan();
            g_wifiSel = 0;
            g_state = g_wifiResults.empty() ? RESULT_MSG : WIFI_RESULTS;
            if (g_wifiResults.empty()) showResult("Wi-Fi Scan", "No networks found", MAIN);
            break;
        }
        case 1: { // BLE Scan
            auto devices = BleTools::scan(5);
            String body;
            for (size_t i = 0; i < devices.size() && i < 6; i++) {
                body += devices[i].address + " " + String(devices[i].rssi) + "\n";
            }
            if (devices.empty()) body = "No devices found";
            showResult("BLE Scan (" + String(devices.size()) + ")", body, MAIN);
            break;
        }
        case 2: { // 2.4GHz Scan
            auto activity = Nrf24Tools::scanChannels(20);
            uint8_t bestCh = 0, bestVal = 0;
            for (size_t i = 0; i < activity.size(); i++) {
                if (activity[i] > bestVal) { bestVal = activity[i]; bestCh = i; }
            }
            showResult("2.4GHz Scan", "Busiest ch=" + String(bestCh) + " hits=" + String(bestVal), MAIN);
            break;
        }
        case 3: { // Sub-GHz Scan
            int8_t rssi = SubGhz::rssiAt(433.92f);
            showResult("Sub-GHz Scan", "433.92MHz: " + String(rssi) + " dBm", MAIN);
            break;
        }
        case 4: { // Sub-GHz Record
            g_lastCapture = SubGhz::record(433.92f, 5000);
            g_haveCapture = !g_lastCapture.pulsesUs.empty();
            showResult("Sub-GHz Record", String(g_lastCapture.pulsesUs.size()) + " pulses captured", MAIN);
            break;
        }
        case 5: { // Sub-GHz Replay Last
            bool ok = g_haveCapture && SubGhz::replay(g_lastCapture);
            showResult("Sub-GHz Replay",
                       !g_haveCapture ? "No capture yet" : (ok ? "Replayed" : "Blocked: safety switch off"),
                       MAIN);
            break;
        }
        case 6: { // Beacon Spam toggle
            if (BeaconSpam::active()) {
                BeaconSpam::stop();
                showResult("Beacon Spam", "Stopped", MAIN);
            } else {
                bool ok = BeaconSpam::start(splitCsv(DEFAULT_BEACON_SSIDS), true);
                showResult("Beacon Spam", ok ? "Started (default test SSIDs)" : "Blocked: safety switch off", MAIN);
            }
            break;
        }
        case 7: { // Evil Portal toggle
            if (EvilPortal::active()) {
                EvilPortal::stop();
                showResult("Evil Portal", "Stopped", MAIN);
            } else {
                bool ok = EvilPortal::start(DEFAULT_PORTAL_SSID);
                showResult("Evil Portal", ok ? "Started as " DEFAULT_PORTAL_SSID : "Blocked: safety switch off", MAIN);
            }
            break;
        }
        case 8: { // Wardrive snapshot
            size_t rows = Wardriving::captureSnapshot();
            showResult("Wardrive", String(rows) + " rows added", MAIN);
            break;
        }
        case 9: { // Safety switch status
            showResult("Safety switch", SafetySwitch::isArmed() ? "ARMED (TX allowed)" : "SAFE (TX blocked)", MAIN);
            break;
        }
    }
}

void runApAction(int idx) {
    auto &ap = g_wifiResults[g_wifiSel];
    switch (idx) {
        case 0: { // Deauth this AP
            bool ok = Deauth::send(ap.bssid, "", ap.channel);
            showResult("Deauth", ok ? ("Sent to " + ap.bssid) : "Blocked: safety switch off", WIFI_RESULTS);
            break;
        }
        case 1: { // Sniff clients
            auto clients = WifiTools::sniffClients(ap.bssid, ap.channel, 4000);
            String body;
            for (size_t i = 0; i < clients.size() && i < 6; i++) body += clients[i] + "\n";
            if (clients.empty()) body = "None observed";
            showResult("Clients", body, WIFI_RESULTS);
            break;
        }
        case 2: // Back
            g_state = WIFI_RESULTS;
            break;
    }
}

} // namespace

namespace Menu {

void begin() {
    Buttons::begin();
}

bool isActive() { return g_state != HOME; }

void loop() {
    Buttons::Button btn = Buttons::poll();

    if (g_state == HOME) {
        if (btn != Buttons::NONE) {
            g_state = MAIN;
            g_mainSel = 0;
        }
        return; // main.cpp keeps drawing the status screen while HOME
    }

    auto items = mainMenuItems();

    switch (g_state) {
        case MAIN:
            if (btn == Buttons::UP) g_mainSel = (g_mainSel + items.size() - 1) % items.size();
            else if (btn == Buttons::DOWN) g_mainSel = (g_mainSel + 1) % items.size();
            else if (btn == Buttons::SELECT) runMainAction(g_mainSel);
            else if (btn == Buttons::BACK) g_state = HOME;
            if (g_state == MAIN) Display::showList("Main Menu", items, g_mainSel);
            break;

        case WIFI_RESULTS: {
            std::vector<String> lines;
            for (auto &ap : g_wifiResults) lines.push_back(ap.ssid + " " + String(ap.rssi));
            if (btn == Buttons::UP) g_wifiSel = (g_wifiSel + g_wifiResults.size() - 1) % g_wifiResults.size();
            else if (btn == Buttons::DOWN) g_wifiSel = (g_wifiSel + 1) % g_wifiResults.size();
            else if (btn == Buttons::SELECT) { g_apActionSel = 0; g_state = WIFI_AP_ACTION; }
            else if (btn == Buttons::BACK) g_state = MAIN;
            if (g_state == WIFI_RESULTS) Display::showList("Wi-Fi results", lines, g_wifiSel);
            break;
        }

        case WIFI_AP_ACTION: {
            std::vector<String> actions = {"Deauth this AP", "Sniff clients", "Back"};
            if (btn == Buttons::UP) g_apActionSel = (g_apActionSel + actions.size() - 1) % actions.size();
            else if (btn == Buttons::DOWN) g_apActionSel = (g_apActionSel + 1) % actions.size();
            else if (btn == Buttons::SELECT) runApAction(g_apActionSel);
            else if (btn == Buttons::BACK) g_state = WIFI_RESULTS;
            if (g_state == WIFI_AP_ACTION) Display::showList(g_wifiResults[g_wifiSel].ssid, actions, g_apActionSel);
            break;
        }

        case RESULT_MSG:
            Display::showText(g_resultTitle, g_resultBody);
            if (btn == Buttons::BACK || btn == Buttons::SELECT) g_state = g_resultReturnTo;
            break;

        case HOME:
            break; // handled above
    }
}

} // namespace Menu
