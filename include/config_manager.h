#pragma once
#include <Arduino.h>

namespace ConfigManager {

struct NetworkConfig {
    char ssid[64];
    char password[64];
    bool autoConnect;
    uint16_t scanInterval;
};

struct SecurityConfig {
    bool encryptionEnabled;
    uint32_t encryptionKey[8];
    bool rateLimitingEnabled;
    uint32_t maxAttemptsPerMinute;
};

struct DeviceConfig {
    char deviceName[64];
    char deviceId[32];
    uint8_t brightness;
    uint8_t contrast;
    bool debugLogsEnabled;
    uint32_t logLevel;
};

struct ToolConfig {
    bool enabled;
    uint32_t timeout;
    uint32_t maxIterations;
    char lastUsed[32];
};

struct SystemConfig {
    NetworkConfig network;
    SecurityConfig security;
    DeviceConfig device;
    ToolConfig tools;
    uint32_t configVersion;
};

bool loadConfig(const char* filename);
bool saveConfig(const char* filename);
bool resetConfig();
bool validateConfig();

SystemConfig* getConfig();
void updateNetworkConfig(const NetworkConfig& config);
void updateSecurityConfig(const SecurityConfig& config);
void updateDeviceConfig(const DeviceConfig& config);
void updateToolConfig(const ToolConfig& config);

bool exportConfigToCSV(const char* filename);
bool importConfigFromCSV(const char* filename);
void displayConfigStatus();

} // namespace ConfigManager
