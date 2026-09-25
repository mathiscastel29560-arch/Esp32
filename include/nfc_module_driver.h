#pragma once
#include <Arduino.h>

namespace NFCModuleDriver {

struct NFCCard {
    uint8_t uid[10];
    uint8_t uidLength;
    uint16_t atqa;
    uint8_t sak;
    bool detected;
    uint32_t detectionTime;
};

struct NFCModuleStatus {
    bool initialized;
    bool cardDetected;
    uint32_t cardsRead;
    uint32_t lastDetectionTime;
    const char* errorMessage;
};

// UART2 Configuration
#define NFC_MODULE_UART UART_NUM_2
#define NFC_MODULE_RX_PIN 43
#define NFC_MODULE_TX_PIN 44
#define NFC_MODULE_BAUD 9600
#define NFC_MODULE_BUF_SIZE 256

void initNFCModule();
bool beginNFCReading();
void stopNFCReading();

NFCCard readNFCCard();
NFCCard readNFCCardTimeout(uint32_t timeoutMs);
NFCModuleStatus getNFCStatus();

bool writeNFCCard(const uint8_t* data, uint8_t length);
bool formatNFCCard();
bool cloneNFCCard(const NFCCard& sourceCard);

void displayNFCStatus();
void scanNFCEnvironment();

} // namespace NFCModuleDriver
