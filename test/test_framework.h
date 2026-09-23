#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

class TestFramework {
public:
    struct TestResult {
        std::string name;
        bool passed;
        std::string message;
    };

    static TestFramework& instance() {
        static TestFramework tf;
        return tf;
    }

    void test(const char* name, bool condition, const char* message = "") {
        TestResult result;
        result.name = name;
        result.passed = condition;
        result.message = message ? message : "";
        results.push_back(result);

        printf("[%s] %s", condition ? "PASS" : "FAIL", name);
        if (message) printf(" - %s", message);
        printf("\n");
    }

    void assertEqual(const char* name, int expected, int actual) {
        bool passed = (expected == actual);
        char msg[256];
        snprintf(msg, sizeof(msg), "Expected %d, got %d", expected, actual);
        test(name, passed, msg);
    }

    void assertEqual(const char* name, const char* expected, const char* actual) {
        bool passed = (strcmp(expected, actual) == 0);
        test(name, passed, "String comparison");
    }

    int report() {
        printf("\n========== TEST REPORT ==========\n");
        int passed = 0, failed = 0;
        for (const auto& result : results) {
            if (result.passed) passed++;
            else failed++;
        }
        printf("Total: %zu | Passed: %d | Failed: %d\n", results.size(), passed, failed);
        printf("Success Rate: %.1f%%\n", (passed * 100.0) / results.size());
        return failed;
    }

private:
    std::vector<TestResult> results;
};

#define TEST(name, condition) TestFramework::instance().test(name, condition)
#define ASSERT_EQ(name, expected, actual) TestFramework::instance().assertEqual(name, expected, actual)
#define TEST_REPORT() TestFramework::instance().report()

#endif
