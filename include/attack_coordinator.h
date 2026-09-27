#ifndef ATTACK_COORDINATOR_H
#define ATTACK_COORDINATOR_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include "attack_framework.h"

// ============= ATTACK COORDINATION MODES =============

enum class CoordinationMode {
  SEQUENTIAL = 0,      // One attack at a time
  PARALLEL = 1,        // Multiple simultaneous
  DISTRIBUTED = 2,     // Load balanced
  ADAPTIVE = 3,        // Adapt based on results
  AGGRESSIVE = 4       // Maximum resource usage
};

enum class AttackPhase {
  RECONNAISSANCE = 0,
  PREPARATION = 1,
  EXECUTION = 2,
  EXPLOITATION = 3,
  PERSISTENCE = 4
};

// ============= COORDINATED ATTACK PLAN =============

struct AttackPlanStep {
  Attack* attack;
  AttackPhase phase;
  uint32_t delayBefore;    // ms delay before starting
  uint32_t timeout;        // Max duration
  bool critical;           // Fail if this fails?
  std::function<void(AttackResult*)> onSuccess;
  std::function<void()> onFailure;

  AttackPlanStep(Attack* a, AttackPhase p = AttackPhase::RECONNAISSANCE)
    : attack(a), phase(p), delayBefore(0), timeout(60000),
      critical(false), onSuccess(nullptr), onFailure(nullptr) {}
};

// ============= MULTI-STAGE ATTACK COORDINATOR =============

class AttackCoordinator {
public:
  static AttackCoordinator& getInstance() {
    static AttackCoordinator instance;
    return instance;
  }

  // Add attack to plan
  void addAttack(const AttackPlanStep& step);

  // Remove attack by index
  void removeAttack(uint16_t index);

  // Start coordinated attack execution
  bool startCoordinatedAttack(CoordinationMode mode = CoordinationMode::ADAPTIVE);

  // Update coordinator (call from main loop)
  void update();

  // Stop all attacks gracefully
  bool stopAll();

  // Emergency stop (immediate)
  bool emergencyStop();

  // Get attack status
  AttackStatus getCoordinationStatus() const { return coordinationStatus; }

  // Get current phase
  AttackPhase getCurrentPhase() const { return currentPhase; }

  // Statistics
  uint16_t getTotalPlannedAttacks() const { return attackPlan.size(); }
  uint16_t getCompletedAttacks() const { return completedCount; }
  uint16_t getFailedAttacks() const { return failedCount; }

  // Print plan
  void printPlan();
  void printProgress();

private:
  AttackCoordinator() : coordinationStatus(AttackStatus::IDLE),
                       currentPhase(AttackPhase::RECONNAISSANCE),
                       coordinationMode(CoordinationMode::ADAPTIVE),
                       currentStep(0), startTime(0),
                       completedCount(0), failedCount(0),
                       isRunning(false) {}

  std::vector<AttackPlanStep*> attackPlan;
  AttackStatus coordinationStatus;
  AttackPhase currentPhase;
  CoordinationMode coordinationMode;
  uint16_t currentStep;
  uint32_t startTime;
  uint16_t completedCount;
  uint16_t failedCount;
  bool isRunning;

  void executePhaseSequential();
  void executePhaseParallel();
  void executePhaseDistributed();
  void executePhaseAdaptive();
  void executePhaseAggressive();

  void transitionPhase();
  void checkPhaseCompletion();
  void handleAttackFailure(uint16_t stepIndex);
  void handleAttackSuccess(uint16_t stepIndex);
};

// ============= ATTACK CHAIN EXECUTOR =============

class AttackChain {
public:
  AttackChain(const char* name) : chainName(name), executing(false),
                                 chainStartTime(0) {}

  void addAttack(Attack* attack, uint32_t delayMs = 0);

  bool start();
  void update();
  bool stop();

  bool isExecuting() const { return executing; }
  uint16_t getAttackCount() const { return chain.size(); }
  uint16_t getCompletedCount() const { return completedCount; }

  void printStatus();

private:
  struct ChainAttack {
    Attack* attack;
    uint32_t delay;
    uint32_t scheduledTime;
    bool executed;

    ChainAttack(Attack* a, uint32_t d)
      : attack(a), delay(d), scheduledTime(0), executed(false) {}
  };

  char chainName[64];
  std::vector<ChainAttack*> chain;
  bool executing;
  uint32_t chainStartTime;
  uint16_t completedCount;
};

// ============= ADAPTIVE ATTACK MANAGER =============

class AdaptiveAttackManager {
public:
  static AdaptiveAttackManager& getInstance() {
    static AdaptiveAttackManager instance;
    return instance;
  }

  void recordAttackResult(const char* attackName, bool success,
                         uint32_t durationMs, uint16_t resultsCount);

  void selectNextAttack();

  bool shouldEscalate() const;
  bool shouldRetreat() const;
  bool shouldChangeStrategy() const;

  float getSuccessRate() const;
  float getEfficiencyScore() const;

  void printAnalytics();

private:
  AdaptiveAttackManager() : totalAttempts(0), successfulAttempts(0),
                           totalDuration(0), escalationLevel(0) {}

  struct AttackMetrics {
    const char* name;
    uint16_t attempts;
    uint16_t successes;
    uint32_t totalDuration;
    uint16_t totalResults;
  };

  std::vector<AttackMetrics*> metrics;
  uint32_t totalAttempts;
  uint32_t successfulAttempts;
  uint32_t totalDuration;
  uint8_t escalationLevel;
};

#endif
