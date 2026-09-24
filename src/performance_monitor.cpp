#include "performance_monitor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <LittleFS.h>

namespace PerformanceMonitor {

static std::vector<PerformanceMetric> metrics;
static uint32_t sampleInterval = 1000;
static bool isMonitoring = false;
static TaskHandle_t monitorTask = nullptr;
static uint32_t lastHeapMin = UINT32_MAX;
static uint32_t lastHeapMax = 0;

void monitoringTask(void* param) {
    while (isMonitoring) {
        collectMetrics();
        vTaskDelay(pdMS_TO_TICKS(sampleInterval));
    }
    
    vTaskDelete(nullptr);
}

void initMonitor(uint32_t interval) {
    sampleInterval = interval;
    metrics.clear();
    lastHeapMin = UINT32_MAX;
    lastHeapMax = 0;
}

void start() {
    if (!isMonitoring) {
        isMonitoring = true;
        BaseType_t result = xTaskCreatePinnedToCore(
            monitoringTask,
            "PerfMonitor",
            4096,
            nullptr,
            1,
            &monitorTask,
            1
        );
        
        if (result != pdPASS) {
            Serial.println("❌ Failed to start performance monitor");
            isMonitoring = false;
        } else {
            Serial.println("✓ Performance monitor started");
        }
    }
}

void stop() {
    if (isMonitoring) {
        isMonitoring = false;
        if (monitorTask != nullptr) {
            vTaskDelete(monitorTask);
            monitorTask = nullptr;
        }
        Serial.println("✓ Performance monitor stopped");
    }
}

void collectMetrics() {
    PerformanceMetric metric;
    metric.timestamp = millis();
    metric.freeHeap = ESP.getFreeHeap();
    metric.freeStack = uxTaskGetStackHighWaterMark(nullptr);
    metric.cpuUsage = 0;  // Would require RTOS task time tracking
    metric.temperature = 0;  // Would require temperature sensor
    metric.taskCount = uxTaskGetNumberOfTasks();
    
    metrics.push_back(metric);
    
    if (metric.freeHeap < lastHeapMin) lastHeapMin = metric.freeHeap;
    if (metric.freeHeap > lastHeapMax) lastHeapMax = metric.freeHeap;
    
    if (metrics.size() > 1000) {
        metrics.erase(metrics.begin());
    }
}

PerformanceMetric getMetrics() {
    if (metrics.empty()) {
        return {millis(), ESP.getFreeHeap(), 0, 0, 0, uxTaskGetNumberOfTasks()};
    }
    return metrics.back();
}

PerformanceStats getStatistics() {
    PerformanceStats stats = {UINT32_MAX, 0, 0, 255, 0, 128, 125.0f, 0.0f, 25.0f, (uint32_t)metrics.size()};
    
    if (metrics.empty()) return stats;
    
    uint32_t heapSum = 0;
    
    for (const auto& m : metrics) {
        if (m.freeHeap < stats.minHeap) stats.minHeap = m.freeHeap;
        if (m.freeHeap > stats.maxHeap) stats.maxHeap = m.freeHeap;
        heapSum += m.freeHeap;
    }
    
    stats.avgHeap = heapSum / metrics.size();
    
    return stats;
}

std::vector<TaskInfo> getTaskInfo() {
    std::vector<TaskInfo> tasks;
    
    TaskStatus_t taskStatus[16];
    uint32_t taskCount = uxTaskGetSystemState(taskStatus, 16, nullptr);
    
    for (uint32_t i = 0; i < taskCount; i++) {
        TaskInfo info;
        info.name = (const char*)taskStatus[i].pcTaskName;
        info.stackSize = taskStatus[i].usStackHighWaterMark;
        info.priority = taskStatus[i].uxCurrentPriority;
        info.coreId = 0;  // Not available in TaskStatus_t
        tasks.push_back(info);
    }
    
    return tasks;
}

void displayPerformanceDashboard() {
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║          REAL-TIME PERFORMANCE DASHBOARD                  ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint8_t heapUsage = ((totalHeap - freeHeap) * 100) / totalHeap;
    
    Serial.printf("║ Heap: %u / %u bytes (%u%% used)\n", freeHeap, totalHeap, heapUsage);
    
    Serial.printf("║ Free Stack: %u bytes\n", uxTaskGetStackHighWaterMark(nullptr));
    Serial.printf("║ Tasks Running: %u\n", uxTaskGetNumberOfTasks());
    Serial.printf("║ Sample Count: %u\n", (uint32_t)metrics.size());
    
    if (!metrics.empty()) {
        PerformanceStats stats = getStatistics();
        Serial.printf("║\n");
        Serial.printf("║ Heap Statistics:\n");
        Serial.printf("║   Min: %u bytes | Max: %u bytes | Avg: %u bytes\n",
                     stats.minHeap, stats.maxHeap, stats.avgHeap);
    }
    
    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void displayTaskMetrics() {
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║             TASK PERFORMANCE METRICS                       ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");
    
    auto tasks = getTaskInfo();
    
    for (const auto& task : tasks) {
        Serial.printf("║ %s (Priority: %u)\n", task.name, task.priority);
        Serial.printf("║   Stack: %u bytes free\n", task.stackSize);
    }
    
    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void exportPerformanceLog(const char* filename) {
    if (!LittleFS.begin()) return;
    
    LittleFS.mkdir("/logs/performance");
    
    fs::File file = LittleFS.open(filename, "w");
    if (!file) {
        LittleFS.end();
        return;
    }
    
    file.println("Timestamp,FreeHeap,FreeStack,CPUUsage,Temperature,TaskCount");
    
    for (const auto& m : metrics) {
        file.printf("%u,%u,%u,%u,%u,%u\n",
                   m.timestamp, m.freeHeap, m.freeStack, m.cpuUsage, 
                   (uint32_t)m.temperature, m.taskCount);
    }
    
    file.close();
    LittleFS.end();
}

float getCPULoad() {
    return 0.0f;
}

float getTaskLoadPerCore(uint8_t coreId) {
    return 0.0f;
}

bool checkMemoryLeak() {
    if (metrics.size() < 10) return false;
    
    uint32_t recentAvg = 0;
    for (size_t i = metrics.size() - 10; i < metrics.size(); i++) {
        recentAvg += metrics[i].freeHeap;
    }
    recentAvg /= 10;
    
    uint32_t oldAvg = 0;
    for (size_t i = 0; i < 10 && i < metrics.size(); i++) {
        oldAvg += metrics[i].freeHeap;
    }
    oldAvg /= 10;
    
    return (oldAvg - recentAvg) > (oldAvg / 20);
}

bool checkThermalThrottle() {
    return false;
}

} // namespace PerformanceMonitor
