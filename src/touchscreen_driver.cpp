#include "touchscreen_driver.h"
#include "display.h"
#include "config.h"
#include "debug_logger.h"
#include <TFT_eSPI.h>
#include <LittleFS.h>

namespace {

// Touch data structure
struct TouchData {
    uint16_t x = 0;
    uint16_t y = 0;
    bool pressed = false;
    uint32_t lastPressTime = 0;
    uint32_t pressDuration = 0;
};

// Calibration data (4 points)
struct CalibrationData {
    uint16_t topLeftX, topLeftY;
    uint16_t topRightX, topRightY;
    uint16_t bottomLeftX, bottomLeftY;
    uint16_t bottomRightX, bottomRightY;
    uint32_t magic = 0xDEADBEEF; // validation
};

TouchData g_touchData;
CalibrationData g_calibration;
bool g_calibrationLoaded = false;
const char* CALIBRATION_FILE = "/touchcal.bin";
const uint32_t TOUCH_DEBOUNCE_MS = 50;
const uint32_t TOUCH_LONG_PRESS_MS = 500;

// Get reference to TFT for touch operations
TFT_eSPI& getTFT() {
    return Display::raw();
}

// Map screen coordinates to zones
// Returns button action based on touch location
Buttons::Button mapTouchToButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    // Define touch zones as rectangular regions
    // Top 1/4: UP button
    if (y < h / 4) {
        DEBUG_LOG(LogLevel::VERBOSE, "Touch UP zone: y=%d < %d", y, h / 4);
        return Buttons::UP;
    }

    // Bottom 1/4: DOWN button
    if (y > (3 * h) / 4) {
        DEBUG_LOG(LogLevel::VERBOSE, "Touch DOWN zone: y=%d > %d", y, (3 * h) / 4);
        return Buttons::DOWN;
    }

    // Middle half
    if (y >= h / 4 && y <= (3 * h) / 4) {
        // Left half: BACK button
        if (x < w / 2) {
            DEBUG_LOG(LogLevel::VERBOSE, "Touch BACK zone: x=%d < %d", x, w / 2);
            return Buttons::BACK;
        }
        // Right half: SELECT button
        else {
            DEBUG_LOG(LogLevel::VERBOSE, "Touch SELECT zone: x=%d >= %d", x, w / 2);
            return Buttons::SELECT;
        }
    }

    return Buttons::NONE;
}

} // namespace

namespace Touchscreen {

void begin() {
    DEBUG_LOG(LogLevel::INFO, "Initializing XPT2046 touchscreen...");

    // TFT_eSPI already initialized display with touch support
    // Just verify it responds
    uint16_t x = 0, y = 0;
    bool touched = getTFT().getTouch(&x, &y);

    if (!touched) {
        DEBUG_LOG(LogLevel::VERBOSE, "Touchscreen detected (not currently pressed)");
    }

    // Try to load existing calibration
    if (!loadCalibration()) {
        DEBUG_LOG(LogLevel::WARN, "No valid calibration found — touchscreen will be uncalibrated");
        DEBUG_LOG(LogLevel::WARN, "Run touchscreen.calibrate() for best results");
    } else {
        DEBUG_LOG(LogLevel::INFO, "Calibration loaded from LittleFS");
    }
}

Buttons::Button poll() {
    uint16_t x = 0, y = 0;
    uint32_t now = millis();

    // Read raw touch state
    bool touched = getTFT().getTouch(&x, &y);

    if (touched) {
        // Touch detected
        if (!g_touchData.pressed) {
            // Falling edge (touch started)
            if (now - g_touchData.lastPressTime >= TOUCH_DEBOUNCE_MS) {
                g_touchData.pressed = true;
                g_touchData.x = x;
                g_touchData.y = y;
                g_touchData.lastPressTime = now;
                g_touchData.pressDuration = 0;

                // Get screen dimensions
                uint16_t w = getScreenWidth();
                uint16_t h = getScreenHeight();

                DEBUG_LOG(LogLevel::VERBOSE, "Touch press: x=%d, y=%d (screen: %dx%d)", x, y, w, h);

                // Map to button
                return mapTouchToButton(x, y, w, h);
            }
        } else {
            // Touch sustained
            g_touchData.pressDuration = now - g_touchData.lastPressTime;
        }
    } else {
        // No touch
        if (g_touchData.pressed) {
            // Rising edge (touch released)
            g_touchData.pressed = false;
            // Could use pressDuration for long-press detection here
            DEBUG_LOG(LogLevel::VERBOSE, "Touch release after %dms", g_touchData.pressDuration);
        }
    }

    return Buttons::NONE;
}

bool isTouched() {
    uint16_t x = 0, y = 0;
    return getTFT().getTouch(&x, &y);
}

void getRawCoordinates(uint16_t &x, uint16_t &y) {
    x = g_touchData.x;
    y = g_touchData.y;
}

uint16_t getScreenWidth() {
    return getTFT().width();
}

uint16_t getScreenHeight() {
    return getTFT().height();
}

void calibrate() {
    TFT_eSPI& tft = getTFT();

    DEBUG_LOG(LogLevel::INFO, "Starting touchscreen calibration...");
    DEBUG_LOG(LogLevel::INFO, "Press each crosshair in order: top-left, top-right, bottom-left, bottom-right");

    // TFT_eSPI calibration routine
    uint16_t calData[8];
    tft.calibrateTouch(calData, TFT_WHITE, TFT_BLACK, 15);

    DEBUG_LOG(LogLevel::INFO, "Calibration complete!");
    DEBUG_LOG(LogLevel::INFO, "Calibration: %d %d %d %d %d %d %d %d",
             calData[0], calData[1], calData[2], calData[3],
             calData[4], calData[5], calData[6], calData[7]);

    // Save calibration
    if (saveCalibration()) {
        DEBUG_LOG(LogLevel::INFO, "Calibration saved to LittleFS");
    } else {
        DEBUG_LOG(LogLevel::WARN, "Failed to save calibration");
    }
}

bool loadCalibration() {
    if (!LittleFS.exists(CALIBRATION_FILE)) {
        DEBUG_LOG(LogLevel::VERBOSE, "No calibration file found");
        return false;
    }

    File f = LittleFS.open(CALIBRATION_FILE, "r");
    if (!f) {
        DEBUG_LOG(LogLevel::WARN, "Failed to open calibration file");
        return false;
    }

    CalibrationData cal;
    size_t bytesRead = f.read((uint8_t*)&cal, sizeof(cal));
    f.close();

    if (bytesRead != sizeof(cal)) {
        DEBUG_LOG(LogLevel::WARN, "Calibration file incomplete: %d/%d bytes", bytesRead, sizeof(cal));
        return false;
    }

    if (cal.magic != 0xDEADBEEF) {
        DEBUG_LOG(LogLevel::WARN, "Calibration magic mismatch: 0x%X", cal.magic);
        return false;
    }

    g_calibration = cal;
    g_calibrationLoaded = true;
    return true;
}

bool saveCalibration() {
    if (!LittleFS.begin()) {
        DEBUG_LOG(LogLevel::ERROR, "LittleFS mount failed");
        return false;
    }

    g_calibration.magic = 0xDEADBEEF;

    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (!f) {
        DEBUG_LOG(LogLevel::ERROR, "Failed to create calibration file");
        return false;
    }

    size_t bytesWritten = f.write((uint8_t*)&g_calibration, sizeof(g_calibration));
    f.close();

    if (bytesWritten != sizeof(g_calibration)) {
        DEBUG_LOG(LogLevel::ERROR, "Calibration write incomplete: %d/%d bytes", bytesWritten, sizeof(g_calibration));
        return false;
    }

    return true;
}

} // namespace Touchscreen
