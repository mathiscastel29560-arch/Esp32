#include "lorawan_decoder.h"
#include <LittleFS.h>
#include <RadioLib.h>

namespace LoRaWanDecoder {

// RadioLib LoRa instance (assuming SX1276 or similar on standard pins)
// Pins: NSS=5, DIO0=14, RST=27, DIO1=33 (ESP32 standard LoRa shield)
SPIClass* spi = nullptr;
SX1276 radio = new Module(5, 14, 27, 33);

LoRaDecoder::LoRaDecoder() : isRunning_(false) {}

DecoderResult LoRaDecoder::decodeLoRaWan(const DecoderConfig& config) {
  DecoderResult result;
  result.success = false;

  isRunning_ = true;
  unsigned long startTime = millis();

  // Initialize LoRa module for real packet reception
  if (radio.begin(868.1)) {  // LoRaWAN EU868 frequency
    Serial.println("[LoRaWAN] Radio initialized");

    // Configure for LoRaWAN parameters
    radio.setSpreadingFactor(7);      // SF7 (common LoRaWAN)
    radio.setBandwidth(125000);       // 125 kHz bandwidth
    radio.setCodingRate(5);           // CR4/5
    radio.setPreambleLength(8);       // Standard LoRaWAN preamble
    radio.setDio0Action(NULL);        // No interrupt needed for simple RX

  while (isRunning_ && (millis() - startTime) < config.scanDurationMs) {
    if ((esp_random() % 100) < 15) { // 15% chance per cycle
      LoRaFrame frame;
      frame.timestamp = millis();
      frame.mhdr = (esp_random() % 128);
      frame.appEui = (esp_random() % 4294967295);
      frame.devEui = (esp_random() % 4294967295);
      frame.devNonce = (esp_random() % 65535);
      frame.rssi = ((esp_random() % 50) + -120);

    Serial.println("[LoRaWAN] Starting RX mode - scanning for frames");

    while (isRunning_ && (millis() - startTime) < config.scanDurationMs) {
      // Real LoRa packet reception
      uint8_t rxBuf[256];
      size_t rxLen = 255;

      // Receive LoRa packet (non-blocking with timeout)
      int state = radio.receive(rxBuf, &rxLen);

      if (state == RADIOLIB_ERR_NONE && rxLen > 0) {
        // Successfully received LoRa frame
        LoRaFrame frame;
        frame.timestamp = millis();
        frame.rssi = radio.getRSSI();

        // Parse LoRaWAN frame structure
        // Byte 0: MHDR (MAC Header)
        if (rxLen > 0) {
          frame.mhdr = rxBuf[0];

          // Decode MHDR
          uint8_t msgType = (frame.mhdr >> 5) & 0x07;
          uint8_t major = frame.mhdr & 0x03;

          // LoRaWAN message types:
          // 0=Join Request, 1=Join Accept, 2=Unconfirmed Data Up
          // 3=Unconfirmed Data Down, 4=Confirmed Data Up
          // 5=Confirmed Data Down, 6=Rejoin Request, 7=Proprietary

          if (msgType == 0) { // Join Request
            frame.decodedData = "JOIN_REQUEST";
            if (rxLen >= 19) {
              // Extract AppEUI (bytes 1-8) and DevEUI (bytes 9-16)
              frame.appEui = 0;
              frame.devEui = 0;
              for (int i = 0; i < 4; i++) {
                frame.appEui = (frame.appEui << 8) | rxBuf[i + 1];
                frame.devEui = (frame.devEui << 8) | rxBuf[i + 9];
              }
              frame.devNonce = (rxBuf[17] << 8) | rxBuf[18];
              devicesFound++;
            }
          } else if (msgType >= 2 && msgType <= 5) { // Data frames
            frame.decodedData = "DATA_FRAME";
            if (rxLen > 13) {
              frame.appEui = (rxBuf[1] << 24) | (rxBuf[2] << 16) |
                            (rxBuf[3] << 8) | rxBuf[4];
              frame.devNonce = (rxBuf[5] << 8) | rxBuf[6];
            }
          } else {
            frame.decodedData = "UNKNOWN_TYPE_" + String(msgType);
          }

          // Extract and verify MIC if enabled
          if (config.attemptDecrypt && rxLen >= 4) {
            uint32_t mic = (rxBuf[rxLen-4] << 24) | (rxBuf[rxLen-3] << 16) |
                          (rxBuf[rxLen-2] << 8) | rxBuf[rxLen-1];
            frame.decodedData += "_MIC:" + String(mic, HEX);
          }

          result.frames.push_back(frame);
          frameCount++;

          Serial.printf("[LoRaWAN] Frame RX: Type=%d, RSSI=%d, Len=%u\n",
                       msgType, frame.rssi, rxLen);
        }
      } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
        // No frame received in timeout window - normal
      } else if (state != RADIOLIB_ERR_NONE) {
        // Decode error
        decodeErrors++;
        Serial.printf("[LoRaWAN] Decode error: %d\n", state);
      }

      delay(10);  // Small delay to prevent busy-waiting
    }

    result.framesDecoded = frameCount;
    result.devicesDiscovered = devicesFound;
    result.success = (frameCount > 0);

    radio.sleep();  // Put radio to sleep
  } else {
    result.error = "Failed to initialize LoRa radio";
    Serial.println("[LoRaWAN] ERROR: Radio init failed");
  }

  result.logFile = "/logs/handshakes/lorawan.csv";

  // Log results
  if (!LittleFS.begin()) {
    isRunning_ = false;
    return result;
  }

  File logFile = LittleFS.open("/logs/handshakes/lorawan.csv", "a");
  if (!logFile) {
    LittleFS.mkdir("/logs/handshakes");
    logFile = LittleFS.open("/logs/handshakes/lorawan.csv", "a");
  }

  if (logFile) {
    logFile.printf("%lu,LORAWAN_SCAN,%u_frames,%u_devices\n",
                  millis(), result.framesDecoded, result.devicesDiscovered);
    logFile.close();
  }

  LittleFS.end();
  isRunning_ = false;
  return result;
}

void LoRaDecoder::stop() {
  isRunning_ = false;
  radio.sleep();
}

} // namespace LoRaWanDecoder
