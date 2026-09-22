#include "web_ctrl.h"
#include "webui.h"
#include "config.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "tx_arm.h"
#include "wifi_tools.h"
#include "ble_tools.h"
#include "nrf24_tools.h"
#include "subghz.h"
#include "wardriving.h"
#include "deauth.h"
#include "beacon_spam.h"
#include "evil_portal.h"
#include "ir_tools.h"
#include "battery.h"
#include "ble_gatt_audit.h"
#include "ble_fuzzer.h"
#include "ble_spam_detector.h"
#include "handshake_capture.h"

#include <WebServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <vector>

namespace {
WebServer server(8080); // port 80 is reserved for EvilPortal's captive-portal page
String g_lastAction = "booted";
String g_lastCaptureFile = "";
IrTools::IrCapture g_lastIrCapture;
bool g_haveIrCapture = false;

String jsonEscape(const String &in) {
    String out;
    out.reserve(in.length() + 4);
    for (char c : in) {
        if (c == '"' || c == '\\') out += '\\';
        if ((uint8_t)c < 0x20) continue;
        out += c;
    }
    return out;
}

void note(const String &action) {
    g_lastAction = action;
}

void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void handleStatus() {
    try {
        bool defaultPassword = (String(AP_PASSWORD) == "auditctrl123");
        String json = "{";
        json += "\"time\":\"" + jsonEscape(RtcClock::isoTimestamp()) + "\",";
        json += "\"gpsFix\":" + String(GpsModule::hasFix() ? "true" : "false") + ",";
        json += "\"sats\":" + String(GpsModule::satellites()) + ",";
        json += "\"apClients\":" + String(WiFi.softAPgetStationNum()) + ",";
        json += "\"battV\":" + String(Battery::voltage(), 2) + ",";
        json += "\"battPct\":" + String(Battery::percent()) + ",";
        json += "\"safetyArmed\":" + String(TxArm::isArmed() ? "true" : "false") + ",";
        json += "\"defaultApPassword\":" + String(defaultPassword ? "true" : "false");
        json += "}";
        server.send(200, "application/json", json);
    } catch (...) {
        server.send(500, "application/json", "{\"error\":\"status query failed\"}");
    }
}

void handleWifiScan() {
    auto results = WifiTools::scan();
    String json = "[";
    for (size_t i = 0; i < results.size(); i++) {
        if (i) json += ",";
        auto &a = results[i];
        json += "{\"ssid\":\"" + jsonEscape(a.ssid) + "\",\"bssid\":\"" + a.bssid +
                "\",\"rssi\":" + String(a.rssi) + ",\"channel\":" + String(a.channel) +
                ",\"enc\":\"" + a.enc + "\"}";
    }
    json += "]";
    note("Wi-Fi scan: " + String(results.size()) + " APs");
    server.send(200, "application/json", json);
}

void handleWifiSniff() {
    String bssid = server.arg("bssid");
    uint8_t channel = (uint8_t)server.arg("channel").toInt();
    uint32_t ms = server.hasArg("ms") ? server.arg("ms").toInt() : 5000;
    auto clients = WifiTools::sniffClients(bssid, channel, ms);
    String json = "[";
    for (size_t i = 0; i < clients.size(); i++) {
        if (i) json += ",";
        json += "\"" + clients[i] + "\"";
    }
    json += "]";
    note("Sniffed " + String(clients.size()) + " clients on " + bssid);
    server.send(200, "application/json", json);
}

void handleBleScan() {
    uint32_t seconds = server.hasArg("seconds") ? server.arg("seconds").toInt() : 5;
    auto devices = BleTools::scan(seconds);
    String json = "[";
    for (size_t i = 0; i < devices.size(); i++) {
        if (i) json += ",";
        auto &d = devices[i];
        json += "{\"address\":\"" + d.address + "\",\"name\":\"" + jsonEscape(d.name) +
                "\",\"rssi\":" + String(d.rssi) + ",\"manufacturer\":\"" +
                jsonEscape(d.manufacturerName) + "\",\"random\":" +
                (d.randomAddress ? "true" : "false") + "}";
    }
    json += "]";
    note("BLE scan: " + String(devices.size()) + " devices");
    server.send(200, "application/json", json);
}

void handleBleGattAudit() {
    String address = server.arg("address");
    if (address.length() == 0) {
        note("BLE GATT audit error: missing address parameter");
        server.send(400, "application/json", "{\"error\":\"address parameter required\"}");
        return;
    }

    try {
        auto rpt = BleGattAudit::audit(address);
        String json = "{\"connected\":" + String(rpt.connected ? "true" : "false");
        if (rpt.connected) {
            int leaky = 0, weakWrite = 0;
            for (auto &f : rpt.findings) {
                if (f.readableWithoutPairing) leaky++;
                if (f.writableWithoutAuth) weakWrite++;
            }
            json += ",\"chars\":" + String(rpt.findings.size());
            json += ",\"readableWithoutPairing\":" + String(leaky);
            json += ",\"writableWithoutAuth\":" + String(weakWrite);
            json += ",\"bonded\":" + String(rpt.bonded ? "true" : "false");
            json += ",\"authenticated\":" + String(rpt.authenticated ? "true" : "false");
            json += ",\"deviceInfoLeaks\":[";
            for (size_t i = 0; i < rpt.deviceInfoLeaks.size(); i++) {
                if (i) json += ",";
                json += "\"" + jsonEscape(rpt.deviceInfoLeaks[i]) + "\"";
            }
            json += "]";
        }
        json += "}";
        note("BLE GATT audit: " + address);
        server.send(200, "application/json", json);
    } catch (...) {
        note("BLE GATT audit error: exception during audit");
        server.send(500, "application/json", "{\"error\":\"audit failed\"}");
    }
}

void handleBleFuzz() {
    String address = server.arg("address");
    if (address.length() == 0) {
        note("BLE fuzz error: missing address parameter");
        server.send(400, "application/json", "{\"error\":\"address parameter required\"}");
        return;
    }

    try {
        auto rpt = BleFuzzer::fuzz(address);
        String json = "{\"connected\":" + String(rpt.connected ? "true" : "false");
        if (rpt.connected) {
            json += ",\"oversizedWritesAttempted\":" + String(rpt.oversizedWritesAttempted);
            json += ",\"oversizedWritesAccepted\":" + String(rpt.oversizedWritesAccepted);
            json += ",\"readOnlyWritesAttempted\":" + String(rpt.readOnlyWritesAttempted);
            json += ",\"readOnlyWritesAccepted\":" + String(rpt.readOnlyWritesAccepted);
            json += ",\"reconnectCyclesAttempted\":" + String(rpt.reconnectCyclesAttempted);
            json += ",\"reconnectCyclesFailed\":" + String(rpt.reconnectCyclesFailed);
            json += ",\"deviceUnresponsiveAtEnd\":" + String(rpt.deviceUnresponsiveAtEnd ? "true" : "false");
        }
        json += "}";
        note("BLE fuzz: " + address);
        server.send(200, "application/json", json);
    } catch (...) {
        note("BLE fuzz error: exception during fuzzing");
        server.send(500, "application/json", "{\"error\":\"fuzz failed\"}");
    }
}

void handleBleSpamStart() {
    BleSpamDetector::start();
    note("BLE spam watch started");
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleBleSpamStop() {
    BleSpamDetector::stop();
    note("BLE spam watch stopped");
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleBleSpamCheck() {
    auto alert = BleSpamDetector::checkAlert();
    server.send(200, "application/json",
                "{\"active\":" + String(BleSpamDetector::active() ? "true" : "false") +
                    ",\"type\":\"" + jsonEscape(alert.type) + "\",\"distinctMacs\":" +
                    String(alert.distinctMacs) + ",\"strongestRssi\":" + String(alert.strongestRssi) + "}");
}

void handleNrfScan() {
    uint16_t samples = server.hasArg("samples") ? server.arg("samples").toInt() : 50;
    auto activity = Nrf24Tools::scanChannels(samples);
    String json = "[";
    for (size_t i = 0; i < activity.size(); i++) {
        if (i) json += ",";
        json += String(activity[i]);
    }
    json += "]";
    note("NRF24 channel scan done");
    server.send(200, "application/json", json);
}

void handleSubRssi() {
    float freq = server.hasArg("freq") ? server.arg("freq").toFloat() : 433.92f;
    int8_t rssi = SubGhz::rssiAt(freq);
    server.send(200, "application/json", "{\"rssi\":" + String(rssi) + "}");
}

void handleSubRecord() {
    float freq = server.hasArg("freq") ? server.arg("freq").toFloat() : 433.92f;
    uint32_t ms = server.hasArg("ms") ? server.arg("ms").toInt() : 8000;
    auto cap = SubGhz::record(freq, ms);
    String file = String(SUBGHZ_CAPTURE_DIR) + "/" + RtcClock::fileTimestamp() + ".cap";
    SubGhz::saveCapture(cap, file);
    g_lastCaptureFile = file;
    note("Sub-GHz capture: " + String(cap.pulsesUs.size()) + " pulses");
    server.send(200, "application/json",
                "{\"pulses\":" + String(cap.pulsesUs.size()) + ",\"file\":\"" + file + "\"}");
}

void handleSubReplay() {
    if (g_lastCaptureFile.length() == 0) {
        server.send(200, "application/json", "{\"ok\":false,\"reason\":\"no capture yet\"}");
        return;
    }
    auto cap = SubGhz::loadCapture(g_lastCaptureFile);
    bool ok = SubGhz::replay(cap);
    note(ok ? "Replayed sub-GHz capture" : "Replay blocked: hold BACK to confirm");
    server.send(200, "application/json",
                String("{\"ok\":") + (ok ? "true" : "false") + "}");
}

void handleWardriveSnapshot() {
    size_t rows = Wardriving::captureSnapshot();
    note("Wardrive snapshot: +" + String(rows) + " rows");
    server.send(200, "application/json",
                "{\"rows\":" + String(rows) + ",\"total\":" + String(Wardriving::rowCount()) + "}");
}

void handleDeauth() {
    String bssid = server.arg("bssid");
    String client = server.arg("client");
    String channelStr = server.arg("channel");

    // Validate parameters
    if (bssid.length() == 0 || client.length() == 0 || channelStr.length() == 0) {
        note("Deauth error: missing parameters (bssid, client, channel required)");
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing parameters\"}");
        return;
    }

    uint8_t channel = (uint8_t)channelStr.toInt();
    if (channel < 1 || channel > 11) {
        note("Deauth error: invalid channel " + String(channel));
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid channel\"}");
        return;
    }

    uint16_t frames = server.hasArg("frames") ? server.arg("frames").toInt() : 30;
    bool ok = Deauth::send(bssid, client, channel, frames);
    note(ok ? ("Deauth sent to " + bssid) : "Deauth blocked: hold BACK to confirm, or bad BSSID");
    server.send(200, "application/json", String("{\"ok\":") + (ok ? "true" : "false") + "}");
}

void handleBeaconStart() {
    String ssidsArg = server.arg("ssids");
    if (ssidsArg.length() == 0) {
        note("Beacon spam error: missing ssids parameter");
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"ssids parameter required\"}");
        return;
    }

    bool hop = server.arg("hop") != "false";
    std::vector<String> ssids;
    int start = 0;
    while (start < (int)ssidsArg.length()) {
        int comma = ssidsArg.indexOf(',', start);
        if (comma < 0) comma = ssidsArg.length();
        String s = ssidsArg.substring(start, comma);
        s.trim();
        if (s.length() > 0 && s.length() <= 32) { // SSID max length 32
            ssids.push_back(s);
        }
        start = comma + 1;
    }

    if (ssids.empty()) {
        note("Beacon spam error: no valid SSIDs after parsing");
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"no valid SSIDs\"}");
        return;
    }

    bool ok = BeaconSpam::start(ssids, hop);
    note(ok ? ("Beacon spam started: " + String(ssids.size()) + " SSIDs")
            : "Beacon spam blocked: hold BACK to confirm, or no SSIDs given");
    server.send(200, "application/json", String("{\"ok\":") + (ok ? "true" : "false") + "}");
}

void handleBeaconStop() {
    BeaconSpam::stop();
    note("Beacon spam stopped");
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleBeaconStatus() {
    server.send(200, "application/json",
                String("{\"active\":") + (BeaconSpam::active() ? "true" : "false") + "}");
}

void handleHandshakeCapture() {
    String bssid = server.arg("bssid");
    String channelStr = server.arg("channel");
    uint32_t durationMs = server.hasArg("ms") ? server.arg("ms").toInt() : 30000;

    if (bssid.length() == 0 || channelStr.length() == 0) {
        note("Handshake capture error: missing bssid or channel");
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"bssid and channel required\"}");
        return;
    }

    uint8_t channel = (uint8_t)channelStr.toInt();
    if (channel < 1 || channel > 11) {
        note("Handshake capture error: invalid channel " + String(channel));
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid channel (1-11)\"}");
        return;
    }

    auto result = HandshakeCapture::capture(bssid, channel, durationMs);
    note("Handshake capture: " + String(result.eapolFrames) + " EAPOL frames -> " + result.filePath);

    String json = "{\"ok\":" + String(result.filePath.length() > 0 ? "true" : "false");
    json += ",\"eapolFrames\":" + String(result.eapolFrames);
    json += ",\"file\":\"" + jsonEscape(result.filePath) + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void handleHandshakeDownload() {
    String filename = server.arg("file");
    if (filename.length() == 0 || filename.indexOf("..") >= 0) {
        server.send(400, "text/plain", "invalid filename");
        return;
    }

    String filepath = String(HANDSHAKE_CAPTURE_DIR) + "/" + filename;
    if (!LittleFS.exists(filepath)) {
        server.send(404, "text/plain", "file not found");
        return;
    }

    File f = LittleFS.open(filepath, FILE_READ);
    server.streamFile(f, "application/octet-stream");
    f.close();
    note("Handshake file downloaded: " + filename);
}

void handlePortalStart() {
    String ssid = server.arg("ssid");
    uint32_t maxMs = server.hasArg("maxMs") ? server.arg("maxMs").toInt() : 600000;
    bool ok = EvilPortal::start(ssid, maxMs);
    note(ok ? ("Evil portal started as " + ssid)
            : "Evil portal blocked: hold BACK to confirm, or already running");
    server.send(200, "application/json", String("{\"ok\":") + (ok ? "true" : "false") + "}");
}

void handlePortalStop() {
    EvilPortal::stop();
    note("Evil portal stopped");
    server.send(200, "application/json", "{\"ok\":true}");
}

void handlePortalStatus() {
    server.send(200, "application/json",
                String("{\"active\":") + (EvilPortal::active() ? "true" : "false") + "}");
}

void handlePortalLog() {
    if (!LittleFS.exists(EVILPORTAL_LOG_FILE)) {
        server.send(404, "text/plain", "no submissions yet");
        return;
    }
    File f = LittleFS.open(EVILPORTAL_LOG_FILE, FILE_READ);
    server.streamFile(f, "text/csv");
    f.close();
}

void handleIrPower() {
    IrTools::sendUniversalPowerToggle();
    note("IR: sent TV power-toggle codes");
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleIrLearn() {
    uint32_t ms = server.hasArg("ms") ? server.arg("ms").toInt() : 5000;
    g_lastIrCapture = IrTools::learn(ms);
    g_haveIrCapture = !g_lastIrCapture.rawUs.empty();
    note(g_haveIrCapture ? ("IR learn: " + String(g_lastIrCapture.rawUs.size()) + " pulses")
                          : "IR learn: nothing received");
    server.send(200, "application/json",
                String("{\"ok\":") + (g_haveIrCapture ? "true" : "false") +
                    ",\"pulses\":" + String(g_lastIrCapture.rawUs.size()) + "}");
}

void handleIrReplay() {
    if (!g_haveIrCapture) {
        server.send(200, "application/json", "{\"ok\":false,\"reason\":\"no capture yet\"}");
        return;
    }
    IrTools::replay(g_lastIrCapture);
    note("IR: replayed learned capture");
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleWardriveLog() {
    if (!LittleFS.exists(WARDRIVE_LOG_FILE)) {
        server.send(404, "text/plain", "no log yet");
        return;
    }
    File f = LittleFS.open(WARDRIVE_LOG_FILE, FILE_READ);
    server.streamFile(f, "text/csv");
    f.close();
}

} // namespace

namespace WebCtrl {

void begin() {
    server.on("/", handleRoot);
    server.on("/api/status", handleStatus);
    server.on("/api/wifi/scan", handleWifiScan);
    server.on("/api/wifi/sniff", handleWifiSniff);
    server.on("/api/ble/scan", handleBleScan);
    server.on("/api/nrf24/scan", handleNrfScan);
    server.on("/api/subghz/rssi", handleSubRssi);
    server.on("/api/subghz/record", HTTP_POST, handleSubRecord);
    server.on("/api/subghz/replay", HTTP_POST, handleSubReplay);
    server.on("/api/wardrive/snapshot", HTTP_POST, handleWardriveSnapshot);
    server.on("/api/wardrive/log", handleWardriveLog);
    server.on("/api/wifi/deauth", HTTP_POST, handleDeauth);
    server.on("/api/wifi/beacon/start", HTTP_POST, handleBeaconStart);
    server.on("/api/wifi/beacon/stop", HTTP_POST, handleBeaconStop);
    server.on("/api/wifi/beacon/status", handleBeaconStatus);
    server.on("/api/wifi/handshake", HTTP_POST, handleHandshakeCapture);
    server.on("/api/wifi/handshake/download", handleHandshakeDownload);
    server.on("/api/portal/start", HTTP_POST, handlePortalStart);
    server.on("/api/portal/stop", HTTP_POST, handlePortalStop);
    server.on("/api/portal/status", handlePortalStatus);
    server.on("/api/portal/log", handlePortalLog);
    server.on("/api/ir/power", HTTP_POST, handleIrPower);
    server.on("/api/ir/learn", HTTP_POST, handleIrLearn);
    server.on("/api/ir/replay", HTTP_POST, handleIrReplay);
    server.on("/api/ble/gatt-audit", HTTP_POST, handleBleGattAudit);
    server.on("/api/ble/fuzz", HTTP_POST, handleBleFuzz);
    server.on("/api/ble/spam/start", HTTP_POST, handleBleSpamStart);
    server.on("/api/ble/spam/stop", HTTP_POST, handleBleSpamStop);
    server.on("/api/ble/spam/check", handleBleSpamCheck);
    server.begin();
}

void loop() {
    server.handleClient();
}

String lastAction() { return g_lastAction; }

} // namespace WebCtrl
