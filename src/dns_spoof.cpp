#include "dns_spoof.h"
#include "tx_arm.h"
#include <DNSServer.h>
#include <WiFi.h>

namespace DNSSpoof {

static DNSServer dnsServer;
static bool active = false;
static uint32_t interceptCount = 0;

SpoofResult start(const String &domain, const String &spoofIP, uint16_t timeoutMs) {
    SpoofResult result{false, domain, spoofIP, 0};

    if (!TxArm::isArmed()) {
        return result;
    }

    IPAddress ip;
    ip.fromString(spoofIP);
    dnsServer.start(53, domain.c_str(), ip);
    active = true;
    interceptCount = 0;

    Serial.println("DNS Spoof: " + domain + " -> " + spoofIP);

    unsigned long endTime = millis() + timeoutMs;
    while (millis() < endTime && active && TxArm::isArmed()) {
        dnsServer.processNextRequest();
        delay(10);
    }

    result.success = true;
    result.interceptedCount = interceptCount;
    stop();

    return result;
}

void stop() {
    if (active) {
        dnsServer.stop();
        active = false;
    }
}

uint32_t getInterceptedRequests() {
    return interceptCount;
}

} // namespace DNSSpoof
