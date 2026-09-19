#pragma once
#include <Arduino.h>

// Local Wi-Fi captive-portal credential-capture exercise, for testing your
// own devices/users in a controlled lab or CTF setting. The page is
// generic (no real brand/company is imitated). Whatever gets submitted is
// timestamped (RTC) and geotagged (GPS) into a CSV on the device's own
// flash (LittleFS) — nothing is sent off the device.
//
// While active this re-purposes the device's own SoftAP identity (same IP,
// new SSID), so the normal control panel AP is briefly renamed; anything
// connected to it may need to reconnect. Requires the hardware safety
// switch to be armed to start.
namespace EvilPortal {

bool start(const String &fakeSsid, uint32_t maxDurationMs = 600000);
void stop();
void loop(); // call every main-loop iteration
bool active();
String logPath();

} // namespace EvilPortal
