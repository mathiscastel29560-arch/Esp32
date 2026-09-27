#include "adaptive_learning.h"
#include "async_logger.h"

void AttackLearner::recordAttackResult(const char* attackName, bool successful, uint16_t latencyMs,
                                       uint16_t bandwidthKbps, uint8_t detectionRisk) {
  AttackPattern* pattern = findOrCreatePattern(attackName);

  // Update running average
  uint16_t totalTests = pattern->timesUsed + 1;
  pattern->successRate = ((pattern->successRate * pattern->timesUsed) + (successful ? 100 : 0)) / totalTests;
  pattern->averageLatency = ((pattern->averageLatency * pattern->timesUsed) + latencyMs) / totalTests;
  pattern->averageBandwidth = ((pattern->averageBandwidth * pattern->timesUsed) + bandwidthKbps) / totalTests;
  pattern->detectionRisk = ((pattern->detectionRisk * pattern->timesUsed) + detectionRisk) / totalTests;

  pattern->lastUsed = millis();
  pattern->timesUsed++;

  LOG_I("Attack '%s' - Success: %s, Success Rate: %u%%, Risk: %u%%",
    attackName, successful ? "YES" : "NO", pattern->successRate, pattern->detectionRisk);
}

AttackPattern* AttackLearner::findOrCreatePattern(const char* attackName) {
  // Find existing pattern
  for (auto& pattern : patterns) {
    if (strcmp(pattern.attackName, attackName) == 0) {
      return &pattern;
    }
  }

  // Create new pattern if space available
  if (patterns.size() < maxPatterns) {
    AttackPattern newPattern = {};
    newPattern.patternId = nextPatternId++;
    newPattern.attackName = attackName;
    newPattern.successRate = 50;
    newPattern.averageLatency = 500;
    newPattern.averageBandwidth = 100;
    newPattern.detectionRisk = 50;
    newPattern.lastUsed = millis();
    newPattern.timesUsed = 0;

    patterns.push_back(newPattern);
    LOG_I("Created new pattern for attack: %s", attackName);
    return &patterns.back();
  }

  LOG_W("Pattern database full, removing oldest");
  // Remove pattern with lowest success rate
  uint16_t worstIdx = 0;
  for (uint16_t i = 1; i < patterns.size(); i++) {
    if (patterns[i].successRate < patterns[worstIdx].successRate) {
      worstIdx = i;
    }
  }
  patterns.erase(patterns.begin() + worstIdx);

  return findOrCreatePattern(attackName);
}

const AttackPattern* AttackLearner::getPattern(const char* attackName) const {
  for (const auto& pattern : patterns) {
    if (strcmp(pattern.attackName, attackName) == 0) {
      return &pattern;
    }
  }
  return nullptr;
}

const AttackPattern* AttackLearner::getBestPattern() const {
  if (patterns.empty()) return nullptr;

  const AttackPattern* best = &patterns[0];
  for (const auto& pattern : patterns) {
    if (pattern.successRate > best->successRate) {
      best = &pattern;
    }
  }
  return best;
}

const AttackPattern* AttackLearner::getLeastDetectablePattern() const {
  if (patterns.empty()) return nullptr;

  const AttackPattern* stealthiest = &patterns[0];
  for (const auto& pattern : patterns) {
    if (pattern.detectionRisk < stealthiest->detectionRisk) {
      stealthiest = &pattern;
    }
  }
  return stealthiest;
}

float AttackLearner::getSuccessProbability(const char* attackName) const {
  const AttackPattern* pattern = getPattern(attackName);
  return pattern ? (float)pattern->successRate / 100.0f : 0.5f;
}

uint8_t AttackLearner::getRiskLevel(const char* attackName) const {
  const AttackPattern* pattern = getPattern(attackName);
  return pattern ? pattern->detectionRisk : 50;
}

void AttackLearner::printLearningStats() {
  Serial.printf("\n=== ATTACK LEARNING STATISTICS ===\n");
  Serial.printf("Total Patterns Learned: %u\n", patterns.size());

  uint32_t totalAttacks = 0;
  uint8_t avgSuccess = 0;

  for (const auto& pattern : patterns) {
    totalAttacks += pattern.timesUsed;
    avgSuccess += pattern.successRate;
  }

  if (!patterns.empty()) {
    avgSuccess /= patterns.size();
  }

  Serial.printf("Total Attacks Performed: %u\n", totalAttacks);
  Serial.printf("Average Success Rate: %u%%\n", avgSuccess);
  Serial.println("===================================\n");
}

void AttackLearner::printPatterns() {
  Serial.printf("\n=== LEARNED ATTACK PATTERNS ===\n");
  for (const auto& pattern : patterns) {
    Serial.printf("[%u] %s\n", pattern.patternId, pattern.attackName);
    Serial.printf("    Success: %u%% | Risk: %u%% | Latency: %ums | Used: %u times\n",
      pattern.successRate, pattern.detectionRisk, pattern.averageLatency, pattern.timesUsed);
  }
  Serial.println("================================\n");
}

void AttackLearner::exportPatterns() {
  Serial.println("\n[PATTERNS_EXPORT_START]");
  for (const auto& pattern : patterns) {
    Serial.printf("%s,%u,%u,%u,%u\n",
      pattern.attackName, pattern.successRate, pattern.detectionRisk, pattern.averageLatency, pattern.timesUsed);
  }
  Serial.println("[PATTERNS_EXPORT_END]\n");
}

void StrategyOptimizer::adjustStrategy(uint8_t currentSuccessRate) {
  if (currentSuccessRate > 80) {
    escalateStrategy();
  } else if (currentSuccessRate < 30) {
    retreatStrategy();
  } else {
    // Maintain current strategy
    LOG_I("Strategy optimal - success rate: %u%%", currentSuccessRate);
  }
}

void StrategyOptimizer::escalateStrategy() {
  aggressionLevel = (aggressionLevel < 100) ? aggressionLevel + 10 : 100;
  LOG_I("Escalating strategy - aggression: %u%%", aggressionLevel);
}

void StrategyOptimizer::retreatStrategy() {
  aggressionLevel = (aggressionLevel > 0) ? aggressionLevel - 10 : 0;
  stealthLevel = (stealthLevel < 100) ? stealthLevel + 10 : 100;
  LOG_W("Retreating strategy - stealth: %u%%", stealthLevel);
}

const AttackPattern* StrategyOptimizer::selectNextAttack(uint8_t targetType, uint8_t availableResources) {
  AttackLearner& learner = AttackLearner::getInstance();

  if (aggressionLevel > 70) {
    return learner.getBestPattern();
  } else if (stealthLevel > 70) {
    return learner.getLeastDetectablePattern();
  } else {
    return learner.getBestPattern();
  }
}

const AttackPattern* StrategyOptimizer::selectBestAttack(bool prioritizeSpeed, bool prioritizeStealth) {
  AttackLearner& learner = AttackLearner::getInstance();

  if (prioritizeStealth) {
    return learner.getLeastDetectablePattern();
  } else {
    return learner.getBestPattern();
  }
}

void StrategyOptimizer::printStrategy() {
  Serial.printf("\n=== CURRENT STRATEGY ===\n");
  Serial.printf("Aggression Level: %u%%\n", aggressionLevel);
  Serial.printf("Stealth Level: %u%%\n", stealthLevel);
  Serial.println("=======================\n");
}
