#include <Arduino.h>
#include <vector>
#include "config.h"

// Minimal menu implementation - full UI logic can be added incrementally

namespace Menu {

enum MenuState {
    MAIN_MENU,
    OFFENSIVE_TOOLS_MENU,
    SETTINGS_MENU
};

MenuState g_state = MAIN_MENU;
int g_toolSel = 0;
int g_settingsSel = 0;
bool g_chaosActive = false;

void begin() {
    Serial.println("Menu system initialized");
}

void menuLoop() {
    // Main menu loop - placeholder for full implementation
    Serial.println("Menu loop running");
}

void runOffensiveTool(int toolIdx) {
    Serial.println("Running tool: " + String(toolIdx));
    // Tool implementations here
}

void runChaosMode() {
    Serial.println("CHAOS MODE ACTIVATED");
    // Chaos mode implementation
}

void runSettingsMenu(int option) {
    Serial.println("Settings option: " + String(option));
    // Settings implementation
}

} // namespace Menu
