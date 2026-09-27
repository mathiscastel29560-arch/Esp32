#include "audit_statistics.h"
#include "debug_logger.h"

AuditStats AuditStatistics::calculateStats() const {
  auto& history = AuditHistory::getInstance();
  auto records = history.getAllRecords();

  AuditStats stats = {0};
  stats.totalAudits = records.size();

  if (records.empty()) {
    return stats;
  }

  // Initialize
  stats.bestSignal = -50;    // Will be overwritten
  stats.worstSignal = -120;  // Will be overwritten

  int8_t firstSignal = records[0].maxRSSI;
  stats.bestSignal = firstSignal;
  stats.worstSignal = firstSignal;

  // Aggregate data
  for (const auto& rec : records) {
    stats.totalDuration += rec.duration;
    stats.totalDevicesFound += rec.devicesFound;
    stats.totalAttacksExecuted += rec.attacksExecuted;

    if (rec.maxRSSI > stats.bestSignal) {
      stats.bestSignal = rec.maxRSSI;
    }
    if (rec.maxRSSI < stats.worstSignal) {
      stats.worstSignal = rec.maxRSSI;
    }

    if (rec.status == 0) {
      stats.successCount++;
    }

    // Count by type
    switch (rec.auditType) {
      case 0: stats.wifiAudits++; break;
      case 1: stats.bleAudits++; break;
      case 2: stats.rfAudits++; break;
      case 3: stats.iotAudits++; break;
    }
  }

  // Calculate averages
  if (stats.totalAudits > 0) {
    stats.avgDevicesPerAudit = stats.totalDevicesFound / stats.totalAudits;
    stats.avgAttacksPerAudit = stats.totalAttacksExecuted / stats.totalAudits;
    stats.avgDurationSeconds = stats.totalDuration / stats.totalAudits;
    stats.successPercent = (stats.successCount * 100) / stats.totalAudits;

    // Average signal strength
    int32_t signalSum = 0;
    for (const auto& rec : records) {
      signalSum += rec.maxRSSI;
    }
    stats.avgSignalStrength = signalSum / stats.totalAudits;
  }

  return stats;
}

uint16_t AuditStatistics::getAuditCountByType(uint8_t type) const {
  auto& history = AuditHistory::getInstance();
  auto filtered = history.getRecordsByType(type);
  return filtered.size();
}

int8_t AuditStatistics::getMostFrequentSignal() const {
  auto& history = AuditHistory::getInstance();
  auto records = history.getAllRecords();

  if (records.empty()) return 0;

  int8_t mostFrequent = records[0].maxRSSI;
  uint16_t maxCount = 1;

  for (uint16_t i = 0; i < records.size(); i++) {
    uint16_t count = 1;
    for (uint16_t j = i + 1; j < records.size(); j++) {
      if (records[i].maxRSSI == records[j].maxRSSI) {
        count++;
      }
    }
    if (count > maxCount) {
      maxCount = count;
      mostFrequent = records[i].maxRSSI;
    }
  }

  return mostFrequent;
}

float AuditStatistics::getAuditEfficiency() const {
  auto stats = calculateStats();

  if (stats.totalDuration == 0) return 0.0f;

  return (float)stats.totalDevicesFound / (float)stats.totalDuration;
}

int8_t AuditStatistics::getTrend() const {
  auto& history = AuditHistory::getInstance();
  auto last10 = history.getLastRecords(10);

  if (last10.size() < 2) return 0;

  uint32_t firstHalf = 0, secondHalf = 0;

  for (uint16_t i = 0; i < last10.size() / 2; i++) {
    firstHalf += last10[i].devicesFound;
  }

  for (uint16_t i = last10.size() / 2; i < last10.size(); i++) {
    secondHalf += last10[i].devicesFound;
  }

  if (secondHalf > firstHalf) return 1;   // Increasing
  if (secondHalf < firstHalf) return -1;  // Decreasing
  return 0;                                 // Stable
}

void AuditStatistics::printStatistics() const {
  auto stats = calculateStats();

  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║        AUDIT STATISTICS REPORT         ║");
  Serial.println("╚════════════════════════════════════════╝");

  if (stats.totalAudits == 0) {
    Serial.println("No audit data available.\n");
    return;
  }

  Serial.printf("📊 OVERALL STATS:\n");
  Serial.printf("   Total Audits:     %u\n", stats.totalAudits);
  Serial.printf("   Success Rate:     %u%%\n", stats.successPercent);
  Serial.printf("   Total Duration:   %u seconds\n", stats.totalDuration);

  Serial.printf("\n🎯 BY TYPE:\n");
  Serial.printf("   WiFi Audits:      %u\n", stats.wifiAudits);
  Serial.printf("   BLE Audits:       %u\n", stats.bleAudits);
  Serial.printf("   RF Audits:        %u\n", stats.rfAudits);
  Serial.printf("   IoT Audits:       %u\n", stats.iotAudits);

  Serial.printf("\n🔍 DISCOVERIES:\n");
  Serial.printf("   Total Devices:    %u\n", stats.totalDevicesFound);
  Serial.printf("   Avg per Audit:    %u\n", stats.avgDevicesPerAudit);
  Serial.printf("   Total Attacks:    %u\n", stats.totalAttacksExecuted);
  Serial.printf("   Avg per Audit:    %u\n", stats.avgAttacksPerAudit);

  Serial.printf("\n📶 SIGNAL ANALYSIS:\n");
  Serial.printf("   Best Signal:      %d dBm\n", stats.bestSignal);
  Serial.printf("   Worst Signal:     %d dBm\n", stats.worstSignal);
  Serial.printf("   Average Signal:   %d dBm\n", stats.avgSignalStrength);

  Serial.printf("\n⏱️  PERFORMANCE:\n");
  Serial.printf("   Avg Duration:     %u sec\n", stats.avgDurationSeconds);
  Serial.printf("   Efficiency:       %.2f dev/sec\n", getAuditEfficiency());

  int8_t trend = getTrend();
  Serial.printf("\n📈 TREND:\n");
  Serial.printf("   Direction:        %s\n",
    (trend > 0) ? "↑ Increasing" : (trend < 0) ? "↓ Decreasing" : "→ Stable");

  Serial.println("\n");
}

std::string AuditStatistics::getSummaryString() const {
  auto stats = calculateStats();

  char summary[128];
  snprintf(summary, sizeof(summary),
    "%u audits | %u devices | %u%% success | %.1f dev/s",
    stats.totalAudits,
    stats.totalDevicesFound,
    stats.successPercent,
    getAuditEfficiency()
  );

  return std::string(summary);
}
