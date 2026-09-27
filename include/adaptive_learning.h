#ifndef ADAPTIVE_LEARNING_H
#define ADAPTIVE_LEARNING_H

#include <Arduino.h>
#include <vector>

// ============= ATTACK PATTERN LEARNING =============

struct AttackPattern {
  uint32_t patternId;
  const char* attackName;
  uint8_t successRate;      // 0-100%
  uint16_t averageLatency;
  uint16_t averageBandwidth;
  uint8_t detectionRisk;    // 0-100%
  uint32_t lastUsed;
  uint16_t timesUsed;
};

class AttackLearner {
public:
  static AttackLearner& getInstance() {
    static AttackLearner instance;
    return instance;
  }

  // Record attack attempt results
  void recordAttackResult(const char* attackName, bool successful, uint16_t latencyMs,
                         uint16_t bandwidthKbps, uint8_t detectionRisk);

  // Query learned patterns
  const AttackPattern* getPattern(const char* attackName) const;
  const AttackPattern* getBestPattern() const;
  const AttackPattern* getLeastDetectablePattern() const;

  // Optimization suggestions
  float getSuccessProbability(const char* attackName) const;
  uint8_t getRiskLevel(const char* attackName) const;

  void printLearningStats();
  void printPatterns();
  void exportPatterns();

private:
  AttackLearner() : nextPatternId(1) {}

  std::vector<AttackPattern> patterns;
  uint32_t nextPatternId;
  uint16_t maxPatterns = 50;

  AttackPattern* findOrCreatePattern(const char* attackName);
};

// ============= STRATEGY OPTIMIZER =============

class StrategyOptimizer {
public:
  static StrategyOptimizer& getInstance() {
    static StrategyOptimizer instance;
    return instance;
  }

  // Automatic attack sequencing
  const AttackPattern* selectNextAttack(uint8_t targetType, uint8_t availableResources);
  const AttackPattern* selectBestAttack(bool prioritizeSpeed, bool prioritizeStealth);

  // Adaptive strategy adjustment
  void adjustStrategy(uint8_t currentSuccessRate);
  void escalateStrategy();
  void retreatStrategy();

  uint8_t getCurrentAggression() const { return aggressionLevel; }
  uint8_t getCurrentStealth() const { return stealthLevel; }

  void printStrategy();

private:
  StrategyOptimizer() : aggressionLevel(50), stealthLevel(50) {}

  uint8_t aggressionLevel;  // 0-100
  uint8_t stealthLevel;     // 0-100
};

#endif
