#pragma once
#include <Arduino.h>
#include <vector>

namespace TestingFramework {

enum TestStatus {
    TEST_PASSED = 0,
    TEST_FAILED = 1,
    TEST_SKIPPED = 2,
    TEST_TIMEOUT = 3
};

struct TestResult {
    const char* testName;
    TestStatus status;
    uint32_t durationMs;
    const char* errorMessage;
    const char* assertion;
};

struct BenchmarkResult {
    const char* benchmarkName;
    uint32_t iterations;
    uint32_t totalTimeMs;
    double avgTimeMs;
    double minTimeMs;
    double maxTimeMs;
    double throughputPerSecond;
};

struct TestSuite {
    const char* name;
    uint32_t totalTests;
    uint32_t passedTests;
    uint32_t failedTests;
    uint32_t skippedTests;
    uint32_t totalDurationMs;
};

typedef void (*TestFunction)();
typedef void (*BenchmarkFunction)();

void registerTest(const char* name, TestFunction testFunc);
void registerBenchmark(const char* name, BenchmarkFunction benchFunc, uint32_t iterations);

void runAllTests();
void runAllBenchmarks();
void runTestSuite(const char* suiteName);

TestSuite getTestResults();
std::vector<TestResult> getDetailedResults();
std::vector<BenchmarkResult> getBenchmarkResults();

void assertEqualsInt(int expected, int actual, const char* message);
void assertEqualsFloat(float expected, float actual, float tolerance, const char* message);
void assertTrue(bool condition, const char* message);
void assertFalse(bool condition, const char* message);
void assertEqual(const char* expected, const char* actual, const char* message);
void assertNotNull(void* ptr, const char* message);

void displayTestResults();
void displayBenchmarkResults();
void exportTestResults(const char* filename);

} // namespace TestingFramework
