#pragma once
#include <Arduino.h>

// SSD1306 (SPI) status screen. This is a status/telemetry display, not a
// menu — actual control happens through the Wi-Fi web panel (see web_ctrl.h)
// since the hardware has no buttons/encoder for on-device navigation.
namespace Display {

void begin();

// Redraws the whole status screen: clock, GPS fix, AP client count,
// safety-switch state, and a one-line "last action" message.
void update(const String &lastAction);

void splash(const String &line1, const String &line2);

} // namespace Display
