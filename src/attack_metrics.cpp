#include "attack_metrics.h"
#include "async_logger.h"

void AttackMetricsCollector::recordSuccess(uint32_t bytesUsed) {
  currentMetric.successCount++;
  currentMetric.bytesTransmitted += bytesUsed;
  currentMetric.timestamp = millis();
}

void AttackMetricsCollector::recordFailure() {
  currentMetric.failureCount++;
  currentMetric.timestamp = millis();
}

void AttackMetricsCollector::recordPacket(uint16_t bytes) {
  currentMetric.packetsProcessed++;
  currentMetric.bytesTransmitted += bytes;
}

void AttackMetricsCollector::recordLatency(uint32_t latencyMs) {
  currentMetric.currentLatencyMs = latencyMs;
}

void AttackMetricsCollector::recordCPUUsage(uint8_t usage) {
  currentMetric.cpuUsage = (usage > 100) ? 100 : usage;
}

float AttackMetricsCollector::getSuccessRate() const {
  uint32_t total = currentMetric.successCount + currentMetric.failureCount;
  if (total == 0) return 0.0f;
  return ((float)currentMetric.successCount / total) * 100.0f;
}

float AttackMetricsCollector::getThroughputKbps() const {
  if (currentMetric.timestamp == 0) return 0.0f;
  uint32_t elapsed = millis() - sessionStartTime;
  if (elapsed < 1000) return 0.0f;
  return ((float)currentMetric.bytesTransmitted * 8) / (elapsed / 1000.0f) / 1000.0f;
}

uint32_t AttackMetricsCollector::getAverageLatency() const {
  return currentMetric.currentLatencyMs;
}

uint16_t AttackMetricsCollector::getPacketLossPercentage() const {
  uint32_t total = currentMetric.packetsProcessed + currentMetric.failureCount;
  if (total == 0) return 0;
  return ((float)currentMetric.failureCount / total) * 100.0f;
}

void AttackMetricsCollector::update() {
  currentMetric.freeHeap = ESP.getFreeHeap() / 1024;  // KB

  // Add to history every 5 seconds
  static uint32_t lastHistoryAdd = 0;
  if (millis() - lastHistoryAdd > 5000) {
    addToHistory();
    lastHistoryAdd = millis();
  }
}

void AttackMetricsCollector::addToHistory() {
  if (metricsHistory.size() >= maxHistorySize) {
    metricsHistory.erase(metricsHistory.begin());
  }
  metricsHistory.push_back(currentMetric);
}

void AttackMetricsCollector::reset() {
  currentMetric = {};
  metricsHistory.clear();
  sessionStartTime = millis();
  LOG_I("Attack metrics collector reset");
}

void AttackMetricsCollector::printMetrics() {
  Serial.printf("\n=== ATTACK METRICS ===\n");
  Serial.printf("Success Rate: %.1f%% (%u successes, %u failures)\n",
    getSuccessRate(), currentMetric.successCount, currentMetric.failureCount);
  Serial.printf("Throughput: %.2f Kbps\n", getThroughputKbps());
  Serial.printf("Packets: %u processed\n", currentMetric.packetsProcessed);
  Serial.printf("Bytes Transmitted: %u\n", currentMetric.bytesTransmitted);
  Serial.printf("Avg Latency: %ums\n", getAverageLatency());
  Serial.printf("Packet Loss: %u%%\n", getPacketLossPercentage());
  Serial.printf("CPU Usage: %u%%\n", currentMetric.cpuUsage);
  Serial.printf("Free Heap: %u KB\n", currentMetric.freeHeap);
  Serial.println("======================\n");
}

void RealTimeVisualizer::displayStatusBar() {
  AttackMetricsCollector& metrics = AttackMetricsCollector::getInstance();

  if (!showStatusBar) return;

  Serial.printf("\r[");
  drawProgressBar(metrics.getSuccessRate());
  Serial.printf("] %.0f%% | Latency: %ums | Heap: %u KB    ",
    metrics.getSuccessRate(),
    metrics.getAverageLatency(),
    metrics.getLatest().freeHeap);
}

void RealTimeVisualizer::displayMetricsWindow() {
  if (!showMetrics) return;

  AttackMetricsCollector::getInstance().printMetrics();
}

void RealTimeVisualizer::displayProgressBar(uint16_t current, uint16_t total) {
  if (total == 0) total = 1;
  uint8_t percent = (uint16_t)((current * 100) / total);
  drawProgressBar(percent);
}

void RealTimeVisualizer::displayThreatLevel(uint8_t threat) {
  const char* threatNames[] = {"NONE", "MINOR", "MODERATE", "SEVERE", "CRITICAL"};
  const char* threatSymbols[] = {"✓", "⚠", "⚠⚠", "✗", "✗✗"};

  uint8_t level = (threat > 4) ? 4 : threat;
  Serial.printf(" [%s %s] ", threatSymbols[level], threatNames[level]);
}

void RealTimeVisualizer::drawProgressBar(uint8_t percent) {
  uint8_t filled = percent / 10;
  uint8_t empty = 10 - filled;

  Serial.write('[');
  for (uint8_t i = 0; i < filled; i++) Serial.write('=');
  for (uint8_t i = 0; i < empty; i++) Serial.write('-');
  Serial.printf("] %3u%%", percent);
}

void RealTimeVisualizer::updateDisplay() {
  if (millis() - lastUpdateTime < 100) return;  // Update every 100ms
  lastUpdateTime = millis();

  displayStatusBar();
}

void RealTimeVisualizer::clearDisplay() {
  Serial.write('\r');
  for (int i = 0; i < 80; i++) Serial.write(' ');
  Serial.write('\r');
}
