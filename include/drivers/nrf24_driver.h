#pragma once
#include <Arduino.h>

namespace NRF24Driver {

struct Config {
    uint8_t channel;           // 0-125 (2.4 GHz bands)
    uint8_t payloadSize;       // 1-32 bytes
    uint8_t addressWidth;      // 3-5 bytes
    bool rxEnabled;
    bool txEnabled;
};

// Initialize NRF24 module
bool init(const Config& config);

// Deinitialize
void deinit();

// Set RX/TX addresses (5 bytes max)
void setRXAddress(const uint8_t* addr);
void setTXAddress(const uint8_t* addr);

// Set RF channel (0-125)
void setChannel(uint8_t channel);

// Set payload size
void setPayloadSize(uint8_t size);

// Enable/disable RX or TX
void setRX(bool enable);
void setTX(bool enable);

// Transmit data
bool transmit(const uint8_t* data, uint8_t len);

// Receive data
bool receive(uint8_t* buffer, uint8_t* len);

// Check if data available in RX FIFO
bool isDataAvailable();

// Get RSSI (by measuring on multiple channels)
int8_t getRSSI();

// Get TX power setting (0-3 = -18,-12,-6,0 dBm)
uint8_t getTXPower();

// Set TX power
void setTXPower(uint8_t power);

}  // namespace NRF24Driver
