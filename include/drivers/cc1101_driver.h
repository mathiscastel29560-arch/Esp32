#pragma once
#include <Arduino.h>
#include <SPI.h>

namespace CC1101Driver {

// CC1101 Register addresses
enum CC1101_Registers {
    CC1101_IOCFG2    = 0x00,
    CC1101_IOCFG1    = 0x01,
    CC1101_IOCFG0    = 0x02,
    CC1101_FIFOTHR   = 0x03,
    CC1101_SYNC1     = 0x04,
    CC1101_SYNC0     = 0x05,
    CC1101_PKTLEN    = 0x06,
    CC1101_PKTCTRL1  = 0x07,
    CC1101_PKTCTRL0  = 0x08,
    CC1101_ADDR      = 0x09,
    CC1101_HWVERSION = 0x1F,
    CC1101_FREQ2     = 0x0F,
    CC1101_FREQ1     = 0x10,
    CC1101_FREQ0     = 0x11,
    CC1101_MDMCFG4   = 0x12,
    CC1101_MDMCFG3   = 0x13,
    CC1101_MDMCFG2   = 0x14,
    CC1101_MDMCFG1   = 0x15,
    CC1101_MDMCFG0   = 0x16,
    CC1101_DEVIATN   = 0x17,
    CC1101_MCSM2     = 0x1B,
    CC1101_MCSM1     = 0x1C,
    CC1101_MCSM0     = 0x1D,
    CC1101_FOCCFG    = 0x19,
    CC1101_BSCFG     = 0x1A,
    CC1101_AGCCTRL2  = 0x1E,
    CC1101_AGCCTRL1  = 0x1F,
    CC1101_AGCCTRL0  = 0x20,
    CC1101_WOWEVENT  = 0x78,
    CC1101_TXFIFO    = 0x3F,
    CC1101_RXFIFO    = 0x3E,
};

enum CC1101_Command {
    CC1101_SRES      = 0x30,  // Reset
    CC1101_SFSTXON   = 0x31,  // Enable and calibrate frequency synthesizer
    CC1101_SFTX      = 0x3B,  // Flush TX FIFO
    CC1101_SFRX      = 0x3A,  // Flush RX FIFO
    CC1101_STX       = 0x35,  // Enable TX
    CC1101_SRX       = 0x34,  // Enable RX
    CC1101_SIDLE     = 0x36,  // Exit RX/TX
};

struct Config {
    uint32_t frequency;        // In Hz (e.g., 433000000 for 433 MHz)
    uint16_t baudrate;         // SPI speed in kHz
    uint8_t modulation;        // 0=FSK, 1=GFSK, 2=ASK, 3=4-FSK
    bool rxEnabled;
    bool txEnabled;
};

// Initialize CC1101 module
bool init(const Config& config);

// Deinitialize (power down)
void deinit();

// Set frequency in Hz
void setFrequency(uint32_t freq);

// Set modulation (FSK, GFSK, ASK, etc)
void setModulation(uint8_t mod);

// Enable/disable RX or TX
void setRX(bool enable);
void setTX(bool enable);

// Transmit data
bool transmit(const uint8_t* data, uint8_t len);

// Receive data (non-blocking check)
bool receive(uint8_t* buffer, uint8_t* len, uint8_t maxLen);

// Get RSSI (Received Signal Strength Indicator)
int8_t getRSSI();

// Check if packet is ready in RX FIFO
bool isRXReady();

// Manual SPI register read/write (for advanced config)
uint8_t readReg(uint8_t reg);
void writeReg(uint8_t reg, uint8_t value);

// Send strobe command
void strobe(uint8_t cmd);

}  // namespace CC1101Driver
