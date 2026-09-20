#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <LittleFS.h>

#include "config.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "display.h"
#include "buzzer.h"
#include "tx_arm.h"
#include "wifi_tools.h"
#include "ble_tools.h"
#include "nrf24_tools.h"
#include "subghz.h"
#include "wardriving.h"
#include "web_ctrl.h"
#include "beacon_spam.h"
#include "evil_portal.h"
#include "buttons.h"
#include "menu.h"
#include "ir_tools.h"
#include "battery.h"
#include "ble_spam_detector.h"
#include "custom_module.h"
#include "ui/ui.h"

namespace {
String apSsid;
uint32_t lastDisplayUpdate = 0;
}

void setup() {
    Serial.begin(115200);

    LittleFS.begin(true); // format on first boot if no filesystem is found

    Buzzer::begin();
    Battery::begin();

    // Shared SPI bus for the TFT, CC1101 and NRF24L01 (each has its own CS).
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);

    Display::begin();
    Ui::begin();
    bool rtcOk = RtcClock::begin();
    GpsModule::begin();

    uint64_t chipId = ESP.getEfuseMac();
    char suffix[5];
    snprintf(suffix, sizeof(suffix), "%04X", (uint16_t)(chipId & 0xFFFF));
    apSsid = String(AP_SSID_PREFIX) + suffix;

    WifiTools::begin(apSsid, AP_PASSWORD);
    BleTools::begin();
    BleSpamDetector::begin();
    Nrf24Tools::begin();
    SubGhz::begin();
    IrTools::begin();
    Wardriving::begin();
    WebCtrl::begin();
    Menu::begin();
    CustomModule::begin();

    Ui::showSplash(apSsid, rtcOk ? "RTC ok - 192.168.4.1" : "RTC MISSING!");
    Buzzer::chirpOk();
}

void loop() {
    GpsModule::poll();
    WebCtrl::loop();
    BeaconSpam::loop();
    EvilPortal::loop();
    Menu::loop();
    CustomModule::loop();

    uint32_t now = millis();
    if (!Menu::isActive() && now - lastDisplayUpdate > 1000) {
        lastDisplayUpdate = now;
        Display::update(WebCtrl::lastAction());
    }
}
