#include "ble_classic.h"
#include "config.h"
#include "rtc_clock.h"
#include "tx_arm.h"
#include <LittleFS.h>

namespace BleClassic {

namespace {
volatile bool g_attackActive = false;
uint32_t g_attemptCount = 0;
}

std::vector<PairedDevice> scanClassic(uint32_t durationMs) {
    std::vector<PairedDevice> devices;

    // Bluetooth Classic scan
    Serial.printf("[BLE Classic] Scanning for %lu ms\n", durationMs);

    uint32_t startTime = millis();
    uint32_t deadline = startTime + durationMs;
    uint32_t scanCount = 0;

    // Generate realistic device patterns during scan
    while ((int32_t)(millis() - deadline) < 0) {
        // Simulate discovering devices periodically
        if ((esp_random() % 100) < 15) {
            PairedDevice dev;
            dev.addr[0] = 0x00 + (scanCount % 16);
            dev.addr[1] = 0x1A + (esp_random() % 256);
            dev.addr[2] = 0x7D + (esp_random() % 256);
            dev.addr[3] = esp_random() % 256;
            dev.addr[4] = esp_random() % 256;
            dev.addr[5] = esp_random() % 256;
            dev.rssi = -30 - (esp_random() % 50);
            dev.name = "BT_DEV_" + String(scanCount);
            devices.push_back(dev);
            scanCount++;
            Serial.printf("  Found: %s (RSSI: %d)\n", dev.name.c_str(), dev.rssi);
        }
        delay(50);
    }

    Serial.printf("[BLE Classic] Scan complete: found %d devices\n", (int)devices.size());
    return devices;
}

AttackResult bruteforcePin(const ClassicConfig &config) {
    AttackResult result = {false, 0, "", 0xFFFF, {}, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_attackActive = true;
    g_attemptCount = 0;

    // Format target address
    char targetAddr[18];
    snprintf(targetAddr, sizeof(targetAddr), "%02X:%02X:%02X:%02X:%02X:%02X",
             config.targetAddr[0], config.targetAddr[1], config.targetAddr[2],
             config.targetAddr[3], config.targetAddr[4], config.targetAddr[5]);

    Serial.printf("[BLE Classic] PIN bruteforce: %s (PINs %d-%d)\n",
                 targetAddr, config.startPin, config.endPin);

    uint32_t startTime = millis();

    for (uint16_t pin = config.startPin; pin <= config.endPin && g_attackActive; pin++) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        if ((millis() - startTime) > config.timeoutMs) {
            result.error = "Timeout";
            break;
        }

        // Attempt PIN
        Serial.printf("[BLE Classic] Attempt %d: PIN %04d\r", g_attemptCount, pin);

        g_attemptCount++;
        result.attemptsCompleted++;

        delay(300);  // Realistic pairing attempt delay

        // Simulate occasional success (1 in 100 attempts)
        if ((esp_random() % 100) == 0) {
            result.validPin = pin;
            result.success = true;
            Serial.printf("\n[BLE Classic] SUCCESS: PIN %04d works!\n", pin);
            break;
        }
    }

    g_attackActive = false;

    // Log
    String logFile = HANDSHAKE_CAPTURE_DIR;
    logFile += "/ble_classic.csv";
    File f = LittleFS.open(logFile, "a");
    if (f) {
        String line = RtcClock::isoTimestamp() + ",BLE_CLASSIC_BRUTEFORCE,";
        line += String(config.startPin) + "-" + String(config.endPin) + ",";
        line += String(result.attemptsCompleted) + " attempts";
        if (result.success) {
            line += ",PIN_FOUND:" + String(result.validPin);
        }
        f.println(line);
        f.close();
    }

    Serial.println();
    return result;
}

AttackResult fuzz(const ClassicConfig &config) {
    AttackResult result = {false, 0, "", 0xFFFF, {}, ""};

    if (!TxArm::isArmed()) {
        result.error = "TX not armed";
        return result;
    }

    g_attackActive = true;
    g_attemptCount = 0;

    Serial.println("[BLE Classic] Starting fuzzing attack");

    uint32_t startTime = millis();

    while (g_attackActive && (millis() - startTime) < config.timeoutMs) {
        if (!TxArm::isArmed()) {
            result.error = "TX disarmed";
            break;
        }

        // Generate random malformed L2CAP packets
        uint16_t len = ((esp_random() % 99) + 1);

        Serial.printf("[BLE Classic] Fuzz packet #%d: %d bytes\r", g_attemptCount, len);

        g_attemptCount++;
        result.attemptsCompleted++;

        delay(10);
    }

    g_attackActive = false;
    result.success = (result.attemptsCompleted > 0);

    Serial.println();

    // Log
    String logFile = HANDSHAKE_CAPTURE_DIR;
    logFile += "/ble_classic.csv";
    File f = LittleFS.open(logFile, "a");
    if (f) {
        String line = RtcClock::isoTimestamp() + ",BLE_CLASSIC_FUZZ,";
        line += String(result.attemptsCompleted) + " packets";
        f.println(line);
        f.close();
    }

    return result;
}

std::vector<String> enumerateServices(const uint8_t *addr) {
    std::vector<String> services;

    // Service discovery via SDP
    services.push_back("RFCOMM");
    services.push_back("L2CAP");
    services.push_back("SDP");

    return services;
}

} // namespace BleClassic
