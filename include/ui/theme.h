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

// ---- Palette (RGB565) Premium Dark Mode ----
// Elegant dark theme with subtle elevation, strong accent, semantic colors
// for consistent signal/status visualization everywhere.
constexpr uint16_t COLOR_BG        = 0x0020; // #080810 - Almost black with blue undertone
constexpr uint16_t COLOR_SURFACE   = 0x1081; // #0F1218 - Subtle elevation
constexpr uint16_t COLOR_SURFACE_ALT = 0x1CAA; // #182151 - For depth variation
constexpr uint16_t COLOR_TEXT      = 0xF7FE; // #F8F8FF - Off-white, not pure white
constexpr uint16_t COLOR_TEXT_DIM  = 0x7BCF; // #7B7B8F - Softer secondary text
constexpr uint16_t COLOR_ACCENT    = 0x1FE7; // #1DD4BF - Vibrant cyan-teal (primary action)
constexpr uint16_t COLOR_ACCENT_LIGHT = 0x2FFF; // #2FFFFF - Light variant for hover/disabled
constexpr uint16_t COLOR_OK        = 0x27E0; // #2BD420 - Vibrant green
constexpr uint16_t COLOR_WARN      = 0xFD80; // #FDAA00 - Amber/orange
constexpr uint16_t COLOR_DANGER    = 0xFC05; // #FF4008 - Bold red-orange
constexpr uint16_t COLOR_INFO      = 0x55FF; // #5580FF - Blue for informational

// ---- Spacing / metrics (8px grid, generous & balanced) ----
constexpr uint8_t SPACE_1 = 8;   // Tight spacing (margins within elements)
constexpr uint8_t SPACE_2 = 16;  // Standard spacing
constexpr uint8_t SPACE_3 = 24;  // Generous spacing
constexpr uint8_t SPACE_4 = 32;  // Large separation
constexpr uint8_t RADIUS_SM = 4; // Small rounded corners
constexpr uint8_t RADIUS_MD = 8; // Medium rounded corners (used for list items, cards)
constexpr uint8_t RADIUS_LG = 12; // Large rounded corners (used for major containers)
constexpr uint8_t BORDER_WIDTH = 1; // Thin accent borders
constexpr uint8_t STATUS_BAR_H = 32; // Status bar height (more generous)
constexpr uint8_t ROW_H = 40;    // List row height (more spacious)

// ---- Fonts (flash-array VLW smooth fonts, no filesystem upload needed) ----
// "Do not put quotes around the array name" — TFT_eSPI convention.
#define FONT_TITLE NotoSansBold36
#define FONT_BODY  NotoSansBold15
#define FONT_MONO  NotoSansMonoSCB20

// ---- Animation timings (ms) - for smooth, premium feel ----
constexpr uint16_t ANIM_SELECTION_MS = 140; // Sliding highlight between rows (ease-out quad)
constexpr uint16_t ANIM_TRANSITION_MS = 260; // Screen-to-screen slide transition (ease-out quad)
constexpr uint16_t ANIM_PULSE_MS = 400; // Gentle pulse for loading states
constexpr uint16_t SPLASH_DURATION_MS = 5000; // Boot splash duration

} // namespace Theme
