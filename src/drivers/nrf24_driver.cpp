#include "drivers/nrf24_driver.h"
#include "hw_config.h"
#include <SPI.h>

namespace NRF24Driver {

// NRF24 Register addresses
#define NRF_CONFIG      0x00
#define NRF_EN_AA       0x01
#define NRF_EN_RXADDR   0x02
#define NRF_SETUP_AW    0x03
#define NRF_SETUP_RETR  0x04
#define NRF_RF_CH       0x05
#define NRF_RF_SETUP    0x06
#define NRF_STATUS      0x07
#define NRF_OBSERVE_TX  0x08
#define NRF_RPD         0x09
#define NRF_RX_ADDR_P0  0x0A
#define NRF_TX_ADDR     0x10
#define NRF_RX_PW_P0    0x11
#define NRF_FIFO_STATUS 0x17

#define NRF_CMD_R_REGISTER          0x00
#define NRF_CMD_W_REGISTER          0x20
#define NRF_CMD_R_RX_PAYLOAD        0x61
#define NRF_CMD_W_TX_PAYLOAD        0xA0
#define NRF_CMD_FLUSH_TX            0xE1
#define NRF_CMD_FLUSH_RX            0xE2
#define NRF_CMD_REUSE_TX_PL         0xE3
#define NRF_CMD_R_RX_PL_WID         0x60
#define NRF_CMD_W_ACK_PAYLOAD       0xA8
#define NRF_CMD_NOP                 0xFF

static SPIClass* spi = nullptr;
static bool initialized = false;
static uint8_t payloadSize = 32;

uint8_t readReg(uint8_t reg) {
    uint8_t data;
    digitalWrite(NRF24_CS, LOW);
    spi->transfer(NRF_CMD_R_REGISTER | reg);
    data = spi->transfer(0x00);
    digitalWrite(NRF24_CS, HIGH);
    return data;
}

void writeReg(uint8_t reg, uint8_t value) {
    digitalWrite(NRF24_CS, LOW);
    spi->transfer(NRF_CMD_W_REGISTER | reg);
    spi->transfer(value);
    digitalWrite(NRF24_CS, HIGH);
}

void writeBuf(uint8_t reg, const uint8_t* buf, uint8_t len) {
    digitalWrite(NRF24_CS, LOW);
    spi->transfer(NRF_CMD_W_REGISTER | reg);
    for (uint8_t i = 0; i < len; i++) {
        spi->transfer(buf[i]);
    }
    digitalWrite(NRF24_CS, HIGH);
}

void readBuf(uint8_t reg, uint8_t* buf, uint8_t len) {
    digitalWrite(NRF24_CS, LOW);
    spi->transfer(NRF_CMD_R_REGISTER | reg);
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = spi->transfer(0x00);
    }
    digitalWrite(NRF24_CS, HIGH);
}

void strobe(uint8_t cmd) {
    digitalWrite(NRF24_CS, LOW);
    spi->transfer(cmd);
    digitalWrite(NRF24_CS, HIGH);
}

bool init(const Config& config) {
    if (initialized) return true;

    Serial.println("[NRF24] Initializing SPI...");

    spi = new SPIClass(HSPI);
    spi->begin(SPI_CLK, SPI_MISO, SPI_MOSI, NRF24_CS);
    spi->setFrequency(5000000);  // 5 MHz
    spi->setDataMode(SPI_MODE0);
    spi->setBitOrder(MSBFIRST);

    pinMode(NRF24_CS, OUTPUT);
    pinMode(NRF24_CE, OUTPUT);
    digitalWrite(NRF24_CS, HIGH);
    digitalWrite(NRF24_CE, LOW);

    delay(100);  // Power-up time

    // Configure NRF24
    writeReg(NRF_CONFIG, 0x0C);  // PWR_UP, CRCO=1
    delay(5);

    writeReg(NRF_EN_AA, 0x01);    // Enable auto-ack
    writeReg(NRF_EN_RXADDR, 0x01); // Enable RX on pipe 0
    writeReg(NRF_SETUP_AW, 0x03);  // 5-byte addresses
    writeReg(NRF_SETUP_RETR, 0x1F); // Max retries
    writeReg(NRF_RF_CH, config.channel); // Set channel
    writeReg(NRF_RF_SETUP, 0x0F);  // 2 Mbps, 0 dBm

    payloadSize = config.payloadSize;
    writeReg(NRF_RX_PW_P0, payloadSize);

    // Set default addresses
    uint8_t addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
    writeBuf(NRF_RX_ADDR_P0, addr, 5);
    writeBuf(NRF_TX_ADDR, addr, 5);

    // Verify chip
    uint8_t status = readReg(NRF_STATUS);
    Serial.printf("[NRF24] Status: 0x%02X\n", status);

    initialized = true;
    Serial.println("[NRF24] ✓ Initialized");
    return true;
}

void deinit() {
    if (!initialized) return;

    digitalWrite(NRF24_CE, LOW);
    writeReg(NRF_CONFIG, 0x08);  // Power down
    delay(50);

    if (spi) {
        spi->end();
        delete spi;
        spi = nullptr;
    }

    initialized = false;
}

void setRXAddress(const uint8_t* addr) {
    writeBuf(NRF_RX_ADDR_P0, addr, 5);
}

void setTXAddress(const uint8_t* addr) {
    writeBuf(NRF_TX_ADDR, addr, 5);
}

void setChannel(uint8_t channel) {
    if (channel > 125) channel = 125;
    writeReg(NRF_RF_CH, channel);
}

void setPayloadSize(uint8_t size) {
    if (size > 32) size = 32;
    payloadSize = size;
    writeReg(NRF_RX_PW_P0, size);
}

void setRX(bool enable) {
    if (enable) {
        uint8_t config = readReg(NRF_CONFIG);
        writeReg(NRF_CONFIG, config | 0x01);  // PRIM_RX=1
        digitalWrite(NRF24_CE, HIGH);
        Serial.println("[NRF24] RX enabled");
    } else {
        digitalWrite(NRF24_CE, LOW);
        uint8_t config = readReg(NRF_CONFIG);
        writeReg(NRF_CONFIG, config & ~0x01);
        Serial.println("[NRF24] RX disabled");
    }
}

void setTX(bool enable) {
    if (enable) {
        uint8_t config = readReg(NRF_CONFIG);
        writeReg(NRF_CONFIG, config & ~0x01);  // PRIM_RX=0
        digitalWrite(NRF24_CE, HIGH);
        delayMicroseconds(15);
        digitalWrite(NRF24_CE, LOW);
        Serial.println("[NRF24] TX enabled");
    } else {
        Serial.println("[NRF24] TX disabled");
    }
}

bool transmit(const uint8_t* data, uint8_t len) {
    if (!initialized || !data || len > 32) return false;

    // Flush TX FIFO
    strobe(NRF_CMD_FLUSH_TX);

    // Write payload
    digitalWrite(NRF24_CS, LOW);
    spi->transfer(NRF_CMD_W_TX_PAYLOAD);
    for (uint8_t i = 0; i < len; i++) {
        spi->transfer(data[i]);
    }
    digitalWrite(NRF24_CS, HIGH);

    // Pulse CE to transmit
    digitalWrite(NRF24_CE, HIGH);
    delayMicroseconds(15);
    digitalWrite(NRF24_CE, LOW);

    // Wait for TX complete
    uint32_t timeout = millis() + 100;
    while (millis() < timeout) {
        uint8_t status = readReg(NRF_STATUS);
        if (status & 0x20) {  // TX_DS
            writeReg(NRF_STATUS, 0x20);  // Clear flag
            Serial.printf("[NRF24] TX: %d bytes\n", len);
            return true;
        }
        delay(1);
    }

    Serial.println("[NRF24] TX timeout");
    return false;
}

bool receive(uint8_t* buffer, uint8_t* len) {
    if (!initialized || !buffer || !len) return false;

    if (!isDataAvailable()) return false;

    // Read payload
    digitalWrite(NRF24_CS, LOW);
    spi->transfer(NRF_CMD_R_RX_PAYLOAD);
    for (uint8_t i = 0; i < payloadSize; i++) {
        buffer[i] = spi->transfer(0x00);
    }
    digitalWrite(NRF24_CS, HIGH);

    *len = payloadSize;

    // Clear RX flag
    writeReg(NRF_STATUS, 0x40);

    // Flush RX FIFO
    strobe(NRF_CMD_FLUSH_RX);

    Serial.printf("[NRF24] RX: %d bytes\n", payloadSize);
    return true;
}

bool isDataAvailable() {
    uint8_t status = readReg(NRF_STATUS);
    return (status & 0x40);  // RX_DR
}

int8_t getRSSI() {
    // NRF24 doesn't have direct RSSI, return RPD (carrier detect)
    uint8_t rpd = readReg(NRF_RPD);
    return rpd ? -64 : -90;  // Rough estimate
}

uint8_t getTXPower() {
    uint8_t setup = readReg(NRF_RF_SETUP);
    return (setup >> 1) & 0x03;
}

void setTXPower(uint8_t power) {
    if (power > 3) power = 3;
    uint8_t setup = readReg(NRF_RF_SETUP);
    setup = (setup & ~0x06) | ((power & 0x03) << 1);
    writeReg(NRF_RF_SETUP, setup);
}

}  // namespace NRF24Driver
