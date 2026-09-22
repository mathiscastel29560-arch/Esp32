#include "drivers/cc1101_driver.h"
#include "hw_config.h"

namespace CC1101Driver {

static SPIClass* spi = nullptr;
static bool initialized = false;

// CC1101 default configuration for 433 MHz FSK
static const uint8_t CC1101_CONFIG[] = {
    0x47,  // IOCFG2       GDO2 output pin configuration
    0x0B,  // IOCFG1       GDO1 output pin configuration
    0x3F,  // IOCFG0       GDO0 output pin configuration (packet received)
    0x0E,  // FIFOTHR      RX FIFO and TX FIFO thresholds
    0xD3,  // SYNC1        Sync word, high byte
    0x91,  // SYNC0        Sync word, low byte
    0xFF,  // PKTLEN       Packet length
    0x04,  // PKTCTRL1     Packet automation control
    0x45,  // PKTCTRL0     Packet automation control
    0x00,  // ADDR         Device address
    0x00,  // CHANNR       Channel number
    0x08,  // FSCTRL1      Frequency synthesizer control
    0x00,  // FSCTRL0      Frequency synthesizer control
    0x0E,  // FREQ2        Frequency control word, high byte (433 MHz)
    0x51,  // FREQ1        Frequency control word, middle byte
    0x64,  // FREQ0        Frequency control word, low byte
    0xCA,  // MDMCFG4      Modem configuration
    0x83,  // MDMCFG3      Modem configuration
    0x93,  // MDMCFG2      Modem configuration (FSK, 30/32 sync bits)
    0x22,  // MDMCFG1      Modem configuration
    0xF8,  // MDMCFG0      Modem configuration
    0x47,  // DEVIATN      Modem deviation setting
    0x00,  // MCSM2        Main Radio Control State Machine config
    0x18,  // MCSM1        Main Radio Control State Machine config
    0x18,  // MCSM0        Main Radio Control State Machine config
    0x16,  // FOCCFG       Frequency Offset Compensation Configuration
    0x6C,  // BSCFG        Bit Synchronization Configuration
    0x03,  // AGCCTRL2     AGC control
    0x40,  // AGCCTRL1     AGC control
    0x91,  // AGCCTRL0     AGC control
};

bool init(const Config& config) {
    if (initialized) return true;

    Serial.println("[CC1101] Initializing SPI...");

    // Guard: Warn if RadioLib CC1101 is already in use (SubGhz module)
    // SubGhz uses RadioLib which creates its own CC1101 instance
    Serial.println("[CC1101] ⚠️  WARNING: Ensure SubGhz module is NOT active");
    Serial.println("[CC1101]  Conflicts possible if SubGhz and CC1101Driver used simultaneously");

    // Initialize SPI - use HSPI to avoid conflicts with TFT
    spi = new SPIClass(HSPI);
    spi->begin(SPI_CLK, SPI_MISO, SPI_MOSI, CC1101_CS);
    spi->setFrequency(1000000);  // 1 MHz SPI clock
    spi->setDataMode(SPI_MODE0);
    spi->setBitOrder(MSBFIRST);

    // Configure CS pin
    pinMode(CC1101_CS, OUTPUT);
    digitalWrite(CC1101_CS, HIGH);

    // Configure GDO0 (RX interrupt)
    pinMode(CC1101_GDO0, INPUT);

    // Reset CC1101
    digitalWrite(CC1101_CS, LOW);
    delay(10);
    digitalWrite(CC1101_CS, HIGH);
    delayMicroseconds(40);

    strobe(CC1101_SRES);
    delay(100);

    // Verify chip ID
    uint8_t chipId = readReg(CC1101_HWVERSION);
    Serial.printf("[CC1101] Chip ID: 0x%02X\n", chipId);
    if (chipId != 0x04) {
        Serial.println("[CC1101] ERROR: Invalid chip ID!");
        return false;
    }

    // Load configuration
    for (uint8_t i = 0; i < sizeof(CC1101_CONFIG); i++) {
        writeReg(i, CC1101_CONFIG[i]);
    }

    // Set frequency
    setFrequency(config.frequency);

    // Set modulation
    setModulation(config.modulation);

    // Configure RX/TX
    if (config.rxEnabled) setRX(true);
    if (config.txEnabled) setTX(true);

    initialized = true;
    Serial.println("[CC1101] ✓ Initialized successfully");
    return true;
}

void deinit() {
    if (!initialized) return;

    strobe(CC1101_SIDLE);
    delay(100);

    if (spi) {
        spi->end();
        delete spi;
        spi = nullptr;
    }

    initialized = false;
    Serial.println("[CC1101] Deinitialized");
}

void setFrequency(uint32_t freq) {
    // Fxosc = 26 MHz, Freq = Fxosc / 2^16 * [FREQ2:FREQ0]
    // FREQ = (freq * 2^16) / Fxosc
    uint32_t freqReg = (uint32_t)((freq / 1000000.0) * 65536 / 26);

    uint8_t freq2 = (freqReg >> 16) & 0xFF;
    uint8_t freq1 = (freqReg >> 8) & 0xFF;
    uint8_t freq0 = freqReg & 0xFF;

    writeReg(CC1101_FREQ2, freq2);
    writeReg(CC1101_FREQ1, freq1);
    writeReg(CC1101_FREQ0, freq0);

    Serial.printf("[CC1101] Frequency set to %lu Hz (regs: 0x%02X 0x%02X 0x%02X)\n",
                  freq, freq2, freq1, freq0);
}

void setModulation(uint8_t mod) {
    uint8_t mdmcfg2 = readReg(CC1101_MDMCFG2);
    mdmcfg2 = (mdmcfg2 & 0x8F) | ((mod & 0x07) << 4);
    writeReg(CC1101_MDMCFG2, mdmcfg2);

    const char* modNames[] = {"FSK", "GFSK", "ASK", "4-FSK"};
    Serial.printf("[CC1101] Modulation set to %s\n",
                  (mod < 4) ? modNames[mod] : "Unknown");
}

void setRX(bool enable) {
    if (enable) {
        strobe(CC1101_SRX);
        Serial.println("[CC1101] RX enabled");
    } else {
        strobe(CC1101_SIDLE);
        Serial.println("[CC1101] RX disabled");
    }
}

void setTX(bool enable) {
    if (enable) {
        strobe(CC1101_STX);
        Serial.println("[CC1101] TX enabled");
    } else {
        strobe(CC1101_SIDLE);
        Serial.println("[CC1101] TX disabled");
    }
}

bool transmit(const uint8_t* data, uint8_t len) {
    if (!initialized || !data || len == 0 || len > 255) return false;

    // Flush TX FIFO
    strobe(CC1101_SFTX);
    delay(10);

    // Write packet length
    writeReg(CC1101_PKTLEN, len);

    // Write data to TX FIFO
    digitalWrite(CC1101_CS, LOW);
    spi->transfer(CC1101_TXFIFO | 0x40);  // Write mode
    for (uint8_t i = 0; i < len; i++) {
        spi->transfer(data[i]);
    }
    digitalWrite(CC1101_CS, HIGH);

    // Transmit
    strobe(CC1101_STX);

    // Wait for transmission to complete (~50ms max for 255 bytes)
    uint32_t timeout = millis() + 100;
    while (millis() < timeout) {
        if (digitalRead(CC1101_GDO0) == 0) {
            break;  // Transmission complete
        }
        delay(1);
    }

    strobe(CC1101_SIDLE);
    Serial.printf("[CC1101] TX: %d bytes\n", len);
    return true;
}

bool receive(uint8_t* buffer, uint8_t* len, uint8_t maxLen) {
    if (!initialized || !buffer || !len) return false;

    if (!isRXReady()) return false;

    // Read packet length
    uint8_t pktLen = readReg(CC1101_RXFIFO);
    if (pktLen > maxLen) pktLen = maxLen;

    *len = pktLen;

    // Read packet data
    digitalWrite(CC1101_CS, LOW);
    spi->transfer(CC1101_RXFIFO);  // Read mode (address auto-increment)
    for (uint8_t i = 0; i < pktLen; i++) {
        buffer[i] = spi->transfer(0x00);
    }
    digitalWrite(CC1101_CS, HIGH);

    // Flush RX FIFO
    strobe(CC1101_SFRX);

    Serial.printf("[CC1101] RX: %d bytes\n", pktLen);
    return true;
}

int8_t getRSSI() {
    uint8_t rssi = readReg(0x34);  // RSSI status register
    if (rssi >= 128) {
        return (rssi - 256) / 2 - 74;
    } else {
        return rssi / 2 - 74;
    }
}

bool isRXReady() {
    // Check if GDO0 is high (packet received)
    return digitalRead(CC1101_GDO0) == HIGH;
}

uint8_t readReg(uint8_t reg) {
    uint8_t data;
    digitalWrite(CC1101_CS, LOW);
    spi->transfer(reg | 0x80);  // Read mode
    data = spi->transfer(0x00);
    digitalWrite(CC1101_CS, HIGH);
    return data;
}

void writeReg(uint8_t reg, uint8_t value) {
    digitalWrite(CC1101_CS, LOW);
    spi->transfer(reg);  // Write mode (address)
    spi->transfer(value);
    digitalWrite(CC1101_CS, HIGH);
}

void strobe(uint8_t cmd) {
    digitalWrite(CC1101_CS, LOW);
    spi->transfer(cmd);
    digitalWrite(CC1101_CS, HIGH);
    delayMicroseconds(10);
}

}  // namespace CC1101Driver
