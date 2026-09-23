#pragma once
#include <Arduino.h>

namespace Settings {

struct Config {
    bool audioEffects;
    bool achievements;
    bool chaosMode;
};

extern Config g_config;

void initSettings();
void loadSettings();
void saveSettings();

void toggleAudioEffects();
void toggleAchievements();
void toggleChaosMode();

String getSettingsJSON();

} // namespace Settings
