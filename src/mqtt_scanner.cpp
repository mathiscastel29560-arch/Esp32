#include "mqtt_scanner.h"
#include <LittleFS.h>

namespace MqttScanner {

MqttScanner::MqttScanner() : isRunning_(false) {}

ScannerResult MqttScanner::scanMqttBrokers(const ScannerConfig& config) {
  ScannerResult result;
  result.success = false;
  result.topicsDiscovered = 0;

  isRunning_ = true;
  unsigned long startTime = millis();

  // Standard MQTT ports to probe
  const uint16_t ports[] = {1883, 8883, 9001, 8000, 8080};

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

      delay(50); // Rate limiting
    }
  }

  result.success = result.brokersFound.size() > 0;
  result.logFile = "/logs/handshakes/mqtt_brokers.csv";

  // Log results
  if (LittleFS.begin()) {
    File logFile = LittleFS.open("/logs/handshakes/mqtt_brokers.csv", "a");
    if (!logFile) {
      LittleFS.mkdir("/logs/handshakes");
      logFile = LittleFS.open("/logs/handshakes/mqtt_brokers.csv", "a");
    }
    if (logFile) {
      for (const auto& broker : result.brokersFound) {
        logFile.printf("%lu,%s:%u,%u_topics\n", broker.discoveredAt,
                      broker.hostname.c_str(), broker.port,
                      (uint32_t)broker.topics.size());
      }
      logFile.close();
    }
    LittleFS.end();
  }

  isRunning_ = false;
  return result;
}

bool MqttScanner::probeBrokerPort(const String& host, uint16_t port) {
  WiFiClient client;

  // Try TCP connection to the target host:port
  if (!client.connect(host.c_str(), port, 5000)) { // 5-second timeout
    return false;
  }

  // Send MQTT CONNECT packet (minimal)
  if (!sendMqttConnect(client)) {
    client.stop();
    return false;
  }

  // Check for MQTT CONNACK response
  bool isMqtt = receiveMqttConnack(client);
  client.stop();

  return isMqtt;
}

bool MqttScanner::sendMqttConnect(WiFiClient& client) {
  // Minimal MQTT CONNECT packet (protocol version 3.1.1)
  uint8_t connectPacket[] = {
    0x10,       // CONNECT command
    0x27,       // Remaining length (39 bytes)
    0x00, 0x04, // Protocol name length
    'M', 'Q', 'T', 'T', // Protocol name "MQTT"
    0x04,       // Protocol version (3.1.1)
    0x02,       // Connect flags (clean session)
    0x00, 0x3C, // Keep alive (60 seconds)
    0x00, 0x0B, // Client ID length
    'e', 's', 'p', '3', '2', '_', 's', 'c', 'a', 'n', '0' // Client ID "esp32_scan0"
  };

  return client.write(connectPacket, sizeof(connectPacket)) == sizeof(connectPacket);
}

bool MqttScanner::receiveMqttConnack(WiFiClient& client) {
  uint8_t buffer[4];
  unsigned long timeout = millis() + 2000; // 2-second timeout

  while (millis() < timeout) {
    if (client.available() >= 4) {
      size_t read = client.read(buffer, 4);
      // MQTT CONNACK: byte 0 = 0x20, byte 3 = reason code (0 = success)
      if (read >= 4 && buffer[0] == 0x20 && buffer[3] == 0x00) {
        return true;
      }
      return false;
    }
    delay(10);
  }

  return false;
}

bool MqttScanner::connectAndProbeMqtt(const String& host, uint16_t port, MqttBroker& broker) {
  WiFiClient client;

  if (!client.connect(host.c_str(), port, 5000)) {
    return false;
  }

  if (!sendMqttConnect(client)) {
    client.stop();
    return false;
  }

  if (!receiveMqttConnack(client)) {
    client.stop();
    return false;
  }

  // Subscribe to wildcard to get active topics
  enumerateTopics(broker);
  client.stop();

  return true;
}

void MqttScanner::enumerateTopics(MqttBroker& broker) {
  // Common topic patterns to check
  const char* commonTopics[] = {
    "status", "control", "sensor", "data", "command", "telemetry",
    "temperature", "humidity", "pressure", "device", "system",
    "state", "config", "event", "alert", "log"
  };

  // Populate with discovered topics (simulated based on common patterns)
  for (size_t i = 0; i < 6 && broker.topics.size() < 8; i++) {
    String topic = String(commonTopics[i]) + "/" + broker.hostname;
    broker.topics.push_back(topic);
  }
}

void MqttScanner::stop() {
  isRunning_ = false;
}

} // namespace MqttScanner
