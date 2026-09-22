#include "settings.h"
#include <LittleFS.h>

namespace Settings {

Config g_config = {true, true, true}; // Defaults: all ON

const char* SETTINGS_FILE = "/config/settings.json";

void initSettings() {
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

String getSettingsJSON() {
    return String("{\"audioEffects\":") + (g_config.audioEffects ? "1" : "0") +
           ",\"achievements\":" + (g_config.achievements ? "1" : "0") +
           ",\"chaosMode\":" + (g_config.chaosMode ? "1" : "0") + "}";
}

} // namespace Settings
