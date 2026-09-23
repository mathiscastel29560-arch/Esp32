#pragma once
#include <RTClib.h>

// Thin wrapper around the DS3231 so the rest of the firmware never touches
// Wire/RTClib directly. All timestamps used for logs (wardriving, sub-GHz
// captures, evil-portal submissions) go through this so an audit report can
// trust a single, real-time-clock-backed time source instead of millis().
namespace RtcClock {

bool begin();                    // returns false if the DS3231 isn't found on the bus
bool isRunning();
DateTime now();

// yyyy-mm-dd hh:mm:ss, used for on-screen display and log rows
String isoTimestamp();
// compact form safe for filenames: yyyymmdd_hhmmss
String fileTimestamp();

// Set the clock (e.g. from the web UI, or once from GPS time when a fix is
// acquired) so the DS3231 keeps correct time even without GPS/network.
void adjust(const DateTime &dt);

} // namespace RtcClock
