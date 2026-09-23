#pragma once
#include <Arduino.h>

namespace GPSDriver {

struct Location {
    double latitude;
    double longitude;
    float altitude;
    float speed;
    uint8_t satellites;
    bool hasFix;
};

// Initialize GPS (UART2, 9600 baud)
bool init();
void deinit();

// Update GPS data from UART
void update();

// Get current location
Location getLocation();

// Check if GPS has fix
bool hasFix();

// Get satellite count
uint8_t getSatelliteCount();

}  // namespace GPSDriver
