#ifndef PERFORMANCE_BENCHMARK_H
#define PERFORMANCE_BENCHMARK_H

#include "attack_catalog.h"
#include <cstdint>
#include <vector>

// ============= BENCHMARK STRUCTURE =============
struct BenchmarkResult {
  const char* attackName;
  uint32_t minDurationMs;
  uint32_t maxDurationMs;
  uint32_t avgDurationMs;
  uint32_t minHeapUsed;
  uint32_t maxHeapUsed;
  uint32_t avgHeapUsed;
  uint16_t resultCount;
};

class PerformanceBenchmark {
public:
  static PerformanceBenchmark& getInstance() {
    static PerformanceBenchmark instance;
    return instance;
  }

  // Benchmarks
  void benchmarkAttack(Attack* attack, uint32_t timeout);
  void benchmarkCategory(AttackCategory category);
  void benchmarkAllAttacks();

  // Résultats
  uint16_t getResultCount() const { return results.size(); }
  const BenchmarkResult& getResult(uint16_t index) const { return results[index]; }

  // Rapports
  void printSummary();
  void printMemoryAnalysis();
  void printPerformanceAnalysis();
  void exportToCSV(const char* filename);

private:
  PerformanceBenchmark();
  std::vector<BenchmarkResult> results;

  uint32_t getHeapUsage();
  void recordBenchmark(const char* name, uint32_t duration, uint16_t resultCount);
};

#endif // PERFORMANCE_BENCHMARK_H
