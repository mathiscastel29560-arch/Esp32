#pragma once
#include <Arduino.h>
#include <vector>

// Presentation layer: screens here draw into an off-screen sprite
// (allocated in PSRAM by TFT_eSPI automatically) and push the finished
// frame to the TFT in one go, so nothing flickers mid-draw. Screens take
// data handed to them by the caller (menu.cpp, main.cpp) — this layer
// never polls WifiTools/BleTools/RtcClock/etc. itself, so the UI stays a
// pure presentation layer over the existing module logic. Widgets used by
// these screens live in ui/widgets.h.
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

// Idle status screen (shown whenever the 4-button menu isn't active):
// status bar, a big clock, and the last action taken — with the same
// shark swim-by cameo the old display.cpp had. Call about once a second.
void showHome(const StatusInfo &status, const String &lastAction);

// One row in a list screen. `label` alone (the rest defaulted) behaves
// like a plain text menu item — WifiTools/BleTools results additionally
// set hasRssi/badge for the signal bar and manufacturer/type chip.
struct ListItem {
    String label;
    bool hasRssi = false;
    int rssi = 0;
    String badge;

    ListItem() = default;
    ListItem(const String &l) : label(l) {}
};

// Status bar + a scrollable, animated list. `title` doubles as the
// screen's identity: calling this again with the same title just updates
// the live list (selection-slide animation only); a different title
// triggers the screen-to-screen slide transition. Call every frame the
// screen is showing.
void showList(const StatusInfo &status, const String &title,
              const std::vector<ListItem> &items, int selectedIndex, bool scanning = false);

// Convenience overload for plain-text menus (main menu, action submenus).
void showList(const StatusInfo &status, const String &title,
              const std::vector<String> &items, int selectedIndex);

struct DetailRow {
    String label;
    String value;
};
struct Badge {
    String text;
    uint16_t color;
};

void showDetail(const StatusInfo &status, const String &title, const std::vector<DetailRow> &rows,
                 const std::vector<Badge> &badges = {});

// Renders free-form text (e.g. an existing showResult() body) as detail
// rows: each line is split on its first ':' into label/value, or shown as
// a bare value line if there's no ':'. Bridges the old text-block results
// (menu.cpp's showResult()) onto the new detail screen without every call
// site needing to build a DetailRow vector by hand.
void showTextBlock(const StatusInfo &status, const String &title, const String &body);

// Blocking yes/no confirmation. OK = true, BACK = false. Available for any
// screen that wants an extra visual confirmation step on top of the
// hardware TX-arm interlock (see tx_arm.h) — not wired into any action
// automatically.
bool confirm(const String &title, const String &message);

} // namespace Ui
