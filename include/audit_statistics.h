#pragma once

#include <stdint.h>
#include <string>
#include "audit_history.h"

// ============= AUDIT STATISTICS ANALYZER =============
struct AuditStats {
  // General
  uint16_t totalAudits;
  uint32_t totalDuration;         // Total time spent (seconds)

  // By Type
  uint16_t wifiAudits;
  uint16_t bleAudits;
  uint16_t rfAudits;
  uint16_t iotAudits;

  // Aggregates
  uint32_t totalDevicesFound;     // Total across all audits
  uint32_t totalAttacksExecuted;
  int8_t bestSignal;              // Best (strongest) signal
  int8_t worstSignal;             // Worst (weakest) signal

  // Averages
  uint16_t avgDevicesPerAudit;
  uint16_t avgAttacksPerAudit;
  uint16_t avgDurationSeconds;
  int8_t avgSignalStrength;

  // Success Rate
  uint16_t successCount;          // Completed audits
  uint16_t successPercent;        // Percentage
};

class AuditStatistics {
public:
  static AuditStatistics& getInstance() {
    static AuditStatistics instance;
    return instance;
  }

  // Calculate statistics from history
  AuditStats calculateStats() const;

  // Get stats for specific type
  uint16_t getAuditCountByType(uint8_t type) const;

  // Get most frequent signal strength
  int8_t getMostFrequentSignal() const;

  // Get audit efficiency (devices per second)
  float getAuditEfficiency() const;

  // Get trend (is activity increasing/decreasing)
  int8_t getTrend() const;  // -1=decreasing, 0=stable, 1=increasing

  // Print stats
  void printStatistics() const;

  // Get summary string
  std::string getSummaryString() const;

private:
  AuditStatistics() = default;
};

#endif // AUDIT_STATISTICS_H
