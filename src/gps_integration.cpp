#include "gps_integration.h"
#include "debug_logger.h"
#include <cstdio>

bool GPSIntegration::begin() {
  // GPS is connected via UART1 (RX=18, TX=17, 9600 baud)
  // Requires TinyGPSPlus library or similar

  running = true;
  DebugLogger::println("[GPS] Integration started");
  DebugLogger::println("[GPS] Waiting for satellite fix...");

  return true;
}

void GPSIntegration::update() {
  if (!running) return;

  // In production, parse NMEA data from Serial1
  // This is a stub showing the interface

  // Example: read from Serial1 and update lastLocation
  parseGPSData();
}

GPSIntegration::GPSLocation GPSIntegration::getLocation() const {
  return lastLocation;
}

std::string GPSIntegration::getLocationString() const {
  if (!lastLocation.isValid) {
    return "No GPS fix";
  }

  char buffer[128];
  snprintf(buffer, sizeof(buffer),
    "%.6f, %.6f (±%.1fm) - %u satellites",
    lastLocation.latitude, lastLocation.longitude,
    lastLocation.accuracy, satelliteCount);

  return buffer;
}

void GPSIntegration::stop() {
  running = false;
  lastLocation.isValid = false;
  DebugLogger::println("[GPS] Integration stopped");
}

void GPSIntegration::parseGPSData() {
  // In production implementation:
  // 1. Read NMEA sentences from Serial1
  // 2. Parse GGA, RMC, GSA sentences
  // 3. Extract: lat/lon, altitude, satellite count, HDOP
  // 4. Validate checksum
  // 5. Update lastLocation

  // This is where actual GPS parsing happens
  // Using TinyGPSPlus or similar library

  if (lastLocation.isValid) {
    DebugLogger::printf("[GPS] Position: %.6f, %.6f (%u sats, HDOP=%.1f)\n",
      lastLocation.latitude, lastLocation.longitude,
      satelliteCount, hdop);
  }
}
