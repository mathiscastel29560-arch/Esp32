#pragma once
#include <Arduino.h>

namespace MqttHijacker {

struct MqttBroker {
    String ipAddress;
    uint16_t port;
    int8_t rssi;
    String hostname;
    bool requiresAuth;
    uint32_t timestamp;
};

struct BrokerScanResult {
    bool success;
    uint32_t brokerCount;
    uint32_t durationMs;
    String strongestBroker;
};

struct MessageInterceptResult {
    bool success;
    uint32_t messagesIntercepted;
    uint32_t durationMs;
    String topicsFound;
};

struct MessageInjectionResult {
    bool success;
    uint32_t messagesInjected;
    uint32_t durationMs;
    String payloadType;
};

// Scan local network for MQTT brokers
BrokerScanResult scanMqttBrokers(uint32_t durationMs = 10000);

// Get discovered brokers
const MqttBroker* getDiscoveredBrokers(uint32_t& outCount);

// Intercept MQTT traffic on network
MessageInterceptResult interceptMqttMessages(uint32_t durationMs = 30000);

// Inject malicious MQTT messages
MessageInjectionResult injectMqttMessages(const char* brokerIp = "192.168.1.1",
                                          const char* topic = "home/+/+",
                                          uint32_t durationMs = 10000);

// Attack: Hijack connected devices via MQTT
struct HijackResult {
    bool success;
    uint32_t devicesHijacked;
    uint32_t durationMs;
    String commandsSent;
};
HijackResult hijackMqttDevices(const char* brokerIp, uint32_t durationMs = 15000);

// MQTT credential brute force
struct BruteforceResult {
    bool success;
    String credentialFound;
    uint32_t attemptsCount;
    uint32_t durationMs;
};
BruteforceResult bruteforceMqttCredentials(const char* brokerIp, uint32_t durationMs = 20000);

// Get MQTT statistics
struct MqttStats {
    uint32_t totalBrokersFound;
    uint32_t secureBrokers;
    uint32_t defaultCredBrokers;
    uint32_t totalMessagesIntercepted;
    String mostActiveTopic;
};
MqttStats getMqttStats();

}  // namespace MqttHijacker
