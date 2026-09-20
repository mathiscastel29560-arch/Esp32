#pragma once
#include "ui/ui.h"

// Simplified renderer for the old, small SSD1306 OLED (see display.h's
// auto-detection): same screens as ui/ui.h's TFT path -- status bar,
// list, detail, confirm -- but plain monochrome text, no sprites, no
// animations. ui/ui.cpp calls into this namespace whenever
// Display::kind() == Display::ScreenKind::OLED; nothing else in the
// firmware (menu.cpp, main.cpp) needs to know this exists.
namespace OledUi {

void showSplash(const String &title, const String &subtitle);
void showHome(const Ui::StatusInfo &status, const String &lastAction);
void showList(const Ui::StatusInfo &status, const String &title,
              const std::vector<Ui::ListItem> &items, int selectedIndex, bool scanning);
void showDetail(const Ui::StatusInfo &status, const String &title,
                 const std::vector<Ui::DetailRow> &rows);

// Draws the confirm prompt only -- Ui::confirm() owns the actual button
// polling loop so it stays shared between backends.
void confirm(const String &title, const String &message);

} // namespace OledUi
