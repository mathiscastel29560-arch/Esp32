#include "alerts_system.h"
#include "debug_logger.h"
#include <algorithm>
#include <ctime>

bool AlertsSystem::begin() {
  DebugLogger::println("[Alerts] System initialized");
  return true;
}

void AlertsSystem::triggerAlert(AlertType type, AlertLevel level, const std::string& message) {
  if (!shouldCreateAlert(type)) return;

  Alert alert;
  alert.type = type;
  alert.level = level;
  alert.message = message;
  alert.timestamp = time(nullptr);
  alert.acknowledged = false;

  alerts.push_back(alert);

  // Maintain size limit
  if (alerts.size() > MAX_ALERTS) {
    alerts.erase(alerts.begin());
  }

  if (alertCallback) {
    alertCallback(alert);
  }

  const char* levelStr = "";
  switch (level) {
    case LEVEL_INFO: levelStr = "INFO"; break;
    case LEVEL_WARNING: levelStr = "WARN"; break;
    case LEVEL_CRITICAL: levelStr = "CRIT"; break;
  }

  DebugLogger::printf("[Alert %s] Type=%u: %s\n", levelStr, type, message.c_str());
}

void AlertsSystem::setBatteryLowThreshold(uint8_t percent) {
  batteryLowThreshold = percent;
}

void AlertsSystem::setBatteryCriticalThreshold(uint8_t percent) {
  batteryCriticalThreshold = percent;
}

void AlertsSystem::setMemoryLowThreshold(uint32_t bytes) {
  memoryLowThreshold = bytes;
}

void AlertsSystem::setCPUHighThreshold(uint8_t percent) {
  cpuHighThreshold = percent;
}

void AlertsSystem::setAlertCallback(AlertCallback cb) {
  alertCallback = cb;
}

std::vector<AlertsSystem::Alert> AlertsSystem::getRecentAlerts(uint8_t count) {
  std::vector<Alert> result;

  size_t start = (alerts.size() > count) ? (alerts.size() - count) : 0;
  for (size_t i = start; i < alerts.size(); i++) {
    result.push_back(alerts[i]);
  }

  std::reverse(result.begin(), result.end());
  return result;
}

void AlertsSystem::acknowledgeAlert(uint32_t timestamp) {
  for (auto& alert : alerts) {
    if (alert.timestamp == timestamp) {
      alert.acknowledged = true;
      return;
    }
  }
}

void AlertsSystem::clearAcknowledged() {
  alerts.erase(
    std::remove_if(alerts.begin(), alerts.end(),
      [](const Alert& a) { return a.acknowledged; }),
    alerts.end()
  );
}

void AlertsSystem::updateMonitoring() {
  uint32_t freeHeap = ESP.getFreeHeap();
  if (freeHeap < memoryLowThreshold) {
    triggerAlert(ALERT_MEMORY_LOW, LEVEL_WARNING,
      "Low memory: " + std::to_string(freeHeap / 1024) + "KB");
  }
}

AlertsSystem::AlertStats AlertsSystem::getStats() {
  AlertStats stats = {0, 0, 0, 0};

  stats.totalAlerts = alerts.size();

  for (const auto& alert : alerts) {
    if (!alert.acknowledged) stats.unacknowledgedCount++;
    if (alert.level == LEVEL_CRITICAL) stats.criticalCount++;
    if (alert.level == LEVEL_WARNING) stats.warningCount++;
  }

  return stats;
}

bool AlertsSystem::shouldCreateAlert(AlertType type) {
  // Prevent duplicate alerts of same type within 60 seconds
  uint32_t now = time(nullptr);

  for (const auto& alert : alerts) {
    if (alert.type == type && (now - alert.timestamp) < 60) {
      return false;
    }
  }

  return true;
}

void AlertsSystem::addAlert(AlertType type, AlertLevel level, const std::string& message) {
  triggerAlert(type, level, message);
}
