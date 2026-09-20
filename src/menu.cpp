#include "menu.h"
#include "buttons.h"
#include "display.h"
#include "config.h"
#include "tx_arm.h"
#include "wifi_tools.h"
#include "ble_tools.h"
#include "nrf24_tools.h"
#include "subghz.h"
#include "deauth.h"
#include "beacon_spam.h"
#include "evil_portal.h"
#include "wardriving.h"
#include "ir_tools.h"
#include "ble_gatt_audit.h"
#include "ble_fuzzer.h"
#include "ble_spam_detector.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "battery.h"
#include "dualboot.h"
#include "ui/ui.h"

#include <vector>

namespace {

enum State {
    HOME,
    MAIN,
    WIFI_RESULTS,
    WIFI_AP_ACTION,
    BLE_RESULTS,
    BLE_DEVICE_ACTION,
    RESULT_MSG,
};

State g_state = HOME;
State g_resultReturnTo = MAIN;
int g_mainSel = 0;
int g_wifiSel = 0;
int g_apActionSel = 0;
int g_bleSel = 0;
int g_bleActionSel = 0;
std::vector<WifiTools::ApInfo> g_wifiResults;
std::vector<BleTools::BleDevice> g_bleResults;
SubGhz::Capture g_lastCapture;
bool g_haveCapture = false;
IrTools::IrCapture g_lastIrCapture;
bool g_haveIrCapture = false;
String g_resultTitle, g_resultBody;

// BACK is dual-purpose: a quick tap navigates back, but holding it is the
// TX-arm signal (see tx_arm.h). We can't tell which until it's released,
// so navigation only fires on release-if-it-was-short, computed here
// instead of from Buttons::poll()'s press-edge.
bool g_backHeld = false;
uint32_t g_backHeldSince = 0;

bool consumeBackTap() {
    bool now = Buttons::isHeld(Buttons::BACK);
    bool tapped = false;
    if (now && !g_backHeld) {
        g_backHeldSince = millis();
    } else if (!now && g_backHeld) {
        if (millis() - g_backHeldSince < 500) tapped = true;
    }
    g_backHeld = now;
    return tapped;
}

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
        "IR: TV Power Toggle",
        "IR: Learn",
        "IR: Replay Last",
        String("BLE Spam Watch: ") + (BleSpamDetector::active() ? "STOP" : "start"),
        "BLE Spam: Check Alert",
        "TX arm status (hold BACK)",
        "Boot into Bruce",
    };
}

Ui::StatusInfo currentStatus() {
    Ui::StatusInfo status;
    status.time = RtcClock::isoTimestamp();
    status.gpsFix = GpsModule::hasFix();
    status.battPercent = Battery::percent();
    status.radioActive = BeaconSpam::active() || EvilPortal::active() || BleSpamDetector::active();
    return status;
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
            g_bleResults = BleTools::scan(5);
            g_bleSel = 0;
            g_state = g_bleResults.empty() ? RESULT_MSG : BLE_RESULTS;
            if (g_bleResults.empty()) showResult("BLE Scan", "No devices found", MAIN);
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
                       !g_haveCapture ? "No capture yet" : (ok ? "Replayed" : "Blocked: hold BACK to confirm"),
                       MAIN);
            break;
        }
        case 6: { // Beacon Spam toggle
            if (BeaconSpam::active()) {
                BeaconSpam::stop();
                showResult("Beacon Spam", "Stopped", MAIN);
            } else {
                bool ok = BeaconSpam::start(splitCsv(DEFAULT_BEACON_SSIDS), true);
                showResult("Beacon Spam", ok ? "Started (default test SSIDs)" : "Blocked: hold BACK to confirm", MAIN);
            }
            break;
        }
        case 7: { // Evil Portal toggle
            if (EvilPortal::active()) {
                EvilPortal::stop();
                showResult("Evil Portal", "Stopped", MAIN);
            } else {
                bool ok = EvilPortal::start(DEFAULT_PORTAL_SSID);
                showResult("Evil Portal", ok ? "Started as " DEFAULT_PORTAL_SSID : "Blocked: hold BACK to confirm", MAIN);
            }
            break;
        }
        case 8: { // Wardrive snapshot
            size_t rows = Wardriving::captureSnapshot();
            showResult("Wardrive", String(rows) + " rows added", MAIN);
            break;
        }
        case 9: { // IR: TV Power Toggle
            IrTools::sendUniversalPowerToggle();
            showResult("IR Power Toggle", "Sent (best-effort code list)", MAIN);
            break;
        }
        case 10: { // IR: Learn
            g_lastIrCapture = IrTools::learn(5000);
            g_haveIrCapture = !g_lastIrCapture.rawUs.empty();
            showResult("IR Learn", g_haveIrCapture
                           ? (String(g_lastIrCapture.rawUs.size()) + " pulses captured")
                           : "Nothing received",
                       MAIN);
            break;
        }
        case 11: { // IR: Replay Last
            if (g_haveIrCapture) IrTools::replay(g_lastIrCapture);
            showResult("IR Replay", g_haveIrCapture ? "Sent" : "No capture yet", MAIN);
            break;
        }
        case 12: { // BLE Spam Watch toggle
            if (BleSpamDetector::active()) {
                BleSpamDetector::stop();
                showResult("BLE Spam Watch", "Stopped", MAIN);
            } else {
                BleSpamDetector::start();
                showResult("BLE Spam Watch", "Started (passive, background)", MAIN);
            }
            break;
        }
        case 13: { // BLE Spam: Check Alert
            auto alert = BleSpamDetector::checkAlert();
            showResult("BLE Spam Alert",
                       alert.type.length()
                           ? (alert.type + ": " + String(alert.distinctMacs) +
                              " MACs, strongest " + String(alert.strongestRssi) + "dBm")
                           : "Nothing over threshold",
                       MAIN);
            break;
        }
        case 14: { // TX arm status
            showResult("TX arm (hold BACK)", TxArm::isArmed() ? "Currently HELD - TX allowed" : "Not held - TX blocked", MAIN);
            break;
        }
        case 15: { // Boot into Bruce
            bool go = Ui::confirm("Boot into Bruce", "Reboot into Bruce now? Power-cycle to come back.");
            if (go) {
                if (!DualBoot::bootIntoBruce()) showResult("Boot into Bruce", "Bruce not flashed to ota_1", MAIN);
                // on success this never returns -- the device restarts
            } else {
                g_state = MAIN;
            }
            break;
        }
    }
}

void runBleDeviceAction(int idx) {
    auto &dev = g_bleResults[g_bleSel];
    switch (idx) {
        case 0: { // GATT Audit
            auto rpt = BleGattAudit::audit(dev.address);
            String body;
            if (!rpt.connected) {
                body = "Could not connect";
            } else {
                int leaky = 0, weakWrite = 0;
                for (auto &f : rpt.findings) {
                    if (f.readableWithoutPairing) leaky++;
                    if (f.writableWithoutAuth) weakWrite++;
                }
                body += "Chars: " + String(rpt.findings.size()) + "\n";
                body += "Readable w/o pairing: " + String(leaky) + "\n";
                body += "Writable w/o auth: " + String(weakWrite) + "\n";
                body += "Pairing: " +
                        String(!rpt.pairingAttempted || !rpt.bonded
                                   ? "failed/none"
                                   : (rpt.authenticated ? "authenticated" : "JUST WORKS (no MITM)")) +
                        "\n";
                if (!rpt.deviceInfoLeaks.empty()) {
                    body += "Device Info leaks:\n";
                    for (auto &s : rpt.deviceInfoLeaks) body += " " + s + "\n";
                }
            }
            showResult("GATT Audit", body, BLE_RESULTS);
            break;
        }
        case 1: { // Fuzz (isolated only!)
            auto rpt = BleFuzzer::fuzz(dev.address);
            String body;
            if (!rpt.connected) {
                body = "Could not connect";
            } else {
                body += "Oversized writes ok: " + String(rpt.oversizedWritesAccepted) + "/" +
                        String(rpt.oversizedWritesAttempted) + "\n";
                body += "Read-only writes ok: " + String(rpt.readOnlyWritesAccepted) + "/" +
                        String(rpt.readOnlyWritesAttempted) + "\n";
                body += "Reconnect fails: " + String(rpt.reconnectCyclesFailed) + "/" +
                        String(rpt.reconnectCyclesAttempted) + "\n";
                body += rpt.deviceUnresponsiveAtEnd ? "Device UNRESPONSIVE after test!"
                                                     : "Device still responsive";
            }
            showResult("BLE Fuzz", body, BLE_RESULTS);
            break;
        }
        case 2: // Back
            g_state = BLE_RESULTS;
            break;
    }
}

void runApAction(int idx) {
    auto &ap = g_wifiResults[g_wifiSel];
    switch (idx) {
        case 0: { // Deauth this AP
            bool ok = Deauth::send(ap.bssid, "", ap.channel);
            showResult("Deauth", ok ? ("Sent to " + ap.bssid) : "Blocked: hold BACK to confirm", WIFI_RESULTS);
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
    // BACK is edge-triggered by poll() like the others, but it's also the
    // TX-arm hold signal: consumeBackTap() only reports it as a navigation
    // tap once released, and only if that hold was short. A long hold
    // (arming a TX action with SELECT) never triggers "go back".
    Buttons::Button btn = Buttons::poll();
    if (btn == Buttons::BACK) btn = Buttons::NONE; // ignore poll()'s press-edge for BACK
    bool backTapped = consumeBackTap();

    if (g_state == HOME) {
        if (btn != Buttons::NONE || backTapped) {
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
            else if (backTapped) g_state = HOME;
            if (g_state == MAIN) Ui::showList(currentStatus(), "Main Menu", items, g_mainSel);
            break;

        case WIFI_RESULTS: {
            std::vector<Ui::ListItem> rows;
            for (auto &ap : g_wifiResults) {
                Ui::ListItem item(ap.ssid.length() ? ap.ssid : "(hidden)");
                item.hasRssi = true;
                item.rssi = ap.rssi;
                item.badge = ap.enc;
                rows.push_back(item);
            }
            if (btn == Buttons::UP) g_wifiSel = (g_wifiSel + g_wifiResults.size() - 1) % g_wifiResults.size();
            else if (btn == Buttons::DOWN) g_wifiSel = (g_wifiSel + 1) % g_wifiResults.size();
            else if (btn == Buttons::SELECT) { g_apActionSel = 0; g_state = WIFI_AP_ACTION; }
            else if (backTapped) g_state = MAIN;
            if (g_state == WIFI_RESULTS) Ui::showList(currentStatus(), "Wi-Fi results", rows, g_wifiSel);
            break;
        }

        case WIFI_AP_ACTION: {
            std::vector<String> actions = {"Deauth this AP", "Sniff clients", "Back"};
            if (btn == Buttons::UP) g_apActionSel = (g_apActionSel + actions.size() - 1) % actions.size();
            else if (btn == Buttons::DOWN) g_apActionSel = (g_apActionSel + 1) % actions.size();
            else if (btn == Buttons::SELECT) runApAction(g_apActionSel); // hold BACK while pressing SELECT to arm "Deauth this AP"
            else if (backTapped) g_state = WIFI_RESULTS;
            if (g_state == WIFI_AP_ACTION)
                Ui::showList(currentStatus(), g_wifiResults[g_wifiSel].ssid, actions, g_apActionSel);
            break;
        }

        case BLE_RESULTS: {
            std::vector<Ui::ListItem> rows;
            for (auto &dev : g_bleResults) {
                Ui::ListItem item(dev.name.length() ? dev.name : dev.address);
                item.hasRssi = true;
                item.rssi = dev.rssi;
                item.badge = dev.manufacturerName;
                rows.push_back(item);
            }
            if (btn == Buttons::UP) g_bleSel = (g_bleSel + g_bleResults.size() - 1) % g_bleResults.size();
            else if (btn == Buttons::DOWN) g_bleSel = (g_bleSel + 1) % g_bleResults.size();
            else if (btn == Buttons::SELECT) { g_bleActionSel = 0; g_state = BLE_DEVICE_ACTION; }
            else if (backTapped) g_state = MAIN;
            if (g_state == BLE_RESULTS) Ui::showList(currentStatus(), "BLE results", rows, g_bleSel);
            break;
        }

        case BLE_DEVICE_ACTION: {
            std::vector<String> actions = {"GATT Audit", "Fuzz (isolated only!)", "Back"};
            if (btn == Buttons::UP) g_bleActionSel = (g_bleActionSel + actions.size() - 1) % actions.size();
            else if (btn == Buttons::DOWN) g_bleActionSel = (g_bleActionSel + 1) % actions.size();
            else if (btn == Buttons::SELECT) runBleDeviceAction(g_bleActionSel);
            else if (backTapped) g_state = BLE_RESULTS;
            if (g_state == BLE_DEVICE_ACTION) {
                auto &dev = g_bleResults[g_bleSel];
                Ui::showList(currentStatus(), dev.name.length() ? dev.name : dev.address, actions,
                              g_bleActionSel);
            }
            break;
        }

        case RESULT_MSG:
            Ui::showTextBlock(currentStatus(), g_resultTitle, g_resultBody);
            if (backTapped || btn == Buttons::SELECT) g_state = g_resultReturnTo;
            break;

        case HOME:
            break; // handled above
    }
}

} // namespace Menu
