#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace DualCoreExecutor {

// Tool execution states
enum ToolState {
    TOOL_QUEUED = 0,
    TOOL_RUNNING = 1,
    TOOL_COMPLETED = 2,
    TOOL_FAILED = 3
};

struct ToolResult {
    bool success;
    uint32_t packetsCount;
    uint32_t duration;
};

struct ToolExecutionStatus {
    bool found;
    uint32_t elapsedMs;
    ToolState state;
    const char* toolName;
};

struct CoreInfo {
    uint8_t coreId;
    uint8_t currentCore;
    uint8_t maxTasks;
    String name;
    String description;
    uint32_t freeStackBytes;
    uint32_t runningTasks;
};

struct MemoryInfo {
    uint32_t totalHeap;
    uint32_t freeHeap;
    uint32_t usedHeap;
    uint32_t usagePercent;
    uint32_t totalPSRAM;
    uint32_t freePSRAM;
    uint32_t usedPSRAM;
};

void initDualCore();
bool executeToolConcurrent(uint32_t toolId, const char* toolName);
ToolExecutionStatus getToolStatus(uint32_t toolId);
CoreInfo getCoreInfo(uint8_t coreId);
MemoryInfo getMemoryInfo();
void stopAllTools();
void displayCoreStatus();

} // namespace DualCoreExecutor
