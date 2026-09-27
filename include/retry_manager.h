#ifndef RETRY_MANAGER_H
#define RETRY_MANAGER_H

#include <Arduino.h>
#include <vector>

// ============= EXPONENTIAL BACKOFF RETRY =============

enum class RetryStrategy {
  LINEAR = 0,           // Delay increases linearly
  EXPONENTIAL = 1,      // Delay doubles each time
  FIBONACCI = 2,        // Delay follows Fibonacci sequence
  ADAPTIVE = 3          // Adjusts based on response patterns
};

struct RetryConfig {
  RetryStrategy strategy;
  uint16_t initialDelayMs;
  uint16_t maxDelayMs;
  uint8_t maxRetries;
  bool jitterEnabled;
  uint8_t jitterPercent;     // 0-50
};

class RetryManager {
public:
  static RetryManager& getInstance() {
    static RetryManager instance;
    return instance;
  }

  // Configure retry behavior
  void setRetryConfig(const RetryConfig& config);

  // Check if should retry
  bool shouldRetry(uint8_t attemptNumber) const;
  uint32_t getNextRetryDelay(uint8_t attemptNumber);

  // Adaptive timeout adjustment
  void recordResponseTime(uint32_t responseTimeMs);
  void adjustTimeoutBasedOnHistory();

  uint16_t getAdaptiveTimeout() const { return adaptiveTimeout; }
  void printRetryStats();

private:
  RetryManager() : adaptiveTimeout(1000) {}

  RetryConfig config;
  uint32_t lastResponseTime;
  std::vector<uint32_t> responseTimeHistory;
  uint16_t adaptiveTimeout;
  uint16_t maxHistorySize = 20;

  uint32_t calculateDelay(uint8_t attemptNumber);
  uint32_t addJitter(uint32_t delay);
  uint32_t fibonacci(uint8_t n);
};

#endif
