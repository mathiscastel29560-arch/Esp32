#include "dual_core_executor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <vector>

namespace DualCoreExecutor {

// FreeRTOS semaphores for synchronization
static SemaphoreHandle_t coreMutex = nullptr;
static SemaphoreHandle_t toolMutex = nullptr;

// Active tools tracking
struct ActiveTool {
    uint32_t toolId;
    const char* toolName;
    uint32_t startTime;
    TaskHandle_t taskHandle;
    bool completed;
    ToolResult result;
};

static std::vector<ActiveTool> activeTools;

// Initialize dual-core system
void initDualCore() {
    if (coreMutex == nullptr) {
        coreMutex = xSemaphoreCreateMutex();
        toolMutex = xSemaphoreCreateMutex();
    }

    // Verify we're on the right cores
    Serial.printf("CORE 0 (Menu): Running on core %d\n", xPortGetCoreID());
}

// Core 0: Menu & UI (main thread)
void menuCore() {
    // Menu loop - user interface
    // Non-blocking, responsive
    vTaskDelay(pdMS_TO_TICKS(10));
}

// Core 1: Tool executor (background)
void toolCore(void* parameter) {
    while (true) {
        xSemaphoreTake(toolMutex, portMAX_DELAY);

        // Find and execute available tools
        for (auto& tool : activeTools) {
            if (!tool.completed) {
                // Execute tool
                // tool.result = executeTool(tool.toolId, tool.startTime);
                tool.completed = true;
            }
        }

        xSemaphoreGive(toolMutex);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Start a tool on Core 1
bool executeToolConcurrent(uint32_t toolId, const char* toolName) {
    if (!xSemaphoreTake(toolMutex, pdMS_TO_TICKS(100))) {
        Serial.println("⚠️  Tool mutex timeout");
        return false;
    }

    // Check if Core 1 task exists
    static TaskHandle_t core1Task = nullptr;
    if (core1Task == nullptr) {
        BaseType_t result = xTaskCreatePinnedToCore(
            toolCore,           // Task function
            "ToolExecutor",     // Task name
            4096,               // Stack size
            nullptr,            // Parameters
            2,                  // Priority
            &core1Task,         // Task handle
            1                   // Core 1
        );
        if (result != pdPASS) {
            Serial.println("❌ Failed to create Core 1 task");
            xSemaphoreGive(toolMutex);
            return false;
        }
    }

    // Add tool to execution queue
    ActiveTool tool;
    tool.toolId = toolId;
    tool.toolName = toolName;
    tool.startTime = millis();
    tool.taskHandle = core1Task;
    tool.completed = false;

    activeTools.push_back(tool);

    xSemaphoreGive(toolMutex);

    Serial.printf("✓ Tool '%s' queued for concurrent execution on Core 1\n", toolName);
    return true;
}

// Get tool execution status
ToolExecutionStatus getToolStatus(uint32_t toolId) {
    ToolExecutionStatus status{false, 0, TOOL_QUEUED, ""};

    if (!xSemaphoreTake(toolMutex, pdMS_TO_TICKS(100))) {
        return status;
    }

    for (const auto& tool : activeTools) {
        if (tool.toolId == toolId) {
            status.found = true;
            status.elapsedMs = millis() - tool.startTime;
            status.state = tool.completed ? TOOL_COMPLETED : TOOL_RUNNING;
            status.toolName = tool.toolName;
            break;
        }
    }

    xSemaphoreGive(toolMutex);
    return status;
}

// Get core-specific information
CoreInfo getCoreInfo(uint8_t coreId) {
    CoreInfo info;
    info.coreId = coreId;
    info.currentCore = xPortGetCoreID();
    info.maxTasks = configMAX_PRIORITIES;

    if (coreId == 0) {
        info.name = "Menu & UI";
        info.description = "User interface, menu navigation";
    } else {
        info.name = "Tool Executor";
        info.description = "Background tool execution";
    }

    // Get free stack space
    info.freeStackBytes = uxTaskGetStackHighWaterMark(nullptr);
    info.runningTasks = uxTaskGetNumberOfTasks();

    return info;
}

// Get free heap memory
MemoryInfo getMemoryInfo() {
    MemoryInfo info;
    info.totalHeap = ESP.getHeapSize();
    info.freeHeap = ESP.getFreeHeap();
    info.usedHeap = info.totalHeap - info.freeHeap;
    info.usagePercent = (info.usedHeap * 100) / info.totalHeap;

    // PSRAM stats if available
    if (psramFound()) {
        info.totalPSRAM = ESP.getPsramSize();
        info.freePSRAM = ESP.getFreePsram();
        info.usedPSRAM = info.totalPSRAM - info.freePSRAM;
    }

    return info;
}

// Stop all concurrent tools
void stopAllTools() {
    if (!xSemaphoreTake(toolMutex, pdMS_TO_TICKS(500))) {
        Serial.println("⚠️  Could not acquire tool mutex to stop tools");
        return;
    }

    for (auto& tool : activeTools) {
        tool.completed = true;
        if (tool.taskHandle != nullptr) {
            vTaskDelete(tool.taskHandle);
        }
    }

    activeTools.clear();
    xSemaphoreGive(toolMutex);
    Serial.println("✓ All concurrent tools stopped");
}

// Display core status
void displayCoreStatus() {
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║           DUAL-CORE EXECUTION STATUS                    ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");

    // Core 0 info
    CoreInfo core0 = getCoreInfo(0);
    Serial.printf("║ CORE 0: %s\n", core0.name.c_str());
    Serial.printf("║   Description: %s\n", core0.description.c_str());
    Serial.printf("║   Free Stack: %u bytes\n", core0.freeStackBytes);

    // Core 1 info
    CoreInfo core1 = getCoreInfo(1);
    Serial.printf("║ CORE 1: %s\n", core1.name.c_str());
    Serial.printf("║   Description: %s\n", core1.description.c_str());
    Serial.printf("║   Running Tools: %u\n", core1.runningTasks);

    // Memory info
    MemoryInfo mem = getMemoryInfo();
    Serial.printf("║\n");
    Serial.printf("║ MEMORY STATUS:\n");
    Serial.printf("║   Heap: %u / %u bytes (%u%%)\n",
                 mem.usedHeap, mem.totalHeap, mem.usagePercent);
    if (psramFound()) {
        Serial.printf("║   PSRAM: %u / %u bytes\n",
                     mem.usedPSRAM, mem.totalPSRAM);
    }

    // Active tools
    Serial.printf("║\n");
    Serial.printf("║ ACTIVE TOOLS (%u):\n", activeTools.size());
    for (const auto& tool : activeTools) {
        const char* state = tool.completed ? "✓ DONE" : "⚙️  RUNNING";
        Serial.printf("║   [%s] %s - %ums\n", state, tool.toolName, tool.startTime);
    }

    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

} // namespace DualCoreExecutor
