#pragma once
#include <Arduino.h>

// ============================================================
// DEBUG LOGGING SYSTEM
// Enable/disable debug output globally
// ============================================================

#define DEBUG_ENABLED 1  // Set to 0 to disable all debug output

#if DEBUG_ENABLED

#define DEBUG_LEVEL_ERROR   0
#define DEBUG_LEVEL_WARN    1
#define DEBUG_LEVEL_INFO    2
#define DEBUG_LEVEL_VERBOSE 3

#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_VERBOSE

// Colored terminal output
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_RESET   "\033[0m"

// Debug macros with levels
#define DBG_ERROR(fmt, ...) do { \
    if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_ERROR) { \
        Serial.printf(COLOR_RED "[ERROR] " fmt COLOR_RESET "\n", ##__VA_ARGS__); \
    } \
} while(0)

#define DBG_WARN(fmt, ...) do { \
    if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_WARN) { \
        Serial.printf(COLOR_YELLOW "[WARN] " fmt COLOR_RESET "\n", ##__VA_ARGS__); \
    } \
} while(0)

#define DBG_INFO(fmt, ...) do { \
    if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_INFO) { \
        Serial.printf(COLOR_GREEN "[INFO] " fmt COLOR_RESET "\n", ##__VA_ARGS__); \
    } \
} while(0)

#define DBG_VERBOSE(fmt, ...) do { \
    if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE) { \
        Serial.printf(COLOR_CYAN "[VERBOSE] " fmt COLOR_RESET "\n", ##__VA_ARGS__); \
    } \
} while(0)

#else
// No-op versions when debug disabled
#define DBG_ERROR(fmt, ...) do {} while(0)
#define DBG_WARN(fmt, ...) do {} while(0)
#define DBG_INFO(fmt, ...) do {} while(0)
#define DBG_VERBOSE(fmt, ...) do {} while(0)
#endif

// Pin/value diagnostics
#define DBG_PIN(pin, label) do { \
    DBG_VERBOSE("GPIO%d (%s) = %d", pin, label, digitalRead(pin)); \
} while(0)

#define DBG_ADC(pin, label) do { \
    DBG_VERBOSE("ADC%d (%s) = %d", pin, label, analogRead(pin)); \
} while(0)

#define DBG_MEMORY(label) do { \
    DBG_INFO("%s - Free heap: %u bytes, PSRAM: %u bytes", \
        label, ESP.getFreeHeap(), ESP.getFreePsram()); \
} while(0)
