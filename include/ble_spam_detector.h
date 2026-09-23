#pragma once
#include <Arduino.h>

// FEATURES.md Module 3 — defensive. Continuous passive BLE scan looking
// for the signature of an advertising-flood ("BLE spam") tool nearby: a
// burst of pairing-style beacons (Apple Continuity / Google Fast Pair /
// Microsoft Swift Pair) coming from an unusually high number of distinct
// random MAC addresses in a short window (see BLE_SPAM_* in config.h).
// Never transmits anything — this only tells you someone else is doing it.
namespace BleSpamDetector {

struct Alert {
    String type;          // e.g. "Apple Continuity" — empty if no alert
    int distinctMacs = 0;
    int strongestRssi = -127;
};

void begin();
void start(); // begins continuous background scanning
void stop();
bool active();

// Call periodically (menu/web poll). Reports whatever is currently over
// threshold within the trailing window, empty type if nothing is.
Alert checkAlert();

} // namespace BleSpamDetector
