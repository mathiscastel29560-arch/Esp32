#include "scheduled_audits.h"
#include "debug_logger.h"
#include <nvs_flash.h>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <cstring>

bool ScheduledAudits::begin() {
  if (!loadFromNVS()) {
    DebugLogger::println("[Scheduler] No existing scheduled audits found");
  }
  DebugLogger::printf("[Scheduler] Initialized with %u audits\n", audits.size());
  return true;
}

std::string ScheduledAudits::createAudit(const std::string& name,
                                        const std::string& auditType,
                                        RecurrenceType recurrence,
                                        const std::string& parameters) {
  std::string auditId = generateAuditID();

  ScheduledAudit audit;
  audit.id = auditId;
  audit.name = name;
  audit.auditType = auditType;
  audit.recurrence = recurrence;
  audit.nextRunTime = time(nullptr) + 60;  // Run in 1 minute
  audit.lastRunTime = 0;
  audit.enabled = true;
  audit.maxRetries = 3;
  audit.parameters = parameters;

  audits.push_back(audit);
  saveToNVS();

  DebugLogger::printf("[Scheduler] Created audit: %s (type=%s)\n", name.c_str(), auditType.c_str());
  return auditId;
}

bool ScheduledAudits::scheduleAuditAt(const std::string& auditId, uint32_t timestamp) {
  for (auto& audit : audits) {
    if (audit.id == auditId) {
      audit.nextRunTime = timestamp;
      saveToNVS();
      return true;
    }
  }
  return false;
}

bool ScheduledAudits::setAuditEnabled(const std::string& auditId, bool enabled) {
  for (auto& audit : audits) {
    if (audit.id == auditId) {
      audit.enabled = enabled;
      saveToNVS();
      return true;
    }
  }
  return false;
}

std::vector<ScheduledAudits::ScheduledAudit> ScheduledAudits::getScheduledAudits() {
  return audits;
}

ScheduledAudits::ScheduledAudit* ScheduledAudits::getAudit(const std::string& auditId) {
  for (auto& audit : audits) {
    if (audit.id == auditId) {
      return &audit;
    }
  }
  return nullptr;
}

bool ScheduledAudits::deleteAudit(const std::string& auditId) {
  auto it = std::find_if(audits.begin(), audits.end(),
    [&auditId](const ScheduledAudit& a) { return a.id == auditId; });

  if (it != audits.end()) {
    audits.erase(it);
    saveToNVS();
    DebugLogger::printf("[Scheduler] Deleted audit: %s\n", auditId.c_str());
    return true;
  }
  return false;
}

void ScheduledAudits::checkAndRunDue() {
  uint32_t now = time(nullptr);

  for (auto& audit : audits) {
    if (!audit.enabled || now < audit.nextRunTime) continue;

    DebugLogger::printf("[Scheduler] Running audit: %s\n", audit.name.c_str());

    if (auditCallback) {
      auditCallback(audit.id);
    }

    audit.lastRunTime = now;
    audit.nextRunTime = calculateNextRun(audit, now);
    saveToNVS();
  }

  lastCheckTime = now;
}

void ScheduledAudits::setAuditCallback(AuditCallback cb) {
  auditCallback = cb;
}

uint32_t ScheduledAudits::getNextRunTime() {
  uint32_t nextTime = UINT32_MAX;

  for (const auto& audit : audits) {
    if (audit.enabled && audit.nextRunTime < nextTime) {
      nextTime = audit.nextRunTime;
    }
  }

  return nextTime;
}

uint8_t ScheduledAudits::getEnabledCount() const {
  uint8_t count = 0;
  for (const auto& audit : audits) {
    if (audit.enabled) count++;
  }
  return count;
}

uint8_t ScheduledAudits::getTotalCount() const {
  return audits.size();
}

uint32_t ScheduledAudits::calculateNextRun(const ScheduledAudit& audit, uint32_t baseTime) {
  switch (audit.recurrence) {
    case RECUR_ONCE:
      return UINT32_MAX;  // Never run again
    case RECUR_HOURLY:
      return baseTime + 3600;
    case RECUR_DAILY:
      return baseTime + 86400;
    case RECUR_WEEKLY:
      return baseTime + (7 * 86400);
    case RECUR_MONTHLY:
      return baseTime + (30 * 86400);
    default:
      return UINT32_MAX;
  }
}

std::string ScheduledAudits::generateAuditID() {
  const char* chars = "0123456789abcdef";
  std::string id;

  for (int i = 0; i < 16; i++) {
    id += chars[rand() % 16];
  }

  return id;
}

bool ScheduledAudits::loadFromNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("scheduler", NVS_READWRITE, &handle);
  if (err != ESP_OK) return false;

  uint32_t auditCount = 0;
  nvs_get_u32(handle, "audit_count", &auditCount);

  for (uint32_t i = 0; i < auditCount && i < MAX_AUDITS; i++) {
    char keyName[32];
    snprintf(keyName, sizeof(keyName), "audit_%u", i);

    char auditData[256];
    size_t len = sizeof(auditData);
    if (nvs_get_str(handle, keyName, auditData, &len) == ESP_OK) {
      // Parse: id|name|type|recur|nextRun|lastRun|enabled
      // Simplified for now
    }
  }

  nvs_close(handle);
  return true;
}

bool ScheduledAudits::saveToNVS() {
  nvs_handle_t handle;
  esp_err_t err = nvs_open("scheduler", NVS_READWRITE, &handle);
  if (err != ESP_OK) return false;

  nvs_set_u32(handle, "audit_count", audits.size());

  for (size_t i = 0; i < audits.size() && i < MAX_AUDITS; i++) {
    char keyName[32];
    snprintf(keyName, sizeof(keyName), "audit_%u", i);

    char auditData[256];
    snprintf(auditData, sizeof(auditData), "%s|%s|%s|%u|%u|%u|%u",
      audits[i].id.c_str(), audits[i].name.c_str(), audits[i].auditType.c_str(),
      audits[i].recurrence, audits[i].nextRunTime, audits[i].lastRunTime,
      audits[i].enabled ? 1 : 0);

    nvs_set_str(handle, keyName, auditData);
  }

  nvs_commit(handle);
  nvs_close(handle);
  return true;
}
