#pragma once
#include <Arduino.h>
#include <vector>

// Presentation layer: screens here draw into an off-screen sprite
// (allocated in PSRAM by TFT_eSPI automatically) and push the finished
// frame to the TFT in one go, so nothing flickers mid-draw. Screens take
// data handed to them by the caller (menu.cpp, main.cpp) — this layer
// never polls WifiTools/BleTools/RtcClock/etc. itself, so the UI stays a
// pure presentation layer over the existing module logic.
//
// Status: step 1 of the redesign — theme + splash + main menu only. Other
// screens (Wi-Fi/BLE results, result text, etc.) still render through the
// old display.h calls until this look is validated on real hardware.
namespace Ui {

void begin();

struct StatusInfo {
    String time;
    bool gpsFix = false;
    uint8_t battPercent = 0;
    bool radioActive = false; // any TX-capable feature currently running
};

// Blocking (~Theme::SPLASH_DURATION_MS) animated boot splash.
void showSplash(const String &title, const String &subtitle);

// Status bar + a scrollable menu list with an animated sliding highlight
// behind the selected row. Call every frame the main menu is showing; it
// tracks the selection-change animation itself between calls.
void showMainMenu(const StatusInfo &status, const String &title,
                   const std::vector<String> &items, int selectedIndex);

} // namespace Ui
