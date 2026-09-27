#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "audit_history.h"

// Advanced filtering system for audit history
class AuditFilter {
public:
  struct FilterCriteria {
    uint32_t startTime = 0;         // Unix timestamp
    uint32_t endTime = UINT32_MAX;  // Unix timestamp
    uint8_t minDevicesFound = 0;
    uint8_t maxDevicesFound = 255;
    int8_t minRSSI = -120;
    int8_t maxRSSI = 0;
    std::string auditTypeFilter = "";  // "" = all types
    bool successOnly = false;
  };

  static AuditFilter& getInstance() {
    static AuditFilter instance;
    return instance;
  }

  // Apply filter to audit records
  std::vector<AuditRecord> filter(const std::vector<AuditRecord>& records,
                                  const FilterCriteria& criteria);

  // Export to CSV format
  std::string exportToCSV(const std::vector<AuditRecord>& records);

  // Export to JSON format
  std::string exportToJSON(const std::vector<AuditRecord>& records);

  // Get summary statistics for filtered records
  struct FilterStats {
    uint32_t count;
    uint32_t totalDevices;
    int8_t avgRSSI;
    uint32_t successCount;
  };

  FilterStats getFilterStats(const std::vector<AuditRecord>& records,
                            const FilterCriteria& criteria);

private:
  AuditFilter() = default;

  bool meetsDateRange(uint32_t timestamp, const FilterCriteria& criteria);
  bool meetsDeviceRange(uint8_t count, const FilterCriteria& criteria);
  bool meetsRSSIRange(int8_t rssi, const FilterCriteria& criteria);
  bool meetsTypeFilter(const std::string& type, const FilterCriteria& criteria);
};

#endif // AUDIT_FILTER_H
