#include <Arduino.h>
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
#include "dualboot.h"
#include "ui/ui.h"
#include "boot_screen.h"
#include "system_diagnostics.h"

namespace {
String apSsid;
uint32_t lastDisplayUpdate = 0;
}

void setup() {
    Serial.begin(115200);

    LittleFS.begin(true); // format on first boot if no filesystem is found

    Buzzer::begin();
    Battery::begin();

    // Display::begin() auto-detects which screen is wired (TFT or OLED,
    // see display.h) and itself calls SPI.begin() for the shared TFT/
    // CC1101/NRF24L01 bus exactly once, in whichever order is safe for
    // the screen it finds -- doing it here unconditionally, before
    // Display::begin() runs, was a redundant second SPI.begin() call on
    // the same global SPI object and the likely cause of a boot crash
    // when no TFT was physically attached.
    Display::begin();
    Ui::begin();

    // Hardware diagnostics: test each component at boot
    auto diagResults = SystemDiagnostics::runDiagnostics();
    bool systemHealthy = SystemDiagnostics::showDiagnosticResults(diagResults);

    bool rtcOk = RtcClock::begin();
    GpsModule::begin();

    uint64_t chipId = ESP.getEfuseMac();
    char suffix[5];
    snprintf(suffix, sizeof(suffix), "%04X", (uint16_t)(chipId & 0xFFFF));
    apSsid = String(AP_SSID_PREFIX) + suffix;

    // Initialize all modules with graceful error handling
    // Each begin() is already designed to not crash if hardware is missing
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

    BootScreen::show("ESP32-S3 AUDIT TOOL v1.0", apSsid);

    if (!systemHealthy) {
        Serial.println("WARNING: Not all critical hardware detected. Some features may not work.");
    }

    // Everything above came up without hanging or crashing -- tell the
    // bootloader this boot is good, so app rollback never reverts us
    // (see dualboot.h). Deliberately last: if something above hangs, this
    // never runs, and a subsequent reset correctly falls back.
    DualBoot::markValid();
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
        Ui::StatusInfo status;
        status.time = RtcClock::isoTimestamp();
        status.gpsFix = GpsModule::hasFix();
        status.battPercent = Battery::percent();
        status.radioActive = BeaconSpam::active() || EvilPortal::active() || BleSpamDetector::active();
        Ui::showHome(status, WebCtrl::lastAction());
    }
}
