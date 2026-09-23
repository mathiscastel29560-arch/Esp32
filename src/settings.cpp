#include "settings.h"
#include "rtc_clock.h"
#include <LittleFS.h>

namespace Settings {

Config g_config;

void initDefaultConfig() {
    g_config.audioEffects = true;
    g_config.achievements = true;
    g_config.chaosMode = true;
    g_config.brightness = 100;
    g_config.contrast = 50;
    g_config.invertColors = false;
    g_config.autoLock = false;
    g_config.lockTimeout = 300;
    g_config.enableLogging = true;
}

const char* SETTINGS_FILE = "/config/settings.json";

void initSettings() {
    initDefaultConfig();
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        return;
    }
    loadSettings();
}

void loadSettings() {
    if (!LittleFS.exists(SETTINGS_FILE)) {
        Serial.println("Settings file not found, using defaults");
        saveSettings();
        return;
    }

    File file = LittleFS.open(SETTINGS_FILE, "r");
    if (!file) {
        Serial.println("Cannot open settings file");
        return;
    }

    uint32_t fileSize = file.size();
    const uint32_t MAX_CONFIG_SIZE = 4096;
    if (fileSize == 0 || fileSize > MAX_CONFIG_SIZE) {
        Serial.println("Settings file size invalid");
        file.close();
        return;
    }

    String content = file.readString();
    file.close();

    // Extract values (simple text parsing)
    if (content.indexOf("\"audioEffects\":1") != -1) {
        g_config.audioEffects = true;
    } else if (content.indexOf("\"audioEffects\":0") != -1) {
        g_config.audioEffects = false;
    }

    if (content.indexOf("\"achievements\":1") != -1) {
        g_config.achievements = true;
    } else if (content.indexOf("\"achievements\":0") != -1) {
        g_config.achievements = false;
    }

    if (content.indexOf("\"chaosMode\":1") != -1) {
        g_config.chaosMode = true;
    } else if (content.indexOf("\"chaosMode\":0") != -1) {
        g_config.chaosMode = false;
    }

    // Load display settings
    int brightnessIdx = content.indexOf("\"brightness\":");
    if (brightnessIdx != -1) {
        int val = content.substring(brightnessIdx + 13, brightnessIdx + 16).toInt();
        g_config.brightness = constrain(val, 0, 100);
    }

    int contrastIdx = content.indexOf("\"contrast\":");
    if (contrastIdx != -1) {
        int val = content.substring(contrastIdx + 11, contrastIdx + 14).toInt();
        g_config.contrast = constrain(val, 0, 100);
    }

    if (content.indexOf("\"invertColors\":1") != -1) {
        g_config.invertColors = true;
    } else if (content.indexOf("\"invertColors\":0") != -1) {
        g_config.invertColors = false;
    }

    // Load system settings
    if (content.indexOf("\"autoLock\":1") != -1) {
        g_config.autoLock = true;
    } else if (content.indexOf("\"autoLock\":0") != -1) {
        g_config.autoLock = false;
    }

    int lockTimeoutIdx = content.indexOf("\"lockTimeout\":");
    if (lockTimeoutIdx != -1) {
        g_config.lockTimeout = content.substring(lockTimeoutIdx + 14, lockTimeoutIdx + 19).toInt();
    }

    if (content.indexOf("\"enableLogging\":1") != -1) {
        g_config.enableLogging = true;
    } else if (content.indexOf("\"enableLogging\":0") != -1) {
        g_config.enableLogging = false;
    }

    Serial.println("Settings loaded from LittleFS");
}

void saveSettings() {
    // Create config directory if needed
    if (!LittleFS.exists("/config")) {
        LittleFS.mkdir("/config");
    }

    File file = LittleFS.open(SETTINGS_FILE, "w");
    if (!file) {
        Serial.println("Cannot create settings file");
        return;
    }

    String json = getSettingsJSON();
    file.print(json);
    file.close();

    Serial.println("Settings saved to LittleFS");
}

void toggleAudioEffects() {
    g_config.audioEffects = !g_config.audioEffects;
    saveSettings();
}

void toggleAchievements() {
    g_config.achievements = !g_config.achievements;
    saveSettings();
}

void toggleChaosMode() {
    g_config.chaosMode = !g_config.chaosMode;
    saveSettings();
}

void setBrightness(uint8_t value) {
    g_config.brightness = constrain(value, 0, 100);
}

void setContrast(uint8_t value) {
    g_config.contrast = constrain(value, 0, 100);
}

void toggleInvertColors() {
    g_config.invertColors = !g_config.invertColors;
}

DateTime getRTCTime() {
    return RtcClock::now();
}

bool setRTCTime(const DateTime& dt) {
    RtcClock::adjust(dt);
    return true;
}

String formatRTCTime(const DateTime& dt) {
    return RtcClock::isoTimestamp();
}

String getSettingsJSON() {
    return String("{\"audioEffects\":") + (g_config.audioEffects ? "1" : "0") +
           ",\"achievements\":" + (g_config.achievements ? "1" : "0") +
           ",\"chaosMode\":" + (g_config.chaosMode ? "1" : "0") +
           ",\"brightness\":" + g_config.brightness +
           ",\"contrast\":" + g_config.contrast +
           ",\"invertColors\":" + (g_config.invertColors ? "1" : "0") +
           ",\"autoLock\":" + (g_config.autoLock ? "1" : "0") +
           ",\"lockTimeout\":" + g_config.lockTimeout +
           ",\"enableLogging\":" + (g_config.enableLogging ? "1" : "0") + "}";
}

} // namespace Settings
