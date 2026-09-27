#include "retry_manager.h"
#include "async_logger.h"

void RetryManager::setRetryConfig(const RetryConfig& cfg) {
  config = cfg;
  LOG_I("Retry config set - strategy: %u, max retries: %u, initial delay: %ums",
    (uint8_t)cfg.strategy, cfg.maxRetries, cfg.initialDelayMs);
}

bool RetryManager::shouldRetry(uint8_t attemptNumber) const {
  if (attemptNumber >= config.maxRetries) {
    LOG_E("Max retries (%u) exceeded", config.maxRetries);
    return false;
  }
  return true;
}

uint32_t RetryManager::getNextRetryDelay(uint8_t attemptNumber) {
  uint32_t delay = calculateDelay(attemptNumber);
  delay = addJitter(delay);

  LOG_I("Retry %u: delay %ums", attemptNumber + 1, delay);
  return delay;
}

uint32_t RetryManager::calculateDelay(uint8_t attemptNumber) {
  uint32_t delay = config.initialDelayMs;

  switch (config.strategy) {
    case RetryStrategy::LINEAR:
      delay = config.initialDelayMs * (attemptNumber + 1);
      break;

    case RetryStrategy::EXPONENTIAL:
      delay = config.initialDelayMs;
      for (uint8_t i = 0; i < attemptNumber; i++) {
        delay *= 2;
        if (delay > config.maxDelayMs) delay = config.maxDelayMs;
      }
      break;

    case RetryStrategy::FIBONACCI:
      delay = config.initialDelayMs * fibonacci(attemptNumber + 1);
      break;

    case RetryStrategy::ADAPTIVE:
      delay = adaptiveTimeout;
      break;
  }

  return (delay > config.maxDelayMs) ? config.maxDelayMs : delay;
}

uint32_t RetryManager::addJitter(uint32_t delay) {
  if (!config.jitterEnabled) return delay;

  uint32_t jitterAmount = (delay * config.jitterPercent) / 100;
  uint32_t randomJitter = random(0, jitterAmount);

  return delay + randomJitter;
}

uint32_t RetryManager::fibonacci(uint8_t n) {
  if (n <= 1) return 1;
  if (n == 2) return 1;

  uint32_t a = 1, b = 1;
  for (uint8_t i = 2; i < n; i++) {
    uint32_t temp = a + b;
    a = b;
    b = temp;
  }
  return b;
}

void RetryManager::recordResponseTime(uint32_t responseTimeMs) {
  if (responseTimeHistory.size() >= maxHistorySize) {
    responseTimeHistory.erase(responseTimeHistory.begin());
  }
  responseTimeHistory.push_back(responseTimeMs);
  lastResponseTime = responseTimeMs;

  adjustTimeoutBasedOnHistory();
}

void RetryManager::adjustTimeoutBasedOnHistory() {
  if (responseTimeHistory.empty()) return;

  // Calculate average + standard deviation
  uint32_t sum = 0;
  for (uint32_t time : responseTimeHistory) {
    sum += time;
  }
  uint32_t average = sum / responseTimeHistory.size();

  // Set adaptive timeout to average + 50%
  adaptiveTimeout = average + (average / 2);

  // Cap at max delay
  if (adaptiveTimeout > config.maxDelayMs) {
    adaptiveTimeout = config.maxDelayMs;
  }
}

void RetryManager::printRetryStats() {
  Serial.printf("\n=== RETRY MANAGER STATS ===\n");

  const char* strategyNames[] = {"LINEAR", "EXPONENTIAL", "FIBONACCI", "ADAPTIVE"};
  Serial.printf("Strategy: %s\n", strategyNames[(uint8_t)config.strategy]);
  Serial.printf("Initial Delay: %ums\n", config.initialDelayMs);
  Serial.printf("Max Delay: %ums\n", config.maxDelayMs);
  Serial.printf("Max Retries: %u\n", config.maxRetries);
  Serial.printf("Jitter: %s (%u%%)\n", config.jitterEnabled ? "ENABLED" : "DISABLED", config.jitterPercent);
  Serial.printf("Adaptive Timeout: %ums\n", adaptiveTimeout);
  Serial.printf("Response Time History: %u entries\n", responseTimeHistory.size());

  Serial.println("===========================\n");
}
