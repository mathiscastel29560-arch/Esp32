#ifndef ATTACK_METRICS_H
#define ATTACK_METRICS_H

#include <Arduino.h>
#include <vector>

// ============= REAL-TIME METRICS COLLECTION =============

struct AttackMetric {
  uint32_t timestamp;
  uint32_t successCount;
  uint32_t failureCount;
  uint32_t packetsProcessed;
  uint32_t bytesTransmitted;
  uint16_t currentLatencyMs;
  uint8_t cpuUsage;         // 0-100%
  uint16_t freeHeap;
};

class AttackMetricsCollector {
public:
  static AttackMetricsCollector& getInstance() {
    static AttackMetricsCollector instance;
    return instance;
  }

  void recordSuccess(uint32_t bytesUsed = 0);
  void recordFailure();
  void recordPacket(uint16_t bytes);
  void recordLatency(uint32_t latencyMs);
  void recordCPUUsage(uint8_t usage);

  float getSuccessRate() const;
  float getThroughputKbps() const;
  uint32_t getAverageLatency() const;
  uint16_t getPacketLossPercentage() const;

  void update();
  void printMetrics();
  void reset();

  const std::vector<AttackMetric>& getHistory() const { return metricsHistory; }
  const AttackMetric& getLatest() const { return currentMetric; }

private:
  AttackMetricsCollector() : metricsHistory(), currentMetric() {}

  AttackMetric currentMetric;
  std::vector<AttackMetric> metricsHistory;
  uint32_t sessionStartTime;
  uint16_t maxHistorySize = 100;

  void addToHistory();
};

// ============= REAL-TIME VISUALIZER =============

class RealTimeVisualizer {
public:
  static RealTimeVisualizer& getInstance() {
    static RealTimeVisualizer instance;
    return instance;
  }

  void displayStatusBar();
  void displayProgressBar(uint16_t current, uint16_t total);
  void displayMetricsWindow();
  void displayThreatLevel(uint8_t threat);

  void updateDisplay();
  void clearDisplay();

  void setMetricsVisible(bool visible) { showMetrics = visible; }
  void setStatusBarVisible(bool visible) { showStatusBar = visible; }

private:
  RealTimeVisualizer() : showMetrics(true), showStatusBar(true),
                        lastUpdateTime(0) {}

  bool showMetrics;
  bool showStatusBar;
  uint32_t lastUpdateTime;

  void drawProgressBar(uint8_t percent);
  void drawThreatIndicator(uint8_t level);
};

#endif
