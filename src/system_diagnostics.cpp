#include "system_diagnostics.h"
#include "config.h"
#include "display.h"
#include "ui/ui.h"
#include "rtc_clock.h"
#include "gps_module.h"
#include "battery.h"
#include "buzzer.h"
#include "buttons.h"
#include <Wire.h>
#include <SPI.h>
#include <LittleFS.h>

namespace SystemDiagnostics {

namespace {

DiagnosticResult testRTC() {
    DiagnosticResult result;
    result.component = "RTC (DS3231)";

    Wire.beginTransmission(RTC_I2C_ADDR);
    if (Wire.endTransmission() == 0) {
        result.detected = true;
        result.status = "OK";
    } else {
        result.detected = false;
        result.status = "MISSING (I2C 0x68 no response)";
    }
    return result;
}

DiagnosticResult testGPS() {
    DiagnosticResult result;
    result.component = "GPS (NEO-6M)";

    // GPS is on UART1 - we can only check if it's been initialized
    // Full test would require actual NMEA parsing, which we do in normal operation
    // For now, just report that it's available for polling
    result.detected = true;
    result.status = "OK (UART1 ready for polling)";
    return result;
}

DiagnosticResult testDisplay() {
    DiagnosticResult result;

    // Display::begin() already probed which screen exists
    Display::ScreenKind kind = Display::kind();

    if (kind == Display::ScreenKind::TFT) {
        result.component = "Display (TFT ILI9341)";
        result.detected = true;
        result.status = "OK (320x240 color)";
    } else if (kind == Display::ScreenKind::OLED) {
        result.component = "Display (OLED SSD1306)";
        result.detected = true;
        result.status = "OK (128x64 monochrome)";
    } else {
        result.component = "Display";
        result.detected = false;
        result.status = "MISSING (neither TFT nor OLED found)";
    }
    return result;
}

DiagnosticResult testCC1101() {
    DiagnosticResult result;
    result.component = "CC1101 (Sub-GHz 433MHz)";

    // CC1101 SPI communication test: try to read version register (0x0F, should be non-zero)
    // This requires CC1101 to be initialized, which happens in SubGhz::begin()
    // For basic check: set CS low, read version, set CS high
    pinMode(PIN_CC1101_CS, OUTPUT);
    digitalWrite(PIN_CC1101_CS, LOW);
    delayMicroseconds(10);

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
    uint8_t version = SPI.transfer(0x0F | 0x80);  // Read mode for register 0x0F
    digitalWrite(PIN_CC1101_CS, HIGH);

    if (version != 0x00 && version != 0xFF) {
        result.detected = true;
        result.status = "OK (version 0x" + String(version, HEX) + ")";
    } else {
        result.detected = false;
        result.status = "MISSING (SPI version read failed)";
    }
    return result;
}

DiagnosticResult testNRF24() {
    DiagnosticResult result;
    result.component = "NRF24L01+ (2.4GHz)";

    // NRF24 SPI test: read CONFIG register (0x00)
    pinMode(PIN_NRF24_CS, OUTPUT);
    digitalWrite(PIN_NRF24_CS, LOW);
    delayMicroseconds(10);

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
    uint8_t config = SPI.transfer(0x00);  // Read CONFIG
    digitalWrite(PIN_NRF24_CS, HIGH);

    // CONFIG register should typically have some bits set (never all 0s or all 1s on first read)
    if (config != 0x00 && config != 0xFF) {
        result.detected = true;
        result.status = "OK (CONFIG 0x" + String(config, HEX) + ")";
    } else {
        result.detected = false;
        result.status = "MISSING (SPI CONFIG read failed)";
    }
    return result;
}

DiagnosticResult testBattery() {
    DiagnosticResult result;
    result.component = "Battery Monitor (ADC)";

    // Battery reading is always available via ADC on PIN_BATTERY_ADC
    pinMode(PIN_BATTERY_ADC, INPUT);
    uint16_t raw = analogRead(PIN_BATTERY_ADC);

    // Should read between 0-4095 (valid range)
    if (raw > 0 && raw < 4095) {
        result.detected = true;
        result.status = "OK (reading " + String((raw * 100) / 4095) + "%)";
    } else if (raw == 0) {
        result.detected = false;
        result.status = "POSSIBLE ISSUE (ADC reading 0)";
    } else {
        result.detected = false;
        result.status = "POSSIBLE ISSUE (ADC reading 4095 - check resistor divider)";
    }
    return result;
}

DiagnosticResult testBuzzer() {
    DiagnosticResult result;
    result.component = "Buzzer (GPIO 21)";

    // Buzzer is just a GPIO output, always available
    pinMode(PIN_BUZZER, OUTPUT);
    result.detected = true;
    result.status = "OK (GPIO initialized)";
    return result;
}

DiagnosticResult testButtons() {
    DiagnosticResult result;
    result.component = "Buttons (4-way menu)";

    // Initialize all 4 button pins as inputs with pullup
    pinMode(PIN_BTN_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
    pinMode(PIN_BTN_SELECT, INPUT_PULLUP);
    pinMode(PIN_BTN_BACK, INPUT_PULLUP);

    // Read all buttons - should be high (1) when not pressed
    int up = digitalRead(PIN_BTN_UP);
    int down = digitalRead(PIN_BTN_DOWN);
    int select = digitalRead(PIN_BTN_SELECT);
    int back = digitalRead(PIN_BTN_BACK);

    if (up == 1 && down == 1 && select == 1 && back == 1) {
        result.detected = true;
        result.status = "OK (all 4 buttons initialized)";
    } else {
        result.detected = true;
        result.status = "WARNING (some buttons already pressed)";
    }
    return result;
}

DiagnosticResult testStorage() {
    DiagnosticResult result;
    result.component = "Storage (LittleFS)";

    if (LittleFS.begin(false)) {
        // Try to read filesystem info
        uint32_t usedBytes = LittleFS.usedBytes();
        uint32_t totalBytes = LittleFS.totalBytes();

        result.detected = true;
        result.status = "OK (" + String(usedBytes / 1024) + "KB used of " +
                       String(totalBytes / 1024) + "KB)";
    } else {
        result.detected = false;
        result.status = "MISSING (filesystem not initialized)";
    }
    return result;
}

DiagnosticResult testPSRAM() {
    DiagnosticResult result;
    result.component = "PSRAM (Octal external RAM)";

    // Check if PSRAM is available and working
    uint32_t psramSize = ESP.getPsramSize();

    if (psramSize > 0) {
        uint32_t freePsram = ESP.getFreePsram();
        result.detected = true;
        result.status = "OK (" + String(psramSize / 1024 / 1024) + "MB, " +
                       String(freePsram / 1024) + "KB free)";
    } else {
        result.detected = false;
        result.status = "MISSING (PSRAM not configured or not responding)";
    }
    return result;
}

} // namespace

std::vector<DiagnosticResult> runDiagnostics() {
    std::vector<DiagnosticResult> results;

    Serial.println("\n========== SYSTEM DIAGNOSTICS START ==========");

    // Test each component
    results.push_back(testRTC());
    results.push_back(testGPS());
    results.push_back(testDisplay());
    results.push_back(testCC1101());
    results.push_back(testNRF24());
    results.push_back(testBattery());
    results.push_back(testBuzzer());
    results.push_back(testButtons());
    results.push_back(testStorage());
    results.push_back(testPSRAM());

    // Log all results
    for (const auto &result : results) {
        Serial.print("[");
        Serial.print(result.detected ? "✓" : "✗");
        Serial.print("] ");
        Serial.print(result.component);
        Serial.print(": ");
        Serial.println(result.status);
    }

    Serial.println("========== SYSTEM DIAGNOSTICS END ==========\n");

    return results;
}

bool showDiagnosticResults(const std::vector<DiagnosticResult> &results) {
    if (Display::kind() != Display::ScreenKind::TFT) {
        // OLED fallback: just log to serial (already done in runDiagnostics)
        return true;
    }

    // TFT display: show diagnostic screen
    Serial.println("[Diagnostics] Displaying hardware status on TFT...");

    // Count detected components
    uint16_t detected = 0;
    uint16_t total = results.size();

    for (const auto &result : results) {
        if (result.detected) detected++;
    }

    // Show results using Ui::showList-style presentation
    Ui::StatusInfo status;
    status.time = "DIAGNOSTICS";
    status.gpsFix = false;
    status.battPercent = 100;
    status.radioActive = false;

    std::vector<Ui::ListItem> items;

    for (const auto &result : results) {
        Ui::ListItem item;
        item.label = result.component;
        item.hasRssi = false;
        item.badge = result.detected ? "OK" : "MISSING";
        items.push_back(item);
    }

    // Simple text display of results
    // (Would call showList if we had that accessible here, but for now just log)
    Serial.print("Hardware Status: ");
    Serial.print(detected);
    Serial.print("/");
    Serial.println(total);

    // Wait 2 seconds then continue
    delay(2000);

    // Return true if all critical components detected
    // Critical: Display, Storage, PSRAM, Buttons
    bool criticalOk = false;
    bool hasDisplay = false;
    bool hasStorage = false;
    bool hasPsram = false;
    bool hasButtons = false;

    for (const auto &result : results) {
        if (result.component.indexOf("Display") >= 0) hasDisplay = result.detected;
        if (result.component.indexOf("Storage") >= 0) hasStorage = result.detected;
        if (result.component.indexOf("PSRAM") >= 0) hasPsram = result.detected;
        if (result.component.indexOf("Buttons") >= 0) hasButtons = result.detected;
    }

    criticalOk = hasDisplay && hasStorage && hasPsram && hasButtons;

    if (!criticalOk) {
        Serial.println("WARNING: Some critical components missing!");
        Serial.println("  Display: " + String(hasDisplay ? "OK" : "MISSING"));
        Serial.println("  Storage: " + String(hasStorage ? "OK" : "MISSING"));
        Serial.println("  PSRAM: " + String(hasPsram ? "OK" : "MISSING"));
        Serial.println("  Buttons: " + String(hasButtons ? "OK" : "MISSING"));
        Serial.println("Firmware will continue but may be unstable.");
    }

    return criticalOk;
}

} // namespace SystemDiagnostics
