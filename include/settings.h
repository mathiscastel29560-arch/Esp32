#pragma once
#include <Arduino.h>
#include <RTClib.h>

namespace Settings {

struct Config {
    bool audioEffects;
    bool achievements;
    bool chaosMode;

    // Display settings
    uint8_t brightness = 100;      // 0-100%
    uint8_t contrast = 50;         // 0-100%
    bool invertColors = false;

    // System settings
    bool autoLock = false;         // Auto-lock TX after timeout
    uint16_t lockTimeout = 300;    // seconds
    bool enableLogging = true;
};

extern Config g_config;

void initSettings();
void loadSettings();
void saveSettings();

void toggleAudioEffects();
void toggleAchievements();
void toggleChaosMode();

// Display settings
void setBrightness(uint8_t value);
void setContrast(uint8_t value);
void toggleInvertColors();

// RTC/Clock functions
DateTime getRTCTime();
bool setRTCTime(const DateTime& dt);
String formatRTCTime(const DateTime& dt);

String getSettingsJSON();

} // namespace Settings
