#ifndef ZIGBEE_COMMAND_INJECTION_H
#define ZIGBEE_COMMAND_INJECTION_H

#include <Arduino.h>

namespace ZigbeeCommandInjection {

struct InjectionConfig {
  uint16_t targetPanId;
  uint8_t targetChannel;
  uint32_t durationMs;
  bool broadcastCommands;
};

struct InjectionResult {
  bool success;
  uint32_t commandsSent;
  uint32_t devicesAffected;
  String logFile;
};

class CommandInjector {
public:
  CommandInjector();
  InjectionResult injectCommands(const InjectionConfig& config);
  void stop();

private:
  bool isRunning_;
};

} // namespace ZigbeeCommandInjection

#endif
