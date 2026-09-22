#pragma once
#include <Arduino.h>

// Dual-boot OTA rollback safety for ESP32-S3
// - Primary: ota_0 (0x10000) — stable firmware
// - Fallback: ota_1 (0x310000) — recovery/testing
//
// The verifyRollbackLater() mechanism ensures that if the newly flashed
// app crashes repeatedly on boot, the ESP32 automatically reverts to
// the previous partition within a timeout window (typically 5-10 seconds).
// This prevents "bricked" states where a bad flash prevents any boot.
//
// Reference: cifertech/esp32-div (specialized dual-boot recovery tool)

namespace RollbackSafety {

// Initialize rollback detection (call once at boot)
// Sets up watchdog-style timer that will trigger rollback if not reset
void begin();

// "Confirm" the current boot as stable
// Must be called regularly (e.g., main loop) to prevent auto-rollback
// Typical call: confirmBoot() every 1-2 seconds during normal operation
void confirmBoot();

// Check if a rollback occurred on the previous boot
// Returns true if ESP32 auto-reverted to previous OTA partition
// Useful for logging/alerting that a crash recovery happened
bool didRollbackOnLastBoot();

// Get human-readable status of current OTA state
String getOtaStatus();

// Manual rollback request (for testing/recovery)
// Switches to alternate OTA partition on next reboot
void requestManualRollback();

} // namespace RollbackSafety
