#pragma once
#include <Arduino.h>
#include <vector>

class TFT_eSPI;

// SSD1306 (SPI) display. Shows either the idle status screen or, when the
// 4-button menu (see menu.h) is active, a scrollable list / result screen.
//
// TRANSITIONAL: the new ui/ layer (see ui/ui.h) is taking over screen
// rendering one screen at a time, starting with the splash and main menu.
// Until every state here is migrated, both this module and ui/ share the
// single physical TFT_eSPI instance via raw() rather than each creating
// their own — creating two instances bound to the same pins would double
// up init() calls and DMA/transaction bookkeeping for no reason.
namespace Display {

void begin();

// The underlying TFT_eSPI object, for ui/ui.cpp to build its sprite on top
// of instead of creating a second instance.
TFT_eSPI &raw();

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
