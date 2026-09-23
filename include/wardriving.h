#pragma once
#include <Arduino.h>

// Geotagged, timestamped recon logging: every Wi-Fi AP and BLE device seen
// gets a row in WARDRIVE_LOG_FILE with the GPS fix (or NO_FIX) and the
// DS3231 timestamp at the moment it was seen, so results can be plotted on
// a map / correlated with a site-visit timeline afterwards.
namespace Wardriving {

void begin();

// Runs one Wi-Fi scan + one short BLE scan and appends every result as a
// row. Returns how many rows were written.
size_t captureSnapshot(uint32_t bleScanSeconds = 3);

size_t rowCount();

} // namespace Wardriving
