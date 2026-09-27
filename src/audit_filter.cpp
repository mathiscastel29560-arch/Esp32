#include "audit_filter.h"
#include "debug_logger.h"
#include <cstdio>
#include <algorithm>

std::vector<AuditRecord> AuditFilter::filter(const std::vector<AuditRecord>& records,
                                             const FilterCriteria& criteria) {
  std::vector<AuditRecord> result;

  for (const auto& record : records) {
    if (!meetsDateRange(record.timestamp, criteria)) continue;
    if (!meetsDeviceRange(record.devicesFound, criteria)) continue;
    if (!meetsRSSIRange(record.maxRSSI, criteria)) continue;
    if (!meetsTypeFilter(record.auditType, criteria)) continue;
    if (criteria.successOnly && record.status != "success") continue;

    result.push_back(record);
  }

  return result;
}

std::string AuditFilter::exportToCSV(const std::vector<AuditRecord>& records) {
  std::string csv = "Timestamp,Type,Devices,RSSI,Status\n";

  for (const auto& record : records) {
    char line[128];
    snprintf(line, sizeof(line), "%u,%s,%u,%d,%s\n",
      record.timestamp, record.auditType, record.devicesFound,
      record.maxRSSI, record.status);
    csv += line;
  }

  return csv;
}

std::string AuditFilter::exportToJSON(const std::vector<AuditRecord>& records) {
  std::string json = "{\"audits\":[";

  for (size_t i = 0; i < records.size(); i++) {
    if (i > 0) json += ",";

    char entry[256];
    snprintf(entry, sizeof(entry),
      "{\"timestamp\":%u,\"type\":\"%s\",\"devices\":%u,\"rssi\":%d,\"status\":\"%s\"}",
      records[i].timestamp, records[i].auditType, records[i].devicesFound,
      records[i].maxRSSI, records[i].status);
    json += entry;
  }

  json += "]}";
  return json;
}

AuditFilter::FilterStats AuditFilter::getFilterStats(const std::vector<AuditRecord>& records,
                                                      const FilterCriteria& criteria) {
  auto filtered = filter(records, criteria);

  FilterStats stats = {0, 0, 0, 0};
  stats.count = filtered.size();

  if (stats.count == 0) return stats;

  int32_t rssiSum = 0;
  for (const auto& record : filtered) {
    stats.totalDevices += record.devicesFound;
    rssiSum += record.maxRSSI;
    if (std::string(record.status) == "success") stats.successCount++;
  }

  stats.avgRSSI = rssiSum / stats.count;
  return stats;
}

bool AuditFilter::meetsDateRange(uint32_t timestamp, const FilterCriteria& criteria) {
  return timestamp >= criteria.startTime && timestamp <= criteria.endTime;
}

bool AuditFilter::meetsDeviceRange(uint8_t count, const FilterCriteria& criteria) {
  return count >= criteria.minDevicesFound && count <= criteria.maxDevicesFound;
}

bool AuditFilter::meetsRSSIRange(int8_t rssi, const FilterCriteria& criteria) {
  return rssi >= criteria.minRSSI && rssi <= criteria.maxRSSI;
}

bool AuditFilter::meetsTypeFilter(const std::string& type, const FilterCriteria& criteria) {
  if (criteria.auditTypeFilter.empty()) return true;
  return type == criteria.auditTypeFilter;
}
