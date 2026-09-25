#include "nfc_module_driver.h"
#include <HardwareSerial.h>

namespace NFCModuleDriver {

static HardwareSerial nfcSerial(2);
static NFCModuleStatus moduleStatus = {false, false, 0, 0, ""};
static NFCCard lastCard = {{0}, 0, 0, 0, false, 0};

void initNFCModule() {
    Serial.println("\n🔍 Initializing NFC Module V3...");
    
    nfcSerial.begin(NFC_MODULE_BAUD, SERIAL_8N1, NFC_MODULE_RX_PIN, NFC_MODULE_TX_PIN);
    
    if (!nfcSerial) {
        moduleStatus.initialized = false;
        moduleStatus.errorMessage = "UART2 initialization failed";
        Serial.println("❌ NFC Module UART initialization failed");
        return;
    }
    
    delay(500);
    
    moduleStatus.initialized = true;
    moduleStatus.errorMessage = "";
    moduleStatus.cardsRead = 0;
    
    Serial.printf("✓ NFC Module V3 initialized on UART2 (GPIO 43/44)\n");
    Serial.printf("✓ Baud rate: %u\n", NFC_MODULE_BAUD);
}

bool beginNFCReading() {
    if (!moduleStatus.initialized) {
        initNFCModule();
    }
    
    if (!nfcSerial) {
        moduleStatus.errorMessage = "UART2 not available";
        return false;
    }
    
    Serial.println("▶️ NFC reading started...");
    return true;
}

void stopNFCReading() {
    Serial.println("⏸️ NFC reading stopped");
}

NFCCard readNFCCard() {
    return readNFCCardTimeout(5000);
}

NFCCard readNFCCardTimeout(uint32_t timeoutMs) {
    NFCCard card = {{0}, 0, 0, 0, false, 0};
    
    if (!moduleStatus.initialized) {
        moduleStatus.errorMessage = "NFC Module not initialized";
        return card;
    }
    
    uint32_t startTime = millis();
    uint8_t buffer[NFC_MODULE_BUF_SIZE];
    uint32_t bufPos = 0;
    
    while ((millis() - startTime) < timeoutMs) {
        if (nfcSerial.available()) {
            uint8_t byte = nfcSerial.read();
            
            if (bufPos < NFC_MODULE_BUF_SIZE) {
                buffer[bufPos++] = byte;
            }
            
            // Check for card detection pattern (simplified)
            if (bufPos >= 7) {
                // UID typically starts at a specific position
                card.uidLength = (bufPos > 10) ? 7 : bufPos;
                memcpy(card.uid, buffer, card.uidLength);
                card.detected = true;
                card.detectionTime = millis();
                
                moduleStatus.cardDetected = true;
                moduleStatus.cardsRead++;
                moduleStatus.lastDetectionTime = millis();
                
                // Display detected card
                Serial.printf("✓ NFC Card detected - UID: ");
                for (uint8_t i = 0; i < card.uidLength; i++) {
                    Serial.printf("%02X ", card.uid[i]);
                }
                Serial.println();
                
                lastCard = card;
                return card;
            }
        }
    }
    
    moduleStatus.cardDetected = false;
    moduleStatus.errorMessage = "No card detected (timeout)";
    
    return card;
}

NFCModuleStatus getNFCStatus() {
    return moduleStatus;
}

bool writeNFCCard(const uint8_t* data, uint8_t length) {
    if (!moduleStatus.initialized || !moduleStatus.cardDetected) {
        moduleStatus.errorMessage = "Card not detected";
        return false;
    }
    
    // Send write command to NFC module
    uint8_t writeCmd[] = {0xFF, 0xD0, 0x00, 0x00};
    nfcSerial.write(writeCmd, sizeof(writeCmd));
    nfcSerial.write(data, length);
    
    delay(500);
    
    Serial.printf("✓ Wrote %u bytes to NFC card\n", length);
    return true;
}

bool formatNFCCard() {
    if (!moduleStatus.initialized || !moduleStatus.cardDetected) {
        moduleStatus.errorMessage = "Card not detected";
        return false;
    }
    
    // Send format command
    uint8_t formatCmd[] = {0xFF, 0xD0, 0x00, 0x00, 0x00};
    nfcSerial.write(formatCmd, sizeof(formatCmd));
    
    delay(1000);
    
    Serial.println("✓ NFC card formatted");
    return true;
}

bool cloneNFCCard(const NFCCard& sourceCard) {
    if (!moduleStatus.initialized) {
        moduleStatus.errorMessage = "NFC Module not initialized";
        return false;
    }
    
    Serial.printf("⏳ Cloning NFC card (UID length: %u)...\n", sourceCard.uidLength);
    
    // Read source card data
    if (!sourceCard.detected) {
        moduleStatus.errorMessage = "Source card not detected";
        return false;
    }
    
    // Place target card and wait
    Serial.println("📌 Place target card on module...");
    delay(2000);
    
    // Write source data to target
    if (writeNFCCard(sourceCard.uid, sourceCard.uidLength)) {
        Serial.println("✓ Card cloned successfully");
        return true;
    }
    
    moduleStatus.errorMessage = "Clone operation failed";
    return false;
}

void displayNFCStatus() {
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║              NFC MODULE V3 STATUS                          ║");
    Serial.println("╠════════════════════════════════════════════════════════════╣");
    
    Serial.printf("║ Status: %s\n", moduleStatus.initialized ? "✓ Initialized" : "❌ Not initialized");
    Serial.printf("║ Card Detected: %s\n", moduleStatus.cardDetected ? "Yes" : "No");
    Serial.printf("║ Cards Read: %u\n", moduleStatus.cardsRead);
    Serial.printf("║ Last Detection: %u ms ago\n", millis() - moduleStatus.lastDetectionTime);
    
    if (lastCard.detected) {
        Serial.printf("║ Last Card UID: ");
        for (uint8_t i = 0; i < lastCard.uidLength; i++) {
            Serial.printf("%02X ", lastCard.uid[i]);
        }
        Serial.println("║");
    }
    
    if (moduleStatus.errorMessage && moduleStatus.errorMessage[0] != '\0') {
        Serial.printf("║ Last Error: %s\n", moduleStatus.errorMessage);
    }
    
    Serial.println("╠════════════════════════════════════════════════════════════╣");
    Serial.printf("║ UART2 Configuration: GPIO 43 (RX) / GPIO 44 (TX)\n");
    Serial.printf("║ Baud Rate: %u\n", NFC_MODULE_BAUD);
    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
}

void scanNFCEnvironment() {
    Serial.println("\n🔎 Starting NFC environment scan (30 seconds)...");
    Serial.println("Keep NFC cards/tags nearby for detection\n");
    
    uint32_t scanStart = millis();
    uint32_t cardsFound = 0;
    
    while ((millis() - scanStart) < 30000) {
        NFCCard card = readNFCCardTimeout(1000);
        
        if (card.detected) {
            cardsFound++;
            Serial.printf("  [%u] Card detected - UID: ", cardsFound);
            for (uint8_t i = 0; i < card.uidLength; i++) {
                Serial.printf("%02X ", card.uid[i]);
            }
            Serial.println();
            
            delay(2000);  // Cooldown to avoid duplicate reads
        }
        
        // Progress indicator
        if ((millis() - scanStart) % 5000 < 100) {
            uint8_t progress = ((millis() - scanStart) * 100) / 30000;
            Serial.printf("  Scanning... %u%%\r", progress);
        }
    }
    
    Serial.printf("\n✓ NFC scan complete - Found %u cards\n", cardsFound);
}

} // namespace NFCModuleDriver
