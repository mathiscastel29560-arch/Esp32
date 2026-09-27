#include "audit_history.h"
#include "debug_logger.h"
#include <nvs.h>
#include <nvs_flash.h>
#include <cstring>

AuditHistory::AuditHistory() {
  loadFromNVS();
  DebugLogger::println("[AuditHistory] Initialized");
}

void AuditHistory::addRecord(AuditRecord record) {
  // If at capacity, remove oldest
  if (records.size() >= MAX_RECORDS) {
    records.erase(records.begin());
  }

  records.push_back(record);
  recordCount = records.size();

  saveToNVS();
  DebugLogger::printf("[AuditHistory] Record added (total: %u)\n", recordCount);
}

std::vector<AuditRecord> AuditHistory::getAllRecords() const {
  return records;
}

std::vector<AuditRecord> AuditHistory::getRecordsByType(uint8_t type) const {
  std::vector<AuditRecord> filtered;
  for (const auto& rec : records) {
    if (rec.auditType == type) {
      filtered.push_back(rec);
    }
  }
  return filtered;
}

std::vector<AuditRecord> AuditHistory::getLastRecords(uint8_t count) const {
  std::vector<AuditRecord> result;
  uint16_t start = (records.size() > count) ? (records.size() - count) : 0;

  for (uint16_t i = start; i < records.size(); i++) {
    result.push_back(records[i]);
  }
  return result;
}

void AuditHistory::clearAll() {
  records.clear();
  recordCount = 0;
  saveToNVS();
  DebugLogger::println("[AuditHistory] All records cleared");
}

void AuditHistory::deleteOldest() {
  if (!records.empty()) {
    records.erase(records.begin());
    recordCount = records.size();
    saveToNVS();
    DebugLogger::println("[AuditHistory] Oldest record deleted");
  }
}

void AuditHistory::loadFromNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);

  if (err != ESP_OK) {
    DebugLogger::println("[AuditHistory] NVS not found, starting fresh");
    return;
  }

  uint16_t count = 0;
  if (nvs_get_u16(handle, NVS_COUNT_KEY, &count) == ESP_OK) {
    records.clear();
    records.reserve(count);

    for (uint16_t i = 0; i < count && i < MAX_RECORDS; i++) {
      char key[16];
      snprintf(key, sizeof(key), "%s%u", NVS_RECORD_PREFIX, i);

      AuditRecord rec = {0};
      size_t required_size = sizeof(AuditRecord);

      if (nvs_get_blob(handle, key, &rec, &required_size) == ESP_OK) {
        records.push_back(rec);
      }
    }
    recordCount = records.size();
  }

  nvs_close(handle);
  DebugLogger::printf("[AuditHistory] Loaded %u records from NVS\n", recordCount);
}

void AuditHistory::saveToNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);

  if (err != ESP_OK) {
    DebugLogger::println("[AuditHistory] Error opening NVS");
    return;
  }

  // Save count
  nvs_set_u16(handle, NVS_COUNT_KEY, recordCount);

  // Save records
  for (uint16_t i = 0; i < recordCount && i < MAX_RECORDS; i++) {
    char key[16];
    snprintf(key, sizeof(key), "%s%u", NVS_RECORD_PREFIX, i);
    nvs_set_blob(handle, key, &records[i], sizeof(AuditRecord));
  }

  nvs_commit(handle);
  nvs_close(handle);
  DebugLogger::println("[AuditHistory] Saved to NVS");
}

void AuditHistory::printHistory() const {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║        AUDIT HISTORY REPORT            ║");
  Serial.println("╚════════════════════════════════════════╝");

  if (records.empty()) {
    Serial.println("No audit records found.\n");
    return;
  }

  const char* typeStr[] = {"WiFi", "BLE", "RF/NRF24", "IoT", "Other"};
  const char* statusStr[] = {"✓ OK", "⚠ INT", "✗ ERR"};

  for (uint16_t i = 0; i < records.size(); i++) {
    const auto& rec = records[i];
    time_t t = rec.timestamp;
    struct tm* timeinfo = localtime(&t);

    Serial.printf("[%u] %04d-%02d-%02d %02d:%02d:%02d\n",
      i + 1,
      timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
      timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec
    );

    Serial.printf("   Type:     %s\n", (rec.auditType < 5) ? typeStr[rec.auditType] : "Unknown");
    Serial.printf("   Devices:  %u found\n", rec.devicesFound);
    Serial.printf("   Attacks:  %u executed\n", rec.attacksExecuted);
    Serial.printf("   Signal:   %d dBm (strongest)\n", rec.maxRSSI);
    Serial.printf("   Duration: %u seconds\n", rec.duration);
    Serial.printf("   Status:   %s\n\n", (rec.status < 3) ? statusStr[rec.status] : "Unknown");
  }

  Serial.println("");
}
