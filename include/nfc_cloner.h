#pragma once
#include <Arduino.h>

namespace NfcCloner {

struct TagData {
    String uid;
    String tagType;  // NTAG, MIFARE Ultralight, Type A, etc.
    String ndef;     // NDEF message content
    uint32_t sizeBytes;
};

struct ReadResult {
    bool success;
    String tagUid;
    String tagContent;
    uint32_t durationMs;
};

// Read NFC tag
ReadResult readNfcTag(uint32_t durationMs = 10000);

// Attack: Clone NFC tag
struct CloneResult {
    bool success;
    String sourceUid;
    String clonedUid;
    uint32_t durationMs;
};
CloneResult cloneNfcTag(const char* sourceUid, uint32_t durationMs = 15000);

// Attack: Write malicious NDEF
struct WriteResult {
    bool success;
    String payload;
    uint32_t durationMs;
};
WriteResult writeNdefPayload(const char* tagUid, const char* maliciousPayload, uint32_t durationMs = 10000);

}  // namespace NfcCloner
