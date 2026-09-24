#ifndef ACTIVE_TOOL_MONITOR_H
#define ACTIVE_TOOL_MONITOR_H

#include <Arduino.h>
#include <string>

// Track currently running tool with progress and statistics
class ActiveToolMonitor {
public:
    static ActiveToolMonitor& instance() {
        static ActiveToolMonitor atm;
        return atm;
    }

    struct ToolStatus {
        String toolName;
        uint32_t startTime;
        uint32_t progress;      // 0-100%
        String status;          // "scanning", "attacking", "processing", etc.
        uint32_t eventCount;    // packets, nodes, devices found
        int8_t signalStrength;  // RSSI or similar
        bool isActive;
    };

    // Start monitoring a tool
    void startTool(const char* name, const char* status = "initializing") {
        tool.toolName = String(name);
        tool.startTime = millis();
        tool.progress = 0;
        tool.status = String(status);
        tool.eventCount = 0;
        tool.signalStrength = -100;
        tool.isActive = true;
    }

    // Update progress (0-100%)
    void setProgress(uint32_t percent) {
        tool.progress = (percent > 100) ? 100 : percent;
    }

    // Update status message
    void setStatus(const char* status) {
        tool.status = String(status);
    }

    // Increment event counter (packets, nodes, etc.)
    void incrementEvents(uint32_t count = 1) {
        tool.eventCount += count;
    }

    // Update signal strength indicator
    void setSignalStrength(int8_t rssi) {
        tool.signalStrength = rssi;
    }

    // Get elapsed time in milliseconds
    uint32_t getElapsedMs() const {
        return tool.isActive ? (millis() - tool.startTime) : 0;
    }

    // Format elapsed time as MM:SS
    String getElapsedFormatted() const {
        uint32_t ms = getElapsedMs();
        uint32_t secs = ms / 1000;
        uint32_t mins = secs / 60;
        secs = secs % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02lu:%02lu", mins, secs);
        return String(buf);
    }

    // Stop monitoring current tool
    void stopTool() {
        tool.isActive = false;
    }

    // Get current tool status
    const ToolStatus& getStatus() const {
        return tool;
    }

    // Check if tool is running
    bool isToolRunning() const {
        return tool.isActive;
    }

    // Format tool status for display
    String formatStatusBar() const {
        if (!tool.isActive) return "";

        String bar = "🔴 " + tool.toolName + " | ";
        bar += tool.status + " | ";
        bar += String(tool.progress) + "% | ";
        bar += getElapsedFormatted();

        return bar;
    }

    // Format compact status for serial output
    String formatCompactStatus() const {
        if (!tool.isActive) return "";

        String status = tool.toolName + " [" + String(tool.progress) + "%] ";
        status += "events:" + String(tool.eventCount) + " ";
        status += "elapsed:" + getElapsedFormatted();

        return status;
    }

private:
    ToolStatus tool = {"", 0, 0, "", 0, -100, false};

    ActiveToolMonitor() {}
};

#endif
