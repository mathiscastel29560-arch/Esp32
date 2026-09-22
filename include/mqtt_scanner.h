#ifndef MQTT_SCANNER_H
#define MQTT_SCANNER_H

#include <Arduino.h>
#include <vector>
#include <WiFi.h>

namespace MqttScanner {

struct MqttBroker {
  String hostname;
  uint16_t port;
  String clientId;
  bool requiresAuth;
  bool tlsEnabled;
  std::vector<String> topics;
  uint32_t discoveredAt;
};

struct ScannerConfig {
  uint32_t scanDurationMs;
  bool attemptConnection;
  bool fuzzTopics;
  std::vector<String> targetHosts; // Optional: scan specific hosts
};

struct ScannerResult {
  bool success;
  std::vector<MqttBroker> brokersFound;
  uint32_t topicsDiscovered;
  String logFile;
  String error;
};

class MqttScanner {
public:
  MqttScanner();
  ScannerResult scanMqttBrokers(const ScannerConfig& config);
  bool probeBrokerPort(const String& host, uint16_t port);
  void enumerateTopics(MqttBroker& broker);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;

  bool connectAndProbeMqtt(const String& host, uint16_t port, MqttBroker& broker);
  bool sendMqttConnect(WiFiClient& client);
  bool receiveMqttConnack(WiFiClient& client);
};

} // namespace MqttScanner

#endif
