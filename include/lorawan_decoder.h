#ifndef LORAWAN_DECODER_H
#define LORAWAN_DECODER_H

#include <Arduino.h>
#include <vector>

namespace LoRaWanDecoder {

struct LoRaFrame {
  uint32_t timestamp;
  uint8_t mhdr;
  uint32_t appEui;
  uint32_t devEui;
  uint16_t devNonce;
  String decodedData;
  int32_t rssi;
};

struct DecoderConfig {
  uint32_t scanDurationMs;
  bool attemptDecrypt;
  bool trackDevices;
};

struct DecoderResult {
  bool success;
  std::vector<LoRaFrame> frames;
  uint32_t framesDecoded;
  uint32_t devicesDiscovered;
  String logFile;
};

class LoRaDecoder {
public:
  LoRaDecoder();
  DecoderResult decodeLoRaWan(const DecoderConfig& config);
  void stop();

private:
  bool isRunning_;
};

} // namespace LoRaWanDecoder

#endif
