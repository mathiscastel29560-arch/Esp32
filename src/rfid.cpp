#include "rfid.h"
#include "config.h"
#include <SPI.h>
#include <LittleFS.h>

namespace {

// Stub implementation - attendant le vrai module PN532
// En production, utiliser PN532 library (Adafruit_PN532 ou adafruit/PN532)
//
// Installation: PlatformIO lib search "PN532"
// Deux options populaires:
//   1. Adafruit_PN532 (meilleure docs)
//   2. elechouse/PN532 (plus bas niveau)
//
// Pin config (SPI mode):
//   PN532 CLK  -> ESP32 GPIO 12 (shared with TFT)
//   PN532 MOSI -> ESP32 GPIO 11 (shared with TFT)
//   PN532 MISO -> ESP32 GPIO 13 (shared with TFT)
//   PN532 SS   -> ESP32 GPIO 8  (separate CS for RFID)
//   PN532 RST  -> ESP32 GPIO 7  (reset line)

// Stub: sera remplacé par Adafruit_PN532 ou elechouse::PN532
// #include <Adafruit_PN532.h>
// Adafruit_PN532 nfc(8, 7);  // (CS pin, RST pin) for SPI mode

volatile uint32_t g_lastScanTime = 0;
RFID::TagData g_lastScannedTag;

// Simule un scan en attendant le vrai hardware PN532
// En production : utiliser PN532 driver réel
void simulateScan() {
    // TODO: Remplacer par:
    // nfc.begin();
    // if (nfc.getFirmwareVersion()) { ... }
    // uint8_t uid[7];
    // uint8_t uidLen;
    // if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen)) { ... }
}

String byteArrayToHex(const uint8_t *data, size_t len) {
    String result;
    for (size_t i = 0; i < len; i++) {
        if (data[i] < 0x10) result += "0";
        result += String(data[i], HEX);
    }
    return result;
}

}

namespace RFID {

ScanResult scan() {
    ScanResult result{false, {}, 0, ""};
    uint32_t startTime = millis();

    // Simulation : attendre 100ms puis retourner stub data
    // En production, utiliser RC522/MFRC522 API
    delay(100);

    // Stub tag pour test
    result.tag.uid[0] = 0x12;
    result.tag.uid[1] = 0x34;
    result.tag.uid[2] = 0x56;
    result.tag.uid[3] = 0x78;
    result.tag.uidLen = 4;
    result.tag.type = "Mifare Classic 1K";
    result.tag.capacity = 1024;
    result.tag.sectorCount = 16;
    result.tag.isWritable = true;

    result.found = false; // Placeholder - vrai code détecterait une vraie tag
    result.readTimeMs = millis() - startTime;

    if (result.found) {
        g_lastScannedTag = result.tag;
        g_lastScanTime = millis();
    }

    return result;
}

CloneResult clone(const TagData &sourceTag) {
    CloneResult result{false, "", "", 0, "No physical RFID module detected"};

    uint32_t startTime = millis();

    // En production :
    // 1. Désactiver la tag source
    // 2. Écrire les données sur la tag cible secteur par secteur
    // 3. Vérifier l'intégrité

    String sourceUid = byteArrayToHex(sourceTag.uid, sourceTag.uidLen);
    result.sourceUid = sourceUid;
    result.targetUid = sourceUid; // Placeholder
    result.bytesWritten = 0;

    // Stub : retourner erreur si pas de hardware
    return result;
}

String saveTagData(const TagData &tag) {
    if (!LittleFS.exists(RFID_CLONES_DIR)) {
        LittleFS.mkdir(RFID_CLONES_DIR);
    }

    String uidHex = byteArrayToHex(tag.uid, tag.uidLen);
    String filename = String(RFID_CLONES_DIR) + "/" + uidHex + ".json";

    // Créer un JSON avec les données de la tag
    String json = "{\"uid\":\"" + uidHex + "\","
                  "\"type\":\"" + tag.type + "\","
                  "\"capacity\":" + String(tag.capacity) + ","
                  "\"sectors\":" + String(tag.sectorCount) + ","
                  "\"writable\":" + (tag.isWritable ? "true" : "false") + ","
                  "\"timestamp\":" + String(millis()) + "}";

    File f = LittleFS.open(filename, FILE_WRITE);
    if (!f) {
        return "";
    }

    f.print(json);
    f.close();

    return filename;
}

TagData loadTagData(const String &filename) {
    TagData tag{};

    if (!LittleFS.exists(filename)) {
        return tag;
    }

    File f = LittleFS.open(filename, FILE_READ);
    if (!f) {
        return tag;
    }

    // Placeholder : parser JSON et reconstruire TagData
    // En production, utiliser ArduinoJson pour parser correctement

    f.close();
    return tag;
}

std::vector<String> listSavedClones() {
    std::vector<String> clones;

    if (!LittleFS.exists(RFID_CLONES_DIR)) {
        return clones;
    }

    File dir = LittleFS.open(RFID_CLONES_DIR);
    File file = dir.openNextFile();

    while (file) {
        if (!file.isDirectory()) {
            clones.push_back(file.name());
        }
        file = dir.openNextFile();
    }

    return clones;
}

} // namespace RFID
