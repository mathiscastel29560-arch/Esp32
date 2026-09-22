#ifndef IR_SNIFFER_H
#define IR_SNIFFER_H

#include <Arduino.h>
#include <vector>
#include <IRrecv.h>
#include <IRutils.h>

namespace IrSniffer {

struct IrCode {
  uint32_t timestamp;
  std::vector<uint16_t> timings; // Pulse/space timings in microseconds
  String protocol; // NEC, RC5, Sony, etc
  uint32_t address;
  uint32_t command;
  int32_t rssi;
};

struct SnifferConfig {
  uint8_t rxPin;
  uint32_t captureTimeMs;
  bool identifyProtocol;
  bool continuousCapture;
};

struct SnifferResult {
  bool success;
  std::vector<IrCode> codes;
  uint32_t codesCapTured;
  String dominantProtocol;
  String logFile;
  String error;
};

class IrSniffer {
public:
  IrSniffer(uint8_t rxPin = 15); // GPIO15 default for IR receiver
  SnifferResult captureIrCodes(const SnifferConfig& config);
  String identifyProtocol(const decode_results& results);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  IRrecv irrecv_;
  bool isRunning_;
  unsigned long startTime_;

  void logCode(const IrCode& code);
  String decodeTypeToString(decode_type_t type);
};

} // namespace IrSniffer

#endif
