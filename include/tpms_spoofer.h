#pragma once
#include <Arduino.h>
#include <vector>

namespace TPMSSpoofer {

// Tire Pressure Monitoring System (TPMS)
// Frequency: 315 MHz (luxury) or 433 MHz (standard)
// Modulation: FSK (Frequency Shift Keying)
// Frame: ID (32-bit) + Pressure (8-bit) + Temperature (8-bit) + Checksum
// Broadcast: Every 60-90 seconds when vehicle moving

struct TPMSSensor {
    uint32_t sensorID;        // Unique 32-bit sensor ID
    uint8_t pressure;         // Pressure in PSI (or 0.25 PSI/bit)
    uint8_t temperature;      // Temperature in Celsius (offset by -40)
};

struct TPMSConfig {
    uint32_t durationMs;      // Attack duration
    uint32_t frequency;       // 315000000 or 433000000 Hz
    uint8_t attackMode;       // 0=spoof pressure low, 1=high, 2=invalid, 3=replay
    std::vector<TPMSSensor> targetSensors;  // Which sensors to spoof
    uint8_t spoofedPressure;  // Fake pressure value
};

struct TPMSResult {
    bool success;
    uint32_t framesTransmitted;
    uint32_t durationMs;
    std::vector<uint32_t> spoofedSensorIDs;
    String attackDescription;
    String error;
};

// Capture legitimate TPMS sensor IDs and data via passive listening
// Prerequisite: CC1101 radio configured for 315/433 MHz
// Captures: Sensor ID, pressure, temperature, transmission rate
TPMSResult captureTPMSSensors(uint32_t captureDurationMs, uint32_t frequency);

// Spoof pressure LOW on target sensor
// Effect: Dashboard warning "Check tire pressure", potential driver distraction/panic
// Safety: Vehicle still operates normally (just annoying alert)
TPMSResult spoofTPMSLow(const TPMSConfig& config);

// Spoof pressure HIGH to mask actual low tire (DANGEROUS)
// Effect: Hide real tire pressure problem, risk blowout/loss of control
// Safety: Lab only, extreme risk
TPMSResult spoofTPMSHigh(const TPMSConfig& config);

// Replay legitimate TPMS frame continuously
// Effect: Vehicle receives duplicate sensor updates, potential system confusion
TPMSResult replayTPMSFrame(const TPMSConfig& config, const TPMSSensor& targetSensor);

// Transmit malformed/invalid TPMS frames
// Effect: ECU confusion, potential DoS on TPMS subsystem
TPMSResult fuzzyTPMSFrames(const TPMSConfig& config);

// Analyze captured TPMS data for patterns
TPMSResult analyzeTPMSData(const std::vector<TPMSSensor>& sensors);

}  // namespace TPMSSpoofer
