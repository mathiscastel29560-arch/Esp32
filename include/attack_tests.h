#ifndef ATTACK_TESTS_H
#define ATTACK_TESTS_H

#include "attack_framework.h"
#include "attack_catalog.h"
#include <cstdint>
#include <vector>

// ============= TEST FRAMEWORK =============
struct TestResult {
  const char* attackName;
  bool passed;
  uint32_t durationMs;
  uint16_t resultsCount;
  AttackStatus finalStatus;
  const char* failureReason;
};

class AttackTestSuite {
public:
  static AttackTestSuite& getInstance() {
    static AttackTestSuite instance;
    return instance;
  }

  // Tests individuels
  void testWiFiAttacks();
  void testBLEAttacks();
  void testRFAttacks();
  void testNFCAttacks();
  void testDoSAttacks();
  void testAdvancedAttacks();

  // Tests completes
  void runAllTests();
  void runCategoryTests(AttackCategory category);
  void runAttackTest(Attack* attack, uint32_t timeout);

  // Résultats
  uint16_t getTestCount() const { return testResults.size(); }
  uint16_t getPassedCount() const;
  uint16_t getFailedCount() const { return getTestCount() - getPassedCount(); }
  float getPassRate() const;

  const TestResult& getResult(uint16_t index) const { return testResults[index]; }

  // Rapport
  void printSummary();
  void printDetailedReport();
  void exportResultsToCSV(const char* filename);

private:
  AttackTestSuite();
  std::vector<TestResult> testResults;

  void recordTest(const char* name, bool passed, uint32_t duration,
                  uint16_t results, AttackStatus status, const char* reason = nullptr);
};

#endif // ATTACK_TESTS_H
