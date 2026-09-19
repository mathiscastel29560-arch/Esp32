#pragma once
#include <Arduino.h>
#include <vector>

// SSD1306 (SPI) display. Shows either the idle status screen or, when the
// 4-button menu (see menu.h) is active, a scrollable list / result screen.
namespace Display {

void begin();

// Redraws the whole status screen: clock, GPS fix, AP client count,
// safety-switch state, and a one-line "last action" message.
void update(const String &lastAction);

void splash(const String &line1, const String &line2);

// Scrollable menu list, keeps `selectedIndex` visible. `>` marks the
// selected row.
void showList(const String &title, const std::vector<String> &items, int selectedIndex);

// Free-form result/status text (word-wrapped by the caller if needed).
void showText(const String &title, const String &body);

} // namespace Display
