#include "attack_coordinator.h"
#include "async_logger.h"

// ============= ATTACK COORDINATOR =============

void AttackCoordinator::addAttack(const AttackPlanStep& step) {
  if (step.attack) {
    attackPlan.push_back(new AttackPlanStep(step));
  }
}

void AttackCoordinator::removeAttack(uint16_t index) {
  if (index < attackPlan.size()) {
    delete attackPlan[index];
    attackPlan.erase(attackPlan.begin() + index);
  }
}

bool AttackCoordinator::startCoordinatedAttack(CoordinationMode mode) {
  if (isRunning) return false;

  coordinationMode = mode;
  coordinationStatus = AttackStatus::SCANNING;
  currentPhase = AttackPhase::RECONNAISSANCE;
  currentStep = 0;
  startTime = millis();
  completedCount = 0;
  failedCount = 0;
  isRunning = true;

  LOG_I("Starting coordinated attack - Mode: %u, Attacks: %u", mode, attackPlan.size());
  return true;
}

void AttackCoordinator::update() {
  if (!isRunning) return;

  uint32_t elapsed = millis() - startTime;

  // Execute based on coordination mode
  switch (coordinationMode) {
    case CoordinationMode::SEQUENTIAL:
      executePhaseSequential();
      break;
    case CoordinationMode::PARALLEL:
      executePhaseParallel();
      break;
    case CoordinationMode::DISTRIBUTED:
      executePhaseDistributed();
      break;
    case CoordinationMode::ADAPTIVE:
      executePhaseAdaptive();
      break;
    case CoordinationMode::AGGRESSIVE:
      executePhaseAggressive();
      break;
  }

  checkPhaseCompletion();

  // Overall timeout after 5 minutes
  if (elapsed > 300000) {
    isRunning = false;
    coordinationStatus = AttackStatus::SUCCESS;
  }
}

bool AttackCoordinator::stopAll() {
  for (auto* step : attackPlan) {
    if (step->attack && step->attack->isActive()) {
      step->attack->stop();
    }
  }
  isRunning = false;
  LOG_I("Coordinated attack stopped");
  return true;
}

bool AttackCoordinator::emergencyStop() {
  for (auto* step : attackPlan) {
    if (step->attack) {
      step->attack->stop();
    }
  }
  isRunning = false;
  coordinationStatus = AttackStatus::FAILED;
  LOG_E("EMERGENCY STOP - Attack aborted");
  return true;
}

void AttackCoordinator::executePhaseSequential() {
  if (currentStep >= attackPlan.size()) {
    transitionPhase();
    return;
  }

  AttackPlanStep* step = attackPlan[currentStep];

  if (!step->attack->isActive()) {
    uint32_t delay = (millis() - startTime) >= step->delayBefore
                    ? 0 : step->delayBefore - (millis() - startTime);

    if (delay == 0) {
      step->attack->start();
    }
  }

  step->attack->update();

  if (!step->attack->isActive()) {
    if (step->attack->getStatus() == AttackStatus::SUCCESS) {
      handleAttackSuccess(currentStep);
    } else {
      handleAttackFailure(currentStep);
    }
    currentStep++;
  }
}

void AttackCoordinator::executePhaseParallel() {
  // Execute all attacks simultaneously
  for (uint16_t i = 0; i < attackPlan.size(); i++) {
    AttackPlanStep* step = attackPlan[i];

    if (!step->attack->isActive()) {
      step->attack->start();
    }

    step->attack->update();

    if (!step->attack->isActive()) {
      if (step->attack->getStatus() == AttackStatus::SUCCESS) {
        completedCount++;
      } else {
        failedCount++;
      }
    }
  }

  if (completedCount + failedCount >= attackPlan.size()) {
    transitionPhase();
  }
}

void AttackCoordinator::executePhaseDistributed() {
  // Distribute attacks to balance load
  static uint32_t lastExecution = 0;
  uint32_t now = millis();

  if (now - lastExecution > 1000) {
    for (uint16_t i = 0; i < attackPlan.size() && i < 2; i++) {
      if (!attackPlan[i]->attack->isActive()) {
        attackPlan[i]->attack->start();
      }
      attackPlan[i]->attack->update();
    }
    lastExecution = now;
  }
}

void AttackCoordinator::executePhaseAdaptive() {
  // Adapt strategy based on results
  AdaptiveAttackManager& manager = AdaptiveAttackManager::getInstance();

  if (manager.shouldEscalate()) {
    coordinationMode = CoordinationMode::AGGRESSIVE;
  } else if (manager.shouldRetreat()) {
    coordinationMode = CoordinationMode::SEQUENTIAL;
  }

  executePhaseSequential();
}

void AttackCoordinator::executePhaseAggressive() {
  // Maximum resource usage - run everything at once
  for (auto* step : attackPlan) {
    if (!step->attack->isActive()) {
      step->attack->start();
    }
    step->attack->update();
  }
}

void AttackCoordinator::transitionPhase() {
  currentPhase = (AttackPhase)((int)currentPhase + 1);
  currentStep = 0;

  if (currentPhase > AttackPhase::PERSISTENCE) {
    isRunning = false;
    coordinationStatus = AttackStatus::SUCCESS;
  }
}

void AttackCoordinator::checkPhaseCompletion() {
  uint32_t completed = 0;
  for (auto* step : attackPlan) {
    if (!step->attack->isActive()) {
      completed++;
    }
  }

  if (completed == attackPlan.size()) {
    transitionPhase();
  }
}

void AttackCoordinator::handleAttackFailure(uint16_t stepIndex) {
  if (stepIndex < attackPlan.size()) {
    AttackPlanStep* step = attackPlan[stepIndex];
    if (step->onFailure) {
      step->onFailure();
    }

    if (step->critical) {
      LOG_E("Critical attack failed - stopping coordination");
      isRunning = false;
      coordinationStatus = AttackStatus::FAILED;
    } else {
      LOG_W("Attack failed - continuing");
      failedCount++;
    }
  }
}

void AttackCoordinator::handleAttackSuccess(uint16_t stepIndex) {
  if (stepIndex < attackPlan.size()) {
    AttackPlanStep* step = attackPlan[stepIndex];
    if (step->onSuccess) {
      for (uint16_t i = 0; i < step->attack->getResultCount(); i++) {
        step->onSuccess(step->attack->getResult(i));
      }
    }
    completedCount++;
  }
}

void AttackCoordinator::printPlan() {
  Serial.println("\n=== ATTACK PLAN ===");
  for (uint16_t i = 0; i < attackPlan.size(); i++) {
    AttackPlanStep* step = attackPlan[i];
    Serial.printf("[%u] %s (phase: %u, timeout: %lums, critical: %s)\n",
      i, step->attack->getName(), (uint8_t)step->phase,
      step->timeout, step->critical ? "YES" : "NO");
  }
  Serial.println("==================\n");
}

void AttackCoordinator::printProgress() {
  Serial.println("\n=== COORDINATION PROGRESS ===");
  Serial.printf("Status: %u | Phase: %u | Step: %u/%u\n",
    (uint8_t)coordinationStatus, (uint8_t)currentPhase,
    currentStep, attackPlan.size());
  Serial.printf("Completed: %u | Failed: %u\n", completedCount, failedCount);
  Serial.println("=============================\n");
}

// ============= ATTACK CHAIN =============

void AttackChain::addAttack(Attack* attack, uint32_t delayMs) {
  if (attack) {
    chain.push_back(new ChainAttack(attack, delayMs));
  }
}

bool AttackChain::start() {
  if (executing) return false;

  executing = true;
  chainStartTime = millis();
  completedCount = 0;
  LOG_I("Starting attack chain: %s", chainName);
  return true;
}

void AttackChain::update() {
  if (!executing) return;

  uint32_t now = millis();

  for (auto* ca : chain) {
    if (!ca->executed && (now - chainStartTime) >= ca->delay) {
      ca->attack->start();
      ca->executed = true;
    }

    if (ca->executed && ca->attack->isActive()) {
      ca->attack->update();
    }

    if (ca->executed && !ca->attack->isActive()) {
      completedCount++;
    }
  }

  if (completedCount >= chain.size()) {
    executing = false;
  }
}

bool AttackChain::stop() {
  for (auto* ca : chain) {
    if (ca->attack->isActive()) {
      ca->attack->stop();
    }
  }
  executing = false;
  return true;
}

void AttackChain::printStatus() {
  Serial.printf("Chain '%s': %u/%u completed\n", chainName, completedCount, chain.size());
}

// ============= ADAPTIVE ATTACK MANAGER =============

void AdaptiveAttackManager::recordAttackResult(const char* attackName, bool success,
                                              uint32_t durationMs, uint16_t resultsCount) {
  for (auto* m : metrics) {
    if (strcmp(m->name, attackName) == 0) {
      m->attempts++;
      if (success) m->successes++;
      m->totalDuration += durationMs;
      m->totalResults += resultsCount;
      return;
    }
  }

  AttackMetrics* m = new AttackMetrics();
  m->name = attackName;
  m->attempts = 1;
  m->successes = success ? 1 : 0;
  m->totalDuration = durationMs;
  m->totalResults = resultsCount;
  metrics.push_back(m);
}

bool AdaptiveAttackManager::shouldEscalate() const {
  return getSuccessRate() > 80.0f && getEfficiencyScore() > 0.8f;
}

bool AdaptiveAttackManager::shouldRetreat() const {
  return getSuccessRate() < 30.0f || getEfficiencyScore() < 0.3f;
}

bool AdaptiveAttackManager::shouldChangeStrategy() const {
  return getEfficiencyScore() < 0.5f;
}

float AdaptiveAttackManager::getSuccessRate() const {
  if (totalAttempts == 0) return 0.0f;
  return ((float)successfulAttempts / totalAttempts) * 100.0f;
}

float AdaptiveAttackManager::getEfficiencyScore() const {
  if (totalDuration == 0) return 0.0f;
  float avgResultsPerMs = ((float)successfulAttempts) / (totalDuration / 1000.0f);
  return (avgResultsPerMs / 100.0f) * 100.0f;
}

void AdaptiveAttackManager::printAnalytics() {
  Serial.println("\n=== ADAPTIVE ATTACK ANALYTICS ===");
  Serial.printf("Success Rate: %.1f%%\n", getSuccessRate());
  Serial.printf("Efficiency: %.1f%%\n", getEfficiencyScore());
  Serial.printf("Escalation Level: %u\n", escalationLevel);

  Serial.println("Attack Metrics:");
  for (auto* m : metrics) {
    float rate = (float)m->successes / m->attempts * 100.0f;
    Serial.printf("  %s: %u/%u (%.0f%%), Avg time: %lums\n",
      m->name, m->successes, m->attempts, rate,
      m->totalDuration / (m->attempts ? m->attempts : 1));
  }
  Serial.println("==================================\n");
}
