#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

// Real-time alert system for monitoring
class AlertsSystem {
public:
  enum AlertLevel {
    LEVEL_INFO = 0,
    LEVEL_WARNING = 1,
    LEVEL_CRITICAL = 2
  };

  enum AlertType {
    ALERT_BATTERY_LOW = 0,
    ALERT_BATTERY_CRITICAL = 1,
    ALERT_MEMORY_LOW = 2,
    ALERT_CPU_HIGH = 3,
    ALERT_WIFI_DISCONNECT = 4,
    ALERT_AUDIT_FAILED = 5,
    ALERT_ANOMALY_DETECTED = 6,
    ALERT_OVERHEAT = 7,
    ALERT_GPS_LOCK_LOST = 8,
    ALERT_DEVICE_ERROR = 9
  };

  struct Alert {
    AlertType type;
    AlertLevel level;
    std::string message;
    uint32_t timestamp;
    bool acknowledged;
  };

  using AlertCallback = std::function<void(const Alert&)>;

  static AlertsSystem& getInstance() {
    static AlertsSystem instance;
    return instance;
  }

  // Initialize alert system
  bool begin();

  // Trigger an alert
  void triggerAlert(AlertType type, AlertLevel level, const std::string& message);

  // Set alert thresholds
  void setBatteryLowThreshold(uint8_t percent);
  void setBatteryCriticalThreshold(uint8_t percent);
  void setMemoryLowThreshold(uint32_t bytes);
  void setCPUHighThreshold(uint8_t percent);

  // Register callback for alerts
  void setAlertCallback(AlertCallback cb);

  // Get recent alerts
  std::vector<Alert> getRecentAlerts(uint8_t count = 10);

  // Acknowledge alert
  void acknowledgeAlert(uint32_t timestamp);

  // Clear all acknowledged alerts
  void clearAcknowledged();

  // Check critical conditions
  void updateMonitoring();

  // Get alert statistics
  struct AlertStats {
    uint32_t totalAlerts;
    uint32_t unacknowledgedCount;
    uint32_t criticalCount;
    uint32_t warningCount;
  };

  AlertStats getStats();

private:
  AlertsSystem() = default;

  std::vector<Alert> alerts;
  AlertCallback alertCallback = nullptr;

  uint8_t batteryLowThreshold = 20;
  uint8_t batteryCriticalThreshold = 5;
  uint32_t memoryLowThreshold = 1024 * 50;  // 50KB
  uint8_t cpuHighThreshold = 80;

  static const size_t MAX_ALERTS = 100;

  // Helper functions
  bool shouldCreateAlert(AlertType type);
  void addAlert(AlertType type, AlertLevel level, const std::string& message);
};

#endif // ALERTS_SYSTEM_H
