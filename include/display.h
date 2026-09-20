#pragma once
#include <Arduino.h>

class TFT_eSPI;
class Adafruit_SSD1306;

// Owns the physical screen: auto-detects at boot which one is actually
// wired (the current TFT ILI9341, the old SSD1306 OLED, or neither) and
// draws a transient plain-text "booting..." placeholder, before
// ui/ui.h's screens (see ui/ui.h) take over everything else. ui/ui.cpp
// checks kind() and routes each screen to the matching renderer:
// TFT_eSPI sprites for ScreenKind::TFT, ui/oled_ui.h for
// ScreenKind::OLED, serial-only logging for ScreenKind::NONE.
namespace Display {

enum class ScreenKind { NONE, TFT, OLED };

void begin();

// Which physical screen was actually found at boot. Stable after
// begin() returns -- never changes at runtime (no hot-swap support).
ScreenKind kind();

// The underlying TFT_eSPI object, for ui/ui.cpp to build its sprite on
// top of instead of creating a second instance bound to the same pins.
// Only meaningful when kind() == ScreenKind::TFT.
TFT_eSPI &raw();

// The underlying Adafruit_SSD1306 object, for ui/oled_ui.cpp to draw
// into directly (no sprite/double-buffer needed -- the library already
// keeps its own off-screen bitmap and only pushes it on display()).
// Only meaningful when kind() == ScreenKind::OLED.
Adafruit_SSD1306 &oled();

} // namespace Display
