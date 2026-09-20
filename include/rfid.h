#pragma once
#include <Arduino.h>
#include <vector>

namespace RFID {

struct TagData {
    uint8_t uid[10];           // UID jusqu'à 10 bytes (typically 4 or 7)
    uint8_t uidLen;            // Longueur réelle du UID
    String type;               // "Mifare Classic", "Mifare Ultralight", etc.
    uint32_t capacity;         // Bytes totaux
    uint8_t sectorCount;       // Nombre de secteurs
    bool isWritable;           // Peut être réécrit
};

struct ScanResult {
    bool found;
    TagData tag;
    uint32_t readTimeMs;
    String error;              // "" si succès
};

struct CloneResult {
    bool success;
    String sourceUid;
    String targetUid;
    uint32_t bytesWritten;
    String error;
};

// Scan pour une tag RFID sur le lecteur (bloquant, timeout 5s)
ScanResult scan();

// Clone une tag déjà scannée sur une nouvelle tag (source -> target)
// Nécessite que la source ait été scannée et que la target soit présente
CloneResult clone(const TagData &sourceTag);

// Sauve les données d'une tag scannée en JSON/binaire sur LittleFS
String saveTagData(const TagData &tag);

// Charge une tag précédemment sauvegardée
TagData loadTagData(const String &filename);

// Liste tous les clones sauvegardés
std::vector<String> listSavedClones();

} // namespace RFID
