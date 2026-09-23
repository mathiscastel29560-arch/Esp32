#pragma once
#include <Arduino.h>

// Simplified outline of mainland France ("l'Hexagone", overseas
// territories and Corsica excluded to keep this a single closed ring —
// good enough for a rough on-device reference map, not surveying).
// Points trace the border in order (Alps -> Mediterranean -> Pyrenees ->
// Atlantic -> Brittany -> Channel -> Belgian/German border -> Alps),
// sourced from a public simplified country-boundary dataset and
// down-sampled to ~45 vertices.
namespace FranceOutline {

struct LonLat {
    float lon;
    float lat;
};

static const LonLat POINTS[] = {
    {7.549596f, 44.127901f},  {7.435185f, 43.693845f},  {6.529245f, 43.128892f},
    {4.556963f, 43.399651f},  {3.100411f, 43.075201f},  {2.985999f, 42.473015f},
    {1.826793f, 42.343385f},  {0.701591f, 42.795734f},  {0.338047f, 42.579546f},
    {-1.502771f, 43.034014f}, {-1.901351f, 43.422802f}, {-1.384225f, 44.02261f},
    {-1.193798f, 46.014918f}, {-2.225724f, 47.064363f}, {-2.963276f, 47.570327f},
    {-4.491555f, 47.954954f}, {-4.59235f, 48.68416f},   {-3.295814f, 48.901692f},
    {-1.616511f, 48.644421f}, {-1.933494f, 49.776342f}, {-0.989469f, 49.347376f},
    {1.338761f, 50.127173f},  {1.639001f, 50.946606f},  {2.513573f, 51.148506f},
    {2.658422f, 50.796848f},  {3.123252f, 50.780363f},  {3.588184f, 50.378992f},
    {4.286023f, 49.907497f},  {5.674052f, 49.529484f},  {6.18632f, 49.463803f},
    {6.65823f, 49.201958f},   {8.099279f, 49.017784f},  {7.593676f, 48.333019f},
    {7.466759f, 47.620582f},  {7.192202f, 47.449766f},  {6.736571f, 47.541801f},
    {6.768714f, 47.287708f},  {6.037389f, 46.725779f},  {6.022609f, 46.27299f},
    {6.5001f, 46.429673f},    {6.843593f, 45.991147f},  {6.802355f, 45.70858f},
    {7.096652f, 45.333099f},  {6.749955f, 45.028518f},  {7.007562f, 44.254767f},
};
constexpr size_t POINT_COUNT = sizeof(POINTS) / sizeof(POINTS[0]);

// Bounding box the points above actually span -- used to scale lon/lat
// (this device's own GPS fix included) into any target pixel rect.
constexpr float MIN_LON = -4.59235f;
constexpr float MAX_LON = 8.099279f;
constexpr float MIN_LAT = 42.343385f;
constexpr float MAX_LAT = 51.148506f;

// Longitude degrees shrink versus latitude degrees the further from the
// equator -- at mainland France's ~46.5N mean latitude, roughly by
// cos(46.5°). Multiply longitude spans by this before fitting to a
// pixel box so the outline isn't stretched noticeably wide.
constexpr float LON_SCALE = 0.688f;

// Projects a lon/lat pair to pixel coordinates within [x,y,w,h], fit
// (letterboxed, aspect-correct) using the bounding box above.
inline void project(float lon, float lat, int16_t x, int16_t y, int16_t w, int16_t h, int16_t &outX,
                     int16_t &outY) {
    float spanLon = (MAX_LON - MIN_LON) * LON_SCALE;
    float spanLat = MAX_LAT - MIN_LAT;
    float scale = min((float)w / spanLon, (float)h / spanLat);
    float mapW = spanLon * scale;
    float mapH = spanLat * scale;
    float offX = x + (w - mapW) / 2.0f;
    float offY = y + (h - mapH) / 2.0f;
    outX = (int16_t)(offX + (lon - MIN_LON) * LON_SCALE * scale);
    outY = (int16_t)(offY + mapH - (lat - MIN_LAT) * scale); // screen Y grows downward, latitude grows upward
}

} // namespace FranceOutline
