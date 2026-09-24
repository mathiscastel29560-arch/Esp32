#ifndef ALERT_SYSTEM_H
#define ALERT_SYSTEM_H

#include <Arduino.h>
#include <vector>

// Alert levels and storage
class AlertSystem {
public:
    enum AlertLevel {
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    struct Alert {
        AlertLevel level;
        String title;
        String message;
        uint32_t timestamp;
        bool acknowledged;
    };

    static AlertSystem& instance() {
        static AlertSystem as;
        return as;
    }

    // Add alert
    void addAlert(AlertLevel level, const char* title, const char* message) {
        if (alerts.size() >= MAX_ALERTS) {
            alerts.erase(alerts.begin());  // Remove oldest
        }

        Alert alert = {level, String(title), String(message), millis(), false};
        alerts.push_back(alert);

        // Print immediately for critical/error
        if (level >= ERROR) {
            printAlert(alert);
        }
    }

    // Quick helpers
    void addInfo(const char* title, const char* msg) {
        addAlert(INFO, title, msg);
    }

    void addWarning(const char* title, const char* msg) {
        addAlert(WARNING, title, msg);
    }

    void addError(const char* title, const char* msg) {
        addAlert(ERROR, title, msg);
    }

    void addCritical(const char* title, const char* msg) {
        addAlert(CRITICAL, title, msg);
    }

    // Get unacknowledged count
    uint32_t getUnacknowledgedCount() const {
        uint32_t count = 0;
        for (const auto& alert : alerts) {
            if (!alert.acknowledged) count++;
        }
        return count;
    }

    // Get all alerts
    const std::vector<Alert>& getAllAlerts() const {
        return alerts;
    }

    // Acknowledge specific alert
    void acknowledgeAlert(size_t idx) {
        if (idx < alerts.size()) {
            alerts[idx].acknowledged = true;
        }
    }

    // Acknowledge all
    void acknowledgeAll() {
        for (auto& alert : alerts) {
            alert.acknowledged = true;
        }
    }

    // Display all unacknowledged alerts
    void displayUnacknowledged() {
        if (getUnacknowledgedCount() == 0) return;

        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║            SYSTEM ALERTS              ║");
        Serial.println("╠════════════════════════════════════════╣");

        for (size_t i = 0; i < alerts.size(); i++) {
            const auto& alert = alerts[i];
            if (!alert.acknowledged) {
                Serial.printf("║ [%u] ", i);
                switch (alert.level) {
                    case INFO:     Serial.print("ℹ️  "); break;
                    case WARNING:  Serial.print("⚠️  "); break;
                    case ERROR:    Serial.print("❌ "); break;
                    case CRITICAL: Serial.print("🔴 "); break;
                }
                Serial.println(alert.title);
                Serial.printf("║     %s\n", alert.message.c_str());
            }
        }

        Serial.println("╚════════════════════════════════════════╝\n");
    }

    // Clear all alerts
    void clearAll() {
        alerts.clear();
    }

private:
    static constexpr uint32_t MAX_ALERTS = 16;
    std::vector<Alert> alerts;

    AlertSystem() {}

    void printAlert(const Alert& alert) {
        Serial.print("\n");
        switch (alert.level) {
            case INFO:     Serial.print("[INFO] "); break;
            case WARNING:  Serial.print("[WARN] "); break;
            case ERROR:    Serial.print("[ERROR] "); break;
            case CRITICAL: Serial.print("[CRITICAL] "); break;
        }
        Serial.printf("%s: %s\n", alert.title.c_str(), alert.message.c_str());
    }
};

#endif
