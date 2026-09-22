#ifndef GPIO_DEBUGGER_H
#define GPIO_DEBUGGER_H

#include <Arduino.h>

namespace GpioDebugger {

enum DebugInterface {
  UART,
  JTAG,
  SWD
};

struct DebugConfig {
  DebugInterface interface;
  uint32_t baudRate;
  uint32_t scanTimeoutMs;
};

struct DebugResult {
  bool success;
  bool interfaceFound;
  String deviceInfo;
  uint32_t registersRead;
  String logFile;
};

class Debugger {
public:
  Debugger();
  DebugResult scanDebugInterfaces(const DebugConfig& config);
  DebugResult dumpFirmware();
  void stop();

private:
  bool isRunning_;
};

} // namespace GpioDebugger

#endif
