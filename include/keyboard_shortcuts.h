#pragma once
#include <Arduino.h>

namespace KeyboardShortcuts {

// Shortcut action types
enum ShortcutAction {
    ACTION_NONE,
    ACTION_HOME,           // Go to home screen
    ACTION_MENU,           // Toggle main menu
    ACTION_BACK,           // Go back one screen
    ACTION_NEXT,           // Next item
    ACTION_SELECT,         // Select current item
    ACTION_EXPORT_LOGS,    // Export audit logs
    ACTION_EXPORT_RESULTS, // Export scan results
    ACTION_CLEAR_LOGS,     // Clear all logs
    ACTION_SYSTEM_INFO,    // Show system info
    ACTION_POWER_OFF,      // Shutdown
    ACTION_RESET,          // Soft reset
    ACTION_CALIBRATION,    // Start calibration
    ACTION_TEST_MODE,      // Enter test mode
    ACTION_EMERGENCY_STOP, // Stop current operation
};

class ShortcutManager {
public:
    // Initialize shortcut system
    static void begin();

    // Check for shortcuts
    static ShortcutAction checkShortcuts();

    // Individual key methods
    static bool isShortcut_Home();       // Double-press BACK
    static bool isShortcut_Export();     // BACK + SELECT
    static bool isShortcut_ClearLogs();  // BACK + NEXT + SELECT (long)
    static bool isShortcut_SystemInfo(); // NEXT (long)
    static bool isShortcut_TestMode();   // SELECT (long)
    static bool isShortcut_Emergency();  // All buttons together

    // Utilities
    static void printShortcutHelp();
    static bool isValidShortcut();
};

}  // namespace KeyboardShortcuts
