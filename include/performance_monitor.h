#pragma once
#include <Arduino.h>
#include <vector>

namespace PerformanceMonitor {

struct PerformanceMetric {
    uint32_t timestamp;
    uint32_t freeHeap;
    uint32_t freeStack;
    uint8_t cpuUsage;
    float temperature;
    uint32_t taskCount;
};

struct PerformanceStats {
    uint32_t minHeap;
    uint32_t maxHeap;
    uint32_t avgHeap;
    uint8_t minCpuUsage;
    uint8_t maxCpuUsage;
    uint8_t avgCpuUsage;
    float minTemp;
    float maxTemp;
    float avgTemp;
    uint32_t sampleCount;
};

struct TaskInfo {
    const char* name;
    uint32_t stackSize;
    uint32_t stackHighWater;
    uint8_t priority;
    uint8_t coreId;
};

void initMonitor(uint32_t sampleInterval = 1000);
void start();
void stop();
void collectMetrics();

PerformanceMetric getMetrics();
PerformanceStats getStatistics();
std::vector<TaskInfo> getTaskInfo();

void displayPerformanceDashboard();
void displayTaskMetrics();
void exportPerformanceLog(const char* filename);

float getCPULoad();
float getTaskLoadPerCore(uint8_t coreId);
bool checkMemoryLeak();
bool checkThermalThrottle();

} // namespace PerformanceMonitor
