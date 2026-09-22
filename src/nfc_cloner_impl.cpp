#include "nfc_cloner.h"

namespace NfcCloner {

ReadResult readNfcTag(uint32_t durationMs) {
    ReadResult result = {true, "", "", 0};

    uint32_t startTime = millis();

    char uidBuf[15];
    snprintf(uidBuf, sizeof(uidBuf), "%014X", (esp_random() % 4294967295) | ((uint64_t)(esp_random() % 4294967295) << 32));

    result.tagUid = String(uidBuf);
    result.tagContent = "NDEF: https://malicious-site.com\n";
    result.tagContent += "Type: NTAG216\n";
    result.tagContent += "Capability: Write-protected";

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

CloneResult cloneNfcTag(const char* sourceUid, uint32_t durationMs) {
    CloneResult result = {true, "", "", 0};

    uint32_t startTime = millis();

    result.sourceUid = String(sourceUid);
    result.clonedUid = String(sourceUid);

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

WriteResult writeNdefPayload(const char* tagUid, const char* maliciousPayload, uint32_t durationMs) {
    WriteResult result = {true, "", 0};

    uint32_t startTime = millis();

    result.payload = String(maliciousPayload);

    delay(durationMs);

    result.durationMs = millis() - startTime;
    result.success = true;

    return result;
}

}  // namespace NfcCloner
