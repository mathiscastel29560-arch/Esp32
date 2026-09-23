#include "visual_notifications.h"
#include "debug_logger.h"
#include "menu_enhanced.h"
#include <vector>

namespace VisualNotifications {

struct Notification {
    NotificationType type;
    String title;
    String message;
    uint32_t createdAt;
    Duration duration;
    bool active;
};

static std::vector<Notification> g_notificationQueue;
static const size_t MAX_QUEUE = 5;

void Notifier::begin() {
    DBG_INFO("VisualNotifications", "Notification system initialized");
}

void Notifier::showSuccess(const String &title, const String &message,
                          Duration duration) {
    if (g_notificationQueue.size() >= MAX_QUEUE) {
        g_notificationQueue.erase(g_notificationQueue.begin());
    }

    Notification notif;
    notif.type = NOTIFY_SUCCESS;
    notif.title = title;
    notif.message = message;
    notif.createdAt = millis();
    notif.duration = duration;
    notif.active = true;

    g_notificationQueue.push_back(notif);

    Serial.println();
    Serial.print(COLOR_GREEN);
    Serial.print("✓ ");
    Serial.print(title);
    Serial.print(": ");
    Serial.println(message);
    Serial.print(COLOR_RESET);
}

void Notifier::showInfo(const String &title, const String &message,
                       Duration duration) {
    if (g_notificationQueue.size() >= MAX_QUEUE) {
        g_notificationQueue.erase(g_notificationQueue.begin());
    }

    Notification notif;
    notif.type = NOTIFY_INFO;
    notif.title = title;
    notif.message = message;
    notif.createdAt = millis();
    notif.duration = duration;
    notif.active = true;

    g_notificationQueue.push_back(notif);

    Serial.println();
    Serial.print(COLOR_CYAN);
    Serial.print("ℹ ");
    Serial.print(title);
    Serial.print(": ");
    Serial.println(message);
    Serial.print(COLOR_RESET);
}

void Notifier::showWarning(const String &title, const String &message,
                          Duration duration) {
    if (g_notificationQueue.size() >= MAX_QUEUE) {
        g_notificationQueue.erase(g_notificationQueue.begin());
    }

    Notification notif;
    notif.type = NOTIFY_WARNING;
    notif.title = title;
    notif.message = message;
    notif.createdAt = millis();
    notif.duration = duration;
    notif.active = true;

    g_notificationQueue.push_back(notif);

    Serial.println();
    Serial.print(COLOR_YELLOW);
    Serial.print("⚠ ");
    Serial.print(title);
    Serial.print(": ");
    Serial.println(message);
    Serial.print(COLOR_RESET);
}

void Notifier::showError(const String &title, const String &message,
                        Duration duration) {
    if (g_notificationQueue.size() >= MAX_QUEUE) {
        g_notificationQueue.erase(g_notificationQueue.begin());
    }

    Notification notif;
    notif.type = NOTIFY_ERROR;
    notif.title = title;
    notif.message = message;
    notif.createdAt = millis();
    notif.duration = duration;
    notif.active = true;

    g_notificationQueue.push_back(notif);

    Serial.println();
    Serial.print(COLOR_RED);
    Serial.print("✗ ");
    Serial.print(title);
    Serial.print(": ");
    Serial.println(message);
    Serial.print(COLOR_RESET);
}

void Notifier::showProgress(const String &title, uint8_t percent) {
    Serial.print("\r");
    Serial.print(COLOR_CYAN);
    Serial.print(title);
    Serial.print(" [");

    uint8_t filled = percent / 10;
    for (uint8_t i = 0; i < 10; i++) {
        if (i < filled) Serial.print(COLOR_GREEN + String("█") + COLOR_CYAN);
        else Serial.print("░");
    }

    Serial.print("] ");
    Serial.print(percent);
    Serial.print("%");
    Serial.print(COLOR_RESET);
}

void Notifier::toast(const String &message, uint16_t duration) {
    Serial.println();
    Serial.print(COLOR_BLUE);
    Serial.print("╔");
    Serial.print(String(message.length() + 2, '═'));
    Serial.println("╗");
    Serial.print("║ ");
    Serial.print(message);
    Serial.println(" ║");
    Serial.print("╚");
    Serial.print(String(message.length() + 2, '═'));
    Serial.println("╝");
    Serial.print(COLOR_RESET);

    delay(duration);
}

void Notifier::updateStatusBar(const String &left, const String &center, const String &right) {
    Serial.print(COLOR_CYAN);
    Serial.print("├─ ");
    Serial.print(left);
    Serial.print(" | ");
    Serial.print(center);
    Serial.print(" | ");
    Serial.println(right);
    Serial.print(COLOR_RESET);
}

void Notifier::alert(const String &title, const String &message) {
    Serial.println();
    Serial.print(COLOR_RED);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print("║ ⚠ ");
    Serial.print(title);
    Serial.println(String(36 - title.length(), ' ') + "║");
    Serial.println("╠═════════════════════════════════════════╣");
    Serial.print("║ ");

    // Word wrap message
    int pos = 0;
    int lines = 0;
    while (pos < message.length() && lines < 4) {
        int nextPos = pos + 37;
        if (nextPos > message.length()) nextPos = message.length();

        String line = message.substring(pos, nextPos);
        Serial.print(line);
        Serial.println(String(37 - line.length(), ' ') + "║");

        pos = nextPos;
        lines++;
    }

    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(COLOR_RESET);
}

bool Notifier::confirm(const String &title, const String &message) {
    Serial.println();
    Serial.print(COLOR_YELLOW);
    Serial.println("╔═════════════════════════════════════════╗");
    Serial.print("║ ? ");
    Serial.print(title);
    Serial.println(String(36 - title.length(), ' ') + "║");
    Serial.println("╠═════════════════════════════════════════╣");
    Serial.print("║ ");
    Serial.print(message);
    Serial.println(String(37 - message.length(), ' ') + "║");
    Serial.println("║ ");
    Serial.println("║ Press SELECT to confirm, BACK to cancel");
    Serial.println("╚═════════════════════════════════════════╝");
    Serial.print(COLOR_RESET);

    // Wait for user input (simplified)
    return true;
}

void Notifier::pulse(const String &message, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        Serial.print(COLOR_BLUE);
        Serial.print("● ");
        Serial.println(message);
        delay(200);

        Serial.print(COLOR_YELLOW);
        Serial.print("◐ ");
        Serial.println(message);
        delay(200);
    }
    Serial.print(COLOR_RESET);
}

void Notifier::showSpinner(const String &message) {
    const char *spinChars[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

    for (uint8_t i = 0; i < 10; i++) {
        Serial.print("\r");
        Serial.print(COLOR_CYAN);
        Serial.print(spinChars[i]);
        Serial.print(" ");
        Serial.print(message);
        Serial.print(COLOR_RESET);
        delay(100);
    }
    Serial.println();
}

void Notifier::showWave(const String &message) {
    const char *waveChars[] = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█", "▇", "▆", "▅", "▄", "▃", "▂"};

    for (uint8_t i = 0; i < 14; i++) {
        Serial.print("\r");
        Serial.print(COLOR_YELLOW);
        Serial.print(waveChars[i]);
        Serial.print(" ");
        Serial.print(message);
        Serial.print(COLOR_RESET);
        delay(100);
    }
    Serial.println();
}

uint8_t Notifier::getQueueSize() {
    return g_notificationQueue.size();
}

void Notifier::clearQueue() {
    g_notificationQueue.clear();
}

}  // namespace VisualNotifications
