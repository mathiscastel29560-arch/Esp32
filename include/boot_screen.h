#pragma once
#include <Arduino.h>

namespace BootScreen {

// Display animated boot screen for 6 seconds
// Shows title, version, progress bars with animations
void show(const String &title = "ESP32-S3 AUDIT TOOL", const String &subtitle = "");

}  // namespace BootScreen
