#include <Arduino.h>
#include <LittleFS.h>
#include "config.h"
#include "display.h"
#include "buttons.h"
#include "buzzer.h"
#include "battery.h"
#include "tx_arm.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "exploit_tracker.h"
#include "audio_effects.h"
#include "settings.h"
#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n=== ESP32-S3 Offensive Security Platform ===");
    Serial.println("Booting...\n");

    // Initialize LittleFS for settings & logs
    if (!LittleFS.begin()) {
        Serial.println("ERROR: LittleFS mount failed");
        while (1) delay(1000);
    }
    Serial.println("LittleFS mounted");

    // Load Settings from JSON
    Settings::loadSettings();
    Serial.println("Settings loaded");

    // Initialize Display
    Display::begin();
    Serial.println("Display initialized");

    // Initialize Hardware
    // Buttons::begin();
    // Serial.println("Buttons initialized");

    // Buzzer::begin();
    Serial.println("Buzzer initialized");

    Battery::begin();
    Serial.println("Battery monitor initialized");

    // TxArm::begin();
    Serial.println("TX Arm system initialized");

    // Initialize Clock & GPS
    RtcClock::begin();
    Serial.println("RTC clock initialized");

    // GpsModule::begin();
    Serial.println("GPS module initialized");

    // Initialize Exploit Tracking
    // ExploitTracker::begin();
    Serial.println("Exploit tracker initialized");

    // Initialize Audio Effects
    // AudioEffects::begin();
    Serial.println("Audio effects initialized");

    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("WiFi initialized (station mode)");

    // Play startup sound
    Buzzer::beep(200, 100);

    Serial.println("\nAll systems ready!");
    Serial.println("Platform booted successfully\n");
}

void loop() {
    // Keep web server running
    yield();
    delay(10);
}
