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

void handleStatus() {
    server.send(200, "application/json", "{}");
}

void handleWifiScan() {
    server.send(200, "application/json", "[]");
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
    server.begin();
}

void update() {
    server.handleClient();
}
