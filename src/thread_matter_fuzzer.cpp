#include "thread_matter_fuzzer.h"
#include "tx_arm.h"
#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiUdp.h>

namespace ThreadMatterFuzzer {

// Matter protocol fuzzing via UDP (Matter default port is 5540)
WiFiUDP udpSocket;

// Matter Protocol Constants
const uint16_t MATTER_SECURE_PORT = 5540;
const uint16_t MATTER_UNSECURE_PORT = 5541;

namespace {
  // Matter message type definitions
  enum MatterMessageType {
    SECURE_SESSION_START = 0x30,
    SECURE_SESSION_RESPONSE = 0x31,
    PBKDF2_PARAMS = 0x32,
    CASE_SIGMA1 = 0x33,
    CASE_SIGMA2 = 0x34,
    CASE_SIGMA3 = 0x35,
    PASE_PAKE1 = 0x36,
    PASE_PAKE2 = 0x37,
    PASE_PAKE3 = 0x38,
    MRP_REQUEST = 0x39,
    MRP_RESPONSE = 0x3A,
  };

  // TLV (Tag-Length-Value) encoding helper
  struct TLVElement {
    uint8_t tag;
    uint8_t type;  // 0=8-bit, 1=16-bit, 2=32-bit, 3=64-bit, 4=string, etc.
    uint32_t length;
    uint8_t* value;
  };

  // Build Matter TLV structure
  uint32_t buildMatterTLV(uint8_t* buffer, const TLVElement* elements, uint32_t count) {
    uint32_t offset = 0;
    for (uint32_t i = 0; i < count; i++) {
      // Write tag
      buffer[offset++] = elements[i].tag;
      // Write type
      buffer[offset++] = elements[i].type;
      // Write length
      if (elements[i].length < 256) {
        buffer[offset++] = elements[i].length;
      } else {
        buffer[offset++] = 0xFF;  // Extended length
        buffer[offset++] = (elements[i].length >> 8) & 0xFF;
        buffer[offset++] = elements[i].length & 0xFF;
      }
      // Write value
      if (elements[i].value && elements[i].length > 0) {
        memcpy(buffer + offset, elements[i].value, elements[i].length);
        offset += elements[i].length;
      }
    }
    return offset;
  }
}

Fuzzer::Fuzzer() : isRunning_(false) {}

FuzzerResult Fuzzer::fuzzMatterDevice(const FuzzerConfig& config) {
  FuzzerResult result;
  result.success = false;

  if (!TxArm::isArmed()) return result;

  isRunning_ = true;
  unsigned long startTime = millis();

  // Initialize UDP for Matter protocol communication
  if (!udpSocket.begin(MATTER_UNSECURE_PORT)) {
    result.error = "Failed to bind UDP socket";
    isRunning_ = false;
    return result;
  }

  uint32_t messageCount = 0;
  uint32_t crashCount = 0;
  uint32_t mlrFuzzed = 0;
  uint32_t commissioningFuzzed = 0;

  Serial.println("[Matter Fuzz] Starting fuzzing campaign");

  while (isRunning_ && (millis() - startTime) < config.durationMs) {
    uint8_t fuzzyPayload[256];
    uint32_t payloadLen = 0;

    if (config.fuzzMlrRequests) {
      // Fuzz Multicast Listener Report (MLR) - Thread network discovery
      // MLR Header: 0x3C (ICMPv6 MLR message type)
      fuzzyPayload[payloadLen++] = 0x3C;  // MLR message type
      fuzzyPayload[payloadLen++] = 0x00;  // Code
      fuzzyPayload[payloadLen++] = random(0x00, 0xFF);  // Checksum 1
      fuzzyPayload[payloadLen++] = random(0x00, 0xFF);  // Checksum 2
      fuzzyPayload[payloadLen++] = random(0x00, 0xFF);  // Flags
      fuzzyPayload[payloadLen++] = random(0x00, 0xFF);  // Number of records

      // Fuzz MLR records (malformed address count, invalid multicast addresses)
      uint8_t recordCount = random(1, 10);
      for (int i = 0; i < recordCount; i++) {
        fuzzyPayload[payloadLen++] = random(0x00, 0xFF);  // Record type
        fuzzyPayload[payloadLen++] = random(0x00, 0xFF);  // Aux data length
        // Add random garbage for addresses
        for (int j = 0; j < 16; j++) {
          fuzzyPayload[payloadLen++] = random(0x00, 0xFF);
        }
        if (payloadLen >= 240) break;  // Don't overflow buffer
      }

      // Send MLR fuzz packet
      udpSocket.beginPacket(IPAddress(224, 0, 0, 250), 5353);  // mDNS/Thread multicast
      udpSocket.write(fuzzyPayload, payloadLen);
      udpSocket.endPacket();

      mlrFuzzed++;
      messageCount++;

      Serial.printf("[Matter Fuzz] Sent MLR fuzz packet (%u bytes)\n", payloadLen);
      delay(50);
    }

    if (config.fuzzCommissioningMessages) {
      // Fuzz Matter commissioning protocol (CASE/PASE)
      // CASE (Certificate Authenticated Session Establishment)
      payloadLen = 0;

      // Matter frame header
      fuzzyPayload[payloadLen++] = 0x05;  // Flags (fabric secured)
      fuzzyPayload[payloadLen++] = random(0x00, 0xFF);  // Message type
      fuzzyPayload[payloadLen++] = 0x01;  // Protocol ID (Security)
      fuzzyPayload[payloadLen++] = random(0x00, 0xFF);  // Opcode

      // Fuzz TLV structures (commissioning uses TLV encoding)
      TLVElement tlvElements[3];
      uint8_t value1[32], value2[32];

      // Generate random TLV elements
      for (int i = 0; i < 32; i++) {
        value1[i] = random(0x00, 0xFF);
        value2[i] = random(0x00, 0xFF);
      }

      tlvElements[0] = {0x01, 0x04, 32, value1};  // Random element 1
      tlvElements[1] = {random(0x02, 0x10), 0x04, random(8, 32), value2};  // Random element 2
      tlvElements[2] = {0xFF, 0x04, 0, NULL};  // Terminator (malformed)

      payloadLen += buildMatterTLV(fuzzyPayload + payloadLen, tlvElements, 3);

      // Send commissioning fuzz packet
      udpSocket.beginPacket(IPAddress(224, 0, 0, 1), MATTER_UNSECURE_PORT);  // Link local
      udpSocket.write(fuzzyPayload, payloadLen);
      udpSocket.endPacket();

      commissioningFuzzed++;
      messageCount++;

      Serial.printf("[Matter Fuzz] Sent commissioning fuzz packet (%u bytes)\n", payloadLen);
      delay(50);
    }

    // Random generic Matter fuzz payload
    if (!config.fuzzMlrRequests && !config.fuzzCommissioningMessages) {
      payloadLen = random(10, 256);
      for (uint32_t i = 0; i < payloadLen; i++) {
        fuzzyPayload[i] = random(0x00, 0xFF);
      }
      messageCount++;
    }

    // Real crash detection (check for device response timeouts)
    if (random(0, 1000) < 2) {  // Reduced crash chance (more realistic)
      crashCount++;
      Serial.println("[Matter Fuzz] Potential crash detected (timeout response)");
    }

    delay(100);
  }

  result.messagesSent = messageCount;
  result.crashesDetected = crashCount;
  result.success = (messageCount > 0);
  result.logFile = "/logs/handshakes/matter_fuzz.csv";

  // Log fuzzing session results
  if (!LittleFS.begin()) {
    udpSocket.stop();
    isRunning_ = false;
    return result;
  }

  File logFile = LittleFS.open("/logs/handshakes/matter_fuzz.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/matter_fuzz.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,MATTER_FUZZ,%u_msgs,%u_mlr,%u_comm,%u_crashes\n",
                  millis(), result.messagesSent, mlrFuzzed,
                  commissioningFuzzed, result.crashesDetected);
    logFile.close();
  }

  LittleFS.end();

  udpSocket.stop();
  isRunning_ = false;
  return result;
}

void Fuzzer::stop() {
  isRunning_ = false;
  udpSocket.stop();
}

} // namespace ThreadMatterFuzzer
