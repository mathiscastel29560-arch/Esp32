#include "attack_tests.h"
#include "logging_system.h"

AttackTestSuite::AttackTestSuite() {
  Logger::getInstance().info("TestSuite", "Suite de tests initialisée");
}

uint16_t AttackTestSuite::getPassedCount() const {
  uint16_t count = 0;
  for (const auto& result : testResults) {
    if (result.passed) count++;
  }
  return count;
}

float AttackTestSuite::getPassRate() const {
  if (testResults.empty()) return 0.0f;
  return (getPassedCount() * 100.0f) / testResults.size();
}

void AttackTestSuite::runAttackTest(Attack* attack, uint32_t timeout) {
  if (!attack) {
    Serial.println("❌ Attaque null");
    return;
  }

  Serial.printf("🧪 Test: %s...", attack->getName());

  uint32_t start = millis();
  bool initialized = attack->begin();

  if (!initialized) {
    recordTest(attack->getName(), false, 0, 0, AttackStatus::ERROR,
               "Initialisation échouée");
    Serial.println(" FAILED (init)");
    return;
  }

  attack->start();

  while (attack->isRunning() && (millis() - start) < timeout) {
    attack->update();
    delay(50);
  }

  if (attack->isRunning()) {
    attack->stop();
  }

  uint32_t duration = millis() - start;
  uint16_t resultCount = attack->getResultCount();
  AttackStatus status = attack->getCurrentStatus();

  bool passed = (status == AttackStatus::SUCCESS ||
                 status == AttackStatus::PARTIAL) &&
                resultCount > 0;

  recordTest(attack->getName(), passed, duration, resultCount, status);

  Serial.printf(" %s (%u ms, %u résultats)\n",
                passed ? "✓" : "✗", duration, resultCount);

  delete attack;
}

void AttackTestSuite::testWiFiAttacks() {
  Serial.println("\n========== WIFI TESTS ==========");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  for (uint8_t i = 0; i < 7; i++) {
    Attack* attack = catalog.createWiFiAttack(i);
    runAttackTest(attack, 16000); // 15 sec + buffer
  }
}

void AttackTestSuite::testBLEAttacks() {
  Serial.println("\n========== BLE TESTS ==========");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  for (uint8_t i = 0; i < 7; i++) {
    Attack* attack = catalog.createBLEAttack(i);
    runAttackTest(attack, 16000);
  }
}

void AttackTestSuite::testRFAttacks() {
  Serial.println("\n========== RF TESTS ==========");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  for (uint8_t i = 0; i < 8; i++) {
    Attack* attack = catalog.createRFAttack(i);
    runAttackTest(attack, 31000); // 30 sec + buffer
  }
}

void AttackTestSuite::testNFCAttacks() {
  Serial.println("\n========== NFC TESTS ==========");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  for (uint8_t i = 0; i < 7; i++) {
    Attack* attack = catalog.createNFCAttack(i);
    runAttackTest(attack, 31000);
  }
}

void AttackTestSuite::testDoSAttacks() {
  Serial.println("\n========== DOS TESTS ==========");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  for (uint8_t i = 0; i < 6; i++) {
    Attack* attack = catalog.createDOSAttack(i);
    runAttackTest(attack, 21000); // 20 sec + buffer
  }
}

void AttackTestSuite::testAdvancedAttacks() {
  Serial.println("\n========== ADVANCED TESTS ==========");
  AttackCatalog& catalog = AttackCatalog::getInstance();

  for (uint8_t i = 0; i < 8; i++) {
    Attack* attack = catalog.createAdvancedAttack(i);
    runAttackTest(attack, 31000);
  }
}

void AttackTestSuite::runAllTests() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║   SUITE TESTS COMPLÈTE (43 attaques)  ║");
  Serial.println("╚════════════════════════════════════╝\n");

  testWiFiAttacks();
  testBLEAttacks();
  testRFAttacks();
  testNFCAttacks();
  testDoSAttacks();
  testAdvancedAttacks();

  printSummary();
}

void AttackTestSuite::runCategoryTests(AttackCategory category) {
  switch (category) {
    case AttackCategory::WIFI:
      testWiFiAttacks();
      break;
    case AttackCategory::BLE:
      testBLEAttacks();
      break;
    case AttackCategory::RF:
      testRFAttacks();
      break;
    case AttackCategory::NFC:
      testNFCAttacks();
      break;
    case AttackCategory::DOS:
      testDoSAttacks();
      break;
    case AttackCategory::ADVANCED:
      testAdvancedAttacks();
      break;
    default:
      Serial.println("Catégorie inconnue");
      break;
  }

  printSummary();
}

void AttackTestSuite::recordTest(const char* name, bool passed, uint32_t duration,
                                 uint16_t results, AttackStatus status,
                                 const char* reason) {
  TestResult result = {
    name,
    passed,
    duration,
    results,
    status,
    reason ? reason : ""
  };
  testResults.push_back(result);
}

void AttackTestSuite::printSummary() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║         RÉSUMÉ TESTS               ║");
  Serial.println("╚════════════════════════════════════╝\n");

  Serial.printf("Total:    %u tests\n", getTestCount());
  Serial.printf("✓ Passés: %u tests (%.1f%%)\n", getPassedCount(), getPassRate());
  Serial.printf("✗ Échoués: %u tests\n", getFailedCount());

  if (getFailedCount() > 0) {
    Serial.println("\n⚠️  Tests échoués:");
    for (const auto& result : testResults) {
      if (!result.passed) {
        Serial.printf("  - %s (%s)\n", result.attackName, result.failureReason);
      }
    }
  }

  Serial.println("\n");
}

void AttackTestSuite::printDetailedReport() {
  Serial.println("\n╔════════════════════════════════════════════════════════╗");
  Serial.println("║         RAPPORT DÉTAILLÉ                               ║");
  Serial.println("╚════════════════════════════════════════════════════════╝\n");

  Serial.printf("%-30s | Status | Duration | Results\n", "Attaque");
  Serial.println("─────────────────────────────────┼────────┼──────────┼─────────");

  for (const auto& result : testResults) {
    const char* status_str = result.passed ? "✓ PASS" : "✗ FAIL";
    Serial.printf("%-30s | %s | %5u ms | %u\n",
                  result.attackName,
                  status_str,
                  result.durationMs,
                  result.resultsCount);
  }

  Serial.println("\n");
  printSummary();
}

void AttackTestSuite::exportResultsToCSV(const char* filename) {
  Serial.printf("Exportation résultats vers %s\n", filename);

  FILE* file = fopen(filename, "w");
  if (!file) {
    Serial.println("❌ Impossible d'ouvrir fichier");
    return;
  }

  // En-tête
  fprintf(file, "Attaque,Status,Durée(ms),Résultats,StatusEnum\n");

  // Données
  for (const auto& result : testResults) {
    const char* status = result.passed ? "PASS" : "FAIL";
    fprintf(file, "%s,%s,%u,%u,%u\n",
            result.attackName,
            status,
            result.durationMs,
            result.resultsCount,
            (uint8_t)result.finalStatus);
  }

  fclose(file);
  Serial.printf("✓ Exporté %u résultats\n", testResults.size());
}
