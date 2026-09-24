#pragma once
#include <Arduino.h>
#include <WebServer.h>

namespace WebDashboard {

struct DashboardConfig {
    uint16_t port;
    const char* ssid;
    const char* password;
    bool enableSSL;
};

struct SystemMetrics {
    uint32_t freeHeap;
    uint32_t totalHeap;
    uint8_t heapUsagePercent;
    float cpuTemp;
    uint8_t wifiSignal;
    uint32_t uptime;
    uint8_t batteryPercent;
};

struct ToolMetrics {
    uint32_t totalExecuted;
    uint32_t successCount;
    uint32_t failureCount;
    uint32_t avgDuration;
};

void initDashboard(const DashboardConfig& config);
void updateMetrics(const SystemMetrics& metrics);
void startServer();
void stopServer();
bool isServerRunning();
String getServerURL();
void handleDashboardRequest();
void handleMetricsJSON();
void handleToolStatusJSON();
void broadcastMetricUpdate(const String& metricsJSON);

} // namespace WebDashboard
