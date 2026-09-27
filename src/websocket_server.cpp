#include "websocket_server.h"
#include "debug_logger.h"
#include "system_settings.h"
#include "audit_history.h"
#include "audit_statistics.h"
#include <cstdio>

bool WebSocketServer::begin(uint16_t port) {
  if (running) return true;

  serverPort = port;
  running = true;

  DebugLogger::printf("[WebSocket] Server starting on port %u\n", port);
  // Note: Full WebSocket implementation requires AsyncWebServer library
  // This is a stub for the interface - actual implementation needs
  // to integrate with ESP32AsyncWebServer library

  return true;
}

void WebSocketServer::stop() {
  running = false;
  clientCount = 0;
  DebugLogger::println("[WebSocket] Server stopped");
}

void WebSocketServer::handleClient() {
  if (!running) return;

  // In a full implementation, this would handle WebSocket frames
  // and maintain client connections
}

void WebSocketServer::broadcast(const std::string& message) {
  if (!running || clientCount == 0) return;

  DebugLogger::printf("[WebSocket] Broadcasting to %u clients: %s\n",
    clientCount, message.substr(0, 50).c_str());
}

void WebSocketServer::sendTo(uint32_t clientId, const std::string& message) {
  if (!running) return;

  DebugLogger::printf("[WebSocket] Sending to client %u: %s\n",
    clientId, message.substr(0, 50).c_str());
}

void WebSocketServer::sendStatusUpdate() {
  auto& sys = SystemSettings::getInstance();

  char json[512];
  snprintf(json, sizeof(json),
    "{\"type\":\"status\",\"uptime\":%u,\"freeHeap\":%u,\"brightness\":%u,"
    "\"volume\":%u,\"batteryMode\":\"%s\"}",
    millis() / 1000, ESP.getFreeHeap(), sys.getBrightness(),
    sys.getVolume(), sys.getBatterySavingModeString());

  broadcast(json);
}

void WebSocketServer::sendStatsUpdate() {
  auto& stats = AuditStatistics::getInstance();
  auto auditStats = stats.calculateStats();

  char json[256];
  snprintf(json, sizeof(json),
    "{\"type\":\"stats\",\"totalAudits\":%u,\"successRate\":%u,\"totalDevices\":%u}",
    auditStats.totalAudits, auditStats.successPercent,
    auditStats.totalDevicesFound);

  broadcast(json);
}

void WebSocketServer::sendAuditUpdate() {
  auto& history = AuditHistory::getInstance();
  auto records = history.getAllRecords();

  if (records.empty()) return;

  auto lastRecord = records.back();

  char json[256];
  snprintf(json, sizeof(json),
    "{\"type\":\"audit\",\"timestamp\":%u,\"type\":\"%s\",\"devices\":%u,\"rssi\":%d}",
    lastRecord.timestamp, lastRecord.auditType, lastRecord.devicesFound,
    lastRecord.maxRSSI);

  broadcast(json);
}
