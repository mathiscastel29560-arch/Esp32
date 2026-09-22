#include "keyboard_shortcuts.h"
#include "buttons.h"
#include "debug_logger.h"

namespace KeyboardShortcuts {

// Shortcut detection state
static uint32_t g_lastBackPress = 0;
static uint32_t g_lastSelectPress = 0;
static uint32_t g_lastNextPress = 0;
static uint8_t g_backPressCount = 0;
static bool g_shortcutDetected = false;

void ShortcutManager::begin() {
    DBG_INFO("KeyboardShortcuts", "Shortcut system initialized");
}

ShortcutAction ShortcutManager::checkShortcuts() {
    // Get current button states
    bool backPressed = !digitalRead(BTN_BACK);
    bool selectPressed = !digitalRead(BTN_SELECT);
    bool nextPressed = !digitalRead(BTN_NEXT);

    uint32_t now = millis();

    // Emergency stop: All buttons pressed simultaneously
    if (backPressed && selectPressed && nextPressed) {
        delay(100);  // Debounce
        if (!digitalRead(BTN_BACK) && !digitalRead(BTN_SELECT) && !digitalRead(BTN_NEXT)) {
            DBG_WARN("KeyboardShortcuts", "Emergency stop activated");
            return ACTION_EMERGENCY_STOP;
        }
    }

    // Double-press BACK for home
    if (backPressed && now - g_lastBackPress < 500) {
        g_backPressCount++;
        if (g_backPressCount >= 2) {
            DBG_INFO("KeyboardShortcuts", "Shortcut: Go Home");
            g_backPressCount = 0;
            return ACTION_HOME;
        }
    } else if (backPressed) {
        g_lastBackPress = now;
        g_backPressCount = 1;
    }

    // BACK + SELECT for export
    if (backPressed && selectPressed) {
        delay(100);
        if (!digitalRead(BTN_BACK) && !digitalRead(BTN_SELECT)) {
            uint32_t pressTime = 0;
            uint32_t pressStart = millis();

            while (!digitalRead(BTN_BACK) && !digitalRead(BTN_SELECT)) {
                pressTime = millis() - pressStart;
                if (pressTime > 2000) {
                    DBG_INFO("KeyboardShortcuts", "Shortcut: Export Results (long)");
                    return ACTION_EXPORT_RESULTS;
                }
                delay(50);
            }

            if (pressTime > 500) {
                DBG_INFO("KeyboardShortcuts", "Shortcut: Export Logs");
                return ACTION_EXPORT_LOGS;
            }
        }
    }

    // NEXT (long) for system info
    if (nextPressed) {
        uint32_t pressStart = millis();
        while (!digitalRead(BTN_NEXT)) {
            if (millis() - pressStart > 2000) {
                DBG_INFO("KeyboardShortcuts", "Shortcut: System Info");
                return ACTION_SYSTEM_INFO;
            }
            delay(50);
        }
    }

    // SELECT (long) for test mode
    if (selectPressed) {
        uint32_t pressStart = millis();
        while (!digitalRead(BTN_SELECT)) {
            if (millis() - pressStart > 3000) {
                DBG_INFO("KeyboardShortcuts", "Shortcut: Test Mode");
                return ACTION_TEST_MODE;
            }
            delay(50);
        }
    }

    // BACK + NEXT + SELECT (long) for clear logs
    if (backPressed && nextPressed && selectPressed) {
        delay(100);
        if (!digitalRead(BTN_BACK) && !digitalRead(BTN_NEXT) && !digitalRead(BTN_SELECT)) {
            uint32_t pressStart = millis();

            while (!digitalRead(BTN_BACK) && !digitalRead(BTN_NEXT) && !digitalRead(BTN_SELECT)) {
                if (millis() - pressStart > 3000) {
                    DBG_WARN("KeyboardShortcuts", "Shortcut: Clear Logs");
                    return ACTION_CLEAR_LOGS;
                }
                delay(50);
            }
        }
    }

    return ACTION_NONE;
}

bool ShortcutManager::isShortcut_Home() {
    return checkShortcuts() == ACTION_HOME;
}

bool ShortcutManager::isShortcut_Export() {
    return checkShortcuts() == ACTION_EXPORT_LOGS;
}

bool ShortcutManager::isShortcut_ClearLogs() {
    return checkShortcuts() == ACTION_CLEAR_LOGS;
}

bool ShortcutManager::isShortcut_SystemInfo() {
    return checkShortcuts() == ACTION_SYSTEM_INFO;
}

bool ShortcutManager::isShortcut_TestMode() {
    return checkShortcuts() == ACTION_TEST_MODE;
}

bool ShortcutManager::isShortcut_Emergency() {
    return checkShortcuts() == ACTION_EMERGENCY_STOP;
}

void ShortcutManager::printShortcutHelp() {
    Serial.println();
    Serial.print(COLOR_BLUE);
    Serial.println("╔═══════════════════════════════════════╗");
    Serial.print("║ ⌨ Keyboard Shortcuts");
    Serial.println(String(16, ' ') + "║");
    Serial.println("╠═══════════════════════════════════════╣");

    Serial.print(COLOR_GREEN);
    Serial.println("║ Double-press BACK           → Home");
    Serial.print(COLOR_CYAN);
    Serial.println("║ BACK + SELECT (1s)          → Export Logs");
    Serial.print(COLOR_YELLOW);
    Serial.println("║ BACK + SELECT (2s)          → Export Results");
    Serial.print(COLOR_BLUE);
    Serial.println("║ NEXT (long, 2s)             → System Info");
    Serial.print(COLOR_MAGENTA);
    Serial.println("║ SELECT (long, 3s)           → Test Mode");
    Serial.print(COLOR_RED);
    Serial.println("║ BACK + NEXT + SELECT (3s)   → Clear Logs");
    Serial.println("║ All 3 buttons               → Emergency Stop");

    Serial.print(COLOR_BLUE);
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

bool ShortcutManager::isValidShortcut() {
    return checkShortcuts() != ACTION_NONE;
}

}  // namespace KeyboardShortcuts
