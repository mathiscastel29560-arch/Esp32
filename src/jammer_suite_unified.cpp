#include "jammer_suite_unified.h"
#include "wifi_jammer_suite.h"
#include "ble_advertising_jammer.h"
#include "advanced_rf_jammer.h"
#include "subghz_jammer_suite.h"
#include "jamming_signal_generator.h"
#include <Arduino.h>

namespace JammerSuite {

namespace {
    volatile bool g_jamming = false;
    volatile JammerType g_currentType = JAMMER_WIFI_CHANNEL;
}

JamResult startJamming(const JammerConfig &config) {
    JamResult result = {false, 0, 0, 0, ""};

    if (g_jamming) {
        result.error = "Jamming already active";
        return result;
    }

    if (config.durationMs == 0) {
        result.error = "Invalid duration (must be > 0)";
        return result;
    }

    g_jamming = true;
    g_currentType = config.type;

    switch (config.type) {
        case JAMMER_WIFI_CHANNEL:
        case JAMMER_WIFI_BEACON:
        case JAMMER_WIFI_ALL: {
            auto wifiResult = WiFiJammerSuite::jamWiFiNetwork(
                config.channel,
                config.durationMs,
                config.method
            );
            result.success = wifiResult.success;
            result.packetsGenerated = wifiResult.jamPacketsCount;
            result.durationMs = wifiResult.durationMs;
            break;
        }

        case JAMMER_BLE_ADVERTISING: {
            auto bleResult = BLEAdvertisingJammer::jamAdvertising(config.durationMs);
            result.success = bleResult.success;
            result.durationMs = bleResult.durationMs;
            break;
        }

        case JAMMER_RF_NOISE:
        case JAMMER_RF_SWEEP:
        case JAMMER_RF_FOLLOW: {
            String method = "NOISE";
            if (config.type == JAMMER_RF_SWEEP) method = "SWEEP";
            if (config.type == JAMMER_RF_FOLLOW) method = "FOLLOW";

            auto rfResult = AdvancedRFJammer::jamRFSignals(
                String(config.frequency / 1000000.0) + "MHz",
                config.durationMs,
                method
            );
            result.success = rfResult.success;
            result.packetsGenerated = rfResult.jamPacketsCount;
            result.durationMs = rfResult.durationMs;
            result.frequency = config.frequency;
            break;
        }

        case JAMMER_SUBGHZ: {
            auto subghzResult = SubghzJammerSuite::jamSubghzDevices(config.durationMs);
            result.success = subghzResult.success;
            result.packetsGenerated = subghzResult.jamPacketsCount;
            result.durationMs = subghzResult.durationMs;
            result.frequency = 433000000;  // 433MHz default
            break;
        }

        case JAMMER_SIGNAL_WHITE:
        case JAMMER_SIGNAL_PINK:
        case JAMMER_SIGNAL_SWEEP: {
            String noiseType = "WHITE";
            if (config.type == JAMMER_SIGNAL_PINK) noiseType = "PINK";
            if (config.type == JAMMER_SIGNAL_SWEEP) noiseType = "SWEEP";

            auto signalResult = JammingSignalGenerator::generateJammingSignal(
                config.durationMs,
                noiseType
            );
            result.success = signalResult.success;
            result.packetsGenerated = signalResult.signalsGenerated;
            result.durationMs = signalResult.durationMs;
            break;
        }

        default:
            result.error = "Unknown jammer type";
            g_jamming = false;
            return result;
    }

    if (result.success) {
        Serial.println("[JammerSuite] Jamming started (type " + String(config.type) + ")");
    } else if (result.error.length() > 0) {
        Serial.println("[JammerSuite] Error: " + result.error);
        g_jamming = false;
    }

    return result;
}

void stop() {
    WiFiJammerSuite::stop();
    BLEAdvertisingJammer::stop();
    AdvancedRFJammer::stop();
    SubghzJammerSuite::stop();
    JammingSignalGenerator::stop();
    g_jamming = false;
    Serial.println("[JammerSuite] All jamming stopped");
}

bool isActive() {
    return g_jamming;
}

JammerType getCurrentType() {
    return g_currentType;
}

}  // namespace JammerSuite
