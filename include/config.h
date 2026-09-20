#pragma once

// ---- Wardriving / logs on LittleFS ----
#define LOG_DIR              "/logs"
#define WARDRIVE_LOG_FILE    "/logs/wardrive.csv"
#define SUBGHZ_CAPTURE_DIR   "/logs/subghz"
#define EVILPORTAL_LOG_FILE  "/logs/portal_submissions.csv"
#define HANDSHAKE_CAPTURE_DIR "/logs/handshakes"

// ---- Fun Features Toggle ----
#define ENABLE_AUDIO_EFFECTS  1   // Enable beep patterns and sound alerts
#define ENABLE_ACHIEVEMENTS   1   // Track exploits and unlock achievements
#define ENABLE_CHAOS_MODE     1   // Enable "launch all tools" chaos mode option
