#pragma once

// On-device menu driven by the 4 buttons (see buttons.h) and drawn on the
// OLED (see display.h). This is a second, independent way to drive the
// device besides the Wi-Fi web panel — pick whichever is convenient.
namespace Menu {

void begin();
void loop(); // call every main-loop iteration; draws to the OLED itself
             // (main.cpp should skip Display::update() while Menu is active)
bool isActive(); // false while idle at the home status screen

} // namespace Menu
