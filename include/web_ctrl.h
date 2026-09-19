#pragma once
#include <Arduino.h>

// HTTP control panel served on the device's own SoftAP (see WifiTools::begin
// for SSID/password). Connect a phone/laptop to that AP and browse to
// http://192.168.4.1/ to drive every module — there is no on-device menu
// since the hardware has no buttons, only the OLED status screen.
namespace WebCtrl {

void begin();
void loop(); // call every iteration of the main loop
String lastAction(); // for the OLED status line

} // namespace WebCtrl
