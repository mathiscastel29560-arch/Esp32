#pragma once

#include <string>
#include <cstdint>

// GPS integration for location-based audits
class GPSIntegration {
public:
  struct GPSLocation {
    float latitude;
    float longitude;
    float altitude;
    float accuracy;
    uint32_t timestamp;
    bool isValid;
  };

  static GPSIntegration& getInstance() {
    static GPSIntegration instance;
    return instance;
  }

  // Initialize GPS
  bool begin();

  // Update GPS position
  void update();

  // Get current location
  GPSLocation getLocation() const;

  // Check if GPS has valid fix
  bool hasValidFix() const { return lastLocation.isValid; }

  // Get satellite count
  uint8_t getSatelliteCount() const { return satelliteCount; }

  // Get HDOP (accuracy metric)
  float getHDOP() const { return hdop; }

  // Get location as formatted string
  std::string getLocationString() const;

  // Stop GPS
  void stop();

  // Check if running
  bool isRunning() const { return running; }

private:
  GPSIntegration() = default;

  bool running = false;
  GPSLocation lastLocation = {0, 0, 0, 0, 0, false};
  uint8_t satelliteCount = 0;
  float hdop = 0.0f;

  // Helper functions
  void parseGPSData();
};

#endif // GPS_INTEGRATION_H
