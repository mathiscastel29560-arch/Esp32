#include "config_manager.h"
#include <LittleFS.h>
#include <cstring>

namespace ConfigManager {

static SystemConfig systemConfig;

bool loadConfig(const char* filename) {
    if (!LittleFS.begin()) return false;
    
    File file = LittleFS.open(filename, "r");
    if (!file) {
        LittleFS.end();
        return false;
    }
    
    size_t readSize = file.read((uint8_t*)&systemConfig, sizeof(SystemConfig));
    file.close();
    LittleFS.end();
    
    return (readSize == sizeof(SystemConfig)) && validateConfig();
}

bool saveConfig(const char* filename) {
    if (!LittleFS.begin()) return false;
    
    LittleFS.mkdir("/config");
    
    fs::File file = LittleFS.open(filename, "w");
    if (!file) {
        LittleFS.end();
        return false;
    }
    
    size_t written = file.write((uint8_t*)&systemConfig, sizeof(SystemConfig));
    file.close();
    LittleFS.end();
    
    return (written == sizeof(SystemConfig));
}

bool resetConfig() {
    memset(&systemConfig, 0, sizeof(SystemConfig));
    
    strncpy(systemConfig.device.deviceName, "ESP32-Audit", sizeof(systemConfig.device.deviceName) - 1);
    strncpy(systemConfig.device.deviceId, "ESP32000001", sizeof(systemConfig.device.deviceId) - 1);
    
    systemConfig.device.brightness = 128;
    systemConfig.device.contrast = 128;
    systemConfig.device.debugLogsEnabled = true;
    systemConfig.device.logLevel = 3;
    
    systemConfig.network.autoConnect = false;
    systemConfig.network.scanInterval = 30;
    
    systemConfig.security.encryptionEnabled = false;
    systemConfig.security.rateLimitingEnabled = false;
    systemConfig.security.maxAttemptsPerMinute = 5;
    
    systemConfig.tools.enabled = true;
    systemConfig.tools.timeout = 5000;
    systemConfig.tools.maxIterations = 1000;
    
    systemConfig.configVersion = 1;
    
    return saveConfig("/config/system.cfg");
}

bool validateConfig() {
    if (systemConfig.configVersion != 1) {
        Serial.println("⚠️ Config version mismatch");
        return false;
    }
    
    if (systemConfig.device.brightness > 255 || systemConfig.device.contrast > 255) {
        Serial.println("⚠️ Invalid display settings");
        return false;
    }
    
    if (systemConfig.security.maxAttemptsPerMinute == 0) {
        Serial.println("⚠️ Invalid rate limit");
        return false;
    }
    
    return true;
}

SystemConfig* getConfig() {
    return &systemConfig;
}

void updateNetworkConfig(const NetworkConfig& config) {
    systemConfig.network = config;
    saveConfig("/config/system.cfg");
}

void updateSecurityConfig(const SecurityConfig& config) {
    systemConfig.security = config;
    saveConfig("/config/system.cfg");
}

void updateDeviceConfig(const DeviceConfig& config) {
    systemConfig.device = config;
    saveConfig("/config/system.cfg");
}

void updateToolConfig(const ToolConfig& config) {
    systemConfig.tools = config;
    saveConfig("/config/system.cfg");
}

bool exportConfigToCSV(const char* filename) {
    if (!LittleFS.begin()) return false;
    
    LittleFS.mkdir("/logs/config");
    
    fs::File file = LittleFS.open(filename, "w");
    if (!file) {
        LittleFS.end();
        return false;
    }
    
    file.println("Parameter,Value");
    file.printf("Device Name,%s\n", systemConfig.device.deviceName);
    file.printf("Device ID,%s\n", systemConfig.device.deviceId);
    file.printf("WiFi SSID,%s\n", systemConfig.network.ssid);
    file.printf("Auto Connect,%s\n", systemConfig.network.autoConnect ? "true" : "false");
    file.printf("Encryption Enabled,%s\n", systemConfig.security.encryptionEnabled ? "true" : "false");
    file.printf("Rate Limiting,%s\n", systemConfig.security.rateLimitingEnabled ? "true" : "false");
    file.printf("Max Attempts/Min,%u\n", systemConfig.security.maxAttemptsPerMinute);
    file.printf("Display Brightness,%u\n", systemConfig.device.brightness);
    file.printf("Display Contrast,%u\n", systemConfig.device.contrast);
    file.printf("Debug Logs,%s\n", systemConfig.device.debugLogsEnabled ? "true" : "false");
    file.printf("Tool Timeout,%u ms\n", systemConfig.tools.timeout);
    file.printf("Config Version,%u\n", systemConfig.configVersion);
    
    file.close();
    LittleFS.end();
    
    return true;
}

bool importConfigFromCSV(const char* filename) {
    if (!LittleFS.begin()) return false;
    
    File file = LittleFS.open(filename, "r");
    if (!file) {
        LittleFS.end();
        return false;
    }
    
    char line[256];
    while (file.available()) {
        size_t len = file.readBytesUntil('\n', line, sizeof(line) - 1);
        if (len > 0) {
            line[len] = '\0';
            
            char* delimiter = strchr(line, ',');
            if (delimiter) {
                *delimiter = '\0';
                char* key = line;
                char* value = delimiter + 1;
                
                if (strcmp(key, "Device Name") == 0) {
                    strncpy(systemConfig.device.deviceName, value, sizeof(systemConfig.device.deviceName) - 1);
                } else if (strcmp(key, "Display Brightness") == 0) {
                    systemConfig.device.brightness = (uint8_t)atoi(value);
                }
            }
        }
    }
    
    file.close();
    LittleFS.end();
    
    return true;
}

void displayConfigStatus() {
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║              SYSTEM CONFIGURATION STATUS                  ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");
    
    Serial.printf("║ Device Name: %s\n", systemConfig.device.deviceName);
    Serial.printf("║ Device ID: %s\n", systemConfig.device.deviceId);
    
    Serial.println("║");
    Serial.println("║ NETWORK:");
    Serial.printf("║   WiFi SSID: %s\n", systemConfig.network.ssid[0] ? systemConfig.network.ssid : "(not configured)");
    Serial.printf("║   Auto-Connect: %s\n", systemConfig.network.autoConnect ? "Enabled" : "Disabled");
    Serial.printf("║   Scan Interval: %u seconds\n", systemConfig.network.scanInterval);
    
    Serial.println("║");
    Serial.println("║ SECURITY:");
    Serial.printf("║   Encryption: %s\n", systemConfig.security.encryptionEnabled ? "Enabled" : "Disabled");
    Serial.printf("║   Rate Limiting: %s\n", systemConfig.security.rateLimitingEnabled ? "Enabled" : "Disabled");
    Serial.printf("║   Max Attempts/Min: %u\n", systemConfig.security.maxAttemptsPerMinute);
    
    Serial.println("║");
    Serial.println("║ DISPLAY:");
    Serial.printf("║   Brightness: %u/255\n", systemConfig.device.brightness);
    Serial.printf("║   Contrast: %u/255\n", systemConfig.device.contrast);
    
    Serial.println("║");
    Serial.println("║ TOOLS:");
    Serial.printf("║   Timeout: %u ms\n", systemConfig.tools.timeout);
    Serial.printf("║   Max Iterations: %u\n", systemConfig.tools.maxIterations);
    Serial.printf("║   Debug Logs: %s\n", systemConfig.device.debugLogsEnabled ? "Enabled" : "Disabled");
    
    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

} // namespace ConfigManager
