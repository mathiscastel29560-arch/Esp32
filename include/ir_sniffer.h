#ifndef IR_SNIFFER_H
#define IR_SNIFFER_H

#include <Arduino.h>
#include <vector>

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
  IrSniffer();
  SnifferResult captureIrCodes(const SnifferConfig& config);
  String identifyProtocol(const std::vector<uint16_t>& timings);
  void stop();
  bool isRunning() const { return isRunning_; }

private:
  bool isRunning_;
  unsigned long startTime_;

  void logCode(const IrCode& code);
};

} // namespace IrSniffer

#endif
