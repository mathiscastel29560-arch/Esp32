#include "rfid.h"
#include "config.h"
#include <SPI.h>
#include <LittleFS.h>

namespace {

// Stub implementation - attendant le vrai module RC522
// En production, utiliser une vraie lib RFID (MFRC522.h, etc)

volatile uint32_t g_lastScanTime = 0;
RFID::TagData g_lastScannedTag;

// Simule un scan en attendant une vraie tag
// En production : utiliser MFRC522 library ou similaire
void simulateScan() {
    // Placeholder - vrai code utiliserait RC522 driver
    // mfrc522.PCD_Init();
    // mfrc522.PICC_IsNewCardPresent();
    // mfrc522.PICC_ReadCardSerial();
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
