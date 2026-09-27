#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

// WebSocket server for real-time dashboard updates
class WebSocketServer {
public:
  static WebSocketServer& getInstance() {
    static WebSocketServer instance;
    return instance;
  }

  bool begin(uint16_t port = 81);
  void stop();
  void handleClient();

  // Broadcast message to all connected clients
  void broadcast(const std::string& message);

  // Send to specific client
  void sendTo(uint32_t clientId, const std::string& message);

  // Check if running
  bool isRunning() const { return running; }

  // Get connected client count
  uint8_t getClientCount() const { return clientCount; }

private:
  WebSocketServer() = default;

  bool running = false;
  uint16_t serverPort = 81;
  uint8_t clientCount = 0;

  // Real-time update methods
  void sendStatusUpdate();
  void sendStatsUpdate();
  void sendAuditUpdate();
};

#endif // WEBSOCKET_SERVER_H
