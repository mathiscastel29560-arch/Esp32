#include "testing_framework.h"
#include <vector>
#include <cstring>
#include <LittleFS.h>

namespace TestingFramework {

struct RegisteredTest {
    const char* name;
    TestFunction func;
};

struct RegisteredBenchmark {
    const char* name;
    BenchmarkFunction func;
    uint32_t iterations;
};

static std::vector<RegisteredTest> registeredTests;
static std::vector<RegisteredBenchmark> registeredBenchmarks;
static std::vector<TestResult> testResults;
static std::vector<BenchmarkResult> benchmarkResults;
static TestSuite currentTestSuite{nullptr, 0, 0, 0, 0, 0};
static const char* lastErrorMessage = "";
static bool assertionFailed = false;

void registerTest(const char* name, TestFunction testFunc) {
    registeredTests.push_back({name, testFunc});
}

void registerBenchmark(const char* name, BenchmarkFunction benchFunc, uint32_t iterations) {
    registeredBenchmarks.push_back({name, benchFunc, iterations});
}

void runAllTests() {
    testResults.clear();
    currentTestSuite.passedTests = 0;
    currentTestSuite.failedTests = 0;
    currentTestSuite.skippedTests = 0;
    currentTestSuite.totalTests = registeredTests.size();
    currentTestSuite.totalDurationMs = 0;

    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║              RUNNING UNIT TESTS                            ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");

    uint32_t suiteStart = millis();

    for (const auto& test : registeredTests) {
        assertionFailed = false;
        lastErrorMessage = "";

        uint32_t testStart = millis();

        Serial.printf("📝 Running: %s... ", test.name);

        test.func();

        uint32_t duration = millis() - testStart;

        TestResult result{test.name, TEST_PASSED, duration, lastErrorMessage, ""};

        if (assertionFailed) {
            result.status = TEST_FAILED;
            currentTestSuite.failedTests++;
            Serial.println("❌ FAILED");
            if (lastErrorMessage) {
                Serial.printf("   Error: %s\n", lastErrorMessage);
            }
        } else {
            currentTestSuite.passedTests++;
            Serial.println("✓ PASSED");
        }

        testResults.push_back(result);
    }

    currentTestSuite.totalDurationMs = millis() - suiteStart;

    Serial.println("╠════════════════════════════════════════════════════════════╣");
    Serial.printf("║ Results: %u passed, %u failed, %u skipped in %u ms      ║\n",
                 currentTestSuite.passedTests,
                 currentTestSuite.failedTests,
                 currentTestSuite.skippedTests,
                 currentTestSuite.totalDurationMs);
    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void runAllBenchmarks() {
    benchmarkResults.clear();

    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║            RUNNING PERFORMANCE BENCHMARKS                  ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");

    for (const auto& bench : registeredBenchmarks) {
        Serial.printf("⏱️  Benchmark: %s (%u iterations)... ", bench.name, bench.iterations);
        Serial.flush();

        uint32_t totalTime = 0;
        uint32_t minTime = UINT32_MAX;
        uint32_t maxTime = 0;

        for (uint32_t i = 0; i < bench.iterations; i++) {
            uint32_t start = micros();
            bench.func();
            uint32_t elapsed = micros() - start;

            totalTime += elapsed;
            if (elapsed < minTime) minTime = elapsed;
            if (elapsed > maxTime) maxTime = elapsed;
        }

        BenchmarkResult result{
            bench.name,
            bench.iterations,
            totalTime / 1000,
            (double)totalTime / (bench.iterations * 1000.0),
            (double)minTime / 1000.0,
            (double)maxTime / 1000.0,
            (bench.iterations * 1000000.0) / totalTime
        };

        benchmarkResults.push_back(result);

        Serial.println("✓");
        Serial.printf("   Avg: %.2f ms | Min: %.2f ms | Max: %.2f ms | %.0f ops/sec\n",
                     result.avgTimeMs, result.minTimeMs, result.maxTimeMs, result.throughputPerSecond);
    }

    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void runTestSuite(const char* suiteName) {
    currentTestSuite.name = suiteName;
    runAllTests();
}

TestSuite getTestResults() {
    return currentTestSuite;
}

std::vector<TestResult> getDetailedResults() {
    return testResults;
}

std::vector<BenchmarkResult> getBenchmarkResults() {
    return benchmarkResults;
}

void assertEqualsInt(int expected, int actual, const char* message) {
    if (expected != actual) {
        assertionFailed = true;
        static char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s (expected: %d, actual: %d)", message, expected, actual);
        lastErrorMessage = buffer;
    }
}

void assertEqualsFloat(float expected, float actual, float tolerance, const char* message) {
    float diff = (expected > actual) ? (expected - actual) : (actual - expected);
    if (diff > tolerance) {
        assertionFailed = true;
        static char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s (expected: %.2f, actual: %.2f)", message, expected, actual);
        lastErrorMessage = buffer;
    }
}

void assertTrue(bool condition, const char* message) {
    if (!condition) {
        assertionFailed = true;
        lastErrorMessage = message;
    }
}

void assertFalse(bool condition, const char* message) {
    if (condition) {
        assertionFailed = true;
        lastErrorMessage = message;
    }
}

void assertEqual(const char* expected, const char* actual, const char* message) {
    if (strcmp(expected, actual) != 0) {
        assertionFailed = true;
        static char buffer[512];
        snprintf(buffer, sizeof(buffer), "%s (expected: '%s', actual: '%s')", message, expected, actual);
        lastErrorMessage = buffer;
    }
}

void assertNotNull(void* ptr, const char* message) {
    if (ptr == nullptr) {
        assertionFailed = true;
        lastErrorMessage = message;
    }
}

void displayTestResults() {
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║              DETAILED TEST RESULTS                         ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");

    for (const auto& result : testResults) {
        const char* statusStr = (result.status == TEST_PASSED) ? "✓ PASS" : "✗ FAIL";
        Serial.printf("║ [%s] %s (%u ms)\n", statusStr, result.testName, result.durationMs);

        if (result.status == TEST_FAILED && result.errorMessage) {
            Serial.printf("║   Error: %s\n", result.errorMessage);
        }
    }

    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void displayBenchmarkResults() {
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║            DETAILED BENCHMARK RESULTS                      ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");

    for (const auto& result : benchmarkResults) {
        Serial.printf("║ %s\n", result.benchmarkName);
        Serial.printf("║   Total: %u ms | Iterations: %u\n", result.totalTimeMs, result.iterations);
        Serial.printf("║   Avg: %.3f ms | Min: %.3f ms | Max: %.3f ms\n",
                     result.avgTimeMs, result.minTimeMs, result.maxTimeMs);
        Serial.printf("║   Throughput: %.0f ops/sec\n", result.throughputPerSecond);
    }

    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void exportTestResults(const char* filename) {
    if (!LittleFS.begin()) return;

    LittleFS.mkdir("/logs/tests");

    fs::File file = LittleFS.open(filename, "w");
    if (!file) {
        LittleFS.end();
        return;
    }

    file.printf("Test Suite: %s\n", currentTestSuite.name);
    file.printf("Timestamp: %u\n", millis());
    file.printf("Total Tests: %u\n", currentTestSuite.totalTests);
    file.printf("Passed: %u\n", currentTestSuite.passedTests);
    file.printf("Failed: %u\n", currentTestSuite.failedTests);
    file.printf("Duration: %u ms\n\n", currentTestSuite.totalDurationMs);

    file.println("=== Test Results ===");
    for (const auto& result : testResults) {
        const char* status = (result.status == TEST_PASSED) ? "PASS" : "FAIL";
        file.printf("[%s] %s (%u ms)\n", status, result.testName, result.durationMs);
    }

    file.println("\n=== Benchmark Results ===");
    for (const auto& result : benchmarkResults) {
        file.printf("%s: %.3f ms (avg), %.0f ops/sec\n",
                   result.benchmarkName, result.avgTimeMs, result.throughputPerSecond);
    }

    file.close();
    LittleFS.end();
}

} // namespace TestingFramework
