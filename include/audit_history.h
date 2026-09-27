#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <ctime>

// ============= AUDIT HISTORY MANAGER =============
struct AuditRecord {
  uint32_t timestamp;           // Unix timestamp
  uint8_t auditType;            // 0=WiFi, 1=BLE, 2=RF/NRF24, 3=IoT, 4=Other
  uint16_t devicesFound;        // Number of devices/networks found
  uint16_t attacksExecuted;     // Number of attacks run
  int8_t maxRSSI;               // Strongest signal detected (dBm)
  uint8_t duration;             // Scan duration in seconds

  // Status: 0=completed, 1=interrupted, 2=error
  uint8_t status;
};

class AuditHistory {
public:
  static AuditHistory& getInstance() {
    static AuditHistory instance;
    return instance;
  }

  // Add a new audit record
  void addRecord(AuditRecord record);

  // Get all records
  std::vector<AuditRecord> getAllRecords() const;

  // Get records by type
  std::vector<AuditRecord> getRecordsByType(uint8_t type) const;

  // Get last N records
  std::vector<AuditRecord> getLastRecords(uint8_t count) const;

  // Get record count
  uint16_t getRecordCount() const { return recordCount; }

  // Clear all records
  void clearAll();

  // Delete oldest record
  void deleteOldest();

  // Save/Load from NVS
  void loadFromNVS();
  void saveToNVS();

  // Debug print
  void printHistory() const;

  // Get size info
  uint16_t getMaxRecords() const { return MAX_RECORDS; }

private:
  AuditHistory();

  static constexpr uint16_t MAX_RECORDS = 100;  // Max 100 records
  std::vector<AuditRecord> records;
  uint16_t recordCount = 0;

  // NVS key constants
  static constexpr const char* NVS_NAMESPACE = "audit_history";
  static constexpr const char* NVS_COUNT_KEY = "count";
  static constexpr const char* NVS_RECORD_PREFIX = "rec_";
};

#endif // AUDIT_HISTORY_H
