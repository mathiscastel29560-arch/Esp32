#ifndef WATCHDOG_TIMER_H
#define WATCHDOG_TIMER_H

#include <Arduino.h>
#include <esp_task_wdt.h>

// Hardware watchdog for detecting deadlocks and crashes
// Automatically reboots if the main loop doesn't respond

class WatchdogTimer {
public:
    static WatchdogTimer& instance() {
        static WatchdogTimer wdt;
        return wdt;
    }

    // Initialize watchdog with timeout in seconds
    // Default: 10 seconds (main loop must respond every 10s)
    bool begin(uint32_t timeout_seconds = 10);

    // Feed the watchdog (call from main loop)
    // If not called within timeout, device reboots
    void feed();

    // Disable watchdog temporarily (for long operations)
    void disable();

    // Re-enable watchdog
    void enable();

    // Get watchdog status
    bool isEnabled() const { return enabled_; }
    uint32_t getTimeoutSeconds() const { return timeout_sec_; }
    uint32_t getLastFeedTime() const { return last_feed_ms_; }

    // Get reboot count since boot
    uint32_t getRebootCount() const { return reboot_count_; }

    // Get reason for last reboot
    const char* getLastRebootReason() const;

private:
    WatchdogTimer() : enabled_(false), timeout_sec_(10), last_feed_ms_(0), reboot_count_(0) {}
    ~WatchdogTimer();

    bool enabled_;
    uint32_t timeout_sec_;
    uint32_t last_feed_ms_;
    uint32_t reboot_count_;

    // Helper: get reason from esp_reset_reason
    void checkRebootReason();
};

#endif
