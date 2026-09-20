#pragma once
#include <Arduino.h>

// HTTP control panel served on the device's own SoftAP (see WifiTools::begin
// for SSID/password). Connect a phone/laptop to that AP and browse to
// http://192.168.4.1:8080/ to drive every module. Port 8080 (not 80) is
// used deliberately so it never collides with EvilPortal's captive-portal
// page, which needs the standard port 80.
namespace WebCtrl {

void begin();
void loop(); // call every iteration of the main loop
String lastAction(); // for the OLED status line

} // namespace WebCtrl
