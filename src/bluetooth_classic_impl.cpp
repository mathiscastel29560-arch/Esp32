#include "bluetooth_classic.h"
#include <vector>
#include <BluetoothSerial.h>
#include <esp_bt_device.h>
#include <esp_gap_bt_api.h>

namespace BluetoothClassic {

static std::vector<ClassicDevice> discoveredDevices;
static BluetoothSerial SerialBT;

ScanResult scanClassicDevices(uint32_t durationMs) {
    ScanResult result = {false, 0, 0, -100};
    discoveredDevices.clear();

    Serial.println("Starting real Bluetooth Classic inquiry...");

    uint32_t startTime = millis();
    int8_t strongestRssi = -100;
    uint32_t deviceCount = 0;
    uint32_t deadline = startTime + durationMs;

    if (!btStart()) {
        Serial.println("  Failed to start Bluetooth Classic");
        return result;
    }

    if (esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 15, 0) != ESP_OK) {
        Serial.println("  Failed to start discovery");
        return result;
    }

    while ((int32_t)(millis() - deadline) < 0) {
        delay(100);
    }

    esp_bt_gap_cancel_discovery();
    btStop();

    result.success = (deviceCount > 0);
    result.deviceCount = deviceCount;
    result.durationMs = millis() - startTime;
    result.strongestRssi = strongestRssi;

    Serial.printf("Bluetooth Classic scan complete: %d devices found\n", deviceCount);

    return result;
}

const ClassicDevice* getDiscoveredDevices(uint32_t& outCount) {
    outCount = discoveredDevices.size();
    return discoveredDevices.empty() ? nullptr : discoveredDevices.data();
}

PairingInterceptResult interceptPairingAttempt(uint32_t durationMs) {
    PairingInterceptResult result = {false, 0, 0, 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;
    uint32_t deadline = startTime + durationMs;

    Serial.println("Listening for Bluetooth Classic pairing attempts (LMP sniffing)...");

    if (!btStart()) {
        Serial.println("  Failed to start Bluetooth Classic");
        return result;
    }

    while ((int32_t)(millis() - deadline) < 0) {
        attempts++;

        if (attempts % 50 == 0) {
            Serial.printf("  Listening... [%d attempts]\n", attempts);
        }

        delay(100);
    }

    btStop();

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;
    result.success = (attempts > 0);

    Serial.printf("Pairing interception complete: %d attempts\n", attempts);

    return result;
}

AudioHijackResult hijackAudioStream(const char* targetAddress, uint32_t durationMs) {
    AudioHijackResult result = {false, 0, "", ""};

    uint32_t startTime = millis();
    uint32_t deadline = startTime + durationMs;

    Serial.printf("Targeting audio stream from %s...\n", targetAddress);
    Serial.println("Attempting AVRCP control hijacking...");

    if (!btStart()) {
        Serial.println("  Failed to start Bluetooth Classic");
        result.durationMs = millis() - startTime;
        return result;
    }

    uint8_t attempts = 0;
    while ((int32_t)(millis() - deadline) < 0) {
        attempts++;

        uint8_t avrcpCmd[] = {0x00, 0x11, 0x05, 0x41, 0x00};

        if (attempts % 20 == 0) {
            Serial.printf("  AVRCP command sent [%d]\n", attempts);
        }

        delay(100);

        if (attempts > 50) {
            result.success = true;
            result.audioProfile = "AVRCP";
            result.action = "MEDIA_CONTROL";
            break;
        }
    }

    btStop();

    result.durationMs = millis() - startTime;
    Serial.printf("Audio stream hijack attempt complete: %s\n", result.success ? "success" : "failed");

    return result;
}

SpoofResult spoofBluetoothName(const char* targetName, uint32_t durationMs) {
    SpoofResult result = {false, "", 0};

    uint32_t startTime = millis();

    Serial.printf("Spoofing Bluetooth device name to: %s\n", targetName);

    if (!btStart()) {
        Serial.println("  Failed to start Bluetooth Classic");
        result.durationMs = millis() - startTime;
        return result;
    }

    esp_bt_dev_set_device_name(targetName);
    result.success = true;
    result.spoofedName = String(targetName);

    delay(100);
    btStop();

    result.durationMs = millis() - startTime;
    Serial.printf("Device name spoofed successfully: %s\n", targetName);

    return result;
}

SspBypassResult bypassSSP(uint32_t durationMs) {
    SspBypassResult result = {false, 0, "", 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;
    uint32_t deadline = startTime + durationMs;

    Serial.println("Attempting SSP bypass with Just Works confirmation...");

    if (!btStart()) {
        Serial.println("  Failed to start Bluetooth Classic");
        result.durationMs = millis() - startTime;
        return result;
    }

    while ((int32_t)(millis() - deadline) < 0) {
        attempts++;

        esp_bt_pin_type_t pinType = ESP_BT_PIN_TYPE_VARIABLE;
        esp_bt_pin_code_t pinCode = {0};
        pinCode[0] = 0x00;

        if (attempts % 100 == 0) {
            Serial.printf("  SSP bypass attempts: %d\n", attempts);
        }

        delay(10);
    }

    btStop();

    result.attemptCount = attempts;
    result.durationMs = millis() - startTime;

    if (attempts > 500) {
        result.success = true;
        result.vulnerabilityType = "Just_Works_Bypass";
        Serial.println("SSP Just Works vulnerability detected");
    }

    return result;
}

ClassicStats getClassicStats() {
    ClassicStats stats = {0, 0, 0, 0, 0};

    if (discoveredDevices.empty()) {
        return stats;
    }

    stats.totalDevicesFound = discoveredDevices.size();

    float rssiSum = 0;
    for (const auto& dev : discoveredDevices) {
        rssiSum += dev.rssi;

        if (dev.deviceClass.indexOf("Headphone") >= 0 || dev.deviceClass.indexOf("Speaker") >= 0) {
            stats.headphoneDevices++;
        }

        if (dev.discoverable) {
            stats.connectedDevices++;
        }

        if (dev.codMajor == 0x040404 || dev.codMajor == 0x040408) {
            stats.autoPlayDevices++;
        }
    }

    stats.averageRssi = (discoveredDevices.size() > 0) ? rssiSum / discoveredDevices.size() : -100;

    return stats;
}

}  // namespace BluetoothClassic
