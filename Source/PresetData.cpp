#include "PresetData.h"

//==============================================================================
// v0.9: Preset category names (filesystem-safe — no / or \ characters)
//==============================================================================
const int NUM_PRESET_CATEGORIES = 9;
const char* const presetCategoryNames[NUM_PRESET_CATEGORIES] = {
    "Classic Delays",
    "Spatial Movement",
    "Ambient + Texture",
    "Height + 3D",
    "Surround Production",
    "Wobble + Modulated",
    "Creative + Experimental",
    "Rhythmic",
    "User"
};

//==============================================================================
// v0.9: Trajectory string ID ↔ index mapping
//==============================================================================
static const char* const trajectoryStringIds[] = {
    "none", "bounce", "circle", "cross", "figure_eight", "heart", "helix",
    "infinity", "line", "orbit", "random", "spiral", "square", "triangle"
};
static constexpr int NUM_TRAJECTORY_SHAPES = 14;

// Legacy v0.6–v0.8 index → current index (old 6-item order → new 14-item alphabetical)
static constexpr int trajectoryLegacyMap[6] = {
    0,   // old 0 (None)     → new 0 (None)
    11,  // old 1 (Spiral)   → new 11 (Spiral)
    9,   // old 2 (Orbit)    → new 9 (Orbit)
    1,   // old 3 (Bounce)   → new 1 (Bounce)
    7,   // old 4 (Figure-8) → new 7 (Infinity — same behavior, renamed)
    10   // old 5 (Random)   → new 10 (Random)
};

int trajectoryStringToIndex (const juce::String& id)
{
    // Migration: old string IDs from intermediate v0.9 builds
    if (id == "lissajous") return 13; // Lissajous → Triangle (shifted by Circle insertion)
    if (id == "figure8")
    {
        // "figure8" was used for the old Lissajous-horizontal behavior (now Infinity)
        // but in new presets it means the new two-tangent-circles Figure-8.
        // Factory presets were updated to use "infinity", so any "figure8" on disk
        // from the brief intermediate build should map to Infinity for safety.
        return 7;  // Infinity is now index 7 (shifted by Circle insertion)
    }
    for (int i = 0; i < NUM_TRAJECTORY_SHAPES; ++i)
        if (id == trajectoryStringIds[i])
            return i;
    return 0; // fallback to None
}

juce::String trajectoryIndexToString (int index)
{
    if (index >= 0 && index < NUM_TRAJECTORY_SHAPES)
        return trajectoryStringIds[index];
    return "none";
}

int trajectoryLegacyToNewIndex (int oldIndex)
{
    if (oldIndex >= 0 && oldIndex < 6)
        return trajectoryLegacyMap[oldIndex];
    return 0; // fallback to None
}

//==============================================================================
// Factory presets (stored as static const)
//==============================================================================
const int NUM_FACTORY_PRESETS = 70;
const PresetData factoryPresets[NUM_FACTORY_PRESETS] =
{
    // 0: Quad Ping-Pong — 4 taps in diagonal cross pattern
    {
        "Quad Ping-Pong", "Classic Delays",
        500.0f, false, 4.0f, 0, 0.3f, 20000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -45.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 1: front-left
            { true,  45.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 2: front-right
            { true, -135.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 3: rear-left
            { true,  135.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 4: rear-right
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 1: Stereo Ping-Pong — 2 taps at ±90°
    {
        "Stereo Ping-Pong", "Classic Delays",
        350.0f, false, 4.0f, 0, 0.5f, 18000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -90.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 1: hard left
            { true,  90.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 2: hard right
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 2: Cardinal — 4 taps at cardinal directions
    {
        "Cardinal", "Spatial Movement",
        250.0f, false, 4.0f, 0, 0.4f, 20000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,   0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 1: front
            { true,  90.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 2: right
            { true, 180.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 3: rear
            { true, -90.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 4: left
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 3: Surround 5.1 — 5 taps at SMPTE 5.1 positions (L, R, C, Ls, Rs)
    {
        "Surround 5.1", "Surround Production",
        300.0f, false, 4.0f, 0, 0.35f, 20000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,   30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // L
            { true,  -30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // R
            { true,    0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // C
            { true,  110.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Ls
            { true, -110.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Rs
            {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 4: Surround 7.1 — 7 taps at SMPTE 7.1 positions (L, R, C, Lss, Rss, Lrs, Rrs)
    {
        "Surround 7.1", "Surround Production",
        250.0f, false, 4.0f, 0, 0.35f, 20000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,   30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // L
            { true,  -30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // R
            { true,    0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // C
            { true,   90.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Lss
            { true,  -90.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Rss
            { true,  135.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Lrs
            { true, -135.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Rrs
            {}, {}, {}, {}, {}
        }
    },
    // 5: Atmos 7.1.4 — 11 taps at SMPTE 7.1.4 positions (L, R, C, Lss, Rss, Lrs, Rrs, Tfl, Tfr, Trl, Trr)
    {
        "Atmos 7.1.4", "Surround Production",
        200.0f, false, 4.0f, 0, 0.3f, 20000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,   30.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // L
            { true,  -30.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // R
            { true,    0.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // C
            { true,   90.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // Lss
            { true,  -90.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // Rss
            { true,  135.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // Lrs
            { true, -135.0f,  0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // Rrs
            { true,   45.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // Tfl
            { true,  -45.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // Tfr
            { true,  135.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // Trl
            { true, -135.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // Trr
            {}
        }
    },
    // 6: Rising Spiral — 8 taps spiraling upward with orbit trajectory
    {
        "Rising Spiral", "Spatial Movement",
        200.0f, false, 4.0f, 0, 0.4f, 16000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  -20.0f, 0.7f, 0.4f, 0.0f, 9 /*Orbit*/, 0.1876f },
            { true,   45.0f,  -10.0f, 0.6f, 0.4f, 0.0f, 9 /*Orbit*/, 0.225f },
            { true,   90.0f,    0.0f, 0.5f, 0.4f, 0.0f, 9 /*Orbit*/, 0.25f },
            { true,  135.0f,   10.0f, 0.5f, 0.4f, 0.0f, 9 /*Orbit*/, 0.275f },
            { true,  180.0f,   20.0f, 0.4f, 0.4f, 0.0f, 9 /*Orbit*/, 0.3126f },
            { true, -135.0f,   30.0f, 0.4f, 0.4f, 0.0f, 9 /*Orbit*/, 0.35f },
            { true,  -90.0f,   40.0f, 0.3f, 0.4f, 0.0f, 9 /*Orbit*/, 0.375f },
            { true,  -45.0f,   50.0f, 0.3f, 0.4f, 0.0f, 9 /*Orbit*/, 0.4f },
            {}, {}, {}, {}
        }
    },
    // 7: Falling Cascade — 6 taps descending with pitch drop
    {
        "Falling Cascade", "Height + 3D",
        350.0f, false, 4.0f, 0, 0.45f, 14000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -30.0f,  40.0f, 0.3f, 0.1f, 0.0f, 0, 1.0f },    // High left
            { true,   60.0f,  25.0f, 0.4f, 0.1f, -1.0f, 0, 1.0f, 0, 1 /*L*/ },   // Mid-high right -1st
            { true, -120.0f,  10.0f, 0.5f, 0.1f, -2.0f, 0, 1.0f, 0, 2 /*R*/ },   // Mid left-rear -2st
            { true,  150.0f,  -5.0f, 0.6f, 0.1f, -3.0f, 0, 1.0f, 0, 1 /*L*/ },   // Low right-rear -3st
            { true,  -60.0f, -20.0f, 0.7f, 0.1f, -4.0f, 0, 1.0f, 0, 2 /*R*/ },   // Lower left -4st
            { true,   90.0f, -35.0f, 0.8f, 0.1f, -5.0f, 0, 1.0f, 0, 1 /*L*/ },   // Lowest right -5st
            {}, {}, {}, {}, {}, {}
        }
    },

    //==========================================================================
    // CLASSIC DELAYS (continued)
    //==========================================================================

    // 8: Slapback — spatial slapback with 4 taps simulating wall reflections
    {
        "Slapback", "Classic Delays",
        80.0f, false, 4.0f, 0, 0.15f, 12000.0f, 80.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,   30.0f,   5.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },   // Tap 1: front-right wall
            { true,  -50.0f,   0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },   // Tap 2: left wall
            { true,  120.0f,  10.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 3: rear-right wall
            { true,  -90.0f,  -5.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f },  // Tap 4: left-side wall
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 9: Dotted Eighth — tempo-synced dotted 8th, The Edge style
    {
        "Dotted Eighth", "Classic Delays",
        350.0f, true, 3.0f /*8th*/, 2 /*Dotted*/, 0.45f, 14000.0f, 60.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -45.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,  45.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 10: Tape Echo — 3 taps fanning L→R, warm filters, wobble on
    {
        "Tape Echo", "Classic Delays",
        400.0f, false, 4.0f, 0, 0.5f, 6000.0f, 200.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, true, 45.0f, 15.0f /*wow-heavy*/, 4 /*VBAP*/, 0,
        {
            { true, -40.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,   0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  40.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 11: Dub Delay — 2 taps, high feedback, heavy filtering
    {
        "Dub Delay", "Classic Delays",
        500.0f, false, 4.0f, 0, 0.7f, 3000.0f, 300.0f, 0.707f, 0.707f, 0.55f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -60.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  60.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 12: Quarter Note Bounce — tempo-synced quarter, rhythmic pulse
    {
        "Quarter Note Bounce", "Classic Delays",
        400.0f, true, 2.0f /*quarter*/, 0 /*Notes*/, 0.4f, 16000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -60.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,  60.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true, -30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 13: Multi-Tap Cascade — 6 taps in semicircle, descending volume via distance
    {
        "Multi-Tap Cascade", "Classic Delays",
        150.0f, false, 4.0f, 0, 0.35f, 18000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -75.0f, 0.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f },
            { true, -45.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f },
            { true, -15.0f, 0.0f, 0.45f, 0.0f, 0.0f, 0, 1.0f },
            { true,  15.0f, 0.0f, 0.55f, 0.0f, 0.0f, 0, 1.0f },
            { true,  45.0f, 0.0f, 0.65f, 0.0f, 0.0f, 0, 1.0f },
            { true,  75.0f, 0.0f, 0.75f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}
        }
    },

    //==========================================================================
    // SPATIAL MOVEMENT (continued)
    //==========================================================================

    // 14: Orbit Dance — 4 taps with orbit trajectories at staggered speeds
    {
        "Orbit Dance", "Spatial Movement",
        300.0f, false, 4.0f, 0, 0.4f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,   0.0f, 0.0f, 0.4f, 0.75f, 0.0f, 9 /*Orbit*/, 0.3125f },
            { true,  90.0f, 0.0f, 0.4f, 0.75f, 0.0f, 9 /*Orbit*/, 0.4688f },
            { true, 180.0f, 0.0f, 0.4f, 0.75f, 0.0f, 9 /*Orbit*/, 0.625f },
            { true, -90.0f, 0.0f, 0.4f, 0.75f, 0.0f, 9 /*Orbit*/, 0.7813f },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 15: Figure-8 Weave — 3 taps on figure-8, interleaved
    {
        "Figure-8 Weave", "Spatial Movement",
        350.0f, false, 4.0f, 0, 0.45f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -60.0f,  0.0f, 0.5f, 0.8f, 0.0f, 7 /*Infinity*/, 0.15f },
            { true,   0.0f, 15.0f, 0.4f, 0.8f, 0.0f, 7 /*Infinity*/, 0.225f },
            { true,  60.0f,  0.0f, 0.5f, 0.8f, 0.0f, 7 /*Infinity*/, 0.3f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 16: Bouncing Balls — 4 taps with bounce, descending elevation
    {
        "Bouncing Balls", "Spatial Movement",
        200.0f, false, 4.0f, 0, 0.4f, 18000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -20.0f,  30.0f, 0.3f, 0.65f, 0.0f, 1 /*Bounce*/, 0.25f },
            { true,  20.0f,  15.0f, 0.4f, 0.65f, 0.0f, 1 /*Bounce*/, 0.3752f },
            { true, -40.0f,   0.0f, 0.5f, 0.65f, 0.0f, 1 /*Bounce*/, 0.5f },
            { true,  40.0f, -15.0f, 0.6f, 0.65f, 0.0f, 1 /*Bounce*/, 0.6252f },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 17: Spiral Descent — 6 taps spiraling downward, reverse direction
    {
        "Spiral Descent", "Spatial Movement",
        250.0f, false, 4.0f, 0, 0.4f, 12000.0f, 50.0f, 1.75f, 1.75f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  50.0f, 0.3f, 1.0f, 0.0f, 11 /*Spiral*/, 0.1407f, 1 /*Rev*/ },
            { true,  -60.0f,  30.0f, 0.4f, 1.0f, 0.0f, 11 /*Spiral*/, 0.1688f, 1 /*Rev*/ },
            { true, -120.0f,  10.0f, 0.5f, 1.0f, 0.0f, 11 /*Spiral*/, 0.197f, 1 /*Rev*/ },
            { true,  180.0f, -10.0f, 0.5f, 1.0f, 0.0f, 11 /*Spiral*/, 0.225f, 1 /*Rev*/ },
            { true,  120.0f, -30.0f, 0.6f, 1.0f, 0.0f, 11 /*Spiral*/, 0.2532f, 1 /*Rev*/ },
            { true,   60.0f, -50.0f, 0.7f, 1.0f, 0.0f, 11 /*Spiral*/, 0.2813f, 1 /*Rev*/ },
            {}, {}, {}, {}, {}, {}
        }
    },
    // 18: Random Walk — 5 taps with random trajectory, spaced at 1,3,6,8,11
    {
        "Random Walk", "Spatial Movement",
        300.0f, false, 4.0f, 0, 0.35f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -30.0f,  10.0f, 0.4f, 1.0f, 0.0f, 10 /*Random*/, 0.0625f },  // Tap 1
            {},
            { true,   45.0f, -10.0f, 0.5f, 1.0f, 0.0f, 10 /*Random*/, 0.0938f },  // Tap 3
            {}, {},
            { true, -120.0f,  20.0f, 0.3f, 1.0f, 0.0f, 10 /*Random*/, 0.125f },   // Tap 6
            {},
            { true,   90.0f,   0.0f, 0.6f, 1.0f, 0.0f, 10 /*Random*/, 0.1563f },  // Tap 8
            {}, {},
            { true,  170.0f, -20.0f, 0.5f, 1.0f, 0.0f, 10 /*Random*/, 0.1875f },  // Tap 11
            {}
        }
    },
    // 19: Pendulum — 2 taps swinging back and forth (slow figure-8)
    {
        "Pendulum", "Spatial Movement",
        450.0f, false, 4.0f, 0, 0.5f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -60.0f, 0.0f, 0.5f, 1.0f, 0.0f, 7 /*Infinity*/, 0.25f },
            { true,  60.0f, 0.0f, 0.5f, 1.0f, 0.0f, 7 /*Infinity*/, 0.25f, 1 /*Rev*/ },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },

    //==========================================================================
    // AMBIENT + TEXTURE
    //==========================================================================

    // 20: Shimmer — crystalline octave shimmer with ascending fifths pitch
    {
        "Shimmer", "Ambient + Texture",
        600.0f, false, 4.0f, 0, 0.65f, 8000.0f, 100.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -30.0f, 20.0f, 0.4f, 0.0f, 5.0f, 0, 1.0f },    // +5st (perfect 4th)
            { true,  30.0f, 20.0f, 0.4f, 0.0f, 7.0f, 0, 1.0f },    // +7st (perfect 5th)
            { true, -90.0f, 10.0f, 0.5f, 0.0f, 10.0f, 0, 1.0f },   // +10st (minor 7th)
            { true,  90.0f, 10.0f, 0.5f, 0.0f, 12.0f, 0, 1.0f },   // +12st (octave)
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 21: Frozen Cascade — near-infinite decay, 8 taps
    {
        "Frozen Cascade", "Ambient + Texture",
        1500.0f, false, 4.0f, 0, 0.85f, 4000.0f, 60.0f, 0.707f, 0.707f, 0.4f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  0.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },
            { true,   45.0f, 10.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,   90.0f, 20.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  135.0f, 10.0f, 0.6f, 0.0f, 0.0f, 0, 1.0f },
            { true,  180.0f,  0.0f, 0.7f, 0.0f, 0.0f, 0, 1.0f },
            { true, -135.0f, 10.0f, 0.6f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -90.0f, 20.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -45.0f, 10.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}
        }
    },
    // 22: Dark Matter — brooding, dark atmosphere, taps at 1,3,4
    {
        "Dark Matter", "Ambient + Texture",
        700.0f, false, 4.0f, 0, 0.55f, 2000.0f, 400.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -120.0f, -10.0f, 0.6f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },   // Tap 1
            {},
            { true,    0.0f,  -5.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 3
            { true,  120.0f, -10.0f, 0.6f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },   // Tap 4
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 23: Ethereal Wash — scattered taps at varied distances, air absorption
    {
        "Ethereal Wash", "Ambient + Texture",
        800.0f, false, 4.0f, 0, 0.6f, 6000.0f, 80.0f, 0.707f, 0.707f, 0.4f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -20.0f,  30.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },
            { true,  70.0f,  10.0f, 0.6f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true, -100.0f, 20.0f, 0.8f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  150.0f,  5.0f, 0.9f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -50.0f, 40.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  110.0f, 15.0f, 0.7f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {}, {}, {}, {}, {}, {}
        }
    },
    // 24: Grain Cloud — all 12 taps, short delay, granular texture
    {
        "Grain Cloud", "Ambient + Texture",
        60.0f, false, 4.0f, 0, 0.3f, 18000.0f, 20.0f, 0.707f, 0.707f, 0.4f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -15.0f,  10.0f, 0.2f, 0.0f, 0.0f, 0, 1.0f },
            { true,   25.0f,  -5.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true,  -50.0f,  15.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,   60.0f,   5.0f, 0.2f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -80.0f,  -8.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  100.0f,  12.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true, -130.0f,   3.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  150.0f,  -3.0f, 0.2f, 0.0f, 0.0f, 0, 1.0f },
            { true, -160.0f,   8.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f },
            { true,  170.0f, -10.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -40.0f,  20.0f, 0.15f, 0.0f, 0.0f, 0, 1.0f },
            { true,   45.0f, -15.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ }
        }
    },
    // 25: Echo Chamber — reverb-like space with air absorption
    {
        "Echo Chamber", "Ambient + Texture",
        600.0f, false, 4.0f, 0, 0.7f, 5000.0f, 100.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -30.0f,  10.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },
            { true,   30.0f,  10.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -90.0f,   5.0f, 0.6f, 0.0f, 0.0f, 0, 1.0f },
            { true,   90.0f,   5.0f, 0.7f, 0.0f, 0.0f, 0, 1.0f },
            { true,  180.0f,  15.0f, 0.9f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 26: Drift — slow orbits with wobble, wandering textures, taps at 1,2,5,9
    {
        "Drift", "Ambient + Texture",
        900.0f, false, 4.0f, 0, 0.55f, 8000.0f, 60.0f, 0.707f, 0.707f, 0.4f, 0.0f, 0.0f,
        true, true, true, 25.0f, 70.0f /*flutter-heavy*/, 4 /*VBAP*/, 0,
        {
            { true, -40.0f,  15.0f, 0.5f, 0.33f, 0.0f, 9 /*Orbit*/, 0.0752f, 0, 1 /*L*/ },   // Tap 1
            { true,  80.0f,   5.0f, 0.6f, 0.33f, 0.0f, 9 /*Orbit*/, 0.1f, 0, 2 /*R*/ },      // Tap 2
            {}, {},
            { true, -150.0f, 10.0f, 0.7f, 0.33f, 0.0f, 9 /*Orbit*/, 0.1252f, 0, 1 /*L*/ },   // Tap 5
            {}, {}, {},
            { true,  160.0f, 20.0f, 0.4f, 0.33f, 0.0f, 9 /*Orbit*/, 0.15f, 0, 2 /*R*/ },     // Tap 9
            {}, {}, {}
        }
    },
    // 27: Fifth Ghost — harmonic ghost tones at perfect fifth
    {
        "Fifth Ghost", "Ambient + Texture",
        500.0f, false, 4.0f, 0, 0.6f, 10000.0f, 80.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -45.0f,  10.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,  45.0f,  10.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,   0.0f, -10.0f, 0.5f, 0.0f, -5.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },

    //==========================================================================
    // HEIGHT + 3D (continued)
    //==========================================================================

    // 28: Rain — 8 taps overhead, varied azimuths, very short delays
    {
        "Rain", "Height + 3D",
        50.0f, false, 4.0f, 0, 0.5f, 16000.0f, 100.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -20.0f, 70.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },
            { true,   50.0f, 80.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f },
            { true, -100.0f, 60.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f },
            { true,  130.0f, 75.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -60.0f, 85.0f, 0.2f, 0.0f, 0.0f, 0, 1.0f },
            { true,   80.0f, 65.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true, -150.0f, 70.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },
            { true,  170.0f, 90.0f, 0.2f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}
        }
    },
    // 29: Hemisphere Spread — 3 above, 3 below, alternating L/R
    {
        "Hemisphere Spread", "Height + 3D",
        300.0f, false, 4.0f, 0, 0.4f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -45.0f,  60.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },  // upper L
            { true,   45.0f,  30.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // upper R
            { true,  -90.0f,  45.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },  // upper L-side
            { true,   90.0f, -30.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // lower R-side
            { true,  -45.0f, -45.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },  // lower L
            { true,   45.0f, -60.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },  // lower R
            {}, {}, {}, {}, {}, {}
        }
    },
    // 30: Overhead Arc — 5 taps arcing from front-low over zenith to rear-low
    {
        "Overhead Arc", "Height + 3D",
        300.0f, false, 4.0f, 0, 0.45f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, -15.0f, 0.6f, 0.2f, 0.0f, 0, 1.0f },   // front-low
            { true,   30.0f,  30.0f, 0.4f, 0.2f, 0.0f, 0, 1.0f },   // front-right rising
            { true,    0.0f,  75.0f, 0.2f, 0.3f, 0.0f, 0, 1.0f },   // near-zenith
            { true,  -30.0f,  30.0f, 0.4f, 0.2f, 0.0f, 0, 1.0f },   // rear-left descending
            { true,  180.0f, -15.0f, 0.6f, 0.2f, 0.0f, 0, 1.0f },   // rear-low
            {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 31: Dome Ring — 6 taps at 45° elevation in a ring
    {
        "Dome Ring", "Height + 3D",
        250.0f, false, 4.0f, 0, 0.35f, 18000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,   60.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  120.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  180.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true, -120.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -60.0f, 45.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}
        }
    },
    // 32: Vertical Ping-Pong — 2 taps bouncing up/down
    {
        "Vertical Ping-Pong", "Height + 3D",
        400.0f, false, 4.0f, 0, 0.5f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, 0.0f,  45.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true, 0.0f, -45.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },

    //==========================================================================
    // SURROUND PRODUCTION (continued)
    //==========================================================================

    // 33: Quad Swirl — 4 taps at quad positions with orbit
    {
        "Quad Swirl", "Surround Production",
        300.0f, false, 4.0f, 0, 0.4f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -45.0f, 0.0f, 0.5f, 0.5f, 0.0f, 9 /*Orbit*/, 0.4f },
            { true,   45.0f, 0.0f, 0.5f, 0.5f, 0.0f, 9 /*Orbit*/, 0.4f },
            { true, -135.0f, 0.0f, 0.5f, 0.5f, 0.0f, 9 /*Orbit*/, 0.4f },
            { true,  135.0f, 0.0f, 0.5f, 0.5f, 0.0f, 9 /*Orbit*/, 0.4f },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 34: 5.1 Rhythmic — surround positions, tempo-synced dotted, spaced at 1,3,5,8,11
    {
        "5.1 Rhythmic", "Surround Production",
        300.0f, true, 3.0f /*8th*/, 2 /*Dotted*/, 0.4f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 1: C
            {},
            { true,  -30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 3: L
            {},
            { true,   30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 5: R
            {}, {},
            { true, -110.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 8: Ls
            {}, {},
            { true,  110.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },   // Tap 11: Rs
            {}
        }
    },
    // 35: Wide Stereo — 2 taps at ±30° standard stereo pair, tempo-synced 1/4
    {
        "Wide Stereo", "Surround Production",
        350.0f, true, 4.0f /*1/4*/, 0 /*Straight*/, 0.4f, 18000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  30.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },

    //==========================================================================
    // WOBBLE + MODULATED
    //==========================================================================

    // 36: Tape Wow — slow tape drift
    {
        "Tape Wow", "Wobble + Modulated",
        400.0f, false, 4.0f, 0, 0.5f, 8000.0f, 150.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, true, 60.0f, 0.0f /*wow-heavy*/, 4 /*VBAP*/, 0,
        {
            { true, -50.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,   0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  50.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 37: Tape Flutter — fast tape jitter
    {
        "Tape Flutter", "Wobble + Modulated",
        350.0f, false, 4.0f, 0, 0.45f, 7000.0f, 180.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, true, 40.0f, 100.0f /*flutter-heavy*/, 4 /*VBAP*/, 0,
        {
            { true, -40.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,  10.0f, 5.0f, 0.45f, 0.0f, 0.0f, 0, 1.0f },
            { true,  50.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 38: Warped Echo — heavily detuned echo
    {
        "Warped Echo", "Wobble + Modulated",
        450.0f, false, 4.0f, 0, 0.55f, 5000.0f, 200.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, true, 80.0f, 50.0f, 4 /*VBAP*/, 0,
        {
            { true, -70.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,  70.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 39: Chorus Delay — short delay spatial chorus, higher feedback
    {
        "Chorus Delay", "Wobble + Modulated",
        30.0f, false, 4.0f, 0, 0.4f, 20000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, true, 100.0f, 100.0f, 4 /*VBAP*/, 0,
        {
            { true, -25.0f,  5.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },
            { true,  25.0f, -5.0f, 0.3f, 0.0f, 0.0f, 0, 1.0f },
            { true, -15.0f, 10.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f },
            { true,  15.0f,-10.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 40: Seasick — extreme pitch warble
    {
        "Seasick", "Wobble + Modulated",
        500.0f, false, 4.0f, 0, 0.6f, 6000.0f, 100.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, true, 100.0f, 30.0f, 4 /*VBAP*/, 0,
        {
            { true, -60.0f,  10.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  60.0f, -10.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,   0.0f,  20.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 41: Subtle Motion — barely perceptible tape character
    {
        "Subtle Motion", "Wobble + Modulated",
        400.0f, false, 4.0f, 0, 0.4f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, true, 10.0f, 60.0f, 4 /*VBAP*/, 0,
        {
            { true, -45.0f,  0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true,  45.0f,  0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f },
            { true, -90.0f, 10.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  90.0f, 10.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },

    //==========================================================================
    // CREATIVE + EXPERIMENTAL
    //==========================================================================

    // 42: Self-Oscillation — edge of runaway feedback
    {
        "Self-Oscillation", "Creative + Experimental",
        350.0f, false, 4.0f, 0, 0.95f, 3000.0f, 200.0f, 0.707f, 0.707f, 0.4f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -60.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            { true,  60.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 43: Reverse Spiral — orbiting taps in reverse with pitch drop
    {
        "Reverse Spiral", "Creative + Experimental",
        250.0f, false, 4.0f, 0, 0.45f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  0.0f, 0.4f, 0.4f, 0.0f, 9 /*Orbit*/, 0.1876f, 1 /*Rev*/ },
            { true,   60.0f, 10.0f, 0.5f, 0.4f, 0.0f, 9 /*Orbit*/, 0.25f, 1 /*Rev*/, 1 /*L*/ },
            { true,  120.0f, 20.0f, 0.4f, 0.4f, 0.0f, 9 /*Orbit*/, 0.3126f, 1 /*Rev*/, 2 /*R*/ },
            { true,  180.0f, 30.0f, 0.5f, 0.4f, 0.0f, 9 /*Orbit*/, 0.375f, 1 /*Rev*/ },
            { true, -120.0f, 20.0f, 0.4f, 0.4f, 0.0f, 9 /*Orbit*/, 0.4376f, 1 /*Rev*/, 1 /*L*/ },
            { true,  -60.0f, 10.0f, 0.5f, 0.4f, 0.0f, 9 /*Orbit*/, 0.5f, 1 /*Rev*/, 2 /*R*/ },
            {}, {}, {}, {}, {}, {}
        }
    },
    // 44: Stereo Split — L input left, R input right
    {
        "Stereo Split", "Creative + Experimental",
        400.0f, false, 4.0f, 0, 0.45f, 10000.0f, 100.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -90.0f,  10.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true, -45.0f,   0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  45.0f,   0.0f, 0.5f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true,  90.0f,  10.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 45: Pitch Ladder — ascending per-tap pitch: +2 to +12 semitones
    {
        "Pitch Ladder", "Creative + Experimental",
        300.0f, false, 4.0f, 0, 0.35f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -75.0f,  0.0f, 0.35f, 0.0f,  2.0f, 0, 1.0f },
            { true, -45.0f, 10.0f, 0.4f,  0.0f,  4.0f, 0, 1.0f },
            { true, -15.0f, 20.0f, 0.45f, 0.0f,  6.0f, 0, 1.0f },
            { true,  15.0f, 20.0f, 0.45f, 0.0f,  8.0f, 0, 1.0f },
            { true,  45.0f, 10.0f, 0.4f,  0.0f, 10.0f, 0, 1.0f },
            { true,  75.0f,  0.0f, 0.35f, 0.0f, 12.0f, 0, 1.0f },
            {}, {}, {}, {}, {}, {}
        }
    },
    // 46: Doppler Storm — high doppler orbits at varied speeds
    {
        "Doppler Storm", "Creative + Experimental",
        200.0f, false, 4.0f, 0, 0.4f, 20000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  0.0f, 0.3f, 1.0f, 0.0f, 9 /*Orbit*/, 0.2813f },
            { true,   60.0f, 15.0f, 0.4f, 1.0f, 0.0f, 9 /*Orbit*/, 0.422f, 0, 1 /*L*/ },
            { true,  120.0f,  0.0f, 0.5f, 1.0f, 0.0f, 9 /*Orbit*/, 0.5625f, 1 /*Rev*/, 2 /*R*/ },
            { true,  180.0f,-15.0f, 0.4f, 1.0f, 0.0f, 9 /*Orbit*/, 0.7032f },
            { true, -120.0f, 10.0f, 0.3f, 1.0f, 0.0f, 9 /*Orbit*/, 0.8438f, 1 /*Rev*/, 1 /*L*/ },
            { true,  -60.0f,  0.0f, 0.5f, 1.0f, 0.0f, 9 /*Orbit*/, 0.9845f, 0, 2 /*R*/ },
            {}, {}, {}, {}, {}, {}
        }
    },
    // 47: Micro Delay — all 12 taps, very short, metallic comb filter
    {
        "Micro Delay", "Creative + Experimental",
        10.0f, false, 4.0f, 0, 0.5f, 20000.0f, 20.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -10.0f,   5.0f, 0.15f, 0.0f, 0.0f, 0, 1.0f },
            { true,  10.0f,  -5.0f, 0.15f, 0.0f, 0.0f, 0, 1.0f },
            { true, -20.0f,  10.0f, 0.2f,  0.0f, 0.0f, 0, 1.0f },
            { true,  20.0f, -10.0f, 0.2f,  0.0f, 0.0f, 0, 1.0f },
            { true, -30.0f,  15.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f },
            { true,  30.0f, -15.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f },
            { true, -40.0f,   5.0f, 0.15f, 0.0f, 0.0f, 0, 1.0f },
            { true,  40.0f,  -5.0f, 0.15f, 0.0f, 0.0f, 0, 1.0f },
            { true, -50.0f,  10.0f, 0.2f,  0.0f, 0.0f, 0, 1.0f },
            { true,  50.0f, -10.0f, 0.2f,  0.0f, 0.0f, 0, 1.0f },
            { true, -60.0f,  15.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f },
            { true,  60.0f, -15.0f, 0.25f, 0.0f, 0.0f, 0, 1.0f }
        }
    },
    // 48: Wide Scatter — 8 taps at max distance, paired with gaps: 1,2,_,4,5,_,7,8,_,10,11
    {
        "Wide Scatter", "Creative + Experimental",
        500.0f, false, 4.0f, 0, 0.35f, 12000.0f, 40.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -25.0f,  15.0f, 1.0f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },    // Tap 1
            { true,   73.0f, -10.0f, 0.95f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },   // Tap 2
            {},
            { true, -112.0f,  25.0f, 1.0f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },    // Tap 4
            { true,  148.0f,   5.0f, 0.9f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },    // Tap 5
            {},
            { true,  -58.0f, -20.0f, 1.0f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },    // Tap 7
            { true,   95.0f,  30.0f, 0.95f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },   // Tap 8
            {},
            { true, -170.0f, -15.0f, 1.0f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },    // Tap 10
            { true,   38.0f,  20.0f, 0.9f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },    // Tap 11
            {}
        }
    },
    // 49: Broken Record — stuck groove, synced, wobble, high feedback
    {
        "Broken Record", "Creative + Experimental",
        500.0f, true, 0.0f /*whole*/, 0, 0.75f, 4000.0f, 300.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        false, true, true, 100.0f, 40.0f, 4 /*VBAP*/, 0,
        {
            { true, -30.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  30.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },

    //==========================================================================
    // RHYTHMIC — Euclidean and step-sequencer-style tap patterns
    //==========================================================================

    // 50: Tresillo — E(3,8) Cuban 3+3+2
    {
        "Tresillo", "Rhythmic",
        300.0f, true, 4.0f /*16th*/, 0, 0.25f, 12000.0f, 40.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -25.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {}, {},
            { true,    0.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f },
            {}, {},
            { true,   25.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {}, {}, {}, {}, {}
        }
    },
    // 51: Cinquillo — E(5,8) Afro-Cuban, 5 hits converging to center
    {
        "Cinquillo", "Rhythmic",
        300.0f, true, 4.0f /*16th*/, 0, 0.20f, 11000.0f, 50.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -60.0f, 0.0f, 0.45f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,   60.0f, 0.0f, 0.46f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true,  -30.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,   30.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true,    0.0f, 0.0f, 0.30f, 0.0f, 0.0f, 0, 1.0f },
            {}, {}, {}, {}, {}
        }
    },
    // 52: Offbeat Pong — all offbeats, strict L/R ping-pong
    {
        "Offbeat Pong", "Rhythmic",
        300.0f, true, 4.0f /*16th*/, 0, 0.15f, 10000.0f, 60.0f, 0.707f, 0.707f, 0.40f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            {},
            { true,  -90.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,   90.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {},
            { true,  -90.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,   90.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {},
            { true,  -90.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,   90.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ }
        }
    },
    // 53: West African Bell — E(7,12) semicircle sweep
    {
        "West African Bell", "Rhythmic",
        300.0f, true, 1.0f /*16th*/, 1 /*Dotted*/, 0.30f, 14000.0f, 30.0f, 0.707f, 0.707f, 0.48f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -90.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,  -55.0f, 0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  -25.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,   10.0f, 0.0f, 0.42f, 0.0f, 0.0f, 0, 1.0f },
            {},
            { true,   40.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true,   65.0f, 0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {},
            { true,   90.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {}
        }
    },
    // 54: Son Clave 3-2 — call (L) and response (R)
    {
        "Son Clave 3-2", "Rhythmic",
        300.0f, true, 1.0f /*16th*/, 0, 0.50f, 13000.0f, 40.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -70.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {}, {},
            { true,  -40.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {}, {},
            { true,  -10.0f, 0.0f, 0.45f, 0.0f, 0.0f, 0, 1.0f },
            {}, {},
            { true,   40.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {},
            { true,   70.0f, 0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ }
        }
    },
    // 55: Paradiddle Pong — RLRR LRLL sticking, pairs with gaps
    {
        "Paradiddle Pong", "Rhythmic",
        300.0f, true, 1.0f /*16th*/, 0, 0.50f, 11000.0f, 80.0f, 0.707f, 0.707f, 0.42f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,   50.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true,  -50.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,   50.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true,   50.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {},
            { true,  -50.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,   50.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {},
            { true,  -50.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  -50.0f, 0.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {}
        }
    },
    // 56: Polyrhythm 3v4 — 3-against-4 cross-rhythm, spatially separated layers
    {
        "Polyrhythm 3v4", "Rhythmic",
        300.0f, true, 1.0f /*16th*/, 0, 0.22f, 13000.0f, 40.0f, 0.707f, 0.707f, 0.48f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, 10.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f },  // Both
            {}, {},
            { true,   60.0f, -5.0f, 0.45f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            { true,  -60.0f, 20.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            {},
            { true,   80.0f, -5.0f, 0.45f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {},
            { true,  -80.0f, 20.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },
            { true,  100.0f, -5.0f, 0.45f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },
            {}, {}
        }
    },
    // 57: Morse SOS — grouped bursts (3+3+2) in three spatial zones
    {
        "Morse SOS", "Rhythmic",
        300.0f, true, 4.0f /*16th*/, 0, 0.25f, 10000.0f, 50.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -45.0f,  0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },  // A
            { true,  -40.0f,  5.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },  // A
            { true,  -35.0f, 10.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 2 /*R*/ },  // A
            {}, {},
            { true,   35.0f,  0.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },  // B
            { true,   40.0f,  5.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },  // B
            { true,   45.0f, 10.0f, 0.40f, 0.0f, 0.0f, 0, 1.0f, 0, 1 /*L*/ },  // B
            {}, {},
            { true,    0.0f, -5.0f, 0.35f, 0.0f, 0.0f, 0, 1.0f },  // C
            { true,    0.0f,  0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f }   // C
        }
    },
    // 58: Fibonacci Scatter — spacing follows Fibonacci, ascending pitch
    {
        "Fibonacci Scatter", "Rhythmic",
        300.0f, true, 3.0f /*8th*/, 0, 0.30f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.50f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, -10.0f, 0.30f, 0.0f, 1.0f, 0, 1.0f },
            { true,   20.0f,   0.0f, 0.38f, 0.0f, 1.0f, 0, 1.0f },
            { true,   40.0f,  10.0f, 0.45f, 0.0f, 2.0f, 0, 1.0f },
            {},
            { true,   60.0f,  25.0f, 0.55f, 0.0f, 3.0f, 0, 1.0f },
            {}, {},
            { true,   80.0f,  40.0f, 0.70f, 0.0f, 5.0f, 0, 1.0f },
            {}, {}, {}, {}
        }
    },
    // 59: Dotted Gallop — dotted 8th + 16th syncopation, L/R bounce
    {
        "Dotted Gallop", "Rhythmic",
        300.0f, true, 4.0f /*16th*/, 0, 0.30f, 14000.0f, 50.0f, 0.707f, 0.707f, 0.45f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -70.0f, 0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f },
            {}, {},
            { true,   70.0f, 0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -70.0f, 0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f },
            {}, {},
            { true,   70.0f, 0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f },
            { true,  -70.0f, 0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f },
            {}, {},
            { true,   70.0f, 0.0f, 0.38f, 0.0f, 0.0f, 0, 1.0f }
        }
    },

    //==========================================================================
    // SPATIAL MOVEMENT (new v1.0 presets — showcasing all trajectory shapes)
    //==========================================================================

    // 60: Heartbeat — 2 taps tracing opposing heart curves
    {
        "Heartbeat", "Spatial Movement",
        400.0f, false, 4.0f, 0, 0.45f, 12000.0f, 60.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -40.0f,   5.0f, 0.4f, 0.5f, 0.0f, 5 /*Heart*/, 0.20f },
            { true,  40.0f,  -5.0f, 0.4f, 0.5f, 0.0f, 5 /*Heart*/, 0.25f, 1 /*Rev*/ },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 61: Box Step — 4 taps marching on square paths at ascending speeds
    {
        "Box Step", "Spatial Movement",
        300.0f, false, 4.0f, 0, 0.4f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -45.0f, 0.0f, 0.35f, 0.6f, 0.0f, 12 /*Square*/, 0.30f },
            { true,   45.0f, 0.0f, 0.35f, 0.6f, 0.0f, 12 /*Square*/, 0.40f },
            { true, -135.0f, 0.0f, 0.35f, 0.6f, 0.0f, 12 /*Square*/, 0.50f },
            { true,  135.0f, 0.0f, 0.35f, 0.6f, 0.0f, 12 /*Square*/, 0.60f },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 62: Trigonometry — 3 taps at 120° tracing triangles, spaced at 1,4,7
    {
        "Trigonometry", "Spatial Movement",
        350.0f, false, 4.0f, 0, 0.5f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, 0.0f, 0.4f, 0.7f, 0.0f, 13 /*Triangle*/, 0.25f },   // Tap 1
            {}, {},
            { true,  120.0f, 0.0f, 0.4f, 0.7f, 0.0f, 13 /*Triangle*/, 0.25f },   // Tap 4
            {}, {},
            { true, -120.0f, 0.0f, 0.4f, 0.7f, 0.0f, 13 /*Triangle*/, 0.25f },   // Tap 7
            {}, {}, {}, {}, {}
        }
    },
    // 63: Crossing Paths — 6 taps on Cross trajectories, 3D lattice
    {
        "Crossing Paths", "Spatial Movement",
        250.0f, false, 4.0f, 0, 0.35f, 18000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        true, false, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,  -60.0f,  20.0f, 0.5f, 0.4f, 0.0f, 3 /*Cross*/, 0.15f },
            { true,   60.0f, -20.0f, 0.5f, 0.4f, 0.0f, 3 /*Cross*/, 0.20f, 1 /*Rev*/ },
            { true, -120.0f,  10.0f, 0.5f, 0.4f, 0.0f, 3 /*Cross*/, 0.25f },
            { true,  120.0f, -10.0f, 0.5f, 0.4f, 0.0f, 3 /*Cross*/, 0.30f, 1 /*Rev*/ },
            { true,    0.0f,  30.0f, 0.4f, 0.4f, 0.0f, 3 /*Cross*/, 0.175f },
            { true,  180.0f, -30.0f, 0.4f, 0.4f, 0.0f, 3 /*Cross*/, 0.225f, 1 /*Rev*/ },
            {}, {}, {}, {}, {}, {}
        }
    },
    // 64: Merry-Go-Round — 8 taps in spinning halo with wobble
    {
        "Merry-Go-Round", "Spatial Movement",
        200.0f, false, 4.0f, 0, 0.4f, 14000.0f, 50.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, true, 20.0f, 40.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, 0.0f, 0.45f, 0.5f, 0.0f, 2 /*Circle*/, 0.20f },
            { true,   45.0f, 0.0f, 0.45f, 0.5f, 0.0f, 2 /*Circle*/, 0.20f },
            { true,   90.0f, 0.0f, 0.45f, 0.5f, 0.0f, 2 /*Circle*/, 0.20f },
            { true,  135.0f, 0.0f, 0.45f, 0.5f, 0.0f, 2 /*Circle*/, 0.20f },
            { true,  180.0f, 0.0f, 0.45f, 0.5f, 0.0f, 2 /*Circle*/, 0.20f },
            { true, -135.0f, 0.0f, 0.45f, 0.5f, 0.0f, 2 /*Circle*/, 0.20f },
            { true,  -90.0f, 0.0f, 0.45f, 0.5f, 0.0f, 2 /*Circle*/, 0.20f },
            { true,  -45.0f, 0.0f, 0.45f, 0.5f, 0.0f, 2 /*Circle*/, 0.20f },
            {}, {}, {}, {}
        }
    },
    // 65: Lemniscate — 3 taps tracing Infinity/lemniscate at stacked elevations
    {
        "Lemniscate", "Spatial Movement",
        500.0f, false, 4.0f, 0, 0.55f, 10000.0f, 50.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, 0.0f,   0.0f, 0.5f, 0.8f, 0.0f, 7 /*Infinity*/, 0.15f },
            { true, 0.0f,  10.0f, 0.4f, 0.8f, 0.0f, 7 /*Infinity*/, 0.225f },
            { true, 0.0f, -10.0f, 0.5f, 0.8f, 0.0f, 7 /*Infinity*/, 0.30f, 1 /*Rev*/ },
            {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 66: Back and Forth — 2 taps on Line trajectories, dramatic cross-pattern sweep
    {
        "Back and Forth", "Spatial Movement",
        450.0f, false, 4.0f, 0, 0.5f, 16000.0f, 30.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true, -90.0f, 0.0f, 0.5f, 1.0f, 0.0f, 8 /*Line*/, 0.20f },
            { true,  90.0f, 0.0f, 0.5f, 1.0f, 0.0f, 8 /*Line*/, 0.20f, 1 /*Rev*/ },
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 67: Corkscrew Duo — 4 taps on Figure-8 trajectories at cardinal directions
    {
        "Corkscrew Duo", "Spatial Movement",
        350.0f, false, 4.0f, 0, 0.45f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,   0.0f,  20.0f, 0.45f, 0.7f, 0.0f, 4 /*Figure-8*/, 0.18f },
            { true,  90.0f, -20.0f, 0.45f, 0.7f, 0.0f, 4 /*Figure-8*/, 0.22f, 1 /*Rev*/ },
            { true, 180.0f,  20.0f, 0.45f, 0.7f, 0.0f, 4 /*Figure-8*/, 0.26f },
            { true, -90.0f, -20.0f, 0.45f, 0.7f, 0.0f, 4 /*Figure-8*/, 0.30f, 1 /*Rev*/ },
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 68: Comet Trail — 5 taps spiraling outward, spaced at 1,2,5,8,12
    {
        "Comet Trail", "Spatial Movement",
        300.0f, false, 4.0f, 0, 0.35f, 12000.0f, 60.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        true, true, false, 0.0f, 0.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  30.0f, 0.3f, 0.8f, 0.0f, 11 /*Spiral*/, 0.30f },   // Tap 1
            { true,   72.0f,  15.0f, 0.35f, 0.8f, 0.0f, 11 /*Spiral*/, 0.25f },  // Tap 2
            {}, {},
            { true,  144.0f,   0.0f, 0.4f, 0.8f, 0.0f, 11 /*Spiral*/, 0.20f },   // Tap 5
            {}, {},
            { true, -144.0f, -15.0f, 0.35f, 0.8f, 0.0f, 11 /*Spiral*/, 0.15f, 1 /*Rev*/ }, // Tap 8
            {}, {}, {},
            { true,  -72.0f, -30.0f, 0.3f, 0.8f, 0.0f, 11 /*Spiral*/, 0.10f, 1 /*Rev*/ }  // Tap 12
        }
    },
    // 69: Kaleidoscope — 6 taps, each with a different trajectory shape, spaced at 1,3,5,7,9,11
    {
        "Kaleidoscope", "Spatial Movement",
        400.0f, false, 4.0f, 0, 0.4f, 14000.0f, 40.0f, 0.707f, 0.707f, 0.5f, 0.0f, 0.0f,
        false, true, true, 15.0f, 50.0f, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  10.0f, 0.4f, 0.6f, 0.0f,  2 /*Circle*/,   0.20f },   // Tap 1
            {},
            { true,   60.0f, -10.0f, 0.4f, 0.6f, 0.0f,  3 /*Cross*/,    0.15f },   // Tap 3
            {},
            { true,  120.0f,  15.0f, 0.35f, 0.6f, 0.0f, 5 /*Heart*/,    0.18f, 1 /*Rev*/ }, // Tap 5
            {},
            { true,  180.0f,  -5.0f, 0.45f, 0.6f, 0.0f, 8 /*Line*/,     0.25f },   // Tap 7
            {},
            { true, -120.0f,  20.0f, 0.35f, 0.6f, 0.0f, 12 /*Square*/,  0.22f },   // Tap 9
            {},
            { true,  -60.0f, -15.0f, 0.4f, 0.6f, 0.0f,  13 /*Triangle*/, 0.17f, 1 /*Rev*/ }, // Tap 11
            {}
        }
    }
};

//==============================================================================
// JSON serialization / deserialization (using juce::JSON)
//==============================================================================
juce::String serializePresetToJson (const PresetData& pd)
{
    auto* obj = new juce::DynamicObject();

    obj->setProperty ("name",          pd.name);
    obj->setProperty ("category",      pd.category);
    obj->setProperty ("factory",       pd.isFactory);
    obj->setProperty ("delayTime",     pd.delayTime);
    obj->setProperty ("tempoSync",     pd.tempoSync);
    obj->setProperty ("noteDivision",  pd.noteDivision);
    obj->setProperty ("syncMode",      pd.syncMode);
    obj->setProperty ("feedback",      pd.feedback);
    obj->setProperty ("filterLP",      pd.filterLP);
    obj->setProperty ("filterHP",      pd.filterHP);
    obj->setProperty ("filterLPQ",     pd.filterLPQ);
    obj->setProperty ("filterHPQ",     pd.filterHPQ);
    obj->setProperty ("dryWet",        pd.dryWet);
    obj->setProperty ("inputGain",     pd.inputGain);
    obj->setProperty ("outputGain",    pd.outputGain);
    obj->setProperty ("airAbsorption", pd.airAbsorption);
    obj->setProperty ("filterEnabled", pd.filterEnabled);
    obj->setProperty ("wobbleEnabled", pd.wobbleEnabled);
    obj->setProperty ("wobbleAmount",  pd.wobbleAmount);
    obj->setProperty ("wobbleMorph",   pd.wobbleMorph);
    obj->setProperty ("algorithm",     pd.algorithm);
    obj->setProperty ("hrtfProfile",   pd.hrtfProfile);

    juce::Array<juce::var> tapsArray;
    for (int i = 0; i < PRESET_MAX_OBJECTS; ++i)
    {
        auto* tapObj = new juce::DynamicObject();
        const auto& tap = pd.taps[i];
        tapObj->setProperty ("enabled",         tap.enabled);
        tapObj->setProperty ("azimuthDeg",      tap.azimuthDeg);
        tapObj->setProperty ("elevationDeg",    tap.elevationDeg);
        tapObj->setProperty ("distance",        tap.distance);
        tapObj->setProperty ("dopplerAmount",   tap.dopplerAmount);
        tapObj->setProperty ("pitchShift",      tap.pitchShift);
        tapObj->setProperty ("trajectoryShape", trajectoryIndexToString (tap.trajectoryShape));
        tapObj->setProperty ("trajectorySpeed", tap.trajectorySpeed);
        tapObj->setProperty ("trajectoryDirection", tap.trajectoryDirection);
        tapObj->setProperty ("inputChannel",        tap.inputChannel);
        tapsArray.add (juce::var (tapObj));
    }
    obj->setProperty ("taps", tapsArray);

    return juce::JSON::toString (juce::var (obj), false);
}

PresetData parsePresetJson (const juce::String& json)
{
    PresetData pd;
    auto parsed = juce::JSON::parse (json);
    if (auto* obj = parsed.getDynamicObject())
    {
        pd.name          = obj->getProperty ("name").toString();
        pd.category      = obj->getProperty ("category").toString();
        pd.isFactory     = static_cast<bool>  (obj->getProperty ("factory"));
        pd.delayTime     = static_cast<float> (obj->getProperty ("delayTime"));
        pd.tempoSync     = static_cast<bool>  (obj->getProperty ("tempoSync"));
        pd.noteDivision  = static_cast<float> (obj->getProperty ("noteDivision"));
        pd.syncMode      = static_cast<int>   (obj->getProperty ("syncMode"));
        pd.feedback      = static_cast<float> (obj->getProperty ("feedback"));
        pd.filterLP      = static_cast<float> (obj->getProperty ("filterLP"));
        pd.filterHP      = static_cast<float> (obj->getProperty ("filterHP"));
        // v0.9: filter resonance — default to Butterworth (0.707) if missing from old presets
        pd.filterLPQ     = obj->hasProperty ("filterLPQ") ? static_cast<float> (obj->getProperty ("filterLPQ")) : 0.707f;
        pd.filterHPQ     = obj->hasProperty ("filterHPQ") ? static_cast<float> (obj->getProperty ("filterHPQ")) : 0.707f;
        pd.dryWet        = static_cast<float> (obj->getProperty ("dryWet"));
        pd.inputGain     = static_cast<float> (obj->getProperty ("inputGain"));
        pd.outputGain    = static_cast<float> (obj->getProperty ("outputGain"));
        pd.airAbsorption = static_cast<bool>  (obj->getProperty ("airAbsorption"));
        pd.filterEnabled = static_cast<bool>  (obj->getProperty ("filterEnabled"));
        pd.wobbleEnabled = static_cast<bool>  (obj->getProperty ("wobbleEnabled"));
        pd.wobbleAmount  = static_cast<float> (obj->getProperty ("wobbleAmount"));
        pd.wobbleMorph   = static_cast<float> (obj->getProperty ("wobbleMorph"));
        pd.algorithm     = static_cast<int>   (obj->getProperty ("algorithm"));
        pd.hrtfProfile   = static_cast<int>   (obj->getProperty ("hrtfProfile"));

        if (auto* tapsArr = obj->getProperty ("taps").getArray())
        {
            int numTaps = juce::jmin (static_cast<int> (tapsArr->size()), PRESET_MAX_OBJECTS);
            for (int i = 0; i < numTaps; ++i)
            {
                if (auto* tapObj = (*tapsArr)[i].getDynamicObject())
                {
                    auto& tap = pd.taps[i];
                    tap.enabled         = static_cast<bool>  (tapObj->getProperty ("enabled"));
                    tap.azimuthDeg      = static_cast<float> (tapObj->getProperty ("azimuthDeg"));
                    tap.elevationDeg    = static_cast<float> (tapObj->getProperty ("elevationDeg"));
                    tap.distance        = static_cast<float> (tapObj->getProperty ("distance"));
                    tap.dopplerAmount   = static_cast<float> (tapObj->getProperty ("dopplerAmount"));
                    tap.pitchShift      = static_cast<float> (tapObj->getProperty ("pitchShift"));
                    {
                        auto shapeProp = tapObj->getProperty ("trajectoryShape");
                        if (shapeProp.isString())
                            tap.trajectoryShape = trajectoryStringToIndex (shapeProp.toString());
                        else
                            tap.trajectoryShape = trajectoryLegacyToNewIndex (static_cast<int> (shapeProp));
                    }
                    tap.trajectorySpeed = static_cast<float> (tapObj->getProperty ("trajectorySpeed"));
                    tap.trajectoryDirection = static_cast<int> (tapObj->getProperty ("trajectoryDirection"));
                    tap.inputChannel        = static_cast<int> (tapObj->getProperty ("inputChannel"));
                }
            }
        }
    }
    return pd;
}

