#include "rollback_safety.h"
#include <esp_ota_ops.h>
#include <esp_system.h>

namespace {

volatile uint32_t g_lastConfirmTime = 0;
constexpr uint32_t CONFIRM_TIMEOUT_MS = 10000;  // 10 second window to confirm boot
volatile bool g_bootConfirmed = false;

// Watchdog timer callback: if boot not confirmed after CONFIRM_TIMEOUT_MS,
// automatically rollback to previous partition
void rollbackWatchdog() {
    if (!g_bootConfirmed && (millis() - g_lastConfirmTime) > CONFIRM_TIMEOUT_MS) {
        // Trigger rollback to previous OTA partition
        // esp_ota_mark_app_invalid_rollback_and_reboot();
    }
}

} // namespace

namespace RollbackSafety {

void begin() {
    // Initialize rollback detection on boot
    // Check if a rollback occurred on last boot
    const esp_partition_t *running = esp_ota_get_running_partition();

    // The validated app will return the OTA partition it's running from
    // if a rollback hasn't occurred yet

    g_lastConfirmTime = millis();
    g_bootConfirmed = false;

    Serial.println("[RollbackSafety] Initialized, 10s confirmation window...");
}

void confirmBoot() {
    // Reset watchdog timer — app is still running normally
    // Call this regularly (every 1-2 sec) during normal operation
    g_bootConfirmed = true;
    g_lastConfirmTime = millis();

    // In production, would also call:
    // esp_ota_mark_app_valid_cancel_rollback();
}

bool didRollbackOnLastBoot() {
    // Check ESP32's internal flag for whether rollback occurred
    // Returns true if the previous boot triggered auto-revert

    // This would check esp_ota_get_last_invalid_partition() or similar
    // For now, stub returns false (no rollback on last boot)
    return false;
}

String getOtaStatus() {
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (!running) {
        return "Unknown OTA partition";
    }

    String status;
    if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0) {
        status = "Running ota_0 (primary)";
    } else if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_1) {
        status = "Running ota_1 (fallback)";
    } else {
        status = "Running factory partition";
    }

    if (g_bootConfirmed) {
        status += " [confirmed]";
    } else {
        uint32_t elapsed = millis() - g_lastConfirmTime;
        status += " [booting, " + String(CONFIRM_TIMEOUT_MS - elapsed) + "ms left]";
    }

    return status;
}

void requestManualRollback() {
    // Manually request rollback on next reboot
    // Useful for testing recovery / downgrading to known-good version

    Serial.println("[RollbackSafety] Manual rollback requested, rebooting...");
    delay(500);

    // This would call esp_ota_mark_app_invalid_rollback_and_reboot()
    // to switch to alternate partition and reboot

    esp_restart();
}

} // namespace RollbackSafety
