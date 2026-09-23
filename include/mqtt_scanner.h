#ifndef MQTT_SCANNER_H
#define MQTT_SCANNER_H

#include <Arduino.h>
#include <vector>

namespace MqttScanner {

struct MqttBroker {
  String hostname;
  uint16_t port;
  String clientId;
  bool requiresAuth;
  std::vector<String> topics;
};

struct ScannerConfig {
  uint32_t scanDurationMs;
  bool attemptConnection;
  bool fuzzTopics;
};

struct ScannerResult {
  bool success;
  std::vector<MqttBroker> brokersFound;
  uint32_t topicsDiscovered;
  String logFile;
};

class MqttScanner {
public:
  MqttScanner();
  ScannerResult scanMqttBrokers(const ScannerConfig& config);
  void stop();

private:
  bool isRunning_;
};

} // namespace MqttScanner

#endif
