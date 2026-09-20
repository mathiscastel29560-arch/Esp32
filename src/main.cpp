#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "buttons.h"
#include "buzzer.h"
#include "battery.h"
#include "gps_module.h"
#include "rtc_clock.h"
#include "tx_arm.h"
#include "dualboot.h"
#include "settings.h"
#include "menu.h"
#include "web_ctrl.h"
#include "exploit_tracker.h"
#include "audio_effects.h"
#include <WiFi.h>
#include <LittleFS.h>

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n=== ESP32 PENTESTING PLATFORM ===");
    Serial.println("Initializing subsystems...\n");

    // Initialize LittleFS for persistent storage
    if (!LittleFS.begin()) {
        Serial.println("✗ LittleFS failed to mount");
    } else {
        Serial.println("✓ LittleFS mounted");
    }

    // Initialize Settings (loads from LittleFS)
    Settings::initSettings();
    Serial.println("✓ Settings loaded");

    // Initialize Display (auto-detects TFT vs OLED)
    Display::init();
    Serial.println("✓ Display initialized");

    // Initialize Hardware
    Buttons::init();
    Serial.println("✓ Buttons initialized");

    Buzzer::init();
    Serial.println("✓ Buzzer initialized");

    Battery::init();
    Serial.println("✓ Battery monitor initialized");

    TxArm::init();
    Serial.println("✓ TX Arm system initialized");

    // Initialize Clock & GPS (optional hardware)
    RtcClock::init();
    Serial.println("✓ RTC clock initialized");

    GpsModule::init();
    Serial.println("✓ GPS module initialized");

    // Initialize Exploit Tracking
    ExploitTracker::initSettings();
    Serial.println("✓ Exploit tracker initialized");

    // Initialize Audio Effects
    AudioEffects::initAudio();
    Serial.println("✓ Audio effects initialized");

    // Initialize WiFi (start in station mode for scanning)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("✓ WiFi initialized (station mode)");

    // Initialize Web Controller
    WebCtrl::begin();
    Serial.println("✓ Web API started on port 8080");

    // Play startup sound & show splash screen
    Buzzer::beep(200, 100, 50);  // short beep

    Serial.println("\n✓ All systems ready!");
    Serial.println("Connect to http://192.168.x.x:8080 for web UI");
    Serial.println("Use buttons or menu for local control\n");

    delay(1000);
}

void loop() {
    // Update all systems
    Buttons::update();
    Battery::update();
    GpsModule::update();
    RtcClock::update();
    TxArm::update();

    // Keep web server running
    yield();

    delay(10);
}
