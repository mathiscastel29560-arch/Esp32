#pragma once

// ---- Wardriving / logs on LittleFS ----
#define LOG_DIR              "/logs"
#define WARDRIVE_LOG_FILE    "/logs/wardrive.csv"
#define SUBGHZ_CAPTURE_DIR   "/logs/subghz"
#define EVILPORTAL_LOG_FILE  "/logs/portal_submissions.csv"
#define HANDSHAKE_CAPTURE_DIR "/logs/handshakes"

// ---- Fun Features Toggle ----
// NOTE: These are now runtime configurable via Settings menu
// Default values are set in settings.cpp (all true/1 by default)
// Changes persist in LittleFS at /config/settings.json
// See: include/settings.h and src/settings.cpp
// Usage: Check Settings::g_config.audioEffects, etc. in runtime code
