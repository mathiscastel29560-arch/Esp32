#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <cstring>

class ConfigManager {
public:
    struct ToolConfig {
        String toolName;
        uint32_t timeout_ms = 60000;
        uint8_t channel = 6;
        uint32_t frequency = 433920000;
        String target_bssid = "";
        String target_ssid = "";
        bool aggressive_mode = false;
        uint16_t retry_count = 3;
    };

    static ConfigManager& instance() {
        static ConfigManager cm;
        return cm;
    }

    // Initialize filesystem
    bool begin() {
        if (!LittleFS.begin()) {
            Serial.println("[ConfigManager] Failed to mount LittleFS");
            return false;
        }

        if (!LittleFS.exists("/config")) {
            LittleFS.mkdir("/config");
        }

        Serial.println("[ConfigManager] Initialized");
        return true;
    }

    // Save tool configuration
    bool saveToolConfig(const String& toolName, const ToolConfig& config) {
        String filepath = "/config/" + toolName + ".json";

        DynamicJsonDocument doc(512);
        doc["tool"] = toolName;
        doc["timeout_ms"] = config.timeout_ms;
        doc["channel"] = config.channel;
        doc["frequency"] = config.frequency;
        doc["target_bssid"] = config.target_bssid;
        doc["target_ssid"] = config.target_ssid;
        doc["aggressive"] = config.aggressive_mode;
        doc["retry_count"] = config.retry_count;

        File file = LittleFS.open(filepath, "w");
        if (!file) {
            Serial.printf("[ConfigManager] Failed to open %s for writing\n", filepath.c_str());
            return false;
        }

        serializeJson(doc, file);
        file.close();

        Serial.printf("[ConfigManager] Saved config: %s\n", toolName.c_str());
        return true;
    }

    // Load tool configuration
    bool loadToolConfig(const String& toolName, ToolConfig& config) {
        String filepath = "/config/" + toolName + ".json";

        if (!LittleFS.exists(filepath)) {
            Serial.printf("[ConfigManager] Config not found: %s\n", toolName.c_str());
            return false;
        }

        File file = LittleFS.open(filepath, "r");
        if (!file) {
            Serial.printf("[ConfigManager] Failed to open %s for reading\n", filepath.c_str());
            return false;
        }

        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            Serial.printf("[ConfigManager] JSON parse error: %s\n", error.c_str());
            return false;
        }

        config.toolName = toolName;
        config.timeout_ms = doc["timeout_ms"] | 60000;
        config.channel = doc["channel"] | 6;
        config.frequency = doc["frequency"] | 433920000;
        config.target_bssid = doc["target_bssid"].as<String>();
        config.target_ssid = doc["target_ssid"].as<String>();
        config.aggressive_mode = doc["aggressive"] | false;
        config.retry_count = doc["retry_count"] | 3;

        Serial.printf("[ConfigManager] Loaded config: %s\n", toolName.c_str());
        return true;
    }

    // List all saved configurations
    void listConfigs() {
        File root = LittleFS.open("/config");
        if (!root) {
            Serial.println("[ConfigManager] Config directory not found");
            return;
        }

        Serial.println("[ConfigManager] Saved configurations:");
        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                Serial.printf("  - %s (%d bytes)\n", file.name(), file.size());
            }
            file = root.openNextFile();
        }
    }

    // Delete configuration
    bool deleteConfig(const String& toolName) {
        String filepath = "/config/" + toolName + ".json";

        if (!LittleFS.exists(filepath)) {
            Serial.printf("[ConfigManager] Config not found: %s\n", toolName.c_str());
            return false;
        }

        if (LittleFS.remove(filepath)) {
            Serial.printf("[ConfigManager] Deleted: %s\n", toolName.c_str());
            return true;
        }

        Serial.printf("[ConfigManager] Failed to delete: %s\n", toolName.c_str());
        return false;
    }

    // Clear all configurations
    bool clearAll() {
        File root = LittleFS.open("/config");
        if (!root) {
            Serial.println("[ConfigManager] Config directory not found");
            return false;
        }

        uint32_t count = 0;
        File file = root.openNextFile();
        while (file) {
            String name = file.name();
            file = root.openNextFile();
            String filepath = "/config/" + name;
            if (LittleFS.remove(filepath)) {
                count++;
            }
        }

        Serial.printf("[ConfigManager] Cleared %u configurations\n", count);
        return true;
    }

    // Get filesystem space info
    void printStats() {
        uint32_t total = LittleFS.totalBytes();
        uint32_t used = LittleFS.usedBytes();
        uint32_t free = total - used;

        Serial.printf("[ConfigManager] LittleFS: %u / %u bytes used (%.1f%%)\n",
                     used, total, (used * 100.0) / total);
        Serial.printf("                Available: %u bytes\n", free);
    }

private:
    ConfigManager() {}
};

#endif
