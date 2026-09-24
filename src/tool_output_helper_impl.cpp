#include "tool_output_helper.h"

namespace ToolOutputHelper {

// ============================================================================
// SCAN PROGRESS BAR IMPLEMENTATION
// ============================================================================

ScanProgressBar::ScanProgressBar(const String &name, uint32_t expectedDurationMs, uint8_t steps)
    : toolName(name), totalDuration(expectedDurationMs), currentStep(0), totalSteps(steps) {
    startTime = millis();
}

void ScanProgressBar::start() {
    printHeader(toolName, "🔍");
    Serial.println();
    Serial.printf("Duration: ~%lums | Steps: %u\n", totalDuration, totalSteps);
    Serial.println(String('─', 50));
}

void ScanProgressBar::step(const String &message) {
    currentStep++;
    uint32_t elapsed = millis() - startTime;
    uint8_t percent = (currentStep * 100) / totalSteps;

    Serial.printf("[%u/%u] ", currentStep, totalSteps);
    drawProgressBar(percent);
    Serial.printf(" %u%%\n", percent);
    Serial.printf("    ⏱️  %lums | %s\n", elapsed, message.c_str());
}

void ScanProgressBar::complete(const String &summary) {
    uint32_t elapsed = millis() - startTime;
    Serial.println(String('─', 50));
    printSuccess("Scan Complete!");
    Serial.printf("Total Time: %lums\n", elapsed);
    Serial.printf("%s\n", summary.c_str());
    Serial.println();
}

uint8_t ScanProgressBar::getPercentage() const {
    return (currentStep * 100) / totalSteps;
}

void ScanProgressBar::drawProgressBar(uint8_t percent) {
    uint8_t filled = (percent * 20) / 100;
    Serial.print("[");
    for (uint8_t i = 0; i < 20; i++) {
        Serial.print(i < filled ? "█" : "░");
    }
    Serial.print("]");
}

// ============================================================================
// SCAN OUTPUT WRAPPERS
// ============================================================================

void displayScanStart(const String &toolName, const String &targetInfo) {
    Serial.println("\n" + String('═', 60));
    Serial.println("🔍 " + toolName);
    Serial.println(String('═', 60));
    if (targetInfo.length() > 0) {
        Serial.println("Target: " + targetInfo);
    }
    Serial.println();
}

void displayScanProgress(uint8_t percent, const String &message) {
    Serial.printf("[%3u%%] ", percent);
    uint8_t filled = (percent * 15) / 100;
    for (uint8_t i = 0; i < 15; i++) {
        Serial.print(i < filled ? "█" : "░");
    }
    Serial.printf(" %s\n", message.c_str());
}

void displayScanComplete(const String &summary, bool success) {
    Serial.println(String('─', 60));
    if (success) {
        printSuccess(summary);
    } else {
        printWarning(summary);
    }
    Serial.println();
}

// ============================================================================
// ATTACK OUTPUT WRAPPERS
// ============================================================================

void displayAttackStart(const String &attackName, uint32_t targetCount) {
    Serial.println("\n" + String('═', 60));
    Serial.println("⚔️  " + attackName);
    Serial.println(String('═', 60));
    Serial.printf("Targets: %u devices\n", targetCount);
    Serial.println();
}

void displayAttackProgress(uint8_t percent, uint32_t currentTarget, const String &status) {
    Serial.printf("[%3u%%] Target %u: ", percent, currentTarget);
    uint8_t filled = (percent * 15) / 100;
    for (uint8_t i = 0; i < 15; i++) {
        Serial.print(i < filled ? "█" : "░");
    }
    Serial.printf(" %s\n", status.c_str());
}

void displayAttackComplete(uint32_t successCount, uint32_t totalTargets, bool anyCompromised) {
    uint8_t successPercent = (successCount * 100) / max(totalTargets, 1U);
    Serial.println(String('─', 60));

    if (anyCompromised) {
        Serial.printf("✅ Attack Successful: %u/%u targets compromised (%u%%)\n",
                     successCount, totalTargets, successPercent);
    } else {
        Serial.printf("⚠️  Attack Partial: %u/%u targets affected (%u%%)\n",
                     successCount, totalTargets, successPercent);
    }
    Serial.println();
}

// ============================================================================
// DEVICE DISCOVERY OUTPUT
// ============================================================================

void displayDeviceDiscovery(const String &deviceType, uint32_t count, int8_t signal) {
    Serial.printf("  ✓ %s detected | Count: %u | Signal: %d dBm\n",
                 deviceType.c_str(), count, signal);
}

void displayDeviceSummary(const String &type, uint32_t found, uint32_t vulnerable) {
    Serial.printf("📊 %s Summary:\n", type.c_str());
    Serial.printf("   Found: %u | Vulnerable: %u (", found, vulnerable);

    uint8_t vulnPercent = (vulnerable * 100) / max(found, 1U);
    Serial.printf("%u%%)\n", vulnPercent);

    Serial.print("   Risk: [");
    uint8_t filled = vulnPercent / 5;
    for (uint8_t i = 0; i < 20; i++) {
        Serial.print(i < filled ? "🔴" : "🟩");
    }
    Serial.println("]");
}

// ============================================================================
// BEAUTIFUL OUTPUT HELPERS
// ============================================================================

void printHeader(const String &title, const String &icon) {
    Serial.println("\n" + String('═', 60));
    if (icon.length() > 0) {
        Serial.printf("%s %s\n", icon.c_str(), title.c_str());
    } else {
        Serial.println(title);
    }
    Serial.println(String('═', 60));
}

void printSubHeader(const String &title) {
    Serial.println("\n" + String('─', 50));
    Serial.println(title);
    Serial.println(String('─', 50));
}

void printSuccess(const String &message) {
    Serial.printf("✅ %s\n", message.c_str());
}

void printWarning(const String &message) {
    Serial.printf("⚠️  %s\n", message.c_str());
}

void printError(const String &message) {
    Serial.printf("❌ %s\n", message.c_str());
}

void printInfo(const String &message) {
    Serial.printf("ℹ️  %s\n", message.c_str());
}

void printBar(uint8_t percent, uint8_t width) {
    Serial.print("[");
    uint8_t filled = (percent * width) / 100;
    for (uint8_t i = 0; i < width; i++) {
        Serial.print(i < filled ? "█" : "░");
    }
    Serial.printf("] %u%%\n", percent);
}

void printKeyValue(const String &key, const String &value) {
    Serial.printf("  %-25s : %s\n", key.c_str(), value.c_str());
}

void printTable(const std::vector<String> &headers, const std::vector<std::vector<String>> &rows) {
    // Print headers
    Serial.print("  ");
    for (const auto &header : headers) {
        Serial.printf("%-20s ", header.c_str());
    }
    Serial.println();

    // Print separator
    Serial.print("  ");
    for (size_t i = 0; i < headers.size(); i++) {
        Serial.print(String('─', 20) + " ");
    }
    Serial.println();

    // Print rows
    for (const auto &row : rows) {
        Serial.print("  ");
        for (size_t i = 0; i < row.size() && i < headers.size(); i++) {
            Serial.printf("%-20s ", row[i].c_str());
        }
        Serial.println();
    }
}

}  // namespace ToolOutputHelper
