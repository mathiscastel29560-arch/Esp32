#include "http_downgrade_attack.h"
#include "tx_arm.h"

namespace {
volatile bool g_downgradeActive = false;
uint32_t g_redirectCount = 0;
uint32_t g_credCount = 0;
}

namespace HTTPDowngradeAttack {

DowngradeResult executeDowngrade(uint32_t durationMs) {
    DowngradeResult result{false, 0, 0, durationMs};

    Serial.println("\n=== HTTP Downgrade Attack (SSL Strip) ===");
    Serial.println("Duration: " + String(durationMs) + "ms");
    Serial.println("Target: HTTPS → HTTP downgrade");

    if (!TxArm::isArmed()) {
        Serial.println("✗ TX not armed");
        return result;
    }

    g_downgradeActive = true;
    g_redirectCount = 0;
    g_credCount = 0;
    uint32_t startTime = millis();

    Serial.println("Starting SSL Strip simulation...");
    Serial.println("Intercepting HTTPS requests...");

    while (millis() - startTime < durationMs && g_downgradeActive) {
        // Simulate HTTPS interception
        // In reality would be done via ARP spoofing + HTTP proxy

        // Simulate redirecting HTTPS to HTTP
        if ((esp_random() % 100) > 60) {
            g_redirectCount++;
            Serial.println("  → Redirected HTTPS request to HTTP");
        }

        // Simulate credential capture
        if ((esp_random() % 100) > 75) {
            g_credCount++;
            Serial.println("  ✓ Credentials captured: user:pass form");
        }

        delay(500);

        if (g_redirectCount % 5 == 0 && g_redirectCount > 0) {
            Serial.println("  [" + String(g_redirectCount) + "] redirects, [" +
                         String(g_credCount) + "] creds captured");
        }
    }

    g_downgradeActive = false;
    result.success = true;
    result.redirectsCount = g_redirectCount;
    result.credentialsIntercepted = g_credCount;

    Serial.println("✓ SSL Strip complete");
    Serial.println("Redirects: " + String(g_redirectCount));
    Serial.println("Credentials captured: " + String(g_credCount));
    Serial.println("Duration: " + String(millis() - startTime) + "ms");

    return result;
}

void stop() {
    g_downgradeActive = false;
}

bool isActive() {
    return g_downgradeActive;
}

}  // namespace HTTPDowngradeAttack
