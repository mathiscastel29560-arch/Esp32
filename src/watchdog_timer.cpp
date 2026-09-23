#include "watchdog_timer.h"
#include "audit_log.h"
#include <esp_system.h>

bool WatchdogTimer::begin(uint32_t timeout_seconds) {
    timeout_sec_ = timeout_seconds;

    // Check if we rebooted due to watchdog in previous boot
    checkRebootReason();

    // Subscribe current task to watchdog
    // Main task (loopTask) will be watched
    esp_err_t err = esp_task_wdt_init(timeout_seconds, true);

    if (err != ESP_OK) {
        Serial.printf("[WatchdogTimer] Failed to initialize: %d\n", err);
        return false;
    }

    // Add current task to watchdog
    err = esp_task_wdt_add(NULL);
    if (err != ESP_OK) {
        Serial.printf("[WatchdogTimer] Failed to add task: %d\n", err);
        return false;
    }

    enabled_ = true;
    last_feed_ms_ = millis();

    char details[96];
    snprintf(details, sizeof(details), "Watchdog initialized, timeout=%ds, reboots=%u",
             timeout_seconds, reboot_count_);
    AuditLog::instance().log(AuditEventType::TOOL_START, "WatchdogTimer", details);

    Serial.printf("[WatchdogTimer] Initialized with %d second timeout\n", timeout_seconds);

    return true;
}

void WatchdogTimer::feed() {
    if (!enabled_) return;

    last_feed_ms_ = millis();
    esp_task_wdt_reset();
}

void WatchdogTimer::disable() {
    if (!enabled_) return;

    enabled_ = false;
    esp_task_wdt_delete(NULL);

    Serial.println("[WatchdogTimer] Disabled");
}

void WatchdogTimer::enable() {
    if (enabled_) return;

    esp_task_wdt_init(timeout_sec_, true);
    esp_task_wdt_add(NULL);
    enabled_ = true;
    last_feed_ms_ = millis();

    Serial.println("[WatchdogTimer] Enabled");
}

void WatchdogTimer::checkRebootReason() {
    esp_reset_reason_t reason = esp_reset_reason();

    // Count reboots from all causes and log if watchdog triggered
    if (reason == ESP_RST_TASK_WDT || reason == ESP_RST_INT_WDT) {
        reboot_count_++;
        Serial.printf("[WatchdogTimer] Detected previous watchdog reboot (count=%u)\n", reboot_count_);

        char details[96];
        const char* reason_str = (reason == ESP_RST_TASK_WDT) ? "Task WDT" : "Interrupt WDT";
        snprintf(details, sizeof(details), "Automatic reboot triggered, reason=%s, total=%u",
                 reason_str, reboot_count_);
        AuditLog::instance().log(AuditEventType::ERROR_OCCURRED, "WatchdogTimer", details);
    }
}

const char* WatchdogTimer::getLastRebootReason() const {
    esp_reset_reason_t reason = esp_reset_reason();

    switch(reason) {
        case ESP_RST_UNKNOWN:        return "Unknown";
        case ESP_RST_POWERON:        return "Power On";
        case ESP_RST_EXT:            return "External Signal";
        case ESP_RST_SW:             return "Software Reset";
        case ESP_RST_PANIC:          return "Exception/Panic";
        case ESP_RST_INT_WDT:        return "Interrupt Watchdog";
        case ESP_RST_TASK_WDT:       return "Task Watchdog";
        case ESP_RST_WDT:            return "Other Watchdog";
        case ESP_RST_DEEPSLEEP:      return "Deep Sleep";
        case ESP_RST_BROWNOUT:       return "Brownout";
        case ESP_RST_SDIO:           return "SDIO";
        default:                     return "Other Reason";
    }
}

WatchdogTimer::~WatchdogTimer() {
    if (enabled_) {
        disable();
    }
}
