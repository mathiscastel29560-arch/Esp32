#include "dual_core_manager.h"
#include "audit_log.h"
#include <freertos/semphr.h>

// Global task tracking
static uint8_t g_core0_tasks = 0;
static uint8_t g_core1_tasks = 0;

// Wrapper to track task lifecycle
struct TaskWrapper {
    DualCoreManager::TaskFunc func;
    void* param;
    uint8_t core_id;
};

static void taskWrapper(void* wrapper_ptr) {
    TaskWrapper* wrapper = (TaskWrapper*)wrapper_ptr;
    uint8_t core_id = wrapper->core_id;
    DualCoreManager::TaskFunc func = wrapper->func;
    void* param = wrapper->param;

    // Increment task counter
    if (core_id == 0) {
        g_core0_tasks++;
    } else {
        g_core1_tasks++;
    }

    // Execute actual task
    func(param);

    // Decrement task counter
    if (core_id == 0) {
        g_core0_tasks--;
    } else {
        g_core1_tasks--;
    }

    free(wrapper);
    vTaskDelete(NULL);
}

bool DualCoreManager::begin() {
    task_mutex_ = xSemaphoreCreateMutex();
    if (!task_mutex_) {
        Serial.println("[DualCoreManager] Failed to create mutex");
        return false;
    }

    // Start monitor task on Core 0
    xTaskCreatePinnedToCore(
        monitorTaskLoad,
        "DCMMonitor",
        2048,
        this,
        2,  // Low priority
        NULL,
        0   // Core 0
    );

    char details[96];
    snprintf(details, sizeof(details), "Dual-core initialized, cores ready for parallel processing");
    AuditLog::instance().log(AuditEventType::TOOL_START, "DualCoreManager", details);

    Serial.println("[DualCoreManager] Initialized for parallel processing");
    return true;
}

bool DualCoreManager::runOnCore1(TaskFunc func, void* param, uint32_t stack_size,
                                 uint8_t priority, const char* name) {
    if (!func) return false;

    // Allocate wrapper
    TaskWrapper* wrapper = (TaskWrapper*)malloc(sizeof(TaskWrapper));
    if (!wrapper) return false;

    wrapper->func = func;
    wrapper->param = param;
    wrapper->core_id = 1;

    // Create task pinned to Core 1
    TaskHandle_t handle = NULL;
    BaseType_t result = xTaskCreatePinnedToCore(
        taskWrapper,
        name ? name : "Core1Task",
        stack_size,
        wrapper,
        priority,
        &handle,
        1  // Core 1
    );

    if (result != pdPASS) {
        free(wrapper);
        Serial.printf("[DualCoreManager] Failed to create Core1 task: %s\n", name);
        return false;
    }

    Serial.printf("[DualCoreManager] Task created on Core1: %s (priority=%u)\n", name, priority);
    return true;
}

bool DualCoreManager::runOnCore0(TaskFunc func, void* param, uint32_t stack_size,
                                 uint8_t priority, const char* name) {
    if (!func) return false;

    TaskWrapper* wrapper = (TaskWrapper*)malloc(sizeof(TaskWrapper));
    if (!wrapper) return false;

    wrapper->func = func;
    wrapper->param = param;
    wrapper->core_id = 0;

    TaskHandle_t handle = NULL;
    BaseType_t result = xTaskCreatePinnedToCore(
        taskWrapper,
        name ? name : "Core0Task",
        stack_size,
        wrapper,
        priority,
        &handle,
        0  // Core 0
    );

    if (result != pdPASS) {
        free(wrapper);
        return false;
    }

    return true;
}

bool DualCoreManager::waitForAllTasksComplete(uint32_t timeout_ms) {
    uint32_t start = millis();

    while (millis() - start < timeout_ms) {
        if (g_core0_tasks == 0 && g_core1_tasks == 0) {
            Serial.println("[DualCoreManager] All tasks completed");
            return true;
        }
        delay(100);
    }

    Serial.printf("[DualCoreManager] Timeout waiting for tasks (C0=%u, C1=%u)\n",
                  g_core0_tasks, g_core1_tasks);
    return false;
}

void DualCoreManager::killTask(TaskHandle_t handle) {
    if (handle) {
        vTaskDelete(handle);
    }
}

void DualCoreManager::updateTaskCounts() {
    xSemaphoreTake(task_mutex_, portMAX_DELAY);
    core0_tasks_ = g_core0_tasks;
    core1_tasks_ = g_core1_tasks;
    xSemaphoreGive(task_mutex_);
}

void DualCoreManager::monitorTaskLoad(void* param) {
    DualCoreManager* dcm = (DualCoreManager*)param;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        // Update task counts every 1 second
        dcm->updateTaskCounts();

        // Calculate CPU load (simplified: based on task count)
        // Real implementation would use more sophisticated metrics
        if (dcm->core0_tasks_ > 0) {
            dcm->core0_load_ = 50 + (dcm->core0_tasks_ * 10);
            if (dcm->core0_load_ > 100) dcm->core0_load_ = 100;
        } else {
            dcm->core0_load_ = 0;
        }

        if (dcm->core1_tasks_ > 0) {
            dcm->core1_load_ = 50 + (dcm->core1_tasks_ * 10);
            if (dcm->core1_load_ > 100) dcm->core1_load_ = 100;
        } else {
            dcm->core1_load_ = 0;
        }

        // Periodic logging
        static uint32_t last_log = 0;
        if (millis() - last_log > 10000) {
            last_log = millis();
            Serial.printf("[DualCoreManager] Load: Core0=%u%% (tasks=%u), Core1=%u%% (tasks=%u)\n",
                         dcm->core0_load_, dcm->core0_tasks_,
                         dcm->core1_load_, dcm->core1_tasks_);
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));
    }
}
