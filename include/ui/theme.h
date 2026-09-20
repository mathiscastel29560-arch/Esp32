#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui/fonts/NotoSansBold15.h"
#include "ui/fonts/NotoSansBold36.h"
#include "ui/fonts/NotoSansMonoSCB20.h"

// Central design tokens for the UI layer. Every screen/widget pulls colors,
// fonts, spacing and timings from here — nothing else in ui/ should hard-code
// a color or a magic pixel value. Change the look of the whole tool by
// editing this one file.
namespace Theme {

// ---- Orientation ----
// Portrait shows more list rows (this tool is mostly menus/lists), but the
// physical case/ribbon routing may assume landscape — change this one
// constant (and re-check HARDWARE.md's screen mounting) if the image comes
// out sideways relative to the enclosure.
constexpr uint8_t ROTATION = 0; // 0 or 2 = portrait (240x320), 1 or 3 = landscape (320x240)

// ---- Palette (RGB565) ----
// Dark, quasi-black base with a raised surface tone, off-white text, and a
// single teal accent. Semantic colors are reused for RSSI bars, alerts and
// confirm/danger dialogs so the vocabulary stays consistent everywhere.
constexpr uint16_t COLOR_BG        = 0x0861; // #0D0D0F
constexpr uint16_t COLOR_SURFACE   = 0x18C3; // #1A1B1E
constexpr uint16_t COLOR_TEXT      = 0xEF5D; // #E8E8EA
constexpr uint16_t COLOR_TEXT_DIM  = 0x8C51; // #8A8A8F
constexpr uint16_t COLOR_ACCENT    = 0x2EB7; // #2DD4BF (cyan-teal)
constexpr uint16_t COLOR_OK        = 0x262B; // #22C55E
constexpr uint16_t COLOR_WARN      = 0xF4E1; // #F59E0B
constexpr uint16_t COLOR_DANGER    = 0xEA28; // #EF4444

// ---- Spacing / metrics (8px grid) ----
constexpr uint8_t SPACE_1 = 8;
constexpr uint8_t SPACE_2 = 16;
constexpr uint8_t SPACE_3 = 24;
constexpr uint8_t RADIUS_SM = 4;
constexpr uint8_t RADIUS_MD = 8;
constexpr uint8_t STATUS_BAR_H = 28;
constexpr uint8_t ROW_H = 32;

// ---- Fonts (flash-array VLW smooth fonts, no filesystem upload needed) ----
// "Do not put quotes around the array name" — TFT_eSPI convention.
#define FONT_TITLE NotoSansBold36
#define FONT_BODY  NotoSansBold15
#define FONT_MONO  NotoSansMonoSCB20

// ---- Animation timings (ms) ----
constexpr uint16_t ANIM_SELECTION_MS = 120; // sliding highlight between rows
constexpr uint16_t ANIM_TRANSITION_MS = 220; // screen-to-screen slide/fade
constexpr uint16_t SPLASH_DURATION_MS = 4000;

} // namespace Theme
