#ifndef DUAL_CORE_MANAGER_H
#define DUAL_CORE_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Dual-core parallel processing for intensive operations
// Distributes long-running attacks across both ESP32 cores

class DualCoreManager {
public:
    static DualCoreManager& instance() {
        static DualCoreManager dcm;
        return dcm;
    }

    // Task function type
    typedef void (*TaskFunc)(void* param);

    // Initialize dual-core system
    bool begin();

    // Run task on secondary core (Core 1)
    // Primary core (Core 0) runs main loop
    bool runOnCore1(TaskFunc func, void* param, uint32_t stack_size = 4096,
                    uint8_t priority = 5, const char* name = "Core1Task");

    // Run task on primary core (Core 0)
    bool runOnCore0(TaskFunc func, void* param, uint32_t stack_size = 4096,
                    uint8_t priority = 5, const char* name = "Core0Task");

    // Get number of active tasks on each core
    uint8_t getCore0TaskCount() const { return core0_tasks_; }
    uint8_t getCore1TaskCount() const { return core1_tasks_; }

    // Get CPU load for each core (0-100%)
    uint8_t getCore0Load() const { return core0_load_; }
    uint8_t getCore1Load() const { return core1_load_; }

    // Check if core is busy
    bool isCore0Busy() const { return core0_tasks_ > 0; }
    bool isCore1Busy() const { return core1_tasks_ > 0; }

    // Wait for all tasks to complete
    bool waitForAllTasksComplete(uint32_t timeout_ms = 30000);

    // Kill specific task
    void killTask(TaskHandle_t handle);

    // Get current core executing
    static uint8_t getCurrentCore() {
        return xPortGetCoreID();
    }

private:
    DualCoreManager() : core0_tasks_(0), core1_tasks_(0),
                       core0_load_(0), core1_load_(0) {}

    uint8_t core0_tasks_;
    uint8_t core1_tasks_;
    uint8_t core0_load_;
    uint8_t core1_load_;

    SemaphoreHandle_t task_mutex_;

    // Helper: update task counters
    void updateTaskCounts();

    // Monitor task: periodically update CPU load
    static void monitorTaskLoad(void* param);
};

// Convenience macro for parallel execution
#define RUN_PARALLEL(func, param) \
    DualCoreManager::instance().runOnCore1(func, param, 4096, 5, #func)

#endif
