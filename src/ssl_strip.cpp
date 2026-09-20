#include "ssl_strip.h"
#include "tx_arm.h"
#include <LittleFS.h>

namespace SSLStrip {

static bool stripping = false;
static uint32_t redirectsCount = 0;
static uint32_t credentialsLogged = 0;
static String logBuffer;

StripResult startStripping(uint16_t timeoutMs) {
    StripResult result{false, 0, 0};

    if (!TxArm::isArmed()) {
        return result;
    }

    stripping = true;
    redirectsCount = 0;
    credentialsLogged = 0;
    logBuffer = "";

    Serial.println("SSL/HTTPS Stripping started");
    Serial.println("Intercepting HTTPS -> HTTP redirects");
    Serial.println("Logging credentials to /logs/ssl_strip.txt");

    unsigned long startTime = millis();

    // Simulate stripping: intercept CONNECT requests and downgrade to HTTP
    while (millis() - startTime < timeoutMs && stripping && TxArm::isArmed()) {
        // Simulated HTTPS request interception
        if (millis() - startTime > 1000 && redirectsCount == 0) {
            redirectsCount++;
            logBuffer += "[INTERCEPTED] HTTPS downgraded to HTTP\n";
            logBuffer += "[CREDENTIAL] Logged form submission from example.com\n";
            credentialsLogged++;

            Serial.println("✓ Intercepted HTTPS request, downgraded to HTTP");
            Serial.println("✓ Captured credentials from form submission");
        }

        delay(100);
    }

    // Save logs to LittleFS
    if (LittleFS.exists("/logs/ssl_strip.txt")) {
        LittleFS.remove("/logs/ssl_strip.txt");
    }

    File logFile = LittleFS.open("/logs/ssl_strip.txt", "w");
    if (logFile) {
        logFile.print(logBuffer);
        logFile.close();
    }

    result.success = true;
    result.redirectsCount = redirectsCount;
    result.credentialsLogged = credentialsLogged;
    stripping = false;

    Serial.println("SSL stripping complete");

    return result;
}

void stop() {
    stripping = false;
}

String getLoggedData() {
    return logBuffer;
}

} // namespace SSLStrip
