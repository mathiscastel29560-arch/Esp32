#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

// Scheduled/automated audits system (cron-like)
class ScheduledAudits {
public:
  enum RecurrenceType {
    RECUR_ONCE = 0,
    RECUR_HOURLY = 1,
    RECUR_DAILY = 2,
    RECUR_WEEKLY = 3,
    RECUR_MONTHLY = 4
  };

  struct ScheduledAudit {
    std::string id;
    std::string name;
    std::string auditType;         // "wifi_scan", "ble_scan", "rf_sweep", etc.
    RecurrenceType recurrence;
    uint32_t nextRunTime;          // Unix timestamp
    uint32_t lastRunTime;
    bool enabled;
    uint8_t maxRetries;
    std::string parameters;        // JSON config
  };

  using AuditCallback = std::function<void(const std::string&)>;

  static ScheduledAudits& getInstance() {
    static ScheduledAudits instance;
    return instance;
  }

  // Initialize scheduler
  bool begin();

  // Create new scheduled audit
  std::string createAudit(const std::string& name,
                         const std::string& auditType,
                         RecurrenceType recurrence,
                         const std::string& parameters);

  // Schedule audit at specific time
  bool scheduleAuditAt(const std::string& auditId, uint32_t timestamp);

  // Enable/disable audit
  bool setAuditEnabled(const std::string& auditId, bool enabled);

  // Get all scheduled audits
  std::vector<ScheduledAudit> getScheduledAudits();

  // Get audit by ID
  ScheduledAudit* getAudit(const std::string& auditId);

  // Delete audit
  bool deleteAudit(const std::string& auditId);

  // Check if any audits need to run
  void checkAndRunDue();

  // Set callback for when audit needs to run
  void setAuditCallback(AuditCallback cb);

  // Get next scheduled run time
  uint32_t getNextRunTime();

  // Statistics
  uint8_t getEnabledCount() const;
  uint8_t getTotalCount() const;

private:
  ScheduledAudits() = default;

  std::vector<ScheduledAudit> audits;
  AuditCallback auditCallback = nullptr;
  uint32_t lastCheckTime = 0;

  static const size_t MAX_AUDITS = 20;

  // Calculate next run time based on recurrence
  uint32_t calculateNextRun(const ScheduledAudit& audit, uint32_t baseTime);

  // Generate unique ID
  std::string generateAuditID();

  // Load/save from NVS
  bool loadFromNVS();
  bool saveToNVS();
};

#endif // SCHEDULED_AUDITS_H
