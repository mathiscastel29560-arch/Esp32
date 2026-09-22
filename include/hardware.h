#pragma once
#include <Arduino.h>

// Hardware abstraction layer - Centralizes all real driver initialization

namespace Hardware {

// Initialize all hardware modules
bool initAll();

// Get hardware status
bool isCC1101Ready();
bool isNRF24Ready();
bool isPN532Ready();
bool isGPSReady();
bool isRTCReady();
bool isBatteryReady();

// Shutdown sequence
void shutdown();

// Get device name/version
String getDeviceName();

}  // namespace Hardware
