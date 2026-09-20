#pragma once
#include <Arduino.h>

// Menu icons: ASCII symbols and Unicode for visual feedback
namespace MenuIcons {

// Category icons
#define ICON_WIFI       "\xF0"  // ≈ WiFi symbol
#define ICON_BLE        "\x42"  // B for Bluetooth
#define ICON_RF         "\x99"  // ∑ for RF/radio
#define ICON_TOOL       "\x54"  // T for Tools
#define ICON_SETTINGS   "\x53"  // S for Settings
#define ICON_HELP       "\x3F"  // ? for Help
#define ICON_GPS        "\x47"  // G for GPS
#define ICON_BATTERY    "\x25"  // % for Battery

// Action status icons
#define ICON_ACTIVE     "\x2A"  // * active
#define ICON_INACTIVE   "\xB7"  // · inactive
#define ICON_ARMED      "\x21"  // ! TX armed
#define ICON_LOCKED     "\x23"  // # TX locked
#define ICON_TRANSMIT   "\x26"  // & transmitting
#define ICON_RECEIVE    "\x3C"  // < receiving
#define ICON_RECORDING  "\x40"  // @ recording
#define ICON_PLAYING    "\x3E"  // > playing

// Status symbols
#define ICON_OK         "\x43"  // C check
#define ICON_ERROR      "\x58"  // X error
#define ICON_WAIT       "\x7E"  // ~ waiting
#define ICON_SIGNAL_1   "\x2D"  // - weak signal
#define ICON_SIGNAL_2   "\x3D"  // = medium signal
#define ICON_SIGNAL_3   "\x23"  // # strong signal

// Menu navigation
#define ICON_ARROW_UP   "\x5E"  // ^ up
#define ICON_ARROW_DN   "\x76"  // v down
#define ICON_ARROW_RT   "\x3E"  // > right
#define ICON_ARROW_LT   "\x3C"  // < left
#define ICON_ENTER      "\x2B"  // + enter/select
#define ICON_BACK       "\x3C"  // < back

// Quantity/value icons
#define ICON_COUNT      "\x23"  // # count
#define ICON_DURATION   "\x54"  // T time
#define ICON_DISTANCE   "\x44"  // D distance
#define ICON_STRENGTH   "\x7C"  // | strength

struct MenuItem {
    const char *icon;
    const char *label;
};

}  // namespace MenuIcons
