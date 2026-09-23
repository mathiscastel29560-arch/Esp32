#pragma once
#include <Arduino.h>

namespace VisualNotifications {

// Notification types
enum NotificationType {
    NOTIFY_SUCCESS,
    NOTIFY_INFO,
    NOTIFY_WARNING,
    NOTIFY_ERROR,
    NOTIFY_PROGRESS,
    NOTIFY_CUSTOM
};

// Notification duration options
enum Duration {
    DURATION_SHORT = 1000,   // 1 second
    DURATION_NORMAL = 3000,  // 3 seconds
    DURATION_LONG = 5000,    // 5 seconds
    DURATION_PERMANENT = 0   // Stay until dismissed
};

class Notifier {
public:
    // Initialize notification system
    static void begin();

    // Show notifications
    static void showSuccess(const String &title, const String &message,
                           Duration duration = DURATION_NORMAL);
    static void showInfo(const String &title, const String &message,
                        Duration duration = DURATION_NORMAL);
    static void showWarning(const String &title, const String &message,
                           Duration duration = DURATION_NORMAL);
    static void showError(const String &title, const String &message,
                         Duration duration = DURATION_LONG);

    // Progress notification
    static void showProgress(const String &title, uint8_t percent);

    // Toast notification (brief, auto-dismiss)
    static void toast(const String &message, uint16_t duration = 2000);

    // Status bar updates
    static void updateStatusBar(const String &left, const String &center, const String &right);

    // Popup alert (blocks until dismissed)
    static void alert(const String &title, const String &message);

    // Confirmation dialog
    static bool confirm(const String &title, const String &message);

    // Pulse effect (visual attention)
    static void pulse(const String &message, uint8_t count = 3);

    // Animation effects
    static void showSpinner(const String &message);
    static void showWave(const String &message);

    // Notification queue
    static uint8_t getQueueSize();
    static void clearQueue();
};

}  // namespace VisualNotifications
