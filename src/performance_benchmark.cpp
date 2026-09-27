#include "performance_benchmark.h"
#include "logging_system.h"
#include <esp_heap_caps.h>

PerformanceBenchmark::PerformanceBenchmark() {
  Logger::getInstance().info("Benchmark", "Benchmark de performance initialisé");
}

uint32_t PerformanceBenchmark::getHeapUsage() {
  return esp_get_free_heap_size();
}

void PerformanceBenchmark::benchmarkAttack(Attack* attack, uint32_t timeout) {
  if (!attack) {
    Serial.println("❌ Attaque null");
    return;
  }

  Serial.printf("⏱️  Benchmark: %s...", attack->getName());

  uint32_t heapBefore = getHeapUsage();
  uint32_t start = millis();

  attack->begin();
  attack->start();

  while (attack->isRunning() && (millis() - start) < timeout) {
    attack->update();
    delay(50);
  }

  if (attack->isRunning()) {
    attack->stop();
  }

  uint32_t duration = millis() - start;
  uint32_t heapAfter = getHeapUsage();
  uint32_t heapUsed = heapBefore - heapAfter;
  uint16_t resultCount = attack->getResultCount();

  recordBenchmark(attack->getName(), duration, resultCount);

  Serial.printf(" %u ms, %u bytes heap\n", duration, heapUsed);

  delete attack;
}

void PerformanceBenchmark::benchmarkCategory(AttackCategory category) {
  AttackCatalog& catalog = AttackCatalog::getInstance();
  uint8_t count = catalog.getAttackCountByCategory(category);

  Serial.printf("\n📊 Benchmark Catégorie: %s\n", catalog.getCategoryName(category));
  Serial.println("─────────────────────────────────────────");

  for (uint8_t i = 0; i < count; i++) {
    Attack* attack = catalog.createAttack(category, i);
    if (attack) {
      uint32_t timeout = 31000; // Timeout par défaut
      benchmarkAttack(attack, timeout);
    }
  }
}

void PerformanceBenchmark::benchmarkAllAttacks() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║   BENCHMARK COMPLET (43 attaques)   ║");
  Serial.println("╚════════════════════════════════════╝\n");

  uint32_t totalStart = millis();

  for (int i = 0; i < 6; i++) {
    benchmarkCategory((AttackCategory)i);
  }

  uint32_t totalDuration = millis() - totalStart;

  Serial.printf("\n⏱️  Durée totale: %u ms (%.1f min)\n",
                totalDuration, totalDuration / 60000.0f);

  printSummary();
}

void PerformanceBenchmark::recordBenchmark(const char* name, uint32_t duration,
                                           uint16_t resultCount) {
  // Chercher si attaque déjà benchmark
  for (auto& result : results) {
    if (strcmp(result.attackName, name) == 0) {
      // Mettre à jour stats
      if (duration < result.minDurationMs) result.minDurationMs = duration;
      if (duration > result.maxDurationMs) result.maxDurationMs = duration;
      result.avgDurationMs = (result.avgDurationMs + duration) / 2;
      result.resultCount = resultCount;
      return;
    }
  }

  // Nouvelle entrée
  BenchmarkResult result = {
    name,
    duration,      // min
    duration,      // max
    duration,      // avg
    0, 0, 0,       // heap stats (non implémentés)
    resultCount
  };
  results.push_back(result);
}

void PerformanceBenchmark::printSummary() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║      RÉSUMÉ BENCHMARK               ║");
  Serial.println("╚════════════════════════════════════╝\n");

  Serial.printf("Attaques benchmark: %u\n", getResultCount());

  uint32_t totalDuration = 0;
  uint16_t totalResults = 0;

  for (const auto& result : results) {
    totalDuration += result.avgDurationMs;
    totalResults += result.resultCount;
  }

  Serial.printf("Durée moyenne par attaque: %.1f ms\n",
                totalDuration / (float)getResultCount());
  Serial.printf("Résultats totaux: %u\n", totalResults);
  Serial.printf("Résultats par attaque: %.1f\n",
                totalResults / (float)getResultCount());

  Serial.println("\n");
}

void PerformanceBenchmark::printMemoryAnalysis() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║      ANALYSE MÉMOIRE                ║");
  Serial.println("╚════════════════════════════════════╝\n");

  uint32_t maxHeap = 0;
  uint32_t minHeap = UINT32_MAX;

  for (const auto& result : results) {
    if (result.maxHeapUsed > maxHeap) maxHeap = result.maxHeapUsed;
    if (result.minHeapUsed < minHeap) minHeap = result.minHeapUsed;
  }

  Serial.printf("Heap max utilisé: %u bytes\n", maxHeap);
  Serial.printf("Heap min utilisé: %u bytes\n", minHeap);

  uint32_t freeHeap = getHeapUsage();
  Serial.printf("Heap disponible: %u bytes\n", freeHeap);

  uint32_t psramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  Serial.printf("PSRAM disponible: %u bytes\n", psramFree);

  Serial.println("\n");
}

void PerformanceBenchmark::printPerformanceAnalysis() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║      ANALYSE PERFORMANCE            ║");
  Serial.println("╚════════════════════════════════════╝\n");

  // Trouver attaque plus rapide et plus lente
  uint32_t minTime = UINT32_MAX;
  uint32_t maxTime = 0;
  const char* fastestAttack = nullptr;
  const char* slowestAttack = nullptr;

  for (const auto& result : results) {
    if (result.avgDurationMs < minTime) {
      minTime = result.avgDurationMs;
      fastestAttack = result.attackName;
    }
    if (result.avgDurationMs > maxTime) {
      maxTime = result.avgDurationMs;
      slowestAttack = result.attackName;
    }
  }

  Serial.printf("⚡ Attaque plus rapide: %s (%u ms)\n", fastestAttack, minTime);
  Serial.printf("🐢 Attaque plus lente: %s (%u ms)\n", slowestAttack, maxTime);

  // Efficacité résultats
  Serial.println("\n📊 Efficacité (résultats/ms):");
  for (const auto& result : results) {
    float efficiency = (float)result.resultCount / result.avgDurationMs;
    Serial.printf("  %-30s: %.2f résultats/ms\n", result.attackName, efficiency);
  }

  Serial.println("\n");
}

void PerformanceBenchmark::exportToCSV(const char* filename) {
  Serial.printf("Exportation benchmark vers %s\n", filename);

  FILE* file = fopen(filename, "w");
  if (!file) {
    Serial.println("❌ Impossible d'ouvrir fichier");
    return;
  }

  // En-tête
  fprintf(file, "Attaque,DuréeMin(ms),DuréeMax(ms),DuréeMoy(ms),Résultats,Efficacité(res/ms)\n");

  // Données
  for (const auto& result : results) {
    float efficiency = (float)result.resultCount / result.avgDurationMs;
    fprintf(file, "%s,%u,%u,%u,%u,%.2f\n",
            result.attackName,
            result.minDurationMs,
            result.maxDurationMs,
            result.avgDurationMs,
            result.resultCount,
            efficiency);
  }

  fclose(file);
  Serial.printf("✓ Benchmark exporté (%u attaques)\n", results.size());
}
