#include "mqtt_scanner.h"
#include <LittleFS.h>
#include <WiFi.h>

namespace MqttScanner {

MqttScanner::MqttScanner() : isRunning_(false) {}

ScannerResult MqttScanner::scanMqttBrokers(const ScannerConfig& config) {
  ScannerResult result;
  result.success = false;
  result.topicsDiscovered = 0;

  isRunning_ = true;
  unsigned long startTime = millis();

  // Common MQTT ports and topics
  const uint16_t ports[] = {1883, 8883, 9001, 8000, 8080};
  const char* commonTopics[] = {"status", "control", "sensor", "data", "command", "telemetry"};

  while (isRunning_ && (millis() - startTime) < config.scanDurationMs) {
    // Simulate broker discovery
    if ((esp_random() % 100) < 25) {
      MqttBroker broker;
      broker.hostname = "mqtt_" + String(((esp_random() % 899) + 100));
      broker.port = ports[(esp_random() % 5)];
      broker.clientId = "esp32_" + String(((esp_random() % 8999) + 1000));
      broker.requiresAuth = (esp_random() % 2) == 1;

      // Simulate topic discovery
      uint32_t topicCount = ((esp_random() % 6) + 2);
      for (uint32_t i = 0; i < topicCount; i++) {
        String topic = commonTopics[(esp_random() % 6)];
        topic += "/" + String((esp_random() % 100));
        broker.topics.push_back(topic);
        result.topicsDiscovered++;
      }

      result.brokersFound.push_back(broker);
    }

    delay(100);
  }

  result.success = result.brokersFound.size() > 0;
  result.logFile = "/logs/handshakes/mqtt_brokers.csv";

  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/mqtt_brokers.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/mqtt_brokers.csv", "a");
  }
  if (logFile) {
    for (const auto& broker : result.brokersFound) {
      logFile.printf("%lu,%s,%u,%u\n", millis(), broker.hostname.c_str(),
                     broker.port, (uint32_t)broker.topics.size());
    }
    logFile.close();
  }
  LittleFS.end();

  isRunning_ = false;
  return result;
}

void MqttScanner::stop() {
  isRunning_ = false;
}

} // namespace MqttScanner
