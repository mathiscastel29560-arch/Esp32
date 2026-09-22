#pragma once
#include <Arduino.h>

namespace MenuEnhanced {

// Theme colors
#define THEME_MAIN      "\033[38;5;33m"  // Blue
#define THEME_ACCENT    "\033[38;5;82m"  // Green
#define THEME_WARNING   "\033[38;5;226m" // Yellow
#define THEME_ALERT     "\033[38;5;196m" // Red
#define THEME_INFO      "\033[38;5;51m"  // Cyan
#define RESET           "\033[0m"

// Draw borders with different styles
void drawTopBorder();
void drawBottomBorder();
void drawSeparator();

// Draw dynamic content
void drawSignalStrength(int rssi);      // For WiFi/RF signal
void drawProgressBar(uint8_t percent);  // For operations
void drawHeader(const String &title, const String &subtitle = "");

// Menu animations
void drawMenuAnimation();
void drawLoadingBar(uint8_t duration);

}  // namespace MenuEnhanced
