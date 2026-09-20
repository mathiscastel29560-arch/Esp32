#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "config.h"
#include "webui.h"
#include "subghz.h"
#include "wardriving.h"
#include "deauth.h"
#include "handshake_capture.h"
#include "beacon_spam.h"
#include "evil_portal.h"
#include "ir_tools.h"
#include "badusb.h"
#include "rfid.h"

WebServer server(8080);
String g_lastAction = "booted";
String g_lastCaptureFile = "";
String g_lastHandshakeFile = "";

void note(const String &msg) {
    // Log message
}

String jsonEscape(const String &str) {
    String result;
    for (int i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c == '"' || c == '\\' || c == '/') {
            result += '\\';
        }
        result += c;
    }
    return result;
}

void handleWifiSniff() {
    server.send(200, "application/json", "{}");
}

void handleHandshakeCapture() {
    String bssid = server.arg("bssid");
    uint8_t channel = (uint8_t)server.arg("channel").toInt();
    uint32_t ms = server.hasArg("ms") ? server.arg("ms").toInt() : 9000;
    auto result = HandshakeCapture::capture(bssid, channel, ms);
    g_lastHandshakeFile = result.filePath;
    note(result.eapolFrames > 0
             ? ("Handshake capture: " + String(result.eapolFrames) + " EAPOL frames from " + bssid)
             : ("Handshake capture: nothing seen on " + bssid));
    server.send(200, "application/json",
                "{\"eapolFrames\":" + String(result.eapolFrames) + ",\"file\":\"" +
                    jsonEscape(result.filePath) + "\"}");
}

void handleHandshakeDownload() {
    if (g_lastHandshakeFile.length() == 0 || !LittleFS.exists(g_lastHandshakeFile)) {
        server.send(404, "text/plain", "no capture yet");
        return;
    }
    File f = LittleFS.open(g_lastHandshakeFile, FILE_READ);
    server.streamFile(f, "application/octet-stream");
    f.close();
}

void handleBleScan() {
    server.send(200, "application/json", "[]");
}

void handleNrfScan() {
    server.send(200, "application/json", "{}");
}

void handleSubRssi() {
    server.send(200, "application/json", "{}");
}

void handleBadUsb() {
    String osStr = server.arg("os");
    uint16_t count = server.hasArg("count") ? server.arg("count").toInt() : 15000;
    uint16_t delay_ms = server.hasArg("delay") ? server.arg("delay").toInt() : 100;

    BadUSB::OSType osType = BadUSB::OS_WINDOWS;
    if (osStr == "linux") osType = BadUSB::OS_LINUX;
    else if (osStr == "macos") osType = BadUSB::OS_MACOS;

    auto result = BadUSB::openWindowsSpam(osType, count, delay_ms);

    note("Bad USB: " + result.message);
    server.send(200, "application/json",
                "{\"status\":\"" + result.status + "\","
                "\"message\":\"" + jsonEscape(result.message) + "\","
                "\"keystrokes\":" + String(result.keystrokes) + "}");
}

void handleStatus() {
    server.send(200, "application/json", "{}");
}

void handleWifiScan() {
    server.send(200, "application/json", "[]");
}

void handleRfidScan() {
    auto result = RFID::scan();
    String uidHex;
    if (result.found) {
        for (int i = 0; i < result.tag.uidLen; i++) {
            if (result.tag.uid[i] < 0x10) uidHex += "0";
            uidHex += String(result.tag.uid[i], HEX);
        }
    }

    note(result.found ? ("RFID: Tag " + uidHex) : "RFID: No tag found");
    server.send(200, "application/json",
                "{\"found\":" + String(result.found ? "true" : "false") + ","
                "\"uid\":\"" + uidHex + "\","
                "\"type\":\"" + jsonEscape(result.tag.type) + "\","
                "\"capacity\":" + String(result.tag.capacity) + ","
                "\"scanTimeMs\":" + String(result.readTimeMs) + "}");
}

void handleRfidClone() {
    auto scanResult = RFID::scan();
    if (!scanResult.found) {
        server.send(400, "application/json", "{\"error\":\"No source tag found\"}");
        return;
    }

    auto cloneResult = RFID::clone(scanResult.tag);
    note(cloneResult.success ? ("RFID Clone: " + cloneResult.sourceUid + " -> " + cloneResult.targetUid)
                             : ("RFID Clone failed: " + cloneResult.error));

    server.send(200, "application/json",
                "{\"success\":" + String(cloneResult.success ? "true" : "false") + ","
                "\"sourceUid\":\"" + cloneResult.sourceUid + "\","
                "\"targetUid\":\"" + cloneResult.targetUid + "\","
                "\"bytesWritten\":" + String(cloneResult.bytesWritten) + ","
                "\"error\":\"" + jsonEscape(cloneResult.error) + "\"}");
}

void handleRfidList() {
    auto clones = RFID::listSavedClones();
    String json = "{\"clones\":[";
    for (size_t i = 0; i < clones.size(); i++) {
        if (i > 0) json += ",";
        json += "\"" + clones[i] + "\"";
    }
    json += "]}";
    server.send(200, "application/json", json);
}

void begin() {
    server.on("/", [](){ server.send(200, "text/html", WEBUI_HTML); });
    server.on("/api/status", handleStatus);
    server.on("/api/wifi/scan", handleWifiScan);
    server.on("/api/wifi/sniff", handleWifiSniff);
    server.on("/api/wifi/handshake", HTTP_POST, handleHandshakeCapture);
    server.on("/api/wifi/handshake/download", handleHandshakeDownload);
    server.on("/api/ble/scan", handleBleScan);
    server.on("/api/nrf24/scan", handleNrfScan);
    server.on("/api/subghz/rssi", handleSubRssi);
    server.on("/api/badusb/inject", HTTP_POST, handleBadUsb);
    server.on("/api/rfid/scan", HTTP_POST, handleRfidScan);
    server.on("/api/rfid/clone", HTTP_POST, handleRfidClone);
    server.on("/api/rfid/list", handleRfidList);
    server.begin();
}

void update() {
    server.handleClient();
}
