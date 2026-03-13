#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "HRTFData.h"

// libmysofa — SOFA file reader
extern "C" {
#include "mysofa.h"
}

// #############################################################################
// SPATIAL MEDIA LIBRARY — Profiles, speaker layouts, and format registry
// Everything in this section (through ~line 356) is reusable spatial
// infrastructure: HRTF profiles, binaural head models, speaker layouts
// (ITU-R BS.775/BS.2051), output format registry, and virtual speaker array.
// #############################################################################

//==============================================================================
// HRTF profile names (6 profiles: 1 Simple Woodworth + 5 HRTF convolution)
//==============================================================================
const char* const OpenSpatialDelayProcessor::hrtfProfileNames[NUM_HRTF_PROFILES] = {
    "Simple (Low CPU)",   // 0: Woodworth ITD+ILD (no convolution) — default
    "Studio Reference",   // 1: MIT KEMAR
    "Immersive",          // 2: SADIE II D2 KU100
    "Natural",            // 3: CIPIC Subject003
    "Precise",            // 4: HUTUBS PP2
    "Spatial"             // 5: Bernschuetz KU100
};

//==============================================================================
// Binaural profiles: simplified head models for ITD + ILD rendering
// (Used by profile 0 "Simple" — Woodworth fallback)
//==============================================================================
const std::array<BinauralProfile, 5> OpenSpatialDelayProcessor::binauralProfiles = {{
    { 0.0875f, 1.0f, 1500.0f, "Studio Reference" },   // MIT KEMAR-inspired
    { 0.0920f, 1.3f, 1200.0f, "Immersive" },           // KU100-inspired (wider)
    { 0.0850f, 0.8f, 1800.0f, "Natural" },             // Human subject, subtler
    { 0.0900f, 1.1f, 1400.0f, "Precise" },             // Cross-validated, balanced
    { 0.0875f, 1.5f, 1100.0f, "Spatial" },             // High-res, exaggerated cues
}};

//==============================================================================
// Output format registry — single source of truth for all speaker layouts
// Reusable across Spatial Media Library plugins
//==============================================================================
const std::array<OpenSpatialDelayProcessor::OutputFormatInfo,
                 OpenSpatialDelayProcessor::NUM_OUTPUT_FORMATS>
    OpenSpatialDelayProcessor::outputFormatRegistry = {{
    //                                                              ch  LFE  height ambi ord  stereo
    // --- Binaural (HRTF head model) — default, listed first ---
    { OutputFormat::Binaural,       "Binaural",         "Bin",    2, false, false, false, 0, false },
    // --- Stereo (mode selected by algorithm param indices 6-10) ---
    { OutputFormat::Stereo,         "Stereo",           "St",     2, false, false, false, 0, true  },
    // --- Surround (ascending channel count) ---
    { OutputFormat::Quad,           "Quadraphonic",     "Quad",   4, false, false, false, 0, false },
    { OutputFormat::Surround5_0,    "5.0 Surround",     "5.0",    5, false, false, false, 0, false },
    { OutputFormat::Surround5_1,    "5.1 Surround",     "5.1",    6, true,  false, false, 0, false },
    { OutputFormat::Surround7_0,    "7.0 Surround",     "7.0",    7, false, false, false, 0, false },
    { OutputFormat::Surround5_1_2,  "5.1.2 Atmos",      "5.1.2",  8, true,  true,  false, 0, false },
    { OutputFormat::Surround7_1,    "7.1 Surround",     "7.1",    8, true,  false, false, 0, false },
    { OutputFormat::Octaphonic,     "Octaphonic",       "Oct",    8, false, false, false, 0, false },
    { OutputFormat::Surround7_0_2,  "7.0.2",            "7.0.2",  9, false, true,  false, 0, false },
    { OutputFormat::Surround5_1_4,  "5.1.4 Atmos",      "5.1.4", 10, true,  true,  false, 0, false },
    { OutputFormat::Surround7_1_2,  "7.1.2 Atmos",      "7.1.2", 10, true,  true,  false, 0, false },
    { OutputFormat::Surround7_1_4,  "7.1.4 Atmos",      "7.1.4", 12, true,  true,  false, 0, false },
    { OutputFormat::Surround7_1_6,  "7.1.6 Atmos",      "7.1.6", 14, true,  true,  false, 0, false },
    { OutputFormat::Surround9_1_6,  "9.1.6 Atmos",      "9.1.6", 16, true,  true,  false, 0, false },
    // --- Ambisonics output (AmbiX ACN/SN3D encoding) ---
    { OutputFormat::AmbisonicsFOA,  "1st Order Ambi",   "FOA",    4, false, false, true,  1, false },
    { OutputFormat::AmbisonicsSOA,  "2nd Order Ambi",   "SOA",    9, false, false, true,  2, false },
    { OutputFormat::AmbisonicsHOA,  "3rd Order Ambi",   "HOA",   16, false, false, true,  3, false },
    { OutputFormat::Ambisonics4OA,  "4th Order Ambi",   "4OA",   25, false, false, true,  4, false },
    { OutputFormat::Ambisonics5OA,  "5th Order Ambi",   "5OA",   36, false, false, true,  5, false },
    { OutputFormat::Ambisonics6OA,  "6th Order Ambi",   "6OA",   49, false, false, true,  6, false },
}};

//==============================================================================
// Virtual speaker layout: 9.1.6 standard + Top Center = 16 speakers
// Based on Dolby Atmos / ITU-R BS.2051 9.1.6 bed (LFE excluded)
// Ear level (9) + Top (6) + Zenith (1) = 16
// Convention: 0° = front, positive azimuth = left, negative = right
//==============================================================================
static constexpr float degToRad (float deg) { return deg * juce::MathConstants<float>::pi / 180.0f; }

// Forward declarations of free functions (defined below, needed by earlier callers)
static float evalSH (int acnIndex, float azimuthRad, float elevationRad);
static void computeVBAPGains2D (const SpeakerLayout& layout, float azimuthRad, float* outGains);
static void computeVBAPGains3D (const SpeakerLayout& layout, const std::vector<VBAPTriplet>& triplets,
                                float azimuthRad, float elevationRad, float* outGains);

const std::array<VirtualSpeaker, OpenSpatialDelayProcessor::NUM_VIRTUAL_SPEAKERS>
    OpenSpatialDelayProcessor::virtualSpeakers = {{
    // --- Ear-level ring (9 speakers at elevation 0°) ---
    { degToRad(   0.0f),  degToRad(  0.0f) },  //  0: C   — Centre
    { degToRad(  30.0f),  degToRad(  0.0f) },  //  1: L   — Left
    { degToRad( -30.0f),  degToRad(  0.0f) },  //  2: R   — Right
    { degToRad(  60.0f),  degToRad(  0.0f) },  //  3: Lw  — Left Wide
    { degToRad( -60.0f),  degToRad(  0.0f) },  //  4: Rw  — Right Wide
    { degToRad(  90.0f),  degToRad(  0.0f) },  //  5: Ls  — Left Surround
    { degToRad( -90.0f),  degToRad(  0.0f) },  //  6: Rs  — Right Surround
    { degToRad( 135.0f),  degToRad(  0.0f) },  //  7: Lrs — Left Rear Surround
    { degToRad(-135.0f),  degToRad(  0.0f) },  //  8: Rrs — Right Rear Surround
    // --- Top ring (6 speakers at elevation +45°) ---
    { degToRad(  45.0f),  degToRad( 45.0f) },  //  9: Tfl — Top Front Left
    { degToRad( -45.0f),  degToRad( 45.0f) },  // 10: Tfr — Top Front Right
    { degToRad(  90.0f),  degToRad( 45.0f) },  // 11: Tsl — Top Side Left
    { degToRad( -90.0f),  degToRad( 45.0f) },  // 12: Tsr — Top Side Right
    { degToRad( 135.0f),  degToRad( 45.0f) },  // 13: Trl — Top Rear Left
    { degToRad(-135.0f),  degToRad( 45.0f) },  // 14: Trr — Top Rear Right
    // --- Zenith (1 speaker, keeps count at 16 for square Ambi decode) ---
    { degToRad(   0.0f),  degToRad( 90.0f) },  // 15: T   — Top Centre (zenith)
}};

//==============================================================================
// v0.2: Static speaker layouts for multi-channel output formats
// ITU-R BS.775 / BS.2051 standard positions
// Convention: 0° = front, positive azimuth = left, negative = right
// LFE is tracked but excluded from spatialization
//==============================================================================
static SpeakerLayout makeQuadLayout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 4;
    l.lfeChannelIndex = -1;
    l.totalChannels = 4;
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(110.0f), 0.0f, 2 };  // Ls
    l.speakers[3] = { degToRad(-110.0f),0.0f, 3 };  // Rs
    return l;
}

static SpeakerLayout make5_1Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 5;
    l.lfeChannelIndex = 3;   // JUCE 5.1: L R C LFE Ls Rs
    l.totalChannels = 6;
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad(110.0f), 0.0f, 4 };  // Ls
    l.speakers[4] = { degToRad(-110.0f),0.0f, 5 };  // Rs
    return l;
}

static SpeakerLayout make7_1Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 7;
    l.lfeChannelIndex = 3;   // JUCE 7.1: L R C LFE Lss Rss Lsr Rsr
    l.totalChannels = 8;
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad( 90.0f), 0.0f, 4 };  // Lss
    l.speakers[4] = { degToRad(-90.0f), 0.0f, 5 };  // Rss
    l.speakers[5] = { degToRad(135.0f), 0.0f, 6 };  // Lsr
    l.speakers[6] = { degToRad(-135.0f),0.0f, 7 };  // Rsr
    return l;
}

static SpeakerLayout make7_1_4Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 11;
    l.lfeChannelIndex = 3;   // SMPTE 7.1.4: L R C LFE Lss Rss Lsr Rsr Tfl Tfr Trl Trr
    l.totalChannels = 12;
    // Ear level (7)
    l.speakers[0]  = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1]  = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2]  = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3]  = { degToRad( 90.0f), 0.0f, 4 };  // Lss  (ch4)
    l.speakers[4]  = { degToRad(-90.0f), 0.0f, 5 };  // Rss  (ch5)
    l.speakers[5]  = { degToRad(135.0f), 0.0f, 6 };  // Lsr  (ch6)
    l.speakers[6]  = { degToRad(-135.0f),0.0f, 7 };  // Rsr  (ch7)
    // Top (4)
    l.speakers[7]  = { degToRad( 45.0f), degToRad(45.0f), 8 };   // Tfl  (ch8)
    l.speakers[8]  = { degToRad(-45.0f), degToRad(45.0f), 9 };   // Tfr  (ch9)
    l.speakers[9]  = { degToRad(135.0f), degToRad(45.0f), 10 };  // Trl  (ch10)
    l.speakers[10] = { degToRad(-135.0f),degToRad(45.0f), 11 };  // Trr  (ch11)
    return l;
}

static SpeakerLayout make9_1_6Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 15;
    l.lfeChannelIndex = 3;   // SMPTE 9.1.6: L R C LFE Lss Rss Lsr Rsr Lw Rw Tfl Tfr Tsl Tsr Trl Trr
    l.totalChannels = 16;
    // Ear level (9)
    l.speakers[0]  = { degToRad( 30.0f), 0.0f, 0 };   // L
    l.speakers[1]  = { degToRad(-30.0f), 0.0f, 1 };   // R
    l.speakers[2]  = { degToRad(  0.0f), 0.0f, 2 };   // C
    l.speakers[3]  = { degToRad( 90.0f), 0.0f, 4 };   // Lss  (ch4)
    l.speakers[4]  = { degToRad(-90.0f), 0.0f, 5 };   // Rss  (ch5)
    l.speakers[5]  = { degToRad(135.0f), 0.0f, 6 };   // Lsr  (ch6)
    l.speakers[6]  = { degToRad(-135.0f),0.0f, 7 };   // Rsr  (ch7)
    l.speakers[7]  = { degToRad( 60.0f), 0.0f, 8 };   // Lw   (ch8)
    l.speakers[8]  = { degToRad(-60.0f), 0.0f, 9 };   // Rw   (ch9)
    // Top (6)
    l.speakers[9]  = { degToRad( 45.0f), degToRad(45.0f), 10 };  // Tfl  (ch10)
    l.speakers[10] = { degToRad(-45.0f), degToRad(45.0f), 11 };  // Tfr  (ch11)
    l.speakers[11] = { degToRad( 90.0f), degToRad(45.0f), 12 };  // Tsl  (ch12)
    l.speakers[12] = { degToRad(-90.0f), degToRad(45.0f), 13 };  // Tsr  (ch13)
    l.speakers[13] = { degToRad(135.0f), degToRad(45.0f), 14 };  // Trl  (ch14)
    l.speakers[14] = { degToRad(-135.0f),degToRad(45.0f), 15 };  // Trr  (ch15)
    return l;
}

// Octaphonic (8-channel, no LFE) — "Center" configuration
// 8 speakers at 45° intervals: C, RF, R, RR, Rear, RL, L, FL
static SpeakerLayout makeOctaphonicLayout()
{
    auto degToRad = [] (float d) { return juce::degreesToRadians (d); };
    SpeakerLayout l = {};
    l.numSpeakers = 8;
    l.lfeChannelIndex = -1;
    l.totalChannels = 8;
    l.speakers[0] = { degToRad(   0.0f), 0.0f, 0 };   // Center
    l.speakers[1] = { degToRad( -45.0f), 0.0f, 1 };   // Right Front
    l.speakers[2] = { degToRad( -90.0f), 0.0f, 2 };   // Right
    l.speakers[3] = { degToRad(-135.0f), 0.0f, 3 };   // Rear Right
    l.speakers[4] = { degToRad( 180.0f), 0.0f, 4 };   // Rear
    l.speakers[5] = { degToRad( 135.0f), 0.0f, 5 };   // Rear Left
    l.speakers[6] = { degToRad(  90.0f), 0.0f, 6 };   // Left
    l.speakers[7] = { degToRad(  45.0f), 0.0f, 7 };   // Front Left
    return l;
}

// v0.3: Additional surround layouts — ITU-R BS.2051 / Dolby Atmos positions
// Channel indices follow SMPTE ST 2098-1 ordering (bed channels first, then height)

static SpeakerLayout make5_0Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 5;
    l.lfeChannelIndex = -1;  // No LFE
    l.totalChannels = 5;     // JUCE create5point0(): L R C Ls Rs
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad(110.0f), 0.0f, 3 };  // Ls
    l.speakers[4] = { degToRad(-110.0f),0.0f, 4 };  // Rs
    return l;
}

static SpeakerLayout make7_0Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 7;
    l.lfeChannelIndex = -1;  // No LFE
    l.totalChannels = 7;     // JUCE create7point0(): L R C Lss Rss Lsr Rsr
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad( 90.0f), 0.0f, 3 };  // Lss
    l.speakers[4] = { degToRad(-90.0f), 0.0f, 4 };  // Rss
    l.speakers[5] = { degToRad(135.0f), 0.0f, 5 };  // Lsr
    l.speakers[6] = { degToRad(-135.0f),0.0f, 6 };  // Rsr
    return l;
}

static SpeakerLayout make5_1_2Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 7;       // 5 ear + 2 top (spatial speakers)
    l.lfeChannelIndex = 3;   // JUCE create5point1point2(): L R C LFE Ls Rs Tsl Tsr
    l.totalChannels = 8;
    // Ear level (5)
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad(110.0f), 0.0f, 4 };  // Ls
    l.speakers[4] = { degToRad(-110.0f),0.0f, 5 };  // Rs
    // Top (2) — side positions at +45° elevation
    l.speakers[5] = { degToRad( 90.0f), degToRad(45.0f), 6 };  // Tsl
    l.speakers[6] = { degToRad(-90.0f), degToRad(45.0f), 7 };  // Tsr
    return l;
}

static SpeakerLayout make5_1_4Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 9;       // 5 ear + 4 top (spatial speakers)
    l.lfeChannelIndex = 3;   // JUCE create5point1point4(): L R C LFE Ls Rs Tfl Tfr Trl Trr
    l.totalChannels = 10;
    // Ear level (5)
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad(110.0f), 0.0f, 4 };  // Ls
    l.speakers[4] = { degToRad(-110.0f),0.0f, 5 };  // Rs
    // Top (4)
    l.speakers[5] = { degToRad( 45.0f), degToRad(45.0f), 6 };  // Tfl
    l.speakers[6] = { degToRad(-45.0f), degToRad(45.0f), 7 };  // Tfr
    l.speakers[7] = { degToRad(135.0f), degToRad(45.0f), 8 };  // Trl
    l.speakers[8] = { degToRad(-135.0f),degToRad(45.0f), 9 };  // Trr
    return l;
}

static SpeakerLayout make7_0_2Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 9;       // 7 ear + 2 top (spatial speakers)
    l.lfeChannelIndex = -1;  // No LFE
    l.totalChannels = 9;     // JUCE create7point0point2(): L R C Lss Rss Lsr Rsr Tsl Tsr
    // Ear level (7)
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad( 90.0f), 0.0f, 3 };  // Lss
    l.speakers[4] = { degToRad(-90.0f), 0.0f, 4 };  // Rss
    l.speakers[5] = { degToRad(135.0f), 0.0f, 5 };  // Lsr
    l.speakers[6] = { degToRad(-135.0f),0.0f, 6 };  // Rsr
    // Top (2)
    l.speakers[7] = { degToRad( 90.0f), degToRad(45.0f), 7 };  // Tsl
    l.speakers[8] = { degToRad(-90.0f), degToRad(45.0f), 8 };  // Tsr
    return l;
}

static SpeakerLayout make7_1_2Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 9;       // 7 ear + 2 top (spatial speakers)
    l.lfeChannelIndex = 3;   // JUCE create7point1point2(): L R C LFE Lss Rss Lsr Rsr Tsl Tsr
    l.totalChannels = 10;
    // Ear level (7)
    l.speakers[0] = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1] = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2] = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3] = { degToRad( 90.0f), 0.0f, 4 };  // Lss
    l.speakers[4] = { degToRad(-90.0f), 0.0f, 5 };  // Rss
    l.speakers[5] = { degToRad(135.0f), 0.0f, 6 };  // Lsr
    l.speakers[6] = { degToRad(-135.0f),0.0f, 7 };  // Rsr
    // Top (2)
    l.speakers[7] = { degToRad( 90.0f), degToRad(45.0f), 8 };  // Tsl
    l.speakers[8] = { degToRad(-90.0f), degToRad(45.0f), 9 };  // Tsr
    return l;
}

static SpeakerLayout make7_1_6Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 13;      // 7 ear + 6 top (spatial speakers)
    l.lfeChannelIndex = 3;   // SMPTE 7.1.6: L R C LFE Lss Rss Lsr Rsr Tfl Tfr Trl Trr Tsl Tsr
    l.totalChannels = 14;
    // Ear level (7)
    l.speakers[0]  = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1]  = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2]  = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3]  = { degToRad( 90.0f), 0.0f, 4 };  // Lss  (ch4)
    l.speakers[4]  = { degToRad(-90.0f), 0.0f, 5 };  // Rss  (ch5)
    l.speakers[5]  = { degToRad(135.0f), 0.0f, 6 };  // Lsr  (ch6)
    l.speakers[6]  = { degToRad(-135.0f),0.0f, 7 };  // Rsr  (ch7)
    // Top (6)
    l.speakers[7]  = { degToRad( 45.0f), degToRad(45.0f), 8 };   // Tfl  (ch8)
    l.speakers[8]  = { degToRad(-45.0f), degToRad(45.0f), 9 };   // Tfr  (ch9)
    l.speakers[9]  = { degToRad(135.0f), degToRad(45.0f), 10 };  // Trl  (ch10)
    l.speakers[10] = { degToRad(-135.0f),degToRad(45.0f), 11 };  // Trr  (ch11)
    l.speakers[11] = { degToRad( 90.0f), degToRad(45.0f), 12 };  // Tsl  (ch12)
    l.speakers[12] = { degToRad(-90.0f), degToRad(45.0f), 13 };  // Tsr  (ch13)
    return l;
}


// #############################################################################
// MIXED — Parameter layout (spatial params are reusable; delay params are plugin-specific)
// Spatial: outputFormat, algorithm, hrtfProfile, per-object azimuth/elevation/distance
// Delay-specific: delayTime, feedback, filterLP/HP, pitchShift, dryWet, gains
// #############################################################################

//==============================================================================
// Parameter layout
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
    OpenSpatialDelayProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // --- Global parameters ---------------------------------------------------
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("delayTime", 1), "Delay Time",
        juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.3f), 500.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                if (value >= 1000.0f)
                    return juce::String (value / 1000.0f, 2) + " s";
                return juce::String (juce::roundToInt (value)) + " ms";
            })));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("tempoSync", 1), "Tempo Sync", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("noteDivision", 1), "Note Division",
        juce::NormalisableRange<float> (1.0f, 16.0f, 1.0f), 4.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                // value is 1..16 representing number of 16th notes
                // 4 = 1/4 note, 2 = 1/8 note, 1 = 1/16 note, 8 = 1/2 note, 16 = 1/1
                int sixteenths = juce::roundToInt (value);
                if (sixteenths == 16) return juce::String ("1/1");
                if (sixteenths == 8)  return juce::String ("1/2");
                if (sixteenths == 4)  return juce::String ("1/4");
                if (sixteenths == 2)  return juce::String ("1/8");
                if (sixteenths == 1)  return juce::String ("1/16");
                // For irregular values, show x/16
                return juce::String (sixteenths) + "/16";
            })));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("syncMode", 1), "Sync Mode",
        juce::StringArray { "Notes", "Triplet", "Dotted", "16th" }, 0));


    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("feedback", 1), "Feedback",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.3f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (
            [](float value, int) { return juce::String (juce::roundToInt(value * 100.0f)) + "%"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterLP", 1), "Low-Pass Filter",
        juce::NormalisableRange<float> (200.0f, 20000.0f, 1.0f, 0.3f), 20000.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                if (value >= 1000.0f)
                    return juce::String (value / 1000.0f, 1) + " kHz";
                return juce::String (juce::roundToInt (value)) + " Hz";
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterHP", 1), "High-Pass Filter",
        juce::NormalisableRange<float> (20.0f, 5000.0f, 1.0f, 0.3f), 20.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                if (value >= 1000.0f)
                    return juce::String (value / 1000.0f, 1) + " kHz";
                return juce::String (juce::roundToInt (value)) + " Hz";
            })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("pitchShift", 1), "Pitch Shift",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 1) + " st"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("dryWet", 1), "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (
            [](float value, int) { return juce::String (juce::roundToInt(value * 100.0f)) + "%"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("inputGain", 1), "Input Gain",
        juce::NormalisableRange<float> (-60.0f, 12.0f, 0.1f, 2.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 1) + " dB"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("outputGain", 1), "Output Gain",
        juce::NormalisableRange<float> (-60.0f, 12.0f, 0.1f, 2.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 1) + " dB"; })));



    // v0.5: Combined algorithm parameter — surround algorithms (0-5) + stereo modes (6-10)
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("algorithm", 8), "Algorithm",
        juce::StringArray { "Ambisonics (HOA)", "DBAP", "KNN", "MDAP", "VBAP", "VBIP",
                            "Equal Power", "Stereo VBAP", "XY Pair", "MS Encode", "Blumlein" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("hrtfProfile", 3), "HRTF Profile",
        juce::StringArray { "Simple (Low CPU)", "Studio Reference", "Immersive",
                            "Natural", "Precise", "Spatial" }, 0));

    // v0.2: User-selectable output format (bus-constrained in UI)
    {
        juce::StringArray formatNames;
        for (const auto& info : outputFormatRegistry)
            formatNames.add (info.name);
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID ("outputFormat", 8), "Output Format", formatNames, 0));  // default=Binaural (index 0)
    }

    // v0.4: Air absorption — global toggle (distance-driven HF rolloff for all taps)
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("airAbsorption", 4), "Air Absorption", false));

    // --- Per-object parameters ------------------------------------------------
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto id  = [&](const char* suffix) {
            return juce::ParameterID ("object" + juce::String (i + 1) + "_" + suffix, 1);
        };
        auto name = [&](const char* suffix) {
            return "Object " + juce::String (i + 1) + " " + suffix;
        };

        bool defaultEnabled = (i < 4);
        float defaultTime   = 100.0f * (i + 1);  // 100, 200, 300... ms
        const float defaultAzArray[] = {-45.0f, 45.0f, -135.0f, 135.0f,
                                         0.0f, 90.0f, -90.0f, 180.0f,
                                         -30.0f, 30.0f, -60.0f, 60.0f};
        float defaultAz = defaultAzArray[i];

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            id ("enabled"), name ("Enabled"), defaultEnabled));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("time"), name ("Time"),
            juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.3f), defaultTime));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("azimuth"), name ("Azimuth"),
            juce::NormalisableRange<float> (-180.0f, 180.0f, 0.1f), defaultAz,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [](float value, int) { return juce::String (value, 1) + juce::String::charToString (0x00B0); })));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("elevation"), name ("Elevation"),
            juce::NormalisableRange<float> (-90.0f, 90.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [](float value, int) { return juce::String (value, 1) + juce::String::charToString (0x00B0); })));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("distance"), name ("Distance"),
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

        // v0.4: Per-object Doppler amount (0=off, >0=on at that intensity)
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("dopplerAmount"), name ("Doppler Amount"),
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (
                [](float value, int) { return juce::String (juce::roundToInt(value * 100.0f)) + "%"; })));

        // v0.6: Per-object trajectory (shape + speed)
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            id ("trajectoryShape"), name ("Trajectory Shape"),
            juce::StringArray { "None", "Spiral", "Orbit", "Bounce", "Figure-8", "Random" }, 0));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("trajectorySpeed"), name ("Trajectory Speed"),
            juce::NormalisableRange<float> (0.0f, 10.0f, 0.01f), 1.0f));
    }

    // v0.5: ADM-OSC global parameter stub (engine deferred to v0.6+)
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("admOscEnabled", 5), "ADM-OSC Enabled", false));

    return { params.begin(), params.end() };
}

// #############################################################################
// SPATIAL MEDIA LIBRARY — HRTF & binaural rendering implementations
// HRTFDatabase (SOFA loading), PartitionedConvolver (FFT overlap-save),
// BinauralRenderer (per-source HRTF convolution), and HRTF profile management.
// All reusable by any SML plugin needing binaural output.
// #############################################################################

//==============================================================================
// HRTFDatabase implementation — SOFA loading + HRIR lookup
//==============================================================================
HRTFDatabase::HRTFDatabase() {}

HRTFDatabase::~HRTFDatabase()
{
    unload();
}

bool HRTFDatabase::loadFromMemory (const void* data, int dataSize, float targetSampleRate)
{
    unload();

    int filterLength = 0;
    int err = 0;

    easyHandle = mysofa_open_data (static_cast<const char*> (data),
                                   static_cast<long> (dataSize),
                                   targetSampleRate,
                                   &filterLength,
                                   &err);

    if (easyHandle == nullptr || err != MYSOFA_OK)
    {
        DBG ("HRTFDatabase: Failed to load SOFA data, error code: " + juce::String (err));
        easyHandle = nullptr;
        loaded = false;
        return false;
    }

    // Note: mysofa_open_data() already normalizes via mysofa_loudness(), which
    // scales all HRIRs so the frontal HRIR has consistent energy (factor = sqrt(2/E)).
    // This provides cross-profile normalization at the source level.

    irLength = filterLength;
    numPositions = static_cast<int> (easyHandle->hrtf->M);
    loaded = true;

    DBG ("HRTFDatabase: Loaded " + juce::String (numPositions) + " positions, "
         + "IR length = " + juce::String (irLength) + " samples");

    return true;
}

void HRTFDatabase::getInterpolatedHRIR (float azimuthRad, float elevationRad,
                                         float* irL, float* irR,
                                         float& delayL, float& delayR) const
{
    if (! loaded || easyHandle == nullptr)
        return;

    // Convert from our convention (radians) to Cartesian for libmysofa
    // libmysofa uses Cartesian coordinates internally
    // Our convention: az=0 front, positive=left; el=0 ear level, positive=up
    // Convert to Cartesian: x=front, y=left, z=up
    float x = std::cos (elevationRad) * std::cos (azimuthRad);
    float y = std::cos (elevationRad) * std::sin (azimuthRad);
    float z = std::sin (elevationRad);

    mysofa_getfilter_float (easyHandle, x, y, z,
                            irL, irR,
                            &delayL, &delayR);
}

void HRTFDatabase::unload()
{
    if (easyHandle != nullptr)
    {
        mysofa_close (easyHandle);
        easyHandle = nullptr;
    }
    loaded = false;
    irLength = 0;
    numPositions = 0;
}

//==============================================================================
// PartitionedConvolver implementation — overlap-save FFT convolution
//==============================================================================
void PartitionedConvolver::prepare (int maxBlockSize, int irLength_)
{
    irLen = irLength_;
    blockSize = maxBlockSize;

    // FFT size must be >= blockSize + irLen - 1 (linear convolution length)
    // Round up to next power of 2
    int minFFTSize = blockSize + irLen - 1;
    fftOrder = 1;
    while ((1 << fftOrder) < minFFTSize)
        ++fftOrder;
    fftSize = 1 << fftOrder;

    fft = juce::dsp::FFT (fftOrder);

    // Allocate buffers (FFT uses 2× size for complex interleaved)
    irFreqDomain.resize (static_cast<size_t> (fftSize * 2), 0.0f);
    inputAccum.resize (static_cast<size_t> (fftSize * 2), 0.0f);
    fftWorkBuf.resize (static_cast<size_t> (fftSize * 2), 0.0f);
    overlapBuf.resize (static_cast<size_t> (fftSize), 0.0f);

    reset();
}

void PartitionedConvolver::setIR (const float* ir, int length)
{
    if (fftSize == 0) return;

    // Zero-pad IR to fftSize and compute FFT
    std::fill (irFreqDomain.begin(), irFreqDomain.end(), 0.0f);
    for (int i = 0; i < std::min (length, fftSize); ++i)
        irFreqDomain[static_cast<size_t> (i)] = ir[i];

    fft.performRealOnlyForwardTransform (irFreqDomain.data(), true);
    irLen = length;
}

void PartitionedConvolver::process (const float* in, float* out, int numSamples)
{
    if (fftSize == 0 || irLen == 0)
    {
        // Pass-through if no IR set
        if (in != out)
            std::memcpy (out, in, sizeof (float) * static_cast<size_t> (numSamples));
        return;
    }

    // Simple overlap-save convolution:
    // For each block of input, we perform FFT, multiply with IR spectrum, IFFT,
    // and combine with overlap from previous block.

    int samplesProcessed = 0;

    while (samplesProcessed < numSamples)
    {
        // How many samples we can accept before we need to process
        int spaceInAccum = blockSize - inputAccumPos;
        int samplesToAccum = std::min (spaceInAccum, numSamples - samplesProcessed);

        // Accumulate input
        for (int i = 0; i < samplesToAccum; ++i)
            inputAccum[static_cast<size_t> (inputAccumPos + i)] = in[samplesProcessed + i];

        inputAccumPos += samplesToAccum;
        samplesProcessed += samplesToAccum;

        // When we have a full block, process it
        if (inputAccumPos >= blockSize)
        {
            // Copy input to work buffer, zero-pad to fftSize
            std::fill (fftWorkBuf.begin(), fftWorkBuf.end(), 0.0f);
            for (int i = 0; i < blockSize; ++i)
                fftWorkBuf[static_cast<size_t> (i)] = inputAccum[static_cast<size_t> (i)];

            // Forward FFT of input
            fft.performRealOnlyForwardTransform (fftWorkBuf.data(), true);

            // Complex multiply with pre-computed IR spectrum
            // juce::dsp::FFT stores as [re, im, re, im, ...]
            for (int i = 0; i < fftSize * 2; i += 2)
            {
                float re1 = fftWorkBuf[static_cast<size_t> (i)],     im1 = fftWorkBuf[static_cast<size_t> (i + 1)];
                float re2 = irFreqDomain[static_cast<size_t> (i)],   im2 = irFreqDomain[static_cast<size_t> (i + 1)];
                fftWorkBuf[static_cast<size_t> (i)]     = re1 * re2 - im1 * im2;
                fftWorkBuf[static_cast<size_t> (i + 1)] = re1 * im2 + im1 * re2;
            }

            // Inverse FFT
            fft.performRealOnlyInverseTransform (fftWorkBuf.data());

            // Output: first blockSize samples = new output + overlap from previous block
            int outStart = samplesProcessed - blockSize;  // Where in the output buffer to write
            int outSamples = std::min (blockSize, numSamples - outStart);

            for (int i = 0; i < outSamples; ++i)
            {
                out[outStart + i] = fftWorkBuf[static_cast<size_t> (i)]
                                  + overlapBuf[static_cast<size_t> (i)];
            }

            // Save overlap: samples [blockSize .. fftSize-1] for next block
            int overlapLen = fftSize - blockSize;
            for (int i = 0; i < overlapLen; ++i)
                overlapBuf[static_cast<size_t> (i)] = fftWorkBuf[static_cast<size_t> (blockSize + i)];

            // Zero the rest of overlap buffer
            for (int i = overlapLen; i < fftSize; ++i)
                overlapBuf[static_cast<size_t> (i)] = 0.0f;

            inputAccumPos = 0;
        }
    }
}

void PartitionedConvolver::reset()
{
    std::fill (inputAccum.begin(), inputAccum.end(), 0.0f);
    std::fill (overlapBuf.begin(), overlapBuf.end(), 0.0f);
    std::fill (fftWorkBuf.begin(), fftWorkBuf.end(), 0.0f);
    inputAccumPos = 0;
}

//==============================================================================
// BinauralRenderer implementation — manages HRTF convolver banks
//==============================================================================
void BinauralRenderer::prepare (double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    currentBlockSize = maxBlockSize;

    convTmpL.resize (static_cast<size_t> (maxBlockSize), 0.0f);
    convTmpR.resize (static_cast<size_t> (maxBlockSize), 0.0f);
}

void BinauralRenderer::setProfile (int profileIndex, HRTFDatabase& hrtfDb)
{
    activeProfile = profileIndex;

    if (profileIndex == 0 || ! hrtfDb.isLoaded())
    {
        // Simple mode — no convolution needed
        storedNormGain = 1.0f;
        storedIRLength = 0;
        for (int i = 0; i < MAX_SOURCES; ++i)
            sourceConvReady[i] = false;
        return;
    }

    int irLen = hrtfDb.getIRLength();
    storedIRLength = irLen;

    // =========================================================================
    // Compute cross-profile normalization gain.
    // Sample a few reference directions to measure this profile's energy level.
    // mysofa_loudness() normalises the FRONTAL HRIR so sumOfSquares = 2.0,
    // but off-axis HRIRs vary. We sample 6 directions (front, back, L, R, up, down)
    // to get a representative avgRMS, then scale to targetRMS = 1/sqrt(irLen).
    // =========================================================================
    constexpr float pi = juce::MathConstants<float>::pi;
    static const float refDirs[][2] = {
        { 0.0f, 0.0f },                                           // Front
        { pi, 0.0f },                                             // Back
        { pi * 0.5f, 0.0f },                                      // Left
        { -pi * 0.5f, 0.0f },                                     // Right
        { 0.0f, pi * 0.25f },                                     // Above-front
        { 0.0f, -pi * 0.25f }                                     // Below-front
    };
    static constexpr int NUM_REF_DIRS = 6;

    std::vector<float> tmpIRL (static_cast<size_t> (irLen)), tmpIRR (static_cast<size_t> (irLen));
    double totalEnergy = 0.0;

    for (int d = 0; d < NUM_REF_DIRS; ++d)
    {
        float delayL = 0.0f, delayR = 0.0f;
        hrtfDb.getInterpolatedHRIR (refDirs[d][0], refDirs[d][1],
                                     tmpIRL.data(), tmpIRR.data(), delayL, delayR);
        for (int n = 0; n < irLen; ++n)
        {
            totalEnergy += (double) tmpIRL[static_cast<size_t> (n)] * tmpIRL[static_cast<size_t> (n)];
            totalEnergy += (double) tmpIRR[static_cast<size_t> (n)] * tmpIRR[static_cast<size_t> (n)];
        }
    }

    const float targetRMS = 1.0f / std::sqrt ((float) irLen);
    const double avgRMS   = std::sqrt (totalEnergy / (double) (NUM_REF_DIRS * 2 * irLen));
    storedNormGain = (avgRMS > 1e-8) ? (float) (targetRMS / avgRMS) : 1.0f;

    // =========================================================================
    // Prepare per-source convolvers (allocate FFT buffers for irLength).
    // HRIRs are loaded later per-source via updateSourceHRIR() in processBlock.
    // =========================================================================
    for (int i = 0; i < MAX_SOURCES; ++i)
    {
        sourceConvL[i].prepare (currentBlockSize, irLen);
        sourceConvR[i].prepare (currentBlockSize, irLen);
        sourceConvReady[i] = false;   // Force HRIR reload on next processBlock
        cachedSourceAz[i] = -999.0f;  // Invalidate cached positions
        cachedSourceEl[i] = -999.0f;
    }

    DBG ("BinauralRenderer: Profile " + juce::String (profileIndex)
         + " loaded — IR=" + juce::String (irLen)
         + ", normGain=" + juce::String (storedNormGain, 4)
         + " (per-source direct binaural)");
}

void BinauralRenderer::updateSourceHRIR (int sourceIndex, float azRad, float elRad,
                                          HRTFDatabase& db)
{
    if (sourceIndex < 0 || sourceIndex >= MAX_SOURCES || storedIRLength <= 0)
        return;

    // ~1° threshold — skip update if position hasn't changed significantly
    constexpr float THRESHOLD = 0.017f;  // ~1 degree in radians
    if (sourceConvReady[sourceIndex]
        && std::abs (azRad - cachedSourceAz[sourceIndex]) < THRESHOLD
        && std::abs (elRad - cachedSourceEl[sourceIndex]) < THRESHOLD)
        return;

    // Query HRTF at exact source direction (realtime-safe: KD-tree lookup, no malloc)
    float delayL = 0.0f, delayR = 0.0f;
    std::vector<float>& tmpL = convTmpL;  // Reuse work buffer (safe: not in render path here)
    std::vector<float>& tmpR = convTmpR;

    // Ensure work buffers are large enough for IR
    if ((int) tmpL.size() < storedIRLength)
    {
        tmpL.resize (static_cast<size_t> (storedIRLength));
        tmpR.resize (static_cast<size_t> (storedIRLength));
    }

    db.getInterpolatedHRIR (azRad, elRad, tmpL.data(), tmpR.data(), delayL, delayR);

    // Apply cross-profile normalization
    for (int n = 0; n < storedIRLength; ++n)
    {
        tmpL[static_cast<size_t> (n)] *= storedNormGain;
        tmpR[static_cast<size_t> (n)] *= storedNormGain;
    }

    // Load into convolver (realtime-safe: in-place FFT in pre-allocated buffers)
    sourceConvL[sourceIndex].setIR (tmpL.data(), storedIRLength);
    sourceConvR[sourceIndex].setIR (tmpR.data(), storedIRLength);

    cachedSourceAz[sourceIndex] = azRad;
    cachedSourceEl[sourceIndex] = elRad;
    sourceConvReady[sourceIndex] = true;
}

void BinauralRenderer::renderSourceBuffers (const float* const* sourceBufs,
                                             const bool* sourceEnabled,
                                             int numSources, int numSamples,
                                             float* outL, float* outR)
{
    // Zero output
    std::memset (outL, 0, sizeof (float) * static_cast<size_t> (numSamples));
    std::memset (outR, 0, sizeof (float) * static_cast<size_t> (numSamples));

    if (convTmpL.size() < static_cast<size_t> (numSamples))
    {
        convTmpL.resize (static_cast<size_t> (numSamples));
        convTmpR.resize (static_cast<size_t> (numSamples));
    }

    for (int src = 0; src < numSources && src < MAX_SOURCES; ++src)
    {
        if (! sourceEnabled[src] || ! sourceConvReady[src])
            continue;

        // Convolve this source's accumulated signal with its L and R HRIRs
        sourceConvL[src].process (sourceBufs[src], convTmpL.data(), numSamples);
        sourceConvR[src].process (sourceBufs[src], convTmpR.data(), numSamples);

        // Sum into output
        for (int s = 0; s < numSamples; ++s)
        {
            outL[s] += convTmpL[static_cast<size_t> (s)];
            outR[s] += convTmpR[static_cast<size_t> (s)];
        }
    }
}

void BinauralRenderer::reset()
{
    for (int i = 0; i < MAX_SOURCES; ++i)
    {
        sourceConvL[i].reset();
        sourceConvR[i].reset();
        sourceConvReady[i] = false;
    }
}

//==============================================================================
// HRTF Profile Loading — maps profile index to SOFA BinaryData
//==============================================================================
//==============================================================================
// v0.3: Load HRTF profile into a specific renderer (for double-buffered swap)
// Called from timerCallback() on the message thread — NOT real-time safe
//==============================================================================
void OpenSpatialDelayProcessor::loadHRTFProfileIntoRenderer (
    int profileIndex, BinauralRenderer& renderer)
{
    if (profileIndex == 0)
    {
        hrtfDatabase.unload();
        renderer.setProfile (0, hrtfDatabase);
        DBG ("HRTF: Switched to Simple (Woodworth) profile");
        return;
    }

    const char* sofaData = nullptr;
    int sofaSize = 0;

    switch (profileIndex)
    {
        case 1:  sofaData = HRTFData::mit_kemar_large_pinna_sofa;
                 sofaSize = HRTFData::mit_kemar_large_pinna_sofaSize;  break;
        case 2:  sofaData = HRTFData::sadie_d2_ku100_sofa;
                 sofaSize = HRTFData::sadie_d2_ku100_sofaSize;         break;
        case 3:  sofaData = HRTFData::cipic_subject_003_sofa;
                 sofaSize = HRTFData::cipic_subject_003_sofaSize;      break;
        case 4:  sofaData = HRTFData::hutubs_pp2_sofa;
                 sofaSize = HRTFData::hutubs_pp2_sofaSize;             break;
        case 5:  sofaData = HRTFData::bernschuetz_ku100_sofa;
                 sofaSize = HRTFData::bernschuetz_ku100_sofaSize;      break;
        default: DBG ("HRTF: Invalid profile index " + juce::String (profileIndex)); return;
    }

    float sampleRate = static_cast<float> (currentSampleRate);
    bool success = hrtfDatabase.loadFromMemory (sofaData, sofaSize, sampleRate);

    if (success)
    {
        renderer.setProfile (profileIndex, hrtfDatabase);
        DBG ("HRTF: Loaded profile " + juce::String (profileIndex)
             + " (" + juce::String (hrtfProfileNames[profileIndex]) + ")"
             + " — " + juce::String (hrtfDatabase.getNumPositions()) + " positions"
             + ", IR=" + juce::String (hrtfDatabase.getIRLength()) + " samples");
    }
    else
    {
        DBG ("HRTF: Failed to load profile " + juce::String (profileIndex)
             + ", falling back to Simple");
    }
}

void OpenSpatialDelayProcessor::loadHRTFProfile (int profileIndex)
{
    if (profileIndex == loadedHRTFProfileIndex)
        return;

    auto& renderer = binauralRenderers[prepareRendererIndex];
    loadHRTFProfileIntoRenderer (profileIndex, renderer);

    // Atomic swap: audio thread now uses the newly loaded renderer
    activeRendererIndex.store (prepareRendererIndex, std::memory_order_release);
    prepareRendererIndex = 1 - prepareRendererIndex;
    loadedHRTFProfileIndex = profileIndex;
}

//==============================================================================
// v0.3: Timer callback — background HRTF loading
// Runs on the message thread at ~20Hz, detects profile changes and loads
// SOFA files off the audio thread
//==============================================================================
void OpenSpatialDelayProcessor::timerCallback()
{
    // --- HRTF profile loading (existing, unchanged) ---
    int wantedProfile = targetHRTFProfile.load (std::memory_order_relaxed);
    if (wantedProfile != loadedHRTFProfileIndex)
    {
        auto& renderer = binauralRenderers[prepareRendererIndex];
        renderer.prepare (currentSampleRate, static_cast<int> (monoInputBuffer.size()));
        loadHRTFProfileIntoRenderer (wantedProfile, renderer);
        activeRendererIndex.store (prepareRendererIndex, std::memory_order_release);
        prepareRendererIndex = 1 - prepareRendererIndex;
        loadedHRTFProfileIndex = wantedProfile;
    }

    // --- v0.6: ADM-OSC connection management (edge-detect enable/disable) ---
    bool admEnabled = cachedParam_admOscEnabled != nullptr
                      && cachedParam_admOscEnabled->load() >= 0.5f;
    if (admEnabled && ! prevAdmOscEnabled)
    {
        // Transition OFF→ON: connect
        oscConnected = oscReceiver.connect (oscReceivePort);
        if (oscConnected)
            oscReceiver.addListener (this);
    }
    else if (! admEnabled && prevAdmOscEnabled)
    {
        // Transition ON→OFF: disconnect
        oscReceiver.disconnect();
        oscReceiver.removeListener (this);
        oscConnected = false;
        // Clear all OSC overrides
        for (int t = 0; t < MAX_OBJECTS; ++t)
            oscOverrideActive[t].store (false, std::memory_order_relaxed);
    }
    prevAdmOscEnabled = admEnabled;

    // --- v0.6: OSC override timeout (500ms since last receive → release override) ---
    if (admEnabled)
    {
        double now = juce::Time::getMillisecondCounterHiRes();
        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (oscOverrideActive[t].load (std::memory_order_relaxed)
                && (now - oscLastReceiveTime[t]) > 500.0)
            {
                oscOverrideActive[t].store (false, std::memory_order_relaxed);
            }
        }
    }

    // --- v0.6: Per-object trajectory animation engine ---
    // Each object has its own trajectory shape, speed, phase, and base position
    {
        constexpr float dt = 1.0f / 60.0f;  // Timer runs at ~60Hz

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            // Skip if OSC is controlling this object
            if (oscOverrideActive[t].load (std::memory_order_relaxed))
                continue;

            int shape = juce::roundToInt (cachedParam_trajectoryShape[t] != nullptr
                                          ? cachedParam_trajectoryShape[t]->load() : 0.0f);

            if (shape == 0)
            {
                // None — leave position alone, reset state for this object
                prevTrajectoryShape[t] = 0;
                continue;
            }

            float speed = cachedParam_trajectorySpeed[t] != nullptr
                          ? cachedParam_trajectorySpeed[t]->load() : 1.0f;

            // Detect shape change: capture base position for THIS object
            if (prevTrajectoryShape[t] == 0 || prevTrajectoryShape[t] != shape)
            {
                baseAzimuth[t]   = cachedParam_azimuth[t]->load();    // -180..+180 degrees
                baseElevation[t] = cachedParam_elevation[t]->load();  // -90..+90 degrees
                baseDistance[t]   = cachedParam_distance[t]->load();   // 0..1
                trajectoryPhase[t] = 0.0f;
            }
            prevTrajectoryShape[t] = shape;

            // Advance phase for this object
            trajectoryPhase[t] += speed * dt;
            if (trajectoryPhase[t] >= 1.0f)
                trajectoryPhase[t] -= std::floor (trajectoryPhase[t]);

            auto result = computeTrajectory (shape, trajectoryPhase[t],
                                             baseAzimuth[t], baseElevation[t], baseDistance[t]);

            auto objStr = juce::String (t + 1);
            if (auto* azParam = apvts.getParameter ("object" + objStr + "_azimuth"))
                azParam->setValueNotifyingHost (azParam->convertTo0to1 (result.azDeg));
            if (auto* elParam = apvts.getParameter ("object" + objStr + "_elevation"))
                elParam->setValueNotifyingHost (elParam->convertTo0to1 (result.elDeg));
            if (auto* distParam = apvts.getParameter ("object" + objStr + "_distance"))
                distParam->setValueNotifyingHost (distParam->convertTo0to1 (result.dist));
        }
    }
}

// #############################################################################
// PLUGIN-SPECIFIC — Constructor, bus layout, format detection, layout activation
// Constructor wires algorithms and caches param pointers.
// Layout activation + format detection are spatial framework helpers.
// #############################################################################

//==============================================================================
// Constructor / Destructor
//==============================================================================
OpenSpatialDelayProcessor::OpenSpatialDelayProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::mono(),   true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize polymorphic algorithm pointer array (O(1) index lookup)
    // 6 algorithms (alphabetical): Ambisonics (0), DBAP (1), KNN (2), MDAP (3), VBAP (4), VBIP (5)
    algorithms[0] = &algAmbisonics;
    algorithms[1] = &algDBAP;
    algorithms[2] = &algKNN;
    algorithms[3] = &algMDAP;
    algorithms[4] = &algVBAP;
    algorithms[5] = &algVBIP;

    // Cache per-object parameter pointers (stable for APVTS lifetime, avoids string lookups in processBlock)
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        cachedParam_enabled[i]       = apvts.getRawParameterValue (prefix + "enabled");
        cachedParam_azimuth[i]       = apvts.getRawParameterValue (prefix + "azimuth");
        cachedParam_elevation[i]     = apvts.getRawParameterValue (prefix + "elevation");
        cachedParam_distance[i]      = apvts.getRawParameterValue (prefix + "distance");
        cachedParam_dopplerAmount[i] = apvts.getRawParameterValue (prefix + "dopplerAmount");
        cachedParam_trajectoryShape[i] = apvts.getRawParameterValue (prefix + "trajectoryShape");
        cachedParam_trajectorySpeed[i] = apvts.getRawParameterValue (prefix + "trajectorySpeed");
    }

    // v0.6: ADM-OSC global parameter pointer
    cachedParam_admOscEnabled = apvts.getRawParameterValue ("admOscEnabled");

    // v0.6: Load user presets from disk at startup
    loadUserPresetsFromDisk();
}

OpenSpatialDelayProcessor::~OpenSpatialDelayProcessor()
{
    // v0.6: Disconnect OSC receiver before destruction
    oscReceiver.disconnect();
    oscReceiver.removeListener (this);
}

//==============================================================================
// v0.6: OSC port change — reconnect if currently connected
//==============================================================================
void OpenSpatialDelayProcessor::setOscReceivePort (int port)
{
    if (port == oscReceivePort)
        return;

    oscReceivePort = port;

    // If currently connected, reconnect on the new port
    if (oscConnected)
    {
        oscReceiver.disconnect();
        oscReceiver.removeListener (this);
        oscConnected = oscReceiver.connect (oscReceivePort);
        if (oscConnected)
            oscReceiver.addListener (this);
    }
}

// #############################################################################
// v0.6: Preset System — Factory presets, load/save, JSON serialization
// #############################################################################

//==============================================================================
// Factory presets (8 presets, stored as static const)
//==============================================================================
const OpenSpatialDelayProcessor::PresetData
    OpenSpatialDelayProcessor::factoryPresets[NUM_FACTORY_PRESETS] =
{
    // 0: Default — 4 taps in diagonal cross pattern
    {
        "Default",
        500.0f, false, 4.0f, 0, 0.3f, 20000.0f, 20.0f, 0.0f, 0.5f, 0.0f, 0.0f,
        false, 4 /*VBAP*/, 0,
        {
            { true, -45.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 1: front-left
            { true,  45.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 2: front-right
            { true, -135.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 3: rear-left
            { true,  135.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 4: rear-right
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 1: Stereo Ping-Pong — 2 taps at ±90°
    {
        "Stereo Ping-Pong",
        350.0f, false, 4.0f, 0, 0.5f, 18000.0f, 20.0f, 0.0f, 0.5f, 0.0f, 0.0f,
        false, 4 /*VBAP*/, 0,
        {
            { true, -90.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 1: hard left
            { true,  90.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 2: hard right
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 2: Circle (Quad) — 4 taps in equidistant ring
    {
        "Circle (Quad)",
        250.0f, false, 4.0f, 0, 0.4f, 20000.0f, 20.0f, 0.0f, 0.5f, 0.0f, 0.0f,
        false, 4 /*VBAP*/, 0,
        {
            { true,   0.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 1: front
            { true,  90.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 2: right
            { true, 180.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 3: rear
            { true, -90.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Tap 4: left
            {}, {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 3: Surround 5.1 — 5 taps at standard 5.1 positions
    {
        "Surround 5.1",
        300.0f, false, 4.0f, 0, 0.35f, 20000.0f, 20.0f, 0.0f, 0.5f, 0.0f, 0.0f,
        false, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // C
            { true,  -30.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // L
            { true,   30.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // R
            { true, -110.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Ls
            { true,  110.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Rs
            {}, {}, {}, {}, {}, {}, {}
        }
    },
    // 4: Surround 7.1 — 7 taps at standard 7.1 positions
    {
        "Surround 7.1",
        250.0f, false, 4.0f, 0, 0.35f, 20000.0f, 20.0f, 0.0f, 0.5f, 0.0f, 0.0f,
        false, 4 /*VBAP*/, 0,
        {
            { true,    0.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // C
            { true,  -30.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // L
            { true,   30.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // R
            { true,  -90.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Ls
            { true,   90.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Rs
            { true, -135.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Lrs
            { true,  135.0f, 0.0f, 0.5f, 0.0f, 0, 1.0f },   // Rrs
            {}, {}, {}, {}, {}
        }
    },
    // 5: Atmos 7.1.4 — 11 taps (ear-level 7.1 + 4 height)
    {
        "Atmos 7.1.4",
        200.0f, false, 4.0f, 0, 0.3f, 20000.0f, 20.0f, 0.0f, 0.5f, 0.0f, 0.0f,
        false, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },  // C
            { true,  -30.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },  // L
            { true,   30.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },  // R
            { true,  -90.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },  // Ls
            { true,   90.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },  // Rs
            { true, -135.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },  // Lrs
            { true,  135.0f,  0.0f, 0.5f, 0.0f, 0, 1.0f },  // Rrs
            { true,  -45.0f, 45.0f, 0.5f, 0.0f, 0, 1.0f },  // Ltf
            { true,   45.0f, 45.0f, 0.5f, 0.0f, 0, 1.0f },  // Rtf
            { true, -135.0f, 45.0f, 0.5f, 0.0f, 0, 1.0f },  // Ltr
            { true,  135.0f, 45.0f, 0.5f, 0.0f, 0, 1.0f },  // Rtr
            {}
        }
    },
    // 6: Rising Spiral — 8 taps spiraling upward with orbit trajectory
    {
        "Rising Spiral",
        200.0f, false, 4.0f, 0, 0.4f, 16000.0f, 40.0f, 2.0f, 0.5f, 0.0f, 0.0f,
        false, 4 /*VBAP*/, 0,
        {
            { true,    0.0f,  -20.0f, 0.7f, 0.2f, 2 /*Orbit*/, 1.5f },
            { true,   45.0f,  -10.0f, 0.6f, 0.2f, 2 /*Orbit*/, 1.8f },
            { true,   90.0f,    0.0f, 0.5f, 0.2f, 2 /*Orbit*/, 2.0f },
            { true,  135.0f,   10.0f, 0.5f, 0.2f, 2 /*Orbit*/, 2.2f },
            { true,  180.0f,   20.0f, 0.4f, 0.2f, 2 /*Orbit*/, 2.5f },
            { true, -135.0f,   30.0f, 0.4f, 0.2f, 2 /*Orbit*/, 2.8f },
            { true,  -90.0f,   40.0f, 0.3f, 0.2f, 2 /*Orbit*/, 3.0f },
            { true,  -45.0f,   50.0f, 0.3f, 0.2f, 2 /*Orbit*/, 3.2f },
            {}, {}, {}, {}
        }
    },
    // 7: Falling Cascade — 6 taps descending with pitch drop
    {
        "Falling Cascade",
        350.0f, false, 4.0f, 0, 0.45f, 14000.0f, 30.0f, -1.0f, 0.5f, 0.0f, 0.0f,
        false, 4 /*VBAP*/, 0,
        {
            { true,  -30.0f,  40.0f, 0.3f, 0.1f, 0, 1.0f },   // High left
            { true,   60.0f,  25.0f, 0.4f, 0.1f, 0, 1.0f },   // Mid-high right
            { true, -120.0f,  10.0f, 0.5f, 0.1f, 0, 1.0f },   // Mid left-rear
            { true,  150.0f,  -5.0f, 0.6f, 0.1f, 0, 1.0f },   // Low right-rear
            { true,  -60.0f, -20.0f, 0.7f, 0.1f, 0, 1.0f },   // Lower left
            { true,   90.0f, -35.0f, 0.8f, 0.1f, 0, 1.0f },   // Lowest right
            {}, {}, {}, {}, {}, {}
        }
    }
};

//==============================================================================
// Preset API implementation
//==============================================================================
int OpenSpatialDelayProcessor::getNumPresets() const
{
    return NUM_FACTORY_PRESETS + static_cast<int> (userPresets.size());
}

juce::StringArray OpenSpatialDelayProcessor::getPresetNames() const
{
    juce::StringArray names;
    for (int i = 0; i < NUM_FACTORY_PRESETS; ++i)
        names.add (factoryPresets[i].name);
    for (const auto& up : userPresets)
        names.add (up.name);
    return names;
}

void OpenSpatialDelayProcessor::loadPreset (int index)
{
    const PresetData* preset = nullptr;

    if (index >= 0 && index < NUM_FACTORY_PRESETS)
        preset = &factoryPresets[index];
    else if (index >= NUM_FACTORY_PRESETS
             && (index - NUM_FACTORY_PRESETS) < static_cast<int> (userPresets.size()))
        preset = &userPresets[static_cast<size_t> (index - NUM_FACTORY_PRESETS)];
    else
        return;

    // Helpers — use convertTo0to1() to handle skewed NormalisableRanges correctly
    auto setFloat = [&] (const juce::String& paramId, float value) {
        if (auto* p = apvts.getParameter (paramId))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    };
    auto setChoice = [&] (const juce::String& paramId, int choiceIndex) {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramId)))
        {
            int numItems = p->choices.size();
            if (numItems > 1)
                p->setValueNotifyingHost (static_cast<float> (choiceIndex) / static_cast<float> (numItems - 1));
        }
    };
    auto setBool = [&] (const juce::String& paramId, bool value) {
        if (auto* p = apvts.getParameter (paramId))
            p->setValueNotifyingHost (value ? 1.0f : 0.0f);
    };

    // Global params
    setFloat  ("delayTime",    preset->delayTime);
    setBool   ("tempoSync",    preset->tempoSync);
    setFloat  ("noteDivision", preset->noteDivision);
    setChoice ("syncMode",     preset->syncMode);
    setFloat  ("feedback",     preset->feedback);
    setFloat  ("filterLP",     preset->filterLP);
    setFloat  ("filterHP",     preset->filterHP);
    setFloat  ("pitchShift",   preset->pitchShift);
    setFloat  ("dryWet",       preset->dryWet);
    setFloat  ("inputGain",    preset->inputGain);
    setFloat  ("outputGain",   preset->outputGain);
    setBool   ("airAbsorption", preset->airAbsorption);
    setChoice ("algorithm",    preset->algorithm);
    setChoice ("hrtfProfile",  preset->hrtfProfile);
    // NOTE: outputFormat, admOscEnabled, oscReceivePort are NOT modified by presets

    // Per-tap params
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        const auto& tap = preset->taps[i];

        setBool   (prefix + "enabled",         tap.enabled);
        setFloat  (prefix + "azimuth",         tap.azimuthDeg);
        setFloat  (prefix + "elevation",       tap.elevationDeg);
        setFloat  (prefix + "distance",        tap.distance);
        setFloat  (prefix + "dopplerAmount",   tap.dopplerAmount);
        setChoice (prefix + "trajectoryShape", tap.trajectoryShape);
        setFloat  (prefix + "trajectorySpeed", tap.trajectorySpeed);
    }

    currentPresetIndex = index;
}

void OpenSpatialDelayProcessor::loadNextPreset()
{
    int total = getNumPresets();
    if (total == 0) return;
    loadPreset ((currentPresetIndex + 1) % total);
}

void OpenSpatialDelayProcessor::loadPreviousPreset()
{
    int total = getNumPresets();
    if (total == 0) return;
    loadPreset ((currentPresetIndex - 1 + total) % total);
}

OpenSpatialDelayProcessor::PresetData OpenSpatialDelayProcessor::captureCurrentState() const
{
    PresetData pd;
    pd.delayTime    = apvts.getRawParameterValue ("delayTime")->load();
    pd.tempoSync    = apvts.getRawParameterValue ("tempoSync")->load() > 0.5f;
    pd.noteDivision = apvts.getRawParameterValue ("noteDivision")->load();
    pd.syncMode     = static_cast<int> (apvts.getRawParameterValue ("syncMode")->load());
    pd.feedback     = apvts.getRawParameterValue ("feedback")->load();
    pd.filterLP     = apvts.getRawParameterValue ("filterLP")->load();
    pd.filterHP     = apvts.getRawParameterValue ("filterHP")->load();
    pd.pitchShift   = apvts.getRawParameterValue ("pitchShift")->load();
    pd.dryWet       = apvts.getRawParameterValue ("dryWet")->load();
    pd.inputGain    = apvts.getRawParameterValue ("inputGain")->load();
    pd.outputGain   = apvts.getRawParameterValue ("outputGain")->load();
    pd.airAbsorption = apvts.getRawParameterValue ("airAbsorption")->load() > 0.5f;
    pd.algorithm    = static_cast<int> (apvts.getRawParameterValue ("algorithm")->load());
    pd.hrtfProfile  = static_cast<int> (apvts.getRawParameterValue ("hrtfProfile")->load());

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        auto& tap   = pd.taps[i];
        tap.enabled         = apvts.getRawParameterValue (prefix + "enabled")->load() > 0.5f;
        tap.azimuthDeg      = apvts.getRawParameterValue (prefix + "azimuth")->load();
        tap.elevationDeg    = apvts.getRawParameterValue (prefix + "elevation")->load();
        tap.distance        = apvts.getRawParameterValue (prefix + "distance")->load();
        tap.dopplerAmount   = apvts.getRawParameterValue (prefix + "dopplerAmount")->load();
        tap.trajectoryShape = static_cast<int> (apvts.getRawParameterValue (prefix + "trajectoryShape")->load());
        tap.trajectorySpeed = apvts.getRawParameterValue (prefix + "trajectorySpeed")->load();
    }

    return pd;
}

//==============================================================================
// JSON serialization / deserialization (using juce::JSON)
//==============================================================================
juce::String OpenSpatialDelayProcessor::serializePresetToJson (const PresetData& pd)
{
    auto* obj = new juce::DynamicObject();

    obj->setProperty ("name",          pd.name);
    obj->setProperty ("delayTime",     pd.delayTime);
    obj->setProperty ("tempoSync",     pd.tempoSync);
    obj->setProperty ("noteDivision",  pd.noteDivision);
    obj->setProperty ("syncMode",      pd.syncMode);
    obj->setProperty ("feedback",      pd.feedback);
    obj->setProperty ("filterLP",      pd.filterLP);
    obj->setProperty ("filterHP",      pd.filterHP);
    obj->setProperty ("pitchShift",    pd.pitchShift);
    obj->setProperty ("dryWet",        pd.dryWet);
    obj->setProperty ("inputGain",     pd.inputGain);
    obj->setProperty ("outputGain",    pd.outputGain);
    obj->setProperty ("airAbsorption", pd.airAbsorption);
    obj->setProperty ("algorithm",     pd.algorithm);
    obj->setProperty ("hrtfProfile",   pd.hrtfProfile);

    juce::Array<juce::var> tapsArray;
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto* tapObj = new juce::DynamicObject();
        const auto& tap = pd.taps[i];
        tapObj->setProperty ("enabled",         tap.enabled);
        tapObj->setProperty ("azimuthDeg",      tap.azimuthDeg);
        tapObj->setProperty ("elevationDeg",    tap.elevationDeg);
        tapObj->setProperty ("distance",        tap.distance);
        tapObj->setProperty ("dopplerAmount",   tap.dopplerAmount);
        tapObj->setProperty ("trajectoryShape", tap.trajectoryShape);
        tapObj->setProperty ("trajectorySpeed", tap.trajectorySpeed);
        tapsArray.add (juce::var (tapObj));
    }
    obj->setProperty ("taps", tapsArray);

    return juce::JSON::toString (juce::var (obj), false);
}

OpenSpatialDelayProcessor::PresetData
    OpenSpatialDelayProcessor::parsePresetJson (const juce::String& json)
{
    PresetData pd;
    auto parsed = juce::JSON::parse (json);
    if (auto* obj = parsed.getDynamicObject())
    {
        pd.name          = obj->getProperty ("name").toString();
        pd.delayTime     = static_cast<float> (obj->getProperty ("delayTime"));
        pd.tempoSync     = static_cast<bool>  (obj->getProperty ("tempoSync"));
        pd.noteDivision  = static_cast<float> (obj->getProperty ("noteDivision"));
        pd.syncMode      = static_cast<int>   (obj->getProperty ("syncMode"));
        pd.feedback      = static_cast<float> (obj->getProperty ("feedback"));
        pd.filterLP      = static_cast<float> (obj->getProperty ("filterLP"));
        pd.filterHP      = static_cast<float> (obj->getProperty ("filterHP"));
        pd.pitchShift    = static_cast<float> (obj->getProperty ("pitchShift"));
        pd.dryWet        = static_cast<float> (obj->getProperty ("dryWet"));
        pd.inputGain     = static_cast<float> (obj->getProperty ("inputGain"));
        pd.outputGain    = static_cast<float> (obj->getProperty ("outputGain"));
        pd.airAbsorption = static_cast<bool>  (obj->getProperty ("airAbsorption"));
        pd.algorithm     = static_cast<int>   (obj->getProperty ("algorithm"));
        pd.hrtfProfile   = static_cast<int>   (obj->getProperty ("hrtfProfile"));

        if (auto* tapsArr = obj->getProperty ("taps").getArray())
        {
            int numTaps = juce::jmin (static_cast<int> (tapsArr->size()), MAX_OBJECTS);
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
                    tap.trajectoryShape = static_cast<int>   (tapObj->getProperty ("trajectoryShape"));
                    tap.trajectorySpeed = static_cast<float> (tapObj->getProperty ("trajectorySpeed"));
                }
            }
        }
    }
    return pd;
}

//==============================================================================
// User preset file I/O
//==============================================================================
juce::File OpenSpatialDelayProcessor::getUserPresetDirectory()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("OpenSpatialDelay")
               .getChildFile ("Presets");
}

void OpenSpatialDelayProcessor::saveUserPreset (const juce::String& name)
{
    auto pd  = captureCurrentState();
    pd.name  = name;
    auto json = serializePresetToJson (pd);

    auto dir  = getUserPresetDirectory();
    dir.createDirectory();

    auto file = dir.getChildFile (name + ".json");
    file.replaceWithText (json);

    // Refresh user preset list and update index
    loadUserPresetsFromDisk();
    // Set index to the newly saved preset
    for (int i = 0; i < static_cast<int> (userPresets.size()); ++i)
    {
        if (userPresets[static_cast<size_t> (i)].name == name)
        {
            currentPresetIndex = NUM_FACTORY_PRESETS + i;
            break;
        }
    }
}

void OpenSpatialDelayProcessor::loadUserPresetsFromDisk()
{
    userPresets.clear();

    auto dir = getUserPresetDirectory();
    if (! dir.isDirectory())
        return;

    auto files = dir.findChildFiles (juce::File::findFiles, false, "*.json");
    files.sort();

    for (const auto& file : files)
    {
        auto json = file.loadFileAsString();
        if (json.isNotEmpty())
        {
            auto pd = parsePresetJson (json);
            if (pd.name.isEmpty())
                pd.name = file.getFileNameWithoutExtension();
            userPresets.push_back (pd);
        }
    }
}

//==============================================================================
bool OpenSpatialDelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Input can be mono or stereo (stereo will be summed to mono internally)
    auto inputSet = layouts.getMainInputChannelSet();
    if (inputSet != juce::AudioChannelSet::mono() &&
        inputSet != juce::AudioChannelSet::stereo())
        return false;

    // v0.2: Stable set of output bus layouts. DO NOT ADD NEW ENTRIES HERE.
    // Adding entries causes DAWs to renegotiate bus layouts during playback,
    // which triggers prepareToPlay() mid-session and zeros the delay buffer.
    // See docs/BUS_LAYOUT_BUG.md for full explanation.
    //
    // New output formats (5.0, 7.0, 5.1.2, Ambisonics, etc.) are handled
    // INTERNALLY via the output format dropdown — they render to whatever
    // channels the bus provides without needing DAW-level bus support.
    auto outputSet = layouts.getMainOutputChannelSet();
    if (outputSet == juce::AudioChannelSet::stereo())              return true;
    if (outputSet == juce::AudioChannelSet::quadraphonic())        return true;
    if (outputSet == juce::AudioChannelSet::create5point1())       return true;
    if (outputSet == juce::AudioChannelSet::create7point1())       return true;
    if (outputSet == juce::AudioChannelSet::create7point1point4()) return true;
    if (outputSet == juce::AudioChannelSet::create9point1point6()) return true;
    if (outputSet == juce::AudioChannelSet::octagonal())           return true;
    if (outputSet == juce::AudioChannelSet::discreteChannels (8))  return true;

    // v0.5: Ambisonics discrete channel buses (FOA through 6th order)
    if (outputSet == juce::AudioChannelSet::discreteChannels (4))  return true;  // FOA
    if (outputSet == juce::AudioChannelSet::discreteChannels (9))  return true;  // SOA
    if (outputSet == juce::AudioChannelSet::discreteChannels (16)) return true;  // HOA (3rd)
    if (outputSet == juce::AudioChannelSet::discreteChannels (25)) return true;  // 4th order
    if (outputSet == juce::AudioChannelSet::discreteChannels (36)) return true;  // 5th order
    if (outputSet == juce::AudioChannelSet::discreteChannels (49)) return true;  // 6th order

    return false;
}

//==============================================================================
// v0.3: Detect output format from AudioChannelSet (preferred) or channel count
// Uses JUCE channel set type to resolve ambiguous channel counts
//==============================================================================
OpenSpatialDelayProcessor::OutputFormat
    OpenSpatialDelayProcessor::detectOutputFormat (int numOutputChannels) const
{
    // Detect from channel count — matches the stable set of bus layouts
    // accepted by isBusesLayoutSupported(). New formats (5.0, 5.1.2, Ambisonics, etc.)
    // are selected by the user via the output format dropdown, not auto-detected.
    switch (numOutputChannels)
    {
        case 4:  return OutputFormat::Quad;
        case 6:  return OutputFormat::Surround5_1;
        case 8:  return OutputFormat::Surround7_1;
        case 12: return OutputFormat::Surround7_1_4;
        case 16: return OutputFormat::Surround9_1_6;
        default: return OutputFormat::Binaural;  // 2ch or unknown → stereo binaural
    }
}

juce::String OpenSpatialDelayProcessor::getOutputFormatName (OutputFormat format)
{
    for (const auto& info : outputFormatRegistry)
        if (info.format == format)
            return info.name;
    return "Unknown";
}

// #############################################################################
// SPATIAL MEDIA LIBRARY — Ambisonics decoding & layout activation
// Reusable: computeAmbiDecodeForLayout(), activateLayout(), format detection.
// #############################################################################

//==============================================================================
// Shared Ambisonics decode matrix computation
// D = E^T (E E^T + epsilon I)^{-1}  (Tikhonov-regularized pseudo-inverse)
// Used by both activateLayout() (surround) and activateMonitoringLayout() (binaural)
//==============================================================================
void OpenSpatialDelayProcessor::computeAmbiDecodeForLayout (
    const SpeakerLayout& layout, float outMatrix[][16], int& outNumSpeakers)
{
    const int N = layout.numSpeakers;
    const int M = HOA_CHANNELS;
    outNumSpeakers = N;

    // Build encoding matrix E[c][s] = evalSH(c, speaker_s_position)
    float E[16][16] = {};
    for (int s = 0; s < N; ++s)
        for (int c = 0; c < M; ++c)
            E[c][s] = evalSH (c, layout.speakers[s].azimuthRad,
                                 layout.speakers[s].elevationRad);

    // Compute EET = E * E^T  (M × M)
    float EET[16][16] = {};
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < M; ++j)
        {
            float sum = 0.0f;
            for (int s = 0; s < N; ++s)
                sum += E[i][s] * E[j][s];
            EET[i][j] = sum;
        }

    // Tikhonov regularization: EET += epsilon * I
    float epsilon = 0.01f;
    for (int i = 0; i < M; ++i)
        EET[i][i] += epsilon;

    // Invert EET via Gauss-Jordan (M × M, small matrix)
    float inv[16][16] = {};
    for (int i = 0; i < M; ++i)
        inv[i][i] = 1.0f;

    float aug[16][16];
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < M; ++j)
            aug[i][j] = EET[i][j];

    for (int col = 0; col < M; ++col)
    {
        int pivot = col;
        for (int row = col + 1; row < M; ++row)
            if (std::abs (aug[row][col]) > std::abs (aug[pivot][col]))
                pivot = row;

        if (pivot != col)
        {
            std::swap_ranges (aug[col], aug[col] + M, aug[pivot]);
            std::swap_ranges (inv[col], inv[col] + M, inv[pivot]);
        }

        float diagVal = aug[col][col];
        if (std::abs (diagVal) < 1e-10f) continue;

        for (int j = 0; j < M; ++j)
        {
            aug[col][j] /= diagVal;
            inv[col][j] /= diagVal;
        }

        for (int row = 0; row < M; ++row)
        {
            if (row == col) continue;
            float factor = aug[row][col];
            for (int j = 0; j < M; ++j)
            {
                aug[row][j] -= factor * aug[col][j];
                inv[row][j] -= factor * inv[col][j];
            }
        }
    }

    // D[s][c] = sum_k E^T[s][k] * inv[k][c] = sum_k E[k][s] * inv[k][c]
    for (int s = 0; s < N; ++s)
        for (int c = 0; c < M; ++c)
        {
            float sum = 0.0f;
            for (int k = 0; k < M; ++k)
                sum += E[k][s] * inv[k][c];
            outMatrix[s][c] = sum;
        }
}

//==============================================================================
// Build 3D VBAP triplets for a speaker layout with height speakers
//==============================================================================
static void buildVBAPTripletsForLayout (const SpeakerLayout& layout,
                                        std::vector<VBAPTriplet>& triplets)
{
    triplets.clear();
    const int N = layout.numSpeakers;

    // Check if layout has height speakers (any elevation != 0)
    bool hasHeight = false;
    for (int s = 0; s < N; ++s)
        if (std::abs (layout.speakers[s].elevationRad) > 0.01f)
        { hasHeight = true; break; }

    if (! hasHeight)
        return;  // 2D-only layout — VBAP uses pair-wise panning, no triplets needed

    for (int a = 0; a < N - 2; ++a)
    {
        for (int b = a + 1; b < N - 1; ++b)
        {
            for (int cc = b + 1; cc < N; ++cc)
            {
                auto toCart = [](float az, float el) -> std::array<float, 3> {
                    return { std::cos(el) * std::sin(az),
                             std::cos(el) * std::cos(az),
                             std::sin(el) };
                };

                auto ca = toCart (layout.speakers[a].azimuthRad, layout.speakers[a].elevationRad);
                auto cb = toCart (layout.speakers[b].azimuthRad, layout.speakers[b].elevationRad);
                auto ccc = toCart (layout.speakers[cc].azimuthRad, layout.speakers[cc].elevationRad);

                float m[3][3] = {
                    { ca[0], cb[0], ccc[0] },
                    { ca[1], cb[1], ccc[1] },
                    { ca[2], cb[2], ccc[2] }
                };

                float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                          - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                          + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

                if (std::abs (det) < 0.01f)
                    continue;

                float invDet = 1.0f / det;
                VBAPTriplet t;
                t.i = a;
                t.j = b;
                t.k = cc;
                t.inv[0][0] =  (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
                t.inv[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet;
                t.inv[0][2] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
                t.inv[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet;
                t.inv[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
                t.inv[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet;
                t.inv[2][0] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
                t.inv[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet;
                t.inv[2][2] =  (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;

                triplets.push_back (t);
            }
        }
    }
}

void OpenSpatialDelayProcessor::activateLayout (OutputFormat format)
{
    auto& buf = layoutBuffers[prepareLayoutIndex];
    buf.format = format;

    switch (format)
    {
        case OutputFormat::Quad:          buf.layout = makeQuadLayout();       break;
        case OutputFormat::Surround5_0:   buf.layout = make5_0Layout();       break;
        case OutputFormat::Surround5_1:   buf.layout = make5_1Layout();       break;
        case OutputFormat::Surround7_0:   buf.layout = make7_0Layout();       break;
        case OutputFormat::Surround7_1:   buf.layout = make7_1Layout();       break;
        case OutputFormat::Surround5_1_2: buf.layout = make5_1_2Layout();     break;
        case OutputFormat::Surround5_1_4: buf.layout = make5_1_4Layout();     break;
        case OutputFormat::Surround7_0_2: buf.layout = make7_0_2Layout();     break;
        case OutputFormat::Surround7_1_2: buf.layout = make7_1_2Layout();     break;
        case OutputFormat::Surround7_1_4: buf.layout = make7_1_4Layout();     break;
        case OutputFormat::Surround7_1_6: buf.layout = make7_1_6Layout();     break;
        case OutputFormat::Surround9_1_6: buf.layout = make9_1_6Layout();     break;
        case OutputFormat::Octaphonic:    buf.layout = makeOctaphonicLayout(); break;

        case OutputFormat::AmbisonicsFOA:
        case OutputFormat::AmbisonicsSOA:
        case OutputFormat::AmbisonicsHOA:
        case OutputFormat::Ambisonics4OA:
        case OutputFormat::Ambisonics5OA:
        case OutputFormat::Ambisonics6OA:
        {
            // Ambisonics output: no speaker layout, just channel count
            int fmtIdx = static_cast<int> (format);
            buf.layout = {};
            buf.layout.numSpeakers = 0;
            buf.layout.lfeChannelIndex = -1;
            buf.layout.totalChannels = outputFormatRegistry[static_cast<size_t> (fmtIdx)].requiredChannels;
            buf.ambiNumSpeakers = 0;
            buf.vbapTriplets.clear();
            activeLayoutIndex.store (prepareLayoutIndex, std::memory_order_release);
            prepareLayoutIndex = 1 - prepareLayoutIndex;
            return;
        }

        case OutputFormat::Binaural:
        case OutputFormat::Stereo:
        default:
            buf.layout = {};
            buf.layout.numSpeakers = 0;
            buf.layout.lfeChannelIndex = -1;
            buf.layout.totalChannels = 2;
            buf.ambiNumSpeakers = 0;
            buf.vbapTriplets.clear();
            activeLayoutIndex.store (prepareLayoutIndex, std::memory_order_release);
            prepareLayoutIndex = 1 - prepareLayoutIndex;
            return;
    }

    // Compute Ambisonics decode matrix using shared helper
    computeAmbiDecodeForLayout (buf.layout, buf.ambiDecodeMatrix, buf.ambiNumSpeakers);

    // Build 3D VBAP triplets using shared helper
    buildVBAPTripletsForLayout (buf.layout, buf.vbapTriplets);

    // Atomic swap: audio thread now reads the fully-populated buffer
    activeLayoutIndex.store (prepareLayoutIndex, std::memory_order_release);
    prepareLayoutIndex = 1 - prepareLayoutIndex;
}

//==============================================================================
// v0.3: Activate monitoring layout (virtual speakers for binaural rendering)
// Called from timerCallback() on the message thread
//==============================================================================
//==============================================================================
// v0.2: Resolve user-selected format against bus channel constraint
//==============================================================================
OpenSpatialDelayProcessor::OutputFormat
    OpenSpatialDelayProcessor::resolveEffectiveFormat (OutputFormat requested, int busChannels) const
{
    // Binaural and stereo variants always work — render to L/R within any bus ≥ 2ch
    if (requested == OutputFormat::Binaural)
        return OutputFormat::Binaural;

    // Stereo variants also render to L/R
    {
        int reqIdx = static_cast<int> (requested);
        if (reqIdx >= 0 && reqIdx < NUM_OUTPUT_FORMATS
            && outputFormatRegistry[static_cast<size_t> (reqIdx)].isStereoVariant)
            return requested;
    }

    // For surround formats, validate that the bus has enough channels
    for (const auto& info : outputFormatRegistry)
    {
        if (info.format == requested)
        {
            if (busChannels >= info.requiredChannels)
                return requested;
            break;
        }
    }

    // Fall back: find the surround format with the most channels that fits the bus.
    // Skip Ambisonics formats — falling back to SH encoding makes no sense for surround buses.
    OutputFormat fallback = OutputFormat::Binaural;
    int bestChannels = 0;
    for (const auto& info : outputFormatRegistry)
    {
        if (info.isAmbisonicsOutput)
            continue;  // Never fall back to Ambisonics output
        if (info.requiredChannels <= busChannels && info.requiredChannels > bestChannels)
        {
            bestChannels = info.requiredChannels;
            fallback = info.format;
        }
    }
    return fallback;
}

//==============================================================================
// v0.2: Runtime output format change (called from editor timer, message thread)
//==============================================================================
void OpenSpatialDelayProcessor::requestOutputFormatChange (int formatIndex)
{
    formatIndex = juce::jlimit (0, NUM_OUTPUT_FORMATS - 1, formatIndex);
    auto requested = outputFormatRegistry[static_cast<size_t> (formatIndex)].format;
    auto effective = resolveEffectiveFormat (requested, maxBusChannels);

    if (effective != getActiveLayout().format)
        activateLayout (effective);
}

//==============================================================================
void OpenSpatialDelayProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Allocate delay buffer (24 seconds max: 12 objects × 2s)
    // v0.5: Round up to power-of-2 for bitmask indexing (eliminates integer division in hot path)
    {
        int minSize = static_cast<int> (sampleRate * MAX_DELAY_SECONDS) + 1;
        delayBufferSize = 1;
        while (delayBufferSize < minSize)
            delayBufferSize <<= 1;
        delayBufferMask = delayBufferSize - 1;
    }
    delayBuffer.assign (static_cast<size_t> (delayBufferSize), 0.0f);
    writePosition = 0;
    feedbackSample = 0.0f;
    
    // Reset all pitch shifter phases (including the feedback shifter at index MAX_OBJECTS)
    for (int i = 0; i < MAX_OBJECTS + 1; ++i)
        pitchPhase[i] = 0.0f;

    // Pre-allocate mono input buffer
    monoInputBuffer.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

    // Initialize filters
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
    feedbackLPFilter.prepare (spec);
    feedbackHPFilter.prepare (spec);
    feedbackLPFilter.reset();
    feedbackHPFilter.reset();
    cachedFeedbackLPFreq = -1.0f;  // Force recalculation on first block
    cachedFeedbackHPFreq = -1.0f;

    // Cache pitch shifter window size (constant within session)
    cachedPitchWindowSamples = static_cast<float> (sampleRate) * 0.250f;

    // Initialize smoothed values — setCurrentAndTargetValue prevents stale ramps
    // after mid-session prepareToPlay calls (e.g., bus renegotiation)
    smoothedDryWet.reset     (sampleRate, 0.02);
    smoothedFeedback.reset   (sampleRate, 0.02);
    smoothedInputGain.reset  (sampleRate, 0.02);
    smoothedOutputGain.reset (sampleRate, 0.02);

    // Read current parameter values so smoothing starts at the right position
    float initDelayMs = apvts.getRawParameterValue ("delayTime")->load();
    float initDryWet  = apvts.getRawParameterValue ("dryWet")->load();
    float initFb      = apvts.getRawParameterValue ("feedback")->load();
    float initInGain  = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("inputGain")->load());
    float initOutGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("outputGain")->load());

    smoothedDryWet.setCurrentAndTargetValue     (initDryWet);
    smoothedFeedback.setCurrentAndTargetValue   (initFb);
    smoothedInputGain.setCurrentAndTargetValue  (initInGain);
    smoothedOutputGain.setCurrentAndTargetValue (initOutGain);

    // Slower smoothing for delay time to create audible pitch bend (100ms)
    smoothedDelayTime.reset  (sampleRate, 0.1);
    smoothedDelayTime.setCurrentAndTargetValue (initDelayMs);

    // Smooth loop multiplier — 50ms ramp prevents clicks when enabling/disabling objects
    smoothedLoopMultiplier.reset (sampleRate, 0.05);
    smoothedLoopMultiplier.setCurrentAndTargetValue (1.0f);

    // Initialize modular 3D audio core
    computeAmbiDecodeMatrix();        // Pre-compute 3rd-order decode matrix for virtual speakers

    // v0.2: Resolve output format from user selection + bus constraint
    maxBusChannels = getTotalNumOutputChannels();
    int userFormatIndex = static_cast<int> (apvts.getRawParameterValue ("outputFormat")->load());
    auto userFormat = outputFormatRegistry[static_cast<size_t> (juce::jlimit (0, NUM_OUTPUT_FORMATS - 1, userFormatIndex))].format;
    auto effectiveFormat = resolveEffectiveFormat (userFormat, maxBusChannels);
    activateLayout (effectiveFormat);

    // v0.2: Configure LFE low-pass filter (120 Hz, 2nd order Butterworth)
    lfeFilter.prepare (spec);
    lfeFilter.reset();
    *lfeFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 120.0f);

    // v0.4: Prepare air absorption filters (per-object LP, distance-driven cutoff)
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        airAbsorptionFilter[i].prepare (spec);
        airAbsorptionFilter[i].reset();
        *airAbsorptionFilter[i].coefficients =
            *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 20000.0f);
    }

    // v0.5: Prepare NFC-HOA filters (per-object, per-SH-order, Ambisonics output only)
    for (int obj = 0; obj < MAX_OBJECTS; ++obj)
    {
        for (int n = 0; n < MAX_AMBI_ORDER; ++n)
        {
            nfcFilters[obj][n].prepare (spec);
            nfcFilters[obj][n].reset();
            *nfcFilters[obj][n].coefficients =
                *juce::dsp::IIR::Coefficients<float>::makeAllPass (sampleRate, 1000.0f);
        }
        prevNfcDistance[obj] = -1.0f;  // Force coefficient update on first block
    }

    // v0.4: Reset Doppler velocity tracking state
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        prevAzimuth[i]   = 0.0f;
        prevElevation[i] = 0.0f;
        prevDistance[i]   = 0.5f;
        dopplerSemitones[i] = 0.0f;
        smoothedRadialVelocity[i] = 0.0f;
    }

    // HRTF convolution: prepare both renderers (double-buffered)
    binauralRenderers[0].prepare (sampleRate, samplesPerBlock);
    binauralRenderers[1].prepare (sampleRate, samplesPerBlock);

    // v0.5: Contiguous per-source accumulation buffers for direct binaural
    sourceAccumBufStorage.resize (static_cast<size_t> (MAX_OBJECTS * samplesPerBlock), 0.0f);
    for (int src = 0; src < MAX_OBJECTS; ++src)
        sourceAccumBufPtrs[src] = sourceAccumBufStorage.data() + src * samplesPerBlock;
    wetBufL.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    wetBufR.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

    // Force HRTF profile reload at new sample rate
    loadedHRTFProfileIndex = -1;

    // v0.6: Timer at ~60Hz for HRTF loading, OSC management, and trajectory animation
    startTimerHz (60);
}

void OpenSpatialDelayProcessor::releaseResources()
{
    delayBuffer.clear();
    monoInputBuffer.clear();
}

// #############################################################################
// DELAY-SPECIFIC — Delay line, pitch shifter, tempo sync
// This section contains the delay engine DSP: circular buffer read/write,
// dual-head Hann crossfade pitch shifter, and tempo-sync note division mapping.
// Other SML plugins would replace this with their own DSP (reverb, chorus, etc.).
// #############################################################################

//==============================================================================
// Delay line helpers
//==============================================================================
void OpenSpatialDelayProcessor::writeDelayLine (float sample)
{
    delayBuffer[static_cast<size_t> (writePosition)] = sample;
    writePosition = (writePosition + 1) & delayBufferMask;  // v0.5: bitmask wrap
}

float OpenSpatialDelayProcessor::readDelayLine (float delaySamples) const
{
    // readPos is floating point index relative to write head
    float readPos = static_cast<float> (writePosition) - delaySamples - 1.0f;

    // Handle wrap-around (add bufSize once is sufficient since readPos > -bufSize)
    if (readPos < 0.0f) readPos += static_cast<float> (delayBufferSize);

    int   i1 = static_cast<int> (readPos);
    float f  = readPos - static_cast<float> (i1);

    // v0.5: Get 4 points for Cubic Hermite Interpolation — bitmask wrap instead of modulo
    int i0 = (i1 - 1) & delayBufferMask;
    int i2 = (i1 + 1) & delayBufferMask;
    int i3 = (i1 + 2) & delayBufferMask;
    i1 = i1 & delayBufferMask;

    float y0 = delayBuffer[static_cast<size_t> (i0)];
    float y1 = delayBuffer[static_cast<size_t> (i1)];
    float y2 = delayBuffer[static_cast<size_t> (i2)];
    float y3 = delayBuffer[static_cast<size_t> (i3)];

    // Cubic Hermite spline interpolation (Catmull-Rom variant)
    // This significantly reduces high-frequency roll-off compared to linear interpolation,
    // which is essential for preserving the brightness required for self-oscillation.
    return y1 + 0.5f * f * (y2 - y0 + f * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3 + f * (3.0f * (y1 - y2) + y3 - y0)));
}

//==============================================================================
// Pitch Shifter — Warm Dual-Head with 250ms Hann Crossfade
// Two read heads with long window. No splice detection — natural phase
// misalignment creates warm, chorused character. ~4 splices/sec for smooth,
// ambient quality.
//==============================================================================
float OpenSpatialDelayProcessor::readPitchShifted (float delaySamples,
                                                     float semitones, int phaseIndex)
{
    if (std::abs (semitones) < 0.005f)
        return readDelayLine (delaySamples);

    const float windowSamples = cachedPitchWindowSamples;
    const float ratio = std::pow (2.0f, semitones / 12.0f);
    const float twoPi = 2.0f * juce::MathConstants<float>::pi;

    float& phase = pitchPhase[phaseIndex];

    // Head 1 position (clamp defensively)
    float p1 = phase;
    if (p1 < 0.0f || p1 >= windowSamples)
    {
        p1 = std::fmod (p1, windowSamples);
        if (p1 < 0.0f) p1 += windowSamples;
    }

    // Head 2: fixed half-window offset (no splice correction = warm character)
    float p2 = std::fmod (phase + windowSamples * 0.5f, windowSamples);
    if (p2 < 0.0f) p2 += windowSamples;

    // Hann crossfade
    float gain1 = 0.5f - 0.5f * std::cos (twoPi * p1 / windowSamples);
    float gain2 = 1.0f - gain1;

    // Read from delay line
    float s1 = readDelayLine (delaySamples + p1);
    float s2 = readDelayLine (delaySamples + p2);

    // Advance phase (continuous accumulation)
    phase += (1.0f - ratio);
    phase = std::fmod (phase, windowSamples);
    if (phase < 0.0f) phase += windowSamples;

    return s1 * gain1 + s2 * gain2;
}

// #############################################################################
// SPATIAL MEDIA LIBRARY — Spatialization algorithm implementations
// All algorithm computeGains() methods, VBAP triplet builder, Ambisonics SH
// evaluation, and decode matrix construction. Reusable across all SML plugins.
// #############################################################################

//==============================================================================
// DirectBinauralAlgorithm — ITD + ILD binaural model (Woodworth)
//==============================================================================
void DirectBinauralAlgorithm::computeGains (const SourcePosition&, const LayoutContext&,
                                             float* outputGains, int numSpeakers) const
{
    // Direct Binaural does not produce speaker gains — zero for safety
    for (int s = 0; s < numSpeakers; ++s)
        outputGains[s] = 0.0f;
}

BinauralGains DirectBinauralAlgorithm::computeBinauralGains (
    const SourcePosition& source, const BinauralContext& ctx) const
{
    const auto& profile = ctx.profiles[juce::jlimit (0, 4, ctx.profileIndex - 1)];

    // Effective lateral angle (azimuth projected by elevation)
    float sinAz  = std::sin (source.azimuthRad);
    float cosEl  = std::cos (source.elevationRad);
    float lateral = sinAz * cosEl;  // effective sine of lateral angle

    // Woodworth ITD model: t = (r/c) * (sin(theta) + theta)
    float itdSeconds = (profile.headRadius / 343.0f)
                     * (std::abs (lateral) + std::asin (std::abs (lateral)));
    float itdSamples = itdSeconds * static_cast<float> (ctx.sampleRate);

    // ILD: broadband gain difference (~8 dB at 90°)
    float ildDb = profile.ildScale * 8.0f * std::abs (lateral);
    float farEarGain = juce::Decibels::decibelsToGain (-ildDb);

    // Distance attenuation (inverse-distance law, clamped)
    float distGain = 1.0f / std::max (0.1f, source.distance * 4.0f + 0.25f);

    BinauralGains gains;

    if (lateral >= 0.0f)  // Source on the LEFT (positive azimuth = left per ADM)
    {
        gains.leftGain          = distGain;
        gains.rightGain         = distGain * farEarGain;
        gains.leftDelaySamples  = 0.0f;
        gains.rightDelaySamples = itdSamples;
    }
    else  // Source on the RIGHT
    {
        gains.leftGain          = distGain * farEarGain;
        gains.rightGain         = distGain;
        gains.leftDelaySamples  = itdSamples;
        gains.rightDelaySamples = 0.0f;
    }

    return gains;
}

//==============================================================================
// Tempo sync
//==============================================================================
float OpenSpatialDelayProcessor::getTempoSyncedDelayMs (int noteDivisionIndex) const
{
    auto playHead = getPlayHead();
    if (playHead == nullptr)
        return 500.0f;

    auto posInfo = playHead->getPosition();
    if (! posInfo.hasValue() || ! posInfo->getBpm().hasValue())
        return 500.0f;

    double bpm = *posInfo->getBpm();
    if (bpm <= 0.0)
        bpm = 120.0;

    // Unit is 1/16th note in milliseconds
    double sixteenthMs = (60000.0 / bpm) / 4.0;

    // Echo knob value (noteDivisionIndex) represents number of 16th units
    double duration = (double)noteDivisionIndex * sixteenthMs;

    // Apply mode multiplier
    int mode = static_cast<int> (apvts.getRawParameterValue ("syncMode")->load());
    if (mode == 1) // Triplet
        duration *= (2.0 / 3.0);
    else if (mode == 2) // Dotted
        duration *= 1.5;
    // Notes and 16th use face value (1.0x)

    return static_cast<float> (duration);
}

//==============================================================================
// Modular 3D Audio Core — VBAP 3D (Pulkki 1997)
//==============================================================================

// Helper: 3x3 matrix inverse, returns false if singular
static bool invert3x3 (const float m[3][3], float inv[3][3])
{
    float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
              - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
              + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    if (std::abs (det) < 1e-10f)
        return false;

    float invDet = 1.0f / det;
    inv[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
    inv[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet;
    inv[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
    inv[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet;
    inv[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
    inv[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet;
    inv[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
    inv[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * invDet;
    inv[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;
    return true;
}

// Pre-computed VBAP triangulation of 16 virtual speakers on the unit sphere
// Layout: 0-8 ear-level (9.1.6), 9-14 top ring (+45° el), 15 zenith (+90°)
// Triangulation covers the upper hemisphere only (sources below horizon are
// projected up); the 9 ear-level speakers provide full 360° horizontal coverage.
const std::vector<VBAPTriplet>& OpenSpatialDelayProcessor::getVBAPTriplets()
{
    static std::vector<VBAPTriplet> triplets;
    static bool initialized = false;

    if (initialized)
        return triplets;

    // Speaker indices for reference:
    //  0: C (0°)      1: L (30°)      2: R (-30°)
    //  3: Lw (60°)    4: Rw (-60°)    5: Ls (90°)
    //  6: Rs (-90°)   7: Lrs (135°)   8: Rrs (-135°)
    //  9: Tfl (45°)  10: Tfr (-45°)  11: Tsl (90°)
    // 12: Tsr (-90°) 13: Trl (135°)  14: Trr (-135°)
    // 15: T (zenith)

    // Triangulation connectivity:
    // Tier 1: Ear-level ring to top ring (connect adjacent ear-level pairs
    //         to the top speaker that sits above the arc between them)
    // Tier 2: Top ring to zenith (6 triangles forming the dome cap)
    const int tris[][3] = {
        // --- Tier 1: Ear-level ↔ Top ring ---
        // Front sector
        { 0,  1,  9 },  // C + L → Tfl
        { 0,  2, 10 },  // C + R → Tfr
        { 1,  3,  9 },  // L + Lw → Tfl
        { 2,  4, 10 },  // R + Rw → Tfr
        // Side sector
        { 3,  5,  9 },  // Lw + Ls → Tfl (Tfl bridges front-to-side left)
        { 4,  6, 10 },  // Rw + Rs → Tfr (Tfr bridges front-to-side right)
        { 5,  7, 11 },  // Ls + Lrs → Tsl
        { 6,  8, 12 },  // Rs + Rrs → Tsr
        // Rear sector
        { 7,  8, 13 },  // Lrs + Rrs → Trl (using Trl at 135°)
        { 7,  8, 14 },  // Lrs + Rrs → Trr (using Trr at -135°)
        // Bridging: connect top speakers to each other through ear-level ring
        { 5,  9, 11 },  // Ls + Tfl + Tsl
        { 6, 10, 12 },  // Rs + Tfr + Tsr
        { 7, 11, 13 },  // Lrs + Tsl + Trl
        { 8, 12, 14 },  // Rrs + Tsr + Trr
        // Front bridging between Tfl/Tfr through C
        { 0,  9, 10 },  // C + Tfl + Tfr
        // Rear bridging between Trl/Trr through rear ear-level
        { 7, 13, 14 },  // Lrs + Trl + Trr  (left rear bridge)
        { 8, 13, 14 },  // Rrs + Trl + Trr  (right rear bridge)

        // --- Tier 2: Top ring → Zenith (dome cap) ---
        {  9, 10, 15 }, // Tfl + Tfr + T  (front cap)
        {  9, 11, 15 }, // Tfl + Tsl + T  (front-left cap)
        { 10, 12, 15 }, // Tfr + Tsr + T  (front-right cap)
        { 11, 13, 15 }, // Tsl + Trl + T  (rear-left cap)
        { 12, 14, 15 }, // Tsr + Trr + T  (rear-right cap)
        { 13, 14, 15 }, // Trl + Trr + T  (rear cap)

        // --- Below-horizon fallback triangles ---
        // For sources below the ear plane, these triangles between adjacent
        // ear-level speakers provide panning across the lower hemisphere.
        // (The signal is effectively "projected" into the ear-level ring.)
        { 0,  1,  2 },  // C + L + R (front)
        { 1,  2,  3 },  // L + R + Lw
        { 2,  3,  4 },  // R + Lw + Rw
        { 3,  4,  5 },  // Lw + Rw + Ls
        { 4,  5,  6 },  // Rw + Ls + Rs
        { 5,  6,  7 },  // Ls + Rs + Lrs
        { 6,  7,  8 },  // Rs + Lrs + Rrs
        { 0,  7,  8 },  // C + Lrs + Rrs (rear wrap — through C for full coverage)
        { 0,  1,  7 },  // C + L + Lrs (left rear quadrant)
        { 0,  2,  8 },  // C + R + Rrs (right rear quadrant)
    };

    constexpr int numTris = sizeof(tris) / sizeof(tris[0]);

    for (int t = 0; t < numTris; ++t)
    {
        VBAPTriplet tri;
        tri.i = tris[t][0];
        tri.j = tris[t][1];
        tri.k = tris[t][2];

        // Build 3x3 matrix of speaker direction vectors
        // Convention: (sin(az)*cos(el), cos(az)*cos(el), sin(el))
        // Matches computeVBAPGains3D and activateLayout toCart lambda
        auto toCart = [](float az, float el) -> std::array<float, 3> {
            return { std::cos(el) * std::sin(az),
                     std::cos(el) * std::cos(az),
                     std::sin(el) };
        };

        auto ci = toCart (virtualSpeakers[static_cast<size_t> (tri.i)].azimuthRad, virtualSpeakers[static_cast<size_t> (tri.i)].elevationRad);
        auto cj = toCart (virtualSpeakers[static_cast<size_t> (tri.j)].azimuthRad, virtualSpeakers[static_cast<size_t> (tri.j)].elevationRad);
        auto ck = toCart (virtualSpeakers[static_cast<size_t> (tri.k)].azimuthRad, virtualSpeakers[static_cast<size_t> (tri.k)].elevationRad);

        float xi = ci[0], yi = ci[1], zi = ci[2];
        float xj = cj[0], yj = cj[1], zj = cj[2];
        float xk = ck[0], yk = ck[1], zk = ck[2];

        // Matrix L = [spk_i | spk_j | spk_k] as columns for g = L^-1 * p
        // (Pulkki 1997: p = L*g → g = L^-1 * p; speakers must be columns)
        float L[3][3] = {
            { xi, xj, xk },
            { yi, yj, yk },
            { zi, zj, zk }
        };

        if (invert3x3 (L, tri.inv))
            triplets.push_back (tri);
    }

    initialized = true;
    return triplets;
}

//==============================================================================
// VBAPAlgorithm — Vector Base Amplitude Panning (2D or 3D)
//==============================================================================
void VBAPAlgorithm::computeGains (const SourcePosition& source, const LayoutContext& ctx,
                                   float* outputGains, int /*numSpeakers*/) const
{
    if (! ctx.triplets.empty())
        computeVBAPGains3D (ctx.layout, ctx.triplets, source.azimuthRad, source.elevationRad, outputGains);
    else
        computeVBAPGains2D (ctx.layout, source.azimuthRad, outputGains);
}

//==============================================================================
// Modular 3D Audio Core — 6th-Order Ambisonics (ACN/SN3D)
//==============================================================================

static float evalSH (int acn, float az, float el)
{
    // Real spherical harmonics, ACN ordering, SN3D normalization
    // az = azimuth (radians), el = elevation (radians)
    float cosAz  = std::cos (az);
    float sinAz  = std::sin (az);
    float cos2Az = std::cos (2.0f * az);
    float sin2Az = std::sin (2.0f * az);
    float cos3Az = std::cos (3.0f * az);
    float sin3Az = std::sin (3.0f * az);
    float sinEl  = std::sin (el);
    float cosEl  = std::cos (el);
    float sinEl2 = sinEl * sinEl;
    float cosEl2 = cosEl * cosEl;

    // Higher-order trig (computed only when needed via fallthrough to default check)
    float cos4Az = 0.0f, sin4Az = 0.0f, cos5Az = 0.0f, sin5Az = 0.0f, cos6Az = 0.0f, sin6Az = 0.0f;
    float cosEl3 = 0.0f, cosEl4 = 0.0f, cosEl5 = 0.0f, cosEl6 = 0.0f;
    float sinEl4 = 0.0f;

    if (acn >= 16)
    {
        cos4Az = std::cos (4.0f * az);  sin4Az = std::sin (4.0f * az);
        cosEl3 = cosEl2 * cosEl;        cosEl4 = cosEl2 * cosEl2;
        sinEl4 = sinEl2 * sinEl2;
        if (acn >= 25)
        {
            cos5Az = std::cos (5.0f * az);  sin5Az = std::sin (5.0f * az);
            cosEl5 = cosEl4 * cosEl;
        }
        if (acn >= 36)
        {
            cos6Az = std::cos (6.0f * az);  sin6Az = std::sin (6.0f * az);
            cosEl6 = cosEl4 * cosEl2;
        }
    }

    switch (acn)
    {
        // Order 0
        case 0: return 1.0f;

        // Order 1 (SN3D: no extra factor needed)
        case 1: return sinAz * cosEl;              // Y1^-1
        case 2: return sinEl;                       // Y1^0
        case 3: return cosAz * cosEl;              // Y1^1

        // Order 2 (SN3D normalization)
        case 4: return std::sqrt (3.0f) * 0.5f * sin2Az * cosEl2;                     // Y2^-2
        case 5: return std::sqrt (3.0f) * sinAz * sinEl * cosEl;                      // Y2^-1
        case 6: return 0.5f * (3.0f * sinEl2 - 1.0f);                                 // Y2^0
        case 7: return std::sqrt (3.0f) * cosAz * sinEl * cosEl;                      // Y2^1
        case 8: return std::sqrt (3.0f) * 0.5f * cos2Az * cosEl2;                     // Y2^2

        // Order 3 (SN3D normalization)
        case  9: return std::sqrt (5.0f / 8.0f) * sin3Az * cosEl * cosEl2;            // Y3^-3
        case 10: return std::sqrt (15.0f) * 0.5f * sin2Az * sinEl * cosEl2;           // Y3^-2
        case 11: return std::sqrt (3.0f / 8.0f) * sinAz * cosEl * (5.0f * sinEl2 - 1.0f);  // Y3^-1
        case 12: return 0.5f * sinEl * (5.0f * sinEl2 - 3.0f);                        // Y3^0
        case 13: return std::sqrt (3.0f / 8.0f) * cosAz * cosEl * (5.0f * sinEl2 - 1.0f);  // Y3^1
        case 14: return std::sqrt (15.0f) * 0.5f * cos2Az * sinEl * cosEl2;           // Y3^2
        case 15: return std::sqrt (5.0f / 8.0f) * cos3Az * cosEl * cosEl2;            // Y3^3

        // Order 4 (SN3D normalization)
        case 16: return std::sqrt (35.0f) * 0.375f * sin4Az * cosEl4;                                  // Y4^-4
        case 17: return std::sqrt (35.0f / 8.0f) * sin3Az * sinEl * cosEl3;                            // Y4^-3
        case 18: return std::sqrt (5.0f) * 0.25f * sin2Az * cosEl2 * (7.0f * sinEl2 - 1.0f);          // Y4^-2
        case 19: return std::sqrt (5.0f / 8.0f) * sinAz * sinEl * cosEl * (7.0f * sinEl2 - 3.0f);     // Y4^-1
        case 20: return 0.125f * (35.0f * sinEl4 - 30.0f * sinEl2 + 3.0f);                             // Y4^0
        case 21: return std::sqrt (5.0f / 8.0f) * cosAz * sinEl * cosEl * (7.0f * sinEl2 - 3.0f);     // Y4^1
        case 22: return std::sqrt (5.0f) * 0.25f * cos2Az * cosEl2 * (7.0f * sinEl2 - 1.0f);          // Y4^2
        case 23: return std::sqrt (35.0f / 8.0f) * cos3Az * sinEl * cosEl3;                            // Y4^3
        case 24: return std::sqrt (35.0f) * 0.375f * cos4Az * cosEl4;                                  // Y4^4

        // Order 5 (SN3D normalization)
        case 25: return std::sqrt (63.0f / 8.0f) * sin5Az * cosEl5;                                                      // Y5^-5
        case 26: return std::sqrt (315.0f) * 0.375f * sin4Az * sinEl * cosEl4;                                            // Y5^-4
        case 27: return std::sqrt (35.0f / 16.0f) * sin3Az * cosEl3 * (9.0f * sinEl2 - 1.0f);                            // Y5^-3
        case 28: return std::sqrt (105.0f / 8.0f) * sin2Az * sinEl * cosEl2 * (3.0f * sinEl2 - 1.0f);                    // Y5^-2
        case 29: return std::sqrt (15.0f) * 0.125f * sinAz * cosEl * (21.0f * sinEl4 - 14.0f * sinEl2 + 1.0f);           // Y5^-1
        case 30: return 0.125f * sinEl * (63.0f * sinEl4 - 70.0f * sinEl2 + 15.0f);                                       // Y5^0
        case 31: return std::sqrt (15.0f) * 0.125f * cosAz * cosEl * (21.0f * sinEl4 - 14.0f * sinEl2 + 1.0f);           // Y5^1
        case 32: return std::sqrt (105.0f / 8.0f) * cos2Az * sinEl * cosEl2 * (3.0f * sinEl2 - 1.0f);                    // Y5^2
        case 33: return std::sqrt (35.0f / 16.0f) * cos3Az * cosEl3 * (9.0f * sinEl2 - 1.0f);                            // Y5^3
        case 34: return std::sqrt (315.0f) * 0.375f * cos4Az * sinEl * cosEl4;                                            // Y5^4
        case 35: return std::sqrt (63.0f / 8.0f) * cos5Az * cosEl5;                                                      // Y5^5

        // Order 6 (SN3D normalization)
        case 36: return std::sqrt (231.0f / 16.0f) * sin6Az * cosEl6;                                                    // Y6^-6
        case 37: return std::sqrt (693.0f / 8.0f) * sin5Az * sinEl * cosEl5;                                              // Y6^-5
        case 38: return std::sqrt (63.0f / 16.0f) * sin4Az * cosEl4 * (11.0f * sinEl2 - 1.0f);                           // Y6^-4
        case 39: return std::sqrt (315.0f / 16.0f) * sin3Az * sinEl * cosEl3 * (11.0f * sinEl2 - 3.0f);                  // Y6^-3
        case 40: return std::sqrt (105.0f / 16.0f) * sin2Az * cosEl2 * (33.0f * sinEl4 - 18.0f * sinEl2 + 1.0f) * 0.25f;// Y6^-2
        case 41: return std::sqrt (21.0f / 16.0f) * sinAz * sinEl * cosEl * (33.0f * sinEl4 - 30.0f * sinEl2 + 5.0f);    // Y6^-1
        case 42: return (231.0f * sinEl4 * sinEl2 - 315.0f * sinEl4 + 105.0f * sinEl2 - 5.0f) / 16.0f;                   // Y6^0
        case 43: return std::sqrt (21.0f / 16.0f) * cosAz * sinEl * cosEl * (33.0f * sinEl4 - 30.0f * sinEl2 + 5.0f);    // Y6^1
        case 44: return std::sqrt (105.0f / 16.0f) * cos2Az * cosEl2 * (33.0f * sinEl4 - 18.0f * sinEl2 + 1.0f) * 0.25f;// Y6^2
        case 45: return std::sqrt (315.0f / 16.0f) * cos3Az * sinEl * cosEl3 * (11.0f * sinEl2 - 3.0f);                  // Y6^3
        case 46: return std::sqrt (63.0f / 16.0f) * cos4Az * cosEl4 * (11.0f * sinEl2 - 1.0f);                           // Y6^4
        case 47: return std::sqrt (693.0f / 8.0f) * cos5Az * sinEl * cosEl5;                                              // Y6^5
        case 48: return std::sqrt (231.0f / 16.0f) * cos6Az * cosEl6;                                                    // Y6^6

        default: return 0.0f;
    }
}

void OpenSpatialDelayProcessor::computeAmbiDecodeMatrix()
{
    // Build the encoding matrix M[channel][speaker] where M[c][s] = Y_c(speaker_s)
    // Mode-matching decode: D = M^{-1}, so gain = D · coeffs gives correct speaker gains
    // Note: M has channels as rows, speakers as columns (standard Ambisonics convention)
    float M[HOA_CHANNELS][NUM_VIRTUAL_SPEAKERS];

    for (int c = 0; c < HOA_CHANNELS; ++c)
        for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
            M[c][s] = evalSH (c, virtualSpeakers[static_cast<size_t> (s)].azimuthRad,
                                  virtualSpeakers[static_cast<size_t> (s)].elevationRad);

    // Compute D = M^{-1} via Gauss-Jordan elimination on [M | I]
    // Since we have 16 channels and 16 speakers, M is square (16x16)
    float augmented[16][32];

    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            augmented[i][j]      = M[i][j];
            augmented[i][j + 16] = (i == j) ? 1.0f : 0.0f;
        }
    }

    // Forward elimination with partial pivoting
    for (int col = 0; col < 16; ++col)
    {
        // Find pivot
        int maxRow = col;
        float maxVal = std::abs (augmented[col][col]);
        for (int row = col + 1; row < 16; ++row)
        {
            if (std::abs (augmented[row][col]) > maxVal)
            {
                maxVal = std::abs (augmented[row][col]);
                maxRow = row;
            }
        }

        // Swap rows
        if (maxRow != col)
            for (int j = 0; j < 32; ++j)
                std::swap (augmented[col][j], augmented[maxRow][j]);

        // Scale pivot row
        float pivot = augmented[col][col];
        if (std::abs (pivot) < 1e-10f)
            continue; // Singular — skip (should not happen with our layout)

        float invPivot = 1.0f / pivot;
        for (int j = 0; j < 32; ++j)
            augmented[col][j] *= invPivot;

        // Eliminate column in other rows
        for (int row = 0; row < 16; ++row)
        {
            if (row == col) continue;
            float factor = augmented[row][col];
            for (int j = 0; j < 32; ++j)
                augmented[row][j] -= factor * augmented[col][j];
        }
    }

    // Extract D = M^{-1} from right half of augmented matrix
    // D[s][c] = M^{-1}[s][c] → gain[s] = sum_c D[s][c] * coeffs[c]
    for (int s = 0; s < NUM_VIRTUAL_SPEAKERS; ++s)
        for (int c = 0; c < HOA_CHANNELS; ++c)
            ambiDecodeMatrix[s][c] = augmented[s][c + 16];
}

//==============================================================================
// AmbisonicsAlgorithm — 3rd-order HOA (ACN/SN3D) with max-rE weighting
//==============================================================================
void AmbisonicsAlgorithm::computeGains (const SourcePosition& source, const LayoutContext& ctx,
                                         float* outputGains, int numSpeakers) const
{
    constexpr int HOA_CH = OpenSpatialDelayProcessor::HOA_CHANNELS;

    // Max-rE weights per order (Zotter & Frank 2012)
    static const float maxrE[4] = {
        1.0f,
        std::cos (juce::MathConstants<float>::pi / 8.0f),
        std::cos (2.0f * juce::MathConstants<float>::pi / 8.0f),
        std::cos (3.0f * juce::MathConstants<float>::pi / 8.0f),
    };
    auto acnToOrder = [](int acn) -> int {
        if (acn < 1) return 0; if (acn < 4) return 1;
        if (acn < 9) return 2; return 3;
    };

    // Step 1: SH encode with max-rE weighting
    float coeffs[HOA_CH];
    for (int c = 0; c < HOA_CH; ++c)
        coeffs[c] = evalSH (c, source.azimuthRad, source.elevationRad) * maxrE[acnToOrder (c)];

    // Step 2: Decode via matrix multiply: gain[s] = sum_c D[s][c] * coeffs[c]
    float totalPower = 0.0f;
    for (int s = 0; s < numSpeakers; ++s)
    {
        float gain = 0.0f;
        for (int c = 0; c < HOA_CH; ++c)
            gain += ctx.ambiDecodeMatrix[s][c] * coeffs[c];

        outputGains[s] = std::max (0.0f, gain);
        totalPower += outputGains[s] * outputGains[s];
    }

    // Step 3: Constant-power normalization
    if (totalPower > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (totalPower);
        for (int s = 0; s < numSpeakers; ++s)
            outputGains[s] *= scale;
    }
}


//==============================================================================
// v0.2: 2D VBAP for flat layouts (Quad, 5.1, 7.1) — azimuth-only panning
//==============================================================================
static void computeVBAPGains2D (const SpeakerLayout& layout,
                                float azimuthRad, float* outGains)
{
    const int N = layout.numSpeakers;
    for (int s = 0; s < N; ++s)
        outGains[s] = 0.0f;

    if (N < 2) return;

    // Normalize azimuth to [-π, π]
    while (azimuthRad > juce::MathConstants<float>::pi)  azimuthRad -= 2.0f * juce::MathConstants<float>::pi;
    while (azimuthRad < -juce::MathConstants<float>::pi) azimuthRad += 2.0f * juce::MathConstants<float>::pi;

    // Find the two speakers that span the source azimuth
    // Sort speaker azimuths for efficient pair finding
    struct SpkAz { int index; float az; };
    SpkAz sorted[16];
    for (int s = 0; s < N; ++s)
    {
        sorted[s].index = s;
        sorted[s].az = layout.speakers[s].azimuthRad;
        // Normalize to [-π, π]
        while (sorted[s].az > juce::MathConstants<float>::pi)  sorted[s].az -= 2.0f * juce::MathConstants<float>::pi;
        while (sorted[s].az < -juce::MathConstants<float>::pi) sorted[s].az += 2.0f * juce::MathConstants<float>::pi;
    }
    std::sort (sorted, sorted + N, [](const SpkAz& a, const SpkAz& b) { return a.az < b.az; });

    // Find spanning pair
    int leftIdx = -1, rightIdx = -1;
    for (int s = 0; s < N; ++s)
    {
        int next = (s + 1) % N;
        float az1 = sorted[s].az;
        float az2 = sorted[next].az;

        // Handle wrap-around
        if (next == 0)
            az2 += 2.0f * juce::MathConstants<float>::pi;

        float srcAz = azimuthRad;
        if (next == 0 && srcAz < az1)
            srcAz += 2.0f * juce::MathConstants<float>::pi;

        if (srcAz >= az1 && srcAz <= az2)
        {
            leftIdx = s;
            rightIdx = next;
            break;
        }
    }

    if (leftIdx < 0)
    {
        // Fallback: nearest speaker
        float minDist = 999.0f;
        int nearest = 0;
        for (int s = 0; s < N; ++s)
        {
            float d = std::abs (sorted[s].az - azimuthRad);
            if (d > juce::MathConstants<float>::pi) d = 2.0f * juce::MathConstants<float>::pi - d;
            if (d < minDist) { minDist = d; nearest = s; }
        }
        outGains[sorted[nearest].index] = 1.0f;
        return;
    }

    float az1 = sorted[leftIdx].az;
    float az2 = sorted[rightIdx].az;
    float srcAz = azimuthRad;

    // Handle wrap
    if (rightIdx == 0)
    {
        az2 += 2.0f * juce::MathConstants<float>::pi;
        if (srcAz < az1) srcAz += 2.0f * juce::MathConstants<float>::pi;
    }

    float span = az2 - az1;
    if (span < 1e-6f)
    {
        outGains[sorted[leftIdx].index] = 1.0f;
        return;
    }

    // Sine law panning (VBAP 2D)
    float g1 = std::sin (az2 - srcAz) / std::sin (span);
    float g2 = std::sin (srcAz - az1) / std::sin (span);

    // Constant-power normalization
    float power = g1 * g1 + g2 * g2;
    if (power > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (power);
        g1 *= scale;
        g2 *= scale;
    }

    outGains[sorted[leftIdx].index] = std::max (0.0f, g1);
    outGains[sorted[rightIdx].index] = std::max (0.0f, g2);
}

//==============================================================================
// v0.2: 3D VBAP for height layouts (7.1.4, 9.1.6) — uses pre-computed triplets
//==============================================================================
static void computeVBAPGains3D (const SpeakerLayout& layout,
                                const std::vector<VBAPTriplet>& triplets,
                                float azimuthRad, float elevationRad,
                                float* outGains)
{
    const int N = layout.numSpeakers;
    for (int s = 0; s < N; ++s)
        outGains[s] = 0.0f;

    // Source direction as unit Cartesian vector
    float px = std::cos (elevationRad) * std::sin (azimuthRad);
    float py = std::cos (elevationRad) * std::cos (azimuthRad);
    float pz = std::sin (elevationRad);

    float bestGainSum = 1e30f;   // Start high — pick MINIMUM sum (tightest enclosing triangle)
    int bestTri = -1;
    float bestG[3] = {};

    for (int t = 0; t < static_cast<int> (triplets.size()); ++t)
    {
        const auto& tri = triplets[static_cast<size_t> (t)];

        float g0 = tri.inv[0][0] * px + tri.inv[0][1] * py + tri.inv[0][2] * pz;
        float g1 = tri.inv[1][0] * px + tri.inv[1][1] * py + tri.inv[1][2] * pz;
        float g2 = tri.inv[2][0] * px + tri.inv[2][1] * py + tri.inv[2][2] * pz;

        if (g0 >= -1e-6f && g1 >= -1e-6f && g2 >= -1e-6f)
        {
            float sum = g0 + g1 + g2;
            if (sum < bestGainSum)   // Min sum = tightest triangle (fixes L/R swap)
            {
                bestGainSum = sum;
                bestTri = t;
                bestG[0] = std::max (0.0f, g0);
                bestG[1] = std::max (0.0f, g1);
                bestG[2] = std::max (0.0f, g2);
            }
        }
    }

    if (bestTri >= 0)
    {
        float power = bestG[0] * bestG[0] + bestG[1] * bestG[1] + bestG[2] * bestG[2];
        float scale = (power > 1e-12f) ? (1.0f / std::sqrt (power)) : 0.0f;

        outGains[triplets[static_cast<size_t> (bestTri)].i] = bestG[0] * scale;
        outGains[triplets[static_cast<size_t> (bestTri)].j] = bestG[1] * scale;
        outGains[triplets[static_cast<size_t> (bestTri)].k] = bestG[2] * scale;
    }
    else
    {
        // Fallback: nearest speaker
        float bestDot = -2.0f;
        int bestSpeaker = 0;
        for (int s = 0; s < N; ++s)
        {
            float sx = std::cos (layout.speakers[s].elevationRad) * std::sin (layout.speakers[s].azimuthRad);
            float sy = std::cos (layout.speakers[s].elevationRad) * std::cos (layout.speakers[s].azimuthRad);
            float sz = std::sin (layout.speakers[s].elevationRad);
            float dot = px * sx + py * sy + pz * sz;
            if (dot > bestDot)
            {
                bestDot = dot;
                bestSpeaker = s;
            }
        }
        outGains[bestSpeaker] = 1.0f;
    }
}

//==============================================================================
// VBIPAlgorithm — intensity-weighted VBAP (squared gains for tighter focus)
//==============================================================================
void VBIPAlgorithm::computeGains (const SourcePosition& source, const LayoutContext& ctx,
                                   float* outputGains, int numSpeakers) const
{
    // Start with VBAP gains
    if (! ctx.triplets.empty())
        computeVBAPGains3D (ctx.layout, ctx.triplets, source.azimuthRad, source.elevationRad, outputGains);
    else
        computeVBAPGains2D (ctx.layout, source.azimuthRad, outputGains);

    // Square all gains for intensity weighting
    float sum = 0.0f;
    for (int s = 0; s < numSpeakers; ++s)
    {
        outputGains[s] = outputGains[s] * outputGains[s];
        sum += outputGains[s];
    }

    // Normalize to constant power
    if (sum > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (sum);
        for (int s = 0; s < numSpeakers; ++s)
            outputGains[s] *= scale;
    }
}

//==============================================================================
// KNNAlgorithm — K-Nearest Neighbor panning (inverse-distance-squared)
//==============================================================================
void KNNAlgorithm::computeGains (const SourcePosition& source, const LayoutContext& ctx,
                                  float* outputGains, int numSpeakers) const
{
    for (int s = 0; s < numSpeakers; ++s)
        outputGains[s] = 0.0f;

    if (numSpeakers == 0) return;
    constexpr int k = 3;
    int kClamped = std::min (k, numSpeakers);

    // Source direction as unit Cartesian vector
    float px = std::cos (source.elevationRad) * std::sin (source.azimuthRad);
    float py = std::cos (source.elevationRad) * std::cos (source.azimuthRad);
    float pz = std::sin (source.elevationRad);

    // Compute angular distances
    struct SpkDist { int index; float dist; };
    SpkDist dists[16];
    for (int s = 0; s < numSpeakers; ++s)
    {
        float sx = std::cos (ctx.layout.speakers[s].elevationRad) * std::sin (ctx.layout.speakers[s].azimuthRad);
        float sy = std::cos (ctx.layout.speakers[s].elevationRad) * std::cos (ctx.layout.speakers[s].azimuthRad);
        float sz = std::sin (ctx.layout.speakers[s].elevationRad);
        float dot = juce::jlimit (-1.0f, 1.0f, px * sx + py * sy + pz * sz);
        dists[s] = { s, std::acos (dot) };
    }

    // Partial sort to find K nearest
    std::partial_sort (dists, dists + kClamped, dists + numSpeakers,
                       [](const SpkDist& a, const SpkDist& b) { return a.dist < b.dist; });

    // Check if source is exactly at a speaker
    if (dists[0].dist < 1e-4f)
    {
        outputGains[dists[0].index] = 1.0f;
        return;
    }

    // Inverse-distance-squared weighting
    float totalWeight = 0.0f;
    float weights[16] = {};
    for (int i = 0; i < kClamped; ++i)
    {
        float w = 1.0f / (dists[i].dist * dists[i].dist + 1e-6f);
        weights[i] = w;
        totalWeight += w;
    }

    // Constant-power normalization
    float totalPower = 0.0f;
    for (int i = 0; i < kClamped; ++i)
    {
        float g = weights[i] / totalWeight;
        outputGains[dists[i].index] = g;
        totalPower += g * g;
    }

    if (totalPower > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (totalPower);
        for (int s = 0; s < numSpeakers; ++s)
            outputGains[s] *= scale;
    }
}

//==============================================================================
// DBAPAlgorithm — Distance-Based Amplitude Panning (Lossius et al., ICMC 2009)
// Gains based on Euclidean distance from source to each speaker in Cartesian space
//==============================================================================
void DBAPAlgorithm::computeGains (const SourcePosition& source, const LayoutContext& ctx,
                                   float* outputGains, int numSpeakers) const
{
    for (int s = 0; s < numSpeakers; ++s)
        outputGains[s] = 0.0f;

    if (numSpeakers == 0) return;

    // Convert source position (azimuth, elevation, distance) to Cartesian
    // Source distance maps to radial position in 3D space (0..1 → 0..10 meters)
    float physicalDist = source.distance * 10.0f;
    float srcX = physicalDist * std::cos (source.elevationRad) * std::sin (source.azimuthRad);
    float srcY = physicalDist * std::cos (source.elevationRad) * std::cos (source.azimuthRad);
    float srcZ = physicalDist * std::sin (source.elevationRad);

    // Compute Euclidean distance from source to each speaker
    // Speakers are placed at unit distance (1.0) on the sphere
    constexpr float speakerRadius = 1.0f;
    constexpr float epsilon = 0.001f;  // Avoid division by zero

    float totalWeight = 0.0f;
    float weights[16] = {};

    for (int s = 0; s < numSpeakers; ++s)
    {
        float spkAz = ctx.layout.speakers[s].azimuthRad;
        float spkEl = ctx.layout.speakers[s].elevationRad;

        float spkX = speakerRadius * std::cos (spkEl) * std::sin (spkAz);
        float spkY = speakerRadius * std::cos (spkEl) * std::cos (spkAz);
        float spkZ = speakerRadius * std::sin (spkEl);

        float dx = srcX - spkX;
        float dy = srcY - spkY;
        float dz = srcZ - spkZ;
        float distSq = dx * dx + dy * dy + dz * dz;

        // Inverse-distance-squared weighting (a = 2, giving 6 dB/doubling rolloff)
        float w = 1.0f / std::max (epsilon, distSq);
        weights[s] = w;
        totalWeight += w;
    }

    // Normalize weights then apply constant-power normalization
    if (totalWeight > 1e-12f)
    {
        float totalPower = 0.0f;
        for (int s = 0; s < numSpeakers; ++s)
        {
            float g = weights[s] / totalWeight;
            outputGains[s] = g;
            totalPower += g * g;
        }

        if (totalPower > 1e-12f)
        {
            float scale = 1.0f / std::sqrt (totalPower);
            for (int s = 0; s < numSpeakers; ++s)
                outputGains[s] *= scale;
        }
    }
}

//==============================================================================
// MDAPAlgorithm — Multiple-Direction Amplitude Panning (Pulkki 2000)
// Creates source spread using N auxiliary VBAP sources on a ring around the
// main direction. Produces wider, more stable spatial images than point VBAP.
//==============================================================================
void MDAPAlgorithm::computeGains (const SourcePosition& source, const LayoutContext& ctx,
                                   float* outputGains, int numSpeakers) const
{
    constexpr int NUM_AUX = 8;  // 8 auxiliary sources on the spread ring

    for (int s = 0; s < numSpeakers; ++s)
        outputGains[s] = 0.0f;

    if (numSpeakers == 0) return;

    // Spread angle: alpha = 0.9 * (180 / L) degrees, clamped to [5, 30]
    float alphaDegs = 0.9f * (180.0f / static_cast<float> (std::max (1, numSpeakers)));
    alphaDegs = juce::jlimit (5.0f, 30.0f, alphaDegs);
    float alphaRad = juce::degreesToRadians (alphaDegs);

    // Main source direction as Cartesian unit vector
    float px = std::cos (source.elevationRad) * std::sin (source.azimuthRad);
    float py = std::cos (source.elevationRad) * std::cos (source.azimuthRad);
    float pz = std::sin (source.elevationRad);

    // Find an orthogonal vector to p for Rodrigues' rotation
    float ax, ay, az;
    if (std::abs (pz) < 0.9f)
    {
        // Cross with z-axis: p × (0,0,1) = (py, -px, 0)
        ax = py;  ay = -px;  az = 0.0f;
    }
    else
    {
        // Cross with x-axis: p × (1,0,0) = (0, pz, -py)
        ax = 0.0f;  ay = pz;  az = -py;
    }
    float aNorm = std::sqrt (ax * ax + ay * ay + az * az);
    if (aNorm > 1e-6f) { ax /= aNorm;  ay /= aNorm;  az /= aNorm; }

    // Sum gains from main source + 8 auxiliary sources
    float tempGains[16] = {};

    // Main source VBAP contribution
    if (! ctx.triplets.empty())
        computeVBAPGains3D (ctx.layout, ctx.triplets, source.azimuthRad, source.elevationRad, tempGains);
    else
        computeVBAPGains2D (ctx.layout, source.azimuthRad, tempGains);

    for (int s = 0; s < numSpeakers; ++s)
        outputGains[s] += tempGains[s];

    // Auxiliary sources on a ring at angle alpha around the main direction
    for (int i = 0; i < NUM_AUX; ++i)
    {
        float phi = 2.0f * juce::MathConstants<float>::pi * static_cast<float> (i) / static_cast<float> (NUM_AUX);

        // Rodrigues' rotation: rotate the orthogonal vector around p by phi
        float cphi = std::cos (phi);
        float sphi = std::sin (phi);
        float dot_pa = px * ax + py * ay + pz * az;
        float cross_x = py * az - pz * ay;
        float cross_y = pz * ax - px * az;
        float cross_z = px * ay - py * ax;

        float rot_ax = ax * cphi + cross_x * sphi + px * dot_pa * (1.0f - cphi);
        float rot_ay = ay * cphi + cross_y * sphi + py * dot_pa * (1.0f - cphi);
        float rot_az = az * cphi + cross_z * sphi + pz * dot_pa * (1.0f - cphi);

        // Rotate p around rot_a by alpha to get auxiliary source direction
        float ca = std::cos (alphaRad);
        float sa = std::sin (alphaRad);
        float dot_rp = rot_ax * px + rot_ay * py + rot_az * pz;
        float cross2_x = rot_ay * pz - rot_az * py;
        float cross2_y = rot_az * px - rot_ax * pz;
        float cross2_z = rot_ax * py - rot_ay * px;

        float qx = px * ca + cross2_x * sa + rot_ax * dot_rp * (1.0f - ca);
        float qy = py * ca + cross2_y * sa + rot_ay * dot_rp * (1.0f - ca);
        float qz = pz * ca + cross2_z * sa + rot_az * dot_rp * (1.0f - ca);

        // Convert back to spherical
        float qNorm = std::sqrt (qx * qx + qy * qy + qz * qz);
        if (qNorm > 1e-6f) { qx /= qNorm;  qy /= qNorm;  qz /= qNorm; }

        float auxEl = std::asin (juce::jlimit (-1.0f, 1.0f, qz));
        float auxAz = std::atan2 (qx, qy);

        // Compute VBAP gains for auxiliary source
        for (int s = 0; s < numSpeakers && s < 16; ++s)
            tempGains[s] = 0.0f;

        if (! ctx.triplets.empty())
            computeVBAPGains3D (ctx.layout, ctx.triplets, auxAz, auxEl, tempGains);
        else
            computeVBAPGains2D (ctx.layout, auxAz, tempGains);

        for (int s = 0; s < numSpeakers; ++s)
            outputGains[s] += tempGains[s];
    }

    // Energy normalization: scale so total power = 1
    float totalPower = 0.0f;
    for (int s = 0; s < numSpeakers; ++s)
        totalPower += outputGains[s] * outputGains[s];

    if (totalPower > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (totalPower);
        for (int s = 0; s < numSpeakers; ++s)
            outputGains[s] *= scale;
    }
}

//==============================================================================
//==============================================================================
// Access object state for editor
//==============================================================================
ObjectState OpenSpatialDelayProcessor::getObjectState (int objectIndex) const
{
    ObjectState state;
    if (objectIndex < 0 || objectIndex >= MAX_OBJECTS)
        return state;

    auto prefix = "object" + juce::String (objectIndex + 1) + "_";
    if (auto* p = apvts.getRawParameterValue (prefix + "enabled"))
        state.enabled = p->load() > 0.5f;
    if (auto* p = apvts.getRawParameterValue (prefix + "time"))
        state.delayTimeMs = p->load();
    if (auto* p = apvts.getRawParameterValue (prefix + "azimuth"))
        state.azimuthDeg = p->load();
    if (auto* p = apvts.getRawParameterValue (prefix + "elevation"))
        state.elevationDeg = p->load();
    if (auto* p = apvts.getRawParameterValue (prefix + "distance"))
        state.distance = p->load();

    return state;
}

// #############################################################################
// DELAY-SPECIFIC — processBlock, render methods, and DSP helpers
// processBlock dispatches to 5 rendering paths. The render methods combine
// the delay engine (above) with the spatial framework (algorithms, HRTF).
// Other SML plugins would replace everything below with their own processBlock
// and render logic, calling the spatial algorithms via computeGains().
// #############################################################################

//==============================================================================
// v0.5: Shared inline helpers for render methods
//==============================================================================
float OpenSpatialDelayProcessor::readObjectSample (int objectIndex, float baseDelaySamples,
                                                    float pitchSemitones)
{
    float objDelaySamples = static_cast<float> (objectIndex + 1) * baseDelaySamples;
    objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);
    float objPitch = static_cast<float> (objectIndex + 1) * pitchSemitones + dopplerSemitones[objectIndex];

    float objMono = readPitchShifted (objDelaySamples, objPitch, objectIndex);
    return airAbsorptionFilter[objectIndex].processSample (objMono);
}

void OpenSpatialDelayProcessor::processFeedbackSample (float currentLoopMult,
                                                        float baseDelaySamples,
                                                        float pitchSemitones, float fb)
{
    float fbDelaySamples = currentLoopMult * baseDelaySamples;
    fbDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), fbDelaySamples);
    float fbPitchShiftAmount = currentLoopMult * pitchSemitones;

    float feedbackRaw = readPitchShifted (fbDelaySamples, fbPitchShiftAmount, MAX_OBJECTS);
    float filtered = feedbackLPFilter.processSample (feedbackRaw);
    filtered = feedbackHPFilter.processSample (filtered);
    const float makeupGain = 1.0f + (fb * fb * 0.2f);
    feedbackSample = softClip (filtered * makeupGain);
    if (! std::isfinite (feedbackSample))
    {
        feedbackSample = 0.0f;
        feedbackLPFilter.reset();
        feedbackHPFilter.reset();
    }
}

//==============================================================================
// Main audio processing
//==============================================================================
void OpenSpatialDelayProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& /*midi*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto numSamples       = buffer.getNumSamples();
    auto numInputChannels = getTotalNumInputChannels();

    if (delayBuffer.empty() || numSamples <= 0)
        return;

    // --- Read parameter values -----------------------------------------------
    bool  tempoSync       = apvts.getRawParameterValue ("tempoSync")->load() > 0.5f;
    int   noteDivision    = static_cast<int> (apvts.getRawParameterValue ("noteDivision")->load());
    float lpFreq          = apvts.getRawParameterValue ("filterLP")->load();
    float hpFreq          = apvts.getRawParameterValue ("filterHP")->load();
    int   profileIndex    = static_cast<int> (apvts.getRawParameterValue ("hrtfProfile")->load());
    float pitchSemitones  = apvts.getRawParameterValue ("pitchShift")->load();

    // v0.4: Air absorption parameter read
    bool  useAirAbsorption = apvts.getRawParameterValue ("airAbsorption")->load() > 0.5f;

    // (#3) Calculate target base delay
    float targetBaseDelayMs;
    if (tempoSync)
        targetBaseDelayMs = getTempoSyncedDelayMs (noteDivision);
    else
        targetBaseDelayMs = apvts.getRawParameterValue ("delayTime")->load();

    smoothedDelayTime.setTargetValue (targetBaseDelayMs);

    float dryWetTarget    = apvts.getRawParameterValue ("dryWet")->load();
    float fbTarget        = apvts.getRawParameterValue ("feedback")->load();
    float inGainDb        = apvts.getRawParameterValue ("inputGain")->load();
    float outGainDb       = apvts.getRawParameterValue ("outputGain")->load();

    smoothedDryWet.setTargetValue     (dryWetTarget);
    smoothedFeedback.setTargetValue   (fbTarget);
    smoothedInputGain.setTargetValue  (juce::Decibels::decibelsToGain (inGainDb));
    smoothedOutputGain.setTargetValue (juce::Decibels::decibelsToGain (outGainDb));

    // --- Update feedback filters (only when frequency changes) ----------------
    if (std::abs (lpFreq - cachedFeedbackLPFreq) > 0.1f)
    {
        feedbackLPFilter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, lpFreq);
        cachedFeedbackLPFreq = lpFreq;
    }
    if (std::abs (hpFreq - cachedFeedbackHPFreq) > 0.1f)
    {
        feedbackHPFilter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, hpFreq);
        cachedFeedbackHPFreq = hpFreq;
    }

    // --- Read output layout (once per block for consistent snapshot) -----------
    const auto& layoutState = getActiveLayout();
    const auto& surLayout = layoutState.layout;
    int fmtEnumIdx = static_cast<int> (layoutState.format);
    bool isStereoVariant = (fmtEnumIdx >= 0 && fmtEnumIdx < NUM_OUTPUT_FORMATS)
                           && outputFormatRegistry[static_cast<size_t> (fmtEnumIdx)].isStereoVariant;
    bool isBinaural = (layoutState.format == OutputFormat::Binaural);
    bool isAmbiOutput = (fmtEnumIdx >= 0 && fmtEnumIdx < NUM_OUTPUT_FORMATS)
                        && outputFormatRegistry[static_cast<size_t> (fmtEnumIdx)].isAmbisonicsOutput;

    // --- Read algorithm selection (block-rate, surround only) -----------------
    int algorithmIndex = static_cast<int> (apvts.getRawParameterValue ("algorithm")->load());
    auto* algo = algorithms[juce::jlimit (0, NUM_ALGORITHMS - 1, algorithmIndex)];

    // For surround output, fall back if algorithm doesn't support speakers
    if (! isBinaural && ! isStereoVariant && ! algo->supportsSurround())
        algo = algorithms[4];  // Fall back to VBAP (index 4 in alphabetical order)

    // --- HRTF profile management (block-rate, binaural only) ---
    bool useHRTF = false;
    if (isBinaural)
    {
        // Signal target profile for background loading (timer thread handles actual load)
        targetHRTFProfile.store (profileIndex, std::memory_order_relaxed);

        // Use active double-buffered renderer
        auto& activeRenderer = binauralRenderers[activeRendererIndex.load (std::memory_order_acquire)];
        useHRTF = ! activeRenderer.isSimpleMode();
    }

    // --- Build layout context for surround only ---
    LayoutContext layoutCtx { surLayout, layoutState.vbapTriplets,
                              layoutState.ambiDecodeMatrix, layoutState.ambiNumSpeakers };

    BinauralContext binCtx { profileIndex, currentSampleRate, binauralProfiles.data() };

    // --- Read object states and pre-compute spatial gains -------------------
    ObjectState objects[MAX_OBJECTS];
    BinauralGains objGains[MAX_OBJECTS];                              // Simple Woodworth binaural
    float objChannelGains[MAX_OBJECTS][16] = {};                      // Discrete surround path
    float objDistGain[MAX_OBJECTS] = {};                              // Distance attenuation
    int lastEnabledObjectIndex = -1;

    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        // Read per-object state via cached pointers (no string lookups)
        objects[t].enabled      = cachedParam_enabled[t]->load()   > 0.5f;
        objects[t].azimuthDeg   = cachedParam_azimuth[t]->load();
        objects[t].elevationDeg = cachedParam_elevation[t]->load();
        objects[t].distance     = cachedParam_distance[t]->load();

        if (objects[t].enabled)
        {
            lastEnabledObjectIndex = t;
            float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
            float elRad = juce::degreesToRadians (objects[t].elevationDeg);

            objDistGain[t] = 1.0f / std::max (0.1f, objects[t].distance * 4.0f + 0.25f);

            SourcePosition src { azRad, elRad, objects[t].distance };

            if (isBinaural && useHRTF)
            {
                // v0.3: Direct binaural — HRIR update happens below (per-source)
                // No algorithm dispatch needed; position + distGain are all we need
            }
            else if (isBinaural)
            {
                // Simple (Woodworth): direct binaural gains via DirectBinauralAlgorithm
                objGains[t] = algDirectBinaural.computeBinauralGains (src, binCtx);
            }
            else if (isAmbiOutput)
            {
                // Ambisonics output: no gain dispatch — SH encoding is done per-sample
            }
            else
            {
                // Surround: algorithm → physical speaker gains
                algo->computeGains (src, layoutCtx, objChannelGains[t], surLayout.numSpeakers);

            }
        }
    }

    // --- v0.4: Per-block Doppler velocity computation + air absorption filter update ---
    {
        float blockDuration = static_cast<float> (numSamples) / static_cast<float> (currentSampleRate);
        constexpr float speedOfSound = 343.0f;   // m/s at 20°C
        constexpr float emaAlpha = 0.1f;          // Smoothing factor for velocity
        constexpr float maxDopplerSemitones = 12.0f;

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled)
            {
                dopplerSemitones[t] = 0.0f;
                // Set air absorption filter to transparent when object is disabled
                *airAbsorptionFilter[t].coefficients =
                    *juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, 20000.0f);
                continue;
            }

            float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
            float elRad = juce::degreesToRadians (objects[t].elevationDeg);
            float dist  = objects[t].distance;

            // Check if position has changed since last block (skip expensive math when static)
            bool positionChanged = (std::abs (azRad - prevAzimuth[t]) > 1e-5f)
                                || (std::abs (elRad - prevElevation[t]) > 1e-5f)
                                || (std::abs (dist  - prevDistance[t])  > 1e-5f);

            // --- Doppler velocity tracking (per-object amount: 0=off, >0=on) ---
            float objDopplerAmount = cachedParam_dopplerAmount[t]->load();

            if (objDopplerAmount > 0.001f && blockDuration > 0.0f)
            {
                if (positionChanged)
                {
                    // Convert current and previous positions to Cartesian (physical scale: 0..10m)
                    float physDist     = dist * 10.0f;
                    float prevPhysDist = prevDistance[t] * 10.0f;

                    float cx = physDist     * std::cos (elRad) * std::sin (azRad);
                    float cy = physDist     * std::cos (elRad) * std::cos (azRad);
                    float cz = physDist     * std::sin (elRad);
                    float px = prevPhysDist * std::cos (prevElevation[t]) * std::sin (prevAzimuth[t]);
                    float py = prevPhysDist * std::cos (prevElevation[t]) * std::cos (prevAzimuth[t]);
                    float pz = prevPhysDist * std::sin (prevElevation[t]);

                    // Radial velocity = rate of change of distance from source to listener (origin)
                    float curDist3D  = std::sqrt (cx * cx + cy * cy + cz * cz);
                    float prevDist3D = std::sqrt (px * px + py * py + pz * pz);
                    float rawRadialVelocity = (curDist3D - prevDist3D) / blockDuration;

                    // Exponential moving average smoothing to avoid clicks
                    smoothedRadialVelocity[t] = emaAlpha * rawRadialVelocity
                                              + (1.0f - emaAlpha) * smoothedRadialVelocity[t];

                    // Doppler pitch: semitones = 12 * log2(c / (c + v * amount))
                    float v = smoothedRadialVelocity[t] * objDopplerAmount;
                    float denominator = speedOfSound + v;
                    if (denominator > 1.0f)  // Prevent extreme values
                    {
                        float ratio = speedOfSound / denominator;
                        dopplerSemitones[t] = 12.0f * std::log2 (ratio);
                        dopplerSemitones[t] = juce::jlimit (-maxDopplerSemitones, maxDopplerSemitones,
                                                             dopplerSemitones[t]);
                    }
                    else
                    {
                        dopplerSemitones[t] = -maxDopplerSemitones;
                    }
                }
                else
                {
                    // Position static: decay velocity smoothly toward zero
                    smoothedRadialVelocity[t] *= (1.0f - emaAlpha);
                    if (std::abs (smoothedRadialVelocity[t]) < 1e-6f)
                    {
                        smoothedRadialVelocity[t] = 0.0f;
                        dopplerSemitones[t] = 0.0f;
                    }
                    else
                    {
                        float v = smoothedRadialVelocity[t] * objDopplerAmount;
                        float denominator = speedOfSound + v;
                        if (denominator > 1.0f)
                        {
                            dopplerSemitones[t] = 12.0f * std::log2 (speedOfSound / denominator);
                            dopplerSemitones[t] = juce::jlimit (-maxDopplerSemitones, maxDopplerSemitones,
                                                                 dopplerSemitones[t]);
                        }
                        else
                            dopplerSemitones[t] = -maxDopplerSemitones;
                    }
                }
            }
            else
            {
                dopplerSemitones[t] = 0.0f;
                smoothedRadialVelocity[t] = 0.0f;
            }

            // Store current position for next block's velocity computation
            prevAzimuth[t]   = azRad;
            prevElevation[t] = elRad;
            prevDistance[t]   = dist;

            // --- Air absorption filter coefficient update (only when distance changes) ---
            if (useAirAbsorption && positionChanged)
            {
                constexpr float absorbCoeff = 4.0f;   // Physically motivated absorption coefficient
                float cutoff = 20000.0f * std::exp (-absorbCoeff * dist);
                cutoff = juce::jlimit (200.0f, 20000.0f, cutoff);
                *airAbsorptionFilter[t].coefficients =
                    *juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, cutoff);
            }
            else if (! useAirAbsorption && positionChanged)
            {
                // Bypass: set cutoff to 20kHz (transparent) — only update when position changes
                *airAbsorptionFilter[t].coefficients =
                    *juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, 20000.0f);
            }
        }
    }

    // --- Prepare mono input --------------------------------------------------
    if (monoInputBuffer.size() < static_cast<size_t> (numSamples))
        monoInputBuffer.resize (static_cast<size_t> (numSamples));

    if (numInputChannels == 1)
    {
        auto* in = buffer.getReadPointer (0);
        for (int i = 0; i < numSamples; ++i)
            monoInputBuffer[static_cast<size_t> (i)] = in[i];
    }
    else
    {
        auto* inL = buffer.getReadPointer (0);
        auto* inR = buffer.getReadPointer (1);
        for (int i = 0; i < numSamples; ++i)
            monoInputBuffer[static_cast<size_t> (i)] = (inL[i] + inR[i]) * 0.5f;
    }

    // --- Clear output buffer -------------------------------------------------
    buffer.clear();

    // --- Per-sample processing -----------------------------------------------
    // Signal flow (SEQUENTIAL SPATIAL PING-PONG):
    //   1. INPUT: gain mono input signal
    //   2. WRITE: input + (processed_feedback × fb) → delay line
    //   3. READ OBJECTS: read from delay line at offsets (1..N) × baseDelay
    //   4. SPATIALIZE: Each object applies sequential pitch + spatial rendering
    //   5. UPDATE FEEDBACK: mono read from END of chain
    //   6. OUTPUT MIX: dry/wet blend → output channels

    // Determine the loop length multiplier based on the highest enabled object index
    // Smoothed to prevent clicks when enabling/disabling objects mid-playback
    float loopMultiplierTarget = static_cast<float>(std::max (1, lastEnabledObjectIndex + 1));
    smoothedLoopMultiplier.setTargetValue (loopMultiplierTarget);

    // --- Dispatch to appropriate render method --------------------------------
    if (isStereoVariant)
    {
        // Stereo mode from algorithm parameter: indices 6-10 → modes 0-4
        int stereoMode = juce::jlimit (6, 10, algorithmIndex) - 6;
        renderStereoVariant (buffer, numSamples, objects, objDistGain,
                             pitchSemitones, stereoMode);
    }
    else if (isBinaural && useHRTF)
    {
        renderDirectBinauralHRTF (buffer, numSamples, objects, objDistGain, pitchSemitones);
    }
    else if (isBinaural)
    {
        renderSimpleBinauralWoodworth (buffer, numSamples, objects, objGains, pitchSemitones);
    }
    else if (isAmbiOutput)
    {
        const int fmtIdx2 = static_cast<int> (layoutState.format);
        const int ambiOrder = outputFormatRegistry[static_cast<size_t> (fmtIdx2)].ambiOrder;
        renderAmbisonicsOutput (buffer, numSamples, objects, objDistGain, pitchSemitones, ambiOrder);
    }
    else
    {
        renderDiscreteSurround (buffer, numSamples, objects, objChannelGains,
                                objDistGain, pitchSemitones, surLayout);
    }
}

//==============================================================================
// v0.5: Extracted render methods (identical behavior to v0.4 processBlock paths)
//==============================================================================

void OpenSpatialDelayProcessor::renderDirectBinauralHRTF (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const float* objDistGain,
    float pitchSemitones)
{
    // =========================================================================
    // DIRECT BINAURAL PATH — per-source HRTF convolution (v0.3)
    // 3-pass architecture:
    //   Pass 1: Per-sample delay engine → per-source mono accumulation
    //   Pass 2: Per-block HRTF convolution (per source) → wet L/R
    //   Pass 3: Per-sample dry/wet mix + output gain
    // No algorithm dispatch — HRTF at exact source position IS the rendering
    // =========================================================================
    auto* outL = buffer.getWritePointer (0);
    auto* outR = buffer.getWritePointer (1);

    auto& activeRenderer = binauralRenderers[activeRendererIndex.load (std::memory_order_acquire)];

    // Update per-source HRIRs at block boundary for any taps that moved
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        if (objects[t].enabled)
        {
            float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
            float elRad = juce::degreesToRadians (objects[t].elevationDeg);
            activeRenderer.updateSourceHRIR (t, azRad, elRad, hrtfDatabase);
        }
    }

    // Zero per-source accumulation buffers (contiguous allocation)
    for (int src = 0; src < MAX_OBJECTS; ++src)
        std::memset (sourceAccumBufPtrs[src], 0, sizeof (float) * static_cast<size_t> (numSamples));

    // Ensure wet buffers are sized
    if (wetBufL.size() < static_cast<size_t> (numSamples))
    {
        wetBufL.resize (static_cast<size_t> (numSamples), 0.0f);
        wetBufR.resize (static_cast<size_t> (numSamples), 0.0f);
    }

    // Capture smoothed value start positions for Pass 3 interpolation
    float dwStart      = smoothedDryWet.getCurrentValue();
    float outGainStart = smoothedOutputGain.getCurrentValue();

    // === PASS 1: Per-sample delay engine → per-source accumulation ===
    for (int s = 0; s < numSamples; ++s)
    {
        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * 0.001f * static_cast<float> (currentSampleRate);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain = smoothedInputGain.getNextValue();
        /* dw */       smoothedDryWet.getNextValue();
        float fb     = smoothedFeedback.getNextValue();
        /* outGain */  smoothedOutputGain.getNextValue();

        float rawInput    = monoInputBuffer[static_cast<size_t> (s)];
        float inputSample = rawInput * inGain;

        // STAGE 1: WRITE to delay line
        float delayInput = inputSample + feedbackSample * fb;
        delayInput = softClip (delayInput * 0.98f);
        writeDelayLine (delayInput);

        // STAGE 2: READ & ACCUMULATE (per-source mono × distance gain)
        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled) continue;
            sourceAccumBufPtrs[t][s] = readObjectSample (t, baseDelaySamples, pitchSemitones) * objDistGain[t];
        }

        // STAGE 3: FEEDBACK (mono, pre-spatial)
        processFeedbackSample (currentLoopMult, baseDelaySamples, pitchSemitones, fb);
    }

    // === PASS 2: Per-block per-source HRTF convolution → wet L/R ===
    bool sourceEnabled[MAX_OBJECTS];
    for (int t = 0; t < MAX_OBJECTS; ++t)
        sourceEnabled[t] = objects[t].enabled;

    const float* srcBufPtrs[MAX_OBJECTS];
    for (int t = 0; t < MAX_OBJECTS; ++t)
        srcBufPtrs[t] = sourceAccumBufPtrs[t];

    activeRenderer.renderSourceBuffers (srcBufPtrs, sourceEnabled, MAX_OBJECTS,
                                        numSamples, wetBufL.data(), wetBufR.data());

    // === PASS 3: Per-sample dry/wet mix + output gain ===
    float dwEnd      = smoothedDryWet.getCurrentValue();
    float outGainEnd = smoothedOutputGain.getCurrentValue();
    float invN = (numSamples > 1) ? 1.0f / static_cast<float> (numSamples - 1) : 1.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float frac    = static_cast<float> (s) * invN;
        float dw      = dwStart + frac * (dwEnd - dwStart);
        float outGain = outGainStart + frac * (outGainEnd - outGainStart);
        float rawInput = monoInputBuffer[static_cast<size_t> (s)];

        outL[s] = (rawInput * (1.0f - dw) + wetBufL[static_cast<size_t> (s)] * dw) * outGain;
        outR[s] = (rawInput * (1.0f - dw) + wetBufR[static_cast<size_t> (s)] * dw) * outGain;
    }

    // Zero remaining channels when binaural is selected on a multi-channel bus
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

//------------------------------------------------------------------------------
void OpenSpatialDelayProcessor::renderSimpleBinauralWoodworth (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const BinauralGains* objGains,
    float pitchSemitones)
{
    // =========================================================================
    // SIMPLE (WOODWORTH) BINAURAL PATH — per-sample rendering
    // Used when profile = "Simple (Low CPU)" — no HRTF convolution
    // Always uses DirectBinauralAlgorithm (Woodworth ITD+ILD)
    // =========================================================================
    auto* outL = buffer.getWritePointer (0);
    auto* outR = buffer.getWritePointer (1);

    for (int s = 0; s < numSamples; ++s)
    {
        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * 0.001f * static_cast<float> (currentSampleRate);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float dw      = smoothedDryWet.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();

        float rawInput    = monoInputBuffer[static_cast<size_t> (s)];
        float inputSample = rawInput * inGain;

        // === STAGE 1: WRITE ===
        float delayInput = inputSample + feedbackSample * fb;
        delayInput = softClip (delayInput * 0.98f);
        writeDelayLine (delayInput);

        // === STAGE 2: READ & SPATIALIZE ===
        float wetL = 0.0f, wetR = 0.0f;

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled) continue;
            float objMono = readObjectSample (t, baseDelaySamples, pitchSemitones);

            // Woodworth binaural gains (pre-computed at block start)
            wetL += objMono * objGains[t].leftGain;
            wetR += objMono * objGains[t].rightGain;
        }

        // === STAGE 3: FEEDBACK ===
        processFeedbackSample (currentLoopMult, baseDelaySamples, pitchSemitones, fb);

        // === STAGE 4: OUTPUT MIX ===
        outL[s] = (rawInput * (1.0f - dw) + wetL * dw) * outGain;
        outR[s] = (rawInput * (1.0f - dw) + wetR * dw) * outGain;
    }

    // Zero remaining channels when binaural is selected on a multi-channel bus
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

//------------------------------------------------------------------------------
void OpenSpatialDelayProcessor::renderStereoVariant (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const float* objDistGain,
    float pitchSemitones, int stereoMode)
{
    // =========================================================================
    // STEREO VARIANT PATH — microphone simulation rendering (v0.5)
    // Each variant converts a tap's azimuth + distance → L/R gains
    // Elevation contributes only to distance attenuation (real mic pairs don't
    // capture height in L/R). These are fundamentally different from binaural
    // (HRTF head model) — these simulate how mics capture a 3D scene.
    // =========================================================================
    auto* outL = buffer.getWritePointer (0);
    auto* outR = buffer.getWritePointer (1);

    // Pre-compute stereo gains per enabled object (block-rate, positions fixed within block)
    float objGainL[MAX_OBJECTS] = {};
    float objGainR[MAX_OBJECTS] = {};

    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        if (! objects[t].enabled) continue;

        float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
        float dG = objDistGain[t];

        // stereoMode: 0=Equal Power, 1=VBAP, 2=XY, 3=MS, 4=Blumlein
        switch (stereoMode)
        {
            case 1:  // Stereo VBAP — 2 virtual speakers at ±30°
            {
                constexpr float spkHalf = degToRad (30.0f);
                float lateralPos = -std::sin (azRad);  // +1=right, -1=left
                float tVal = juce::jlimit (0.0f, 1.0f,
                    (lateralPos / std::sin (spkHalf) + 1.0f) * 0.5f);  // 0=L, 1=R
                objGainL[t] = std::cos (tVal * juce::MathConstants<float>::halfPi) * dG;
                objGainR[t] = std::sin (tVal * juce::MathConstants<float>::halfPi) * dG;
                break;
            }
            case 2:  // XY Pair — coincident cardioid pair at ±45°
            {
                constexpr float angle = degToRad (45.0f);
                objGainL[t] = 0.5f * (1.0f + std::cos (azRad - angle)) * dG;
                objGainR[t] = 0.5f * (1.0f + std::cos (azRad + angle)) * dG;
                break;
            }
            case 3:  // MS Encode — Mid = |cos(az)|, Side = sin(az), L = M+S, R = M-S
            {
                float mid  = std::abs (std::cos (azRad)) * dG;
                float side = std::sin (azRad) * dG;
                objGainL[t] = mid + side;
                objGainR[t] = mid - side;
                break;
            }
            case 4:  // Blumlein — crossed figure-8 pair at ±45°
            {
                constexpr float spkHalf = degToRad (45.0f);
                float lateralPos = -std::sin (azRad);
                float tVal = juce::jlimit (0.0f, 1.0f,
                    (lateralPos / std::sin (spkHalf) + 1.0f) * 0.5f);
                objGainL[t] = std::cos (tVal * juce::MathConstants<float>::halfPi) * dG;
                objGainR[t] = std::sin (tVal * juce::MathConstants<float>::halfPi) * dG;
                break;
            }
            default:  // 0 = Equal Power pan law
            {
                float pan = (-std::sin (azRad) + 1.0f) * 0.5f;  // 0=left, 1=right
                objGainL[t] = std::cos (pan * juce::MathConstants<float>::halfPi) * dG;
                objGainR[t] = std::sin (pan * juce::MathConstants<float>::halfPi) * dG;
                break;
            }
        }
    }

    // Per-sample processing (same engine as Woodworth, with stereo gains instead of binaural)
    for (int s = 0; s < numSamples; ++s)
    {
        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * 0.001f * static_cast<float> (currentSampleRate);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float dw      = smoothedDryWet.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();

        float rawInput    = monoInputBuffer[static_cast<size_t> (s)];
        float inputSample = rawInput * inGain;

        // === STAGE 1: WRITE ===
        float delayInput = inputSample + feedbackSample * fb;
        delayInput = softClip (delayInput * 0.98f);
        writeDelayLine (delayInput);

        // === STAGE 2: READ & SPATIALIZE ===
        float wetL = 0.0f, wetR = 0.0f;

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled) continue;
            float objMono = readObjectSample (t, baseDelaySamples, pitchSemitones);

            wetL += objMono * objGainL[t];
            wetR += objMono * objGainR[t];
        }

        // === STAGE 3: FEEDBACK ===
        processFeedbackSample (currentLoopMult, baseDelaySamples, pitchSemitones, fb);

        // === STAGE 4: OUTPUT MIX ===
        outL[s] = (rawInput * (1.0f - dw) + wetL * dw) * outGain;
        outR[s] = (rawInput * (1.0f - dw) + wetR * dw) * outGain;
    }

    // Zero remaining channels when stereo is selected on a multi-channel bus
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

//------------------------------------------------------------------------------
void OpenSpatialDelayProcessor::renderAmbisonicsOutput (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const float* objDistGain,
    float pitchSemitones, int ambiOrder)
{
    // =========================================================================
    // AMBISONICS OUTPUT PATH — SH encode per source (v0.5: up to 6th order)
    // Writes AmbiX (ACN/SN3D) coefficients directly to output channels
    // No algorithm dispatch — encoding is pure spherical harmonic evaluation
    // NFC-HOA applied per-order for near-field distance compensation
    // =========================================================================
    const int numAmbiCh = std::min ((ambiOrder + 1) * (ambiOrder + 1),
                                    static_cast<int> (MAX_AMBI_CHANNELS));

    // Max-rE weights per SH order for perceptual quality (Zotter & Frank 2012)
    // w_n = cos(n * pi / (2*N + 2))  where N = max order used in this block
    const float maxrEDenom = 2.0f * static_cast<float> (ambiOrder) + 2.0f;
    float maxrE[MAX_AMBI_ORDER + 1];
    for (int n = 0; n <= ambiOrder; ++n)
        maxrE[n] = std::cos (static_cast<float> (n) * juce::MathConstants<float>::pi / maxrEDenom);

    auto acnToOrder = [](int acn) -> int {
        if (acn < 1)  return 0;  if (acn < 4)  return 1;
        if (acn < 9)  return 2;  if (acn < 16) return 3;
        if (acn < 25) return 4;  if (acn < 36) return 5;
        return 6;
    };

    // --- NFC-HOA: Update filter coefficients when distance changes (block-rate) ---
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        if (! objects[t].enabled) continue;
        float distMeters = objects[t].distance * 10.0f;  // 0..1 → 0..10m
        if (std::abs (distMeters - prevNfcDistance[t]) > 0.01f)
        {
            for (int ord = 0; ord < ambiOrder && ord < MAX_AMBI_ORDER; ++ord)
            {
                int n = ord + 1;  // SH order (1-based)
                constexpr float c = 343.0f;  // speed of sound, m/s
                float r = std::max (0.05f, distMeters);
                float fPole = static_cast<float> (n) * c / (2.0f * juce::MathConstants<float>::pi * r);
                float fZero = static_cast<float> (n) * c / (2.0f * juce::MathConstants<float>::pi * NFC_REFERENCE_RADIUS);
                float maxFreq = static_cast<float> (currentSampleRate) * 0.25f;
                fPole = std::min (fPole, maxFreq);
                fZero = std::min (fZero, maxFreq);
                float wPole = std::tan (juce::MathConstants<float>::pi * fPole / static_cast<float> (currentSampleRate));
                float wZero = std::tan (juce::MathConstants<float>::pi * fZero / static_cast<float> (currentSampleRate));
                float b0 = (1.0f + wZero);
                float b1 = (wZero - 1.0f);
                float a0 = (1.0f + wPole);
                float a1 = (wPole - 1.0f);
                *nfcFilters[t][ord].coefficients =
                    juce::dsp::IIR::Coefficients<float> (b0 / a0, b1 / a0, 1.0f, a1 / a0);
            }
            prevNfcDistance[t] = distMeters;
        }
    }

    // Pre-compute SH coefficients per enabled tap (block-rate — positions fixed within block)
    float objSHCoeffs[MAX_OBJECTS][MAX_AMBI_CHANNELS] = {};
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        if (! objects[t].enabled) continue;
        float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
        float elRad = juce::degreesToRadians (objects[t].elevationDeg);
        for (int c = 0; c < numAmbiCh; ++c)
            objSHCoeffs[t][c] = evalSH (c, azRad, elRad) * maxrE[acnToOrder (c)];
    }

    // Get output channel write pointers
    float* outChannels[MAX_AMBI_CHANNELS] = {};
    int numOutCh = buffer.getNumChannels();
    for (int ch = 0; ch < numOutCh && ch < MAX_AMBI_CHANNELS; ++ch)
        outChannels[ch] = buffer.getWritePointer (ch);

    for (int s = 0; s < numSamples; ++s)
    {
        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * 0.001f * static_cast<float> (currentSampleRate);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float dw      = smoothedDryWet.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();

        float rawInput    = monoInputBuffer[static_cast<size_t> (s)];
        float inputSample = rawInput * inGain;

        // === STAGE 1: WRITE ===
        float delayInput = inputSample + feedbackSample * fb;
        delayInput = softClip (delayInput * 0.98f);
        writeDelayLine (delayInput);

        // === STAGE 2: READ & SH ENCODE (with NFC-HOA) ===
        float ambiAccum[MAX_AMBI_CHANNELS] = {};

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled) continue;
            float objMono = readObjectSample (t, baseDelaySamples, pitchSemitones);
            float dist = objDistGain[t];
            float scaledMono = objMono * dist;

            // Order 0 (W channel): no NFC needed
            ambiAccum[0] += scaledMono * objSHCoeffs[t][0];

            // Orders 1+: apply NFC-HOA per-order shelf filter
            for (int ord = 0; ord < ambiOrder && ord < MAX_AMBI_ORDER; ++ord)
            {
                float nfcMono = nfcFilters[t][ord].processSample (scaledMono);
                int startACN = (ord + 1) * (ord + 1);
                int endACN = (ord + 2) * (ord + 2);
                for (int c = startACN; c < endACN && c < numAmbiCh; ++c)
                    ambiAccum[c] += nfcMono * objSHCoeffs[t][c];
            }
        }

        // === STAGE 3: FEEDBACK ===
        processFeedbackSample (currentLoopMult, baseDelaySamples, pitchSemitones, fb);

        // === STAGE 4: OUTPUT MIX ===
        // Wet: SH-encoded signal to all Ambisonics channels
        for (int c = 0; c < numAmbiCh && c < numOutCh; ++c)
        {
            if (outChannels[c] != nullptr)
                outChannels[c][s] = ambiAccum[c] * dw * outGain;
        }

        // Dry: omnidirectional (W channel = ACN 0 only)
        if (outChannels[0] != nullptr)
            outChannels[0][s] += rawInput * (1.0f - dw) * outGain;
    }
}

//------------------------------------------------------------------------------
void OpenSpatialDelayProcessor::renderDiscreteSurround (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const float (*objChannelGains)[16],
    const float* objDistGain, float pitchSemitones,
    const SpeakerLayout& surLayout)
{
    // =========================================================================
    // DISCRETE SURROUND PATH (multi-channel output — v0.2)
    // =========================================================================
    const int numSpeakers = surLayout.numSpeakers;
    const int lfeIdx = surLayout.lfeChannelIndex;

    // Get output channel write pointers
    float* outChannels[16] = {};
    int numOutCh = buffer.getNumChannels();
    for (int ch = 0; ch < numOutCh && ch < 16; ++ch)
        outChannels[ch] = buffer.getWritePointer (ch);

    for (int s = 0; s < numSamples; ++s)
    {
        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * 0.001f * static_cast<float> (currentSampleRate);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float dw      = smoothedDryWet.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();

        float rawInput    = monoInputBuffer[static_cast<size_t> (s)];
        float inputSample = rawInput * inGain;

        // === STAGE 1: WRITE ===
        float delayInput = inputSample + feedbackSample * fb;
        delayInput = softClip (delayInput * 0.98f);
        writeDelayLine (delayInput);

        // === STAGE 2: READ & SPATIALIZE ===
        float channelAccum[16] = {};
        float wetMono = 0.0f;  // For LFE generation

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled) continue;
            float objMono = readObjectSample (t, baseDelaySamples, pitchSemitones);
            float dist = objDistGain[t];

            for (int sp = 0; sp < numSpeakers; ++sp)
                channelAccum[sp] += objMono * dist * objChannelGains[t][sp];

            wetMono += objMono * dist;
        }

        // === STAGE 3: FEEDBACK ===
        processFeedbackSample (currentLoopMult, baseDelaySamples, pitchSemitones, fb);

        // === STAGE 4: OUTPUT MIX ===
        // Route spatialized signal to output channels
        for (int sp = 0; sp < numSpeakers; ++sp)
        {
            int ch = surLayout.speakers[sp].channelIndex;
            if (ch >= 0 && ch < numOutCh && outChannels[ch] != nullptr)
                outChannels[ch][s] = channelAccum[sp] * dw * outGain;
        }

        // Dry signal → L and R only (channels 0 and 1)
        float drySignal = rawInput * (1.0f - dw) * outGain;
        if (outChannels[0] != nullptr) outChannels[0][s] += drySignal;
        if (outChannels[1] != nullptr) outChannels[1][s] += drySignal;

        // LFE generation — low-pass filtered mono sum at −10 dB
        if (lfeIdx >= 0 && lfeIdx < numOutCh && outChannels[lfeIdx] != nullptr)
        {
            float lfeSig = lfeFilter.processSample (wetMono) * 0.316f * dw * outGain;  // −10 dB ≈ 0.316
            outChannels[lfeIdx][s] = lfeSig;
        }
    }
}

// #############################################################################
// v0.6 — ADM-OSC Receive & Trajectory Animation
// OSC message parsing (ADM-OSC namespace), position updates via APVTS,
// Cartesian→Polar conversion (ITU-R BS.2127-0), and trajectory shape functions.
// #############################################################################

//==============================================================================
// ADM-OSC Receive — message thread callback (MessageLoopCallback)
//==============================================================================
void OpenSpatialDelayProcessor::oscMessageReceived (const juce::OSCMessage& message)
{
    const auto address = message.getAddressPattern().toString();

    // ADM-OSC namespace: /adm/obj/N/...
    // N is 1-based in ADM-OSC, we convert to 0-based internal index
    if (! address.startsWith ("/adm/obj/"))
        return;

    // Parse object number: /adm/obj/N/...
    auto afterObj = address.substring (9);  // skip "/adm/obj/"
    auto slashIdx = afterObj.indexOf ("/");
    if (slashIdx < 0) return;

    int objNum = afterObj.substring (0, slashIdx).getIntValue();
    if (objNum < 1 || objNum > MAX_OBJECTS) return;
    int objIdx = objNum - 1;  // 0-based

    auto property = afterObj.substring (slashIdx);  // e.g., "/azim", "/aed", "/xyz"

    if (property == "/azim" && message.size() >= 1 && message[0].isFloat32())
    {
        float azDeg = message[0].getFloat32();
        // getRawParameterValue returns actual (denormalized) values: el in -90..+90, dist in 0..1
        handleOSCPosition (objIdx, azDeg,
                           cachedParam_elevation[objIdx]->load(),
                           cachedParam_distance[objIdx]->load());
    }
    else if (property == "/elev" && message.size() >= 1 && message[0].isFloat32())
    {
        float elDeg = message[0].getFloat32();
        handleOSCPosition (objIdx,
                           cachedParam_azimuth[objIdx]->load(),
                           elDeg,
                           cachedParam_distance[objIdx]->load());
    }
    else if (property == "/dist" && message.size() >= 1 && message[0].isFloat32())
    {
        float dist = message[0].getFloat32();
        handleOSCPosition (objIdx,
                           cachedParam_azimuth[objIdx]->load(),
                           cachedParam_elevation[objIdx]->load(),
                           dist);
    }
    else if (property == "/aed" && message.size() >= 3
             && message[0].isFloat32() && message[1].isFloat32() && message[2].isFloat32())
    {
        handleOSCPosition (objIdx,
                           message[0].getFloat32(),   // azimuth degrees
                           message[1].getFloat32(),   // elevation degrees
                           message[2].getFloat32());  // distance 0..1
    }
    else if (property == "/xyz" && message.size() >= 3
             && message[0].isFloat32() && message[1].isFloat32() && message[2].isFloat32())
    {
        // Cartesian→Polar conversion per ITU-R BS.2127-0
        float x = message[0].getFloat32();
        float y = message[1].getFloat32();
        float z = message[2].getFloat32();
        float azDeg = std::atan2 (-x, y) * (180.0f / juce::MathConstants<float>::pi);
        float r = std::sqrt (x * x + y * y);
        float elDeg = std::atan2 (z, r) * (180.0f / juce::MathConstants<float>::pi);
        float dist = juce::jlimit (0.0f, 1.0f, std::sqrt (x * x + y * y + z * z));
        handleOSCPosition (objIdx, azDeg, elDeg, dist);
    }
    else if (property == "/x" && message.size() >= 1 && message[0].isFloat32())
    {
        oscCartesianX[objIdx] = message[0].getFloat32();
        float x = oscCartesianX[objIdx], y = oscCartesianY[objIdx], z = oscCartesianZ[objIdx];
        float azDeg = std::atan2 (-x, y) * (180.0f / juce::MathConstants<float>::pi);
        float r = std::sqrt (x * x + y * y);
        float elDeg = std::atan2 (z, r) * (180.0f / juce::MathConstants<float>::pi);
        float dist = juce::jlimit (0.0f, 1.0f, std::sqrt (x * x + y * y + z * z));
        handleOSCPosition (objIdx, azDeg, elDeg, dist);
    }
    else if (property == "/y" && message.size() >= 1 && message[0].isFloat32())
    {
        oscCartesianY[objIdx] = message[0].getFloat32();
        float x = oscCartesianX[objIdx], y = oscCartesianY[objIdx], z = oscCartesianZ[objIdx];
        float azDeg = std::atan2 (-x, y) * (180.0f / juce::MathConstants<float>::pi);
        float r = std::sqrt (x * x + y * y);
        float elDeg = std::atan2 (z, r) * (180.0f / juce::MathConstants<float>::pi);
        float dist = juce::jlimit (0.0f, 1.0f, std::sqrt (x * x + y * y + z * z));
        handleOSCPosition (objIdx, azDeg, elDeg, dist);
    }
    else if (property == "/z" && message.size() >= 1 && message[0].isFloat32())
    {
        oscCartesianZ[objIdx] = message[0].getFloat32();
        float x = oscCartesianX[objIdx], y = oscCartesianY[objIdx], z = oscCartesianZ[objIdx];
        float azDeg = std::atan2 (-x, y) * (180.0f / juce::MathConstants<float>::pi);
        float r = std::sqrt (x * x + y * y);
        float elDeg = std::atan2 (z, r) * (180.0f / juce::MathConstants<float>::pi);
        float dist = juce::jlimit (0.0f, 1.0f, std::sqrt (x * x + y * y + z * z));
        handleOSCPosition (objIdx, azDeg, elDeg, dist);
    }
}

//==============================================================================
// Handle an OSC position update — write to APVTS via setValueNotifyingHost
//==============================================================================
void OpenSpatialDelayProcessor::handleOSCPosition (int objIdx, float azDeg, float elDeg, float dist)
{
    auto objStr = juce::String (objIdx + 1);

    // setValueNotifyingHost takes 0..1 normalized value; use convertTo0to1 for the parameter's range
    if (auto* azParam = apvts.getParameter ("object" + objStr + "_azimuth"))
    {
        float clampedAz = juce::jlimit (-180.0f, 180.0f, azDeg);
        azParam->setValueNotifyingHost (azParam->convertTo0to1 (clampedAz));
    }
    if (auto* elParam = apvts.getParameter ("object" + objStr + "_elevation"))
    {
        float clampedEl = juce::jlimit (-90.0f, 90.0f, elDeg);
        elParam->setValueNotifyingHost (elParam->convertTo0to1 (clampedEl));
    }
    if (auto* distParam = apvts.getParameter ("object" + objStr + "_distance"))
    {
        float clampedDist = juce::jlimit (0.0f, 1.0f, dist);
        distParam->setValueNotifyingHost (distParam->convertTo0to1 (clampedDist));
    }

    // Mark OSC override active (pauses trajectory for this object)
    oscOverrideActive[objIdx].store (true, std::memory_order_relaxed);
    oscLastReceiveTime[objIdx] = juce::Time::getMillisecondCounterHiRes();
}

//==============================================================================
// Trajectory shape computation — pure function, no side effects
//==============================================================================
OpenSpatialDelayProcessor::TrajectoryResult
OpenSpatialDelayProcessor::computeTrajectory (int shape, float phase,
                                               float baseAz, float baseEl, float baseDist)
{
    TrajectoryResult r;

    switch (shape)
    {
        case 1: // Spiral — azimuth sweeps 360°, elevation oscillates ±45°, distance pulses
            r.azDeg = baseAz + 360.0f * phase;
            r.elDeg = baseEl + 45.0f * std::sin (phase * juce::MathConstants<float>::twoPi);
            r.dist  = 0.3f + 0.7f * (0.5f + 0.5f * std::cos (phase * juce::MathConstants<float>::twoPi));
            break;

        case 2: // Orbit — circular orbit, elevation & distance stay at base
            r.azDeg = baseAz + 360.0f * phase;
            r.elDeg = baseEl;
            r.dist  = baseDist;
            break;

        case 3: // Bounce — azimuth ping-pongs ±90°, elevation bounces ±30°
        {
            // Triangle wave: 0→1→0 over one phase cycle
            float tri = 1.0f - std::abs (2.0f * phase - 1.0f);
            r.azDeg = baseAz + 90.0f * (2.0f * tri - 1.0f);
            r.elDeg = baseEl + 30.0f * (2.0f * tri - 1.0f);
            r.dist  = baseDist;
            break;
        }

        case 4: // Figure-8 — Lissajous pattern
            r.azDeg = baseAz + 90.0f  * std::sin (phase * juce::MathConstants<float>::twoPi);
            r.elDeg = baseEl + 45.0f  * std::sin (phase * juce::MathConstants<float>::twoPi * 2.0f);
            r.dist  = baseDist;
            break;

        case 5: // Random — smoothed random walk using sin-based pseudo-random
        {
            // Use deterministic oscillators at irrational frequency ratios for smooth "random" motion
            float p = phase * juce::MathConstants<float>::twoPi;
            r.azDeg = baseAz + 60.0f * std::sin (p * 1.0f) + 30.0f * std::sin (p * 2.7183f);
            r.elDeg = baseEl + 25.0f * std::sin (p * 1.4142f) + 15.0f * std::sin (p * 3.1416f);
            r.dist  = juce::jlimit (0.0f, 1.0f,
                                    baseDist + 0.3f * std::sin (p * 1.7321f) + 0.1f * std::sin (p * 2.2361f));
            break;
        }

        default: // None (0) or unknown
            r.azDeg = baseAz;
            r.elDeg = baseEl;
            r.dist  = baseDist;
            break;
    }

    // Wrap azimuth to -180..+180
    while (r.azDeg > 180.0f)  r.azDeg -= 360.0f;
    while (r.azDeg < -180.0f) r.azDeg += 360.0f;

    // Clamp elevation and distance
    r.elDeg = juce::jlimit (-90.0f, 90.0f, r.elDeg);
    r.dist  = juce::jlimit (0.0f, 1.0f, r.dist);

    return r;
}

// #############################################################################
// MIXED — State serialization & JUCE plugin factory
// State save/restore includes both spatial params and delay params.
// Format migration (v0.4→v0.5) is plugin-specific.
// #############################################################################

//==============================================================================
// State save / restore
//==============================================================================
void OpenSpatialDelayProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("pluginStateVersion", 10, nullptr);  // v0.6 state format (10 = per-object trajectory, preset system)
    state.setProperty ("oscReceivePort", oscReceivePort, nullptr);  // v0.6: persist OSC port
    state.setProperty ("currentPresetIndex", currentPresetIndex, nullptr);  // v0.6: persist preset selection
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OpenSpatialDelayProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState == nullptr || ! xmlState->hasTagName (apvts.state.getType()))
        return;

    auto tree = juce::ValueTree::fromXml (*xmlState);

    // v0.5: Migrate outputFormat parameter from v0.4 enum ordering to v0.5 ordering
    // v0.4 had 17 formats (Binaural=0..HOA=16), v0.5 has 22 formats (Stereo=0..HOA=21)
    int savedVersion = static_cast<int> (tree.getProperty ("pluginStateVersion", 0));
    if (savedVersion < 5)
    {
        // v0.4 index → v0.5 index mapping (17 old entries)
        static const int v4ToV5FormatMap[17] = {
            5,   // old 0 (Binaural)     → new 5
            6,   // old 1 (Quad)         → new 6
            8,   // old 2 (5.1)          → new 8
            11,  // old 3 (7.1)          → new 11
            16,  // old 4 (7.1.4)        → new 16
            18,  // old 5 (9.1.6)        → new 18
            12,  // old 6 (Oct)          → new 12
            7,   // old 7 (5.0)          → new 7
            9,   // old 8 (7.0)          → new 9
            10,  // old 9 (5.1.2)        → new 10
            14,  // old 10 (5.1.4)       → new 14
            13,  // old 11 (7.0.2)       → new 13
            15,  // old 12 (7.1.2)       → new 15
            17,  // old 13 (7.1.6)       → new 17
            19,  // old 14 (FOA)         → new 19
            20,  // old 15 (SOA)         → new 20
            21,  // old 16 (HOA)         → new 21
        };

        // Search for the outputFormat parameter in the state tree
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (child.hasProperty ("id")
                && child.getProperty ("id").toString() == "outputFormat")
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (17 items → indices 0..16)
                int oldIndex = juce::roundToInt (normalizedOld * 16.0f);
                oldIndex = juce::jlimit (0, 16, oldIndex);
                int newIndex = v4ToV5FormatMap[oldIndex];
                // Re-normalize using new item count (22 items → indices 0..21)
                float normalizedNew = static_cast<float> (newIndex) / 21.0f;
                child.setProperty ("value", normalizedNew, nullptr);
                break;
            }
        }

        tree.setProperty ("pluginStateVersion", 5, nullptr);
    }

    // v0.5 continued: Migrate from state version 5 to 6
    // outputFormat: 22 items (indices 0..21) → 25 items (indices 0..24)
    //   Indices 0-21 are identity (new formats appended at end), just re-normalize
    // algorithm: 5 items → 6 items (MDAP inserted at index 3)
    //   Old: [0=Ambi, 1=DBAP, 2=KNN, 3=VBAP, 4=VBIP]
    //   New: [0=Ambi, 1=DBAP, 2=KNN, 3=MDAP, 4=VBAP, 5=VBIP]
    if (savedVersion < 6)
    {
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            auto paramId = child.getProperty ("id").toString();

            if (paramId == "outputFormat")
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (22 items → indices 0..21)
                int oldIndex = juce::roundToInt (normalizedOld * 21.0f);
                oldIndex = juce::jlimit (0, 21, oldIndex);
                // Indices 0-21 map to themselves (new formats appended at 22,23,24)
                // Re-normalize using new item count (25 items → indices 0..24)
                float normalizedNew = static_cast<float> (oldIndex) / 24.0f;
                child.setProperty ("value", normalizedNew, nullptr);
            }
            else if (paramId == "algorithm")
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (5 items → indices 0..4)
                int oldIndex = juce::roundToInt (normalizedOld * 4.0f);
                oldIndex = juce::jlimit (0, 4, oldIndex);
                // Map: 0→0(Ambi), 1→1(DBAP), 2→2(KNN), 3→4(VBAP), 4→5(VBIP)
                static const int v5ToV6AlgoMap[5] = { 0, 1, 2, 4, 5 };
                int newIndex = v5ToV6AlgoMap[oldIndex];
                // Re-normalize using new item count (6 items → indices 0..5)
                float normalizedNew = static_cast<float> (newIndex) / 5.0f;
                child.setProperty ("value", normalizedNew, nullptr);
            }
        }

        tree.setProperty ("pluginStateVersion", 6, nullptr);
    }

    // v0.5 continued: Migrate from state version 6 to 7
    // outputFormat: 25 items (0..24) → 21 items (0..20)
    //   Old stereo variants 0-4 → new index 0 (single Stereo)
    //   Old 5-24 → new 1-20 (shift down by 4)
    // algorithm: 6 items → 11 items (5 stereo modes appended at indices 6-10)
    //   Surround algorithms 0-5 unchanged, just re-normalized
    //   If old format was a stereo variant (1-4), set algorithm to matching stereo mode
    if (savedVersion < 7)
    {
        // First pass: find outputFormat value to determine if stereo migration is needed
        int oldFormatIdx = -1;
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (child.hasProperty ("id")
                && child.getProperty ("id").toString() == "outputFormat")
            {
                float normOld = static_cast<float> (child.getProperty ("value", 0.0f));
                oldFormatIdx = juce::roundToInt (normOld * 24.0f);  // 25 items (0..24)
                oldFormatIdx = juce::jlimit (0, 24, oldFormatIdx);

                // Map: all stereo variants → 0, everything else shifts down by 4
                int newIdx = (oldFormatIdx <= 4) ? 0 : (oldFormatIdx - 4);
                float normNew = static_cast<float> (newIdx) / 20.0f;  // 21 items (0..20)
                child.setProperty ("value", normNew, nullptr);
                break;
            }
        }

        // Second pass: migrate algorithm parameter
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (child.hasProperty ("id")
                && child.getProperty ("id").toString() == "algorithm")
            {
                if (oldFormatIdx >= 1 && oldFormatIdx <= 4)
                {
                    // Old format was a stereo variant (VBAP/XY/MS/Blumlein)
                    // Map to matching stereo mode: old 1→7, 2→8, 3→9, 4→10
                    int stereoAlgoIdx = oldFormatIdx + 6;
                    float normNew = static_cast<float> (stereoAlgoIdx) / 10.0f;  // 11 items (0..10)
                    child.setProperty ("value", normNew, nullptr);
                }
                else if (oldFormatIdx == 0)
                {
                    // Old format was Stereo (equal power) → set algorithm to Equal Power (index 6)
                    child.setProperty ("value", 6.0f / 10.0f, nullptr);
                }
                else
                {
                    // Non-stereo format: surround algorithms 0-5 unchanged, just re-normalize
                    float normOld = static_cast<float> (child.getProperty ("value", 0.0f));
                    int oldAlgoIdx = juce::roundToInt (normOld * 5.0f);  // 6 items (0..5)
                    oldAlgoIdx = juce::jlimit (0, 5, oldAlgoIdx);
                    float normNew = static_cast<float> (oldAlgoIdx) / 10.0f;  // 11 items (0..10)
                    child.setProperty ("value", normNew, nullptr);
                }
                break;
            }
        }

        tree.setProperty ("pluginStateVersion", 7, nullptr);
    }

    // v0.5 continued: Migrate from state version 7 to 8
    // outputFormat: Binaural and Stereo swapped (old 0=Stereo,1=Binaural → new 0=Binaural,1=Stereo)
    // algorithm: unchanged (11 items, same indices), just re-normalize for version bump
    if (savedVersion < 8)
    {
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (child.hasProperty ("id")
                && child.getProperty ("id").toString() == "outputFormat")
            {
                float normOld = static_cast<float> (child.getProperty ("value", 0.0f));
                int oldIdx = juce::roundToInt (normOld * 20.0f);  // 21 items (0..20)
                oldIdx = juce::jlimit (0, 20, oldIdx);

                int newIdx;
                if (oldIdx == 0)       newIdx = 1;   // old Stereo(0) → new Stereo(1)
                else if (oldIdx == 1)  newIdx = 0;   // old Binaural(1) → new Binaural(0)
                else                   newIdx = oldIdx;  // 2-20 unchanged

                float normNew = static_cast<float> (newIdx) / 20.0f;
                child.setProperty ("value", normNew, nullptr);
                break;
            }
        }

        tree.setProperty ("pluginStateVersion", 8, nullptr);
    }

    // v0.6: Migrate from state version 9 to 10
    // Global trajectory params (trajectoryShape, trajectorySpeed) → per-object params
    // Replicate global values to all 12 objects, then remove global nodes
    if (savedVersion >= 9 && savedVersion < 10)
    {
        // Find global trajectory params in the state tree
        float globalShapeNorm = -1.0f;
        float globalSpeedNorm = -1.0f;
        int globalShapeChildIdx = -1;
        int globalSpeedChildIdx = -1;

        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            auto paramId = child.getProperty ("id").toString();
            if (paramId == "trajectoryShape")
            {
                globalShapeNorm = static_cast<float> (child.getProperty ("value", 0.0f));
                globalShapeChildIdx = i;
            }
            else if (paramId == "trajectorySpeed")
            {
                globalSpeedNorm = static_cast<float> (child.getProperty ("value", 0.0f));
                globalSpeedChildIdx = i;
            }
        }

        // Create per-object trajectory params from global values
        if (globalShapeNorm >= 0.0f || globalSpeedNorm >= 0.0f)
        {
            for (int obj = 0; obj < MAX_OBJECTS; ++obj)
            {
                auto prefix = "object" + juce::String (obj + 1) + "_";

                // Check if per-object params already exist (shouldn't, but be safe)
                bool shapeExists = false, speedExists = false;
                for (int i = 0; i < tree.getNumChildren(); ++i)
                {
                    auto child = tree.getChild (i);
                    if (child.hasProperty ("id"))
                    {
                        auto pid = child.getProperty ("id").toString();
                        if (pid == prefix + "trajectoryShape") shapeExists = true;
                        if (pid == prefix + "trajectorySpeed") speedExists = true;
                    }
                }

                if (! shapeExists && globalShapeNorm >= 0.0f)
                {
                    juce::ValueTree shapeNode ("PARAM");
                    shapeNode.setProperty ("id", prefix + "trajectoryShape", nullptr);
                    shapeNode.setProperty ("value", globalShapeNorm, nullptr);
                    tree.addChild (shapeNode, -1, nullptr);
                }

                if (! speedExists && globalSpeedNorm >= 0.0f)
                {
                    juce::ValueTree speedNode ("PARAM");
                    speedNode.setProperty ("id", prefix + "trajectorySpeed", nullptr);
                    speedNode.setProperty ("value", globalSpeedNorm, nullptr);
                    tree.addChild (speedNode, -1, nullptr);
                }
            }
        }

        // Remove global trajectory nodes (iterate in reverse to keep indices valid)
        if (globalSpeedChildIdx >= 0)
            tree.removeChild (globalSpeedChildIdx, nullptr);
        if (globalShapeChildIdx >= 0)
        {
            // Adjust index if speed was after shape and was already removed
            if (globalSpeedChildIdx >= 0 && globalSpeedChildIdx < globalShapeChildIdx)
                ; // speed was before shape, no adjustment needed for shape
            tree.removeChild (globalShapeChildIdx, nullptr);
        }

        tree.setProperty ("pluginStateVersion", 10, nullptr);
    }

    // v0.6: Restore OSC receive port (non-APVTS property)
    oscReceivePort = static_cast<int> (tree.getProperty ("oscReceivePort", 4002));

    // v0.6: Restore preset index (non-APVTS property)
    currentPresetIndex = static_cast<int> (tree.getProperty ("currentPresetIndex", 0));

    apvts.replaceState (tree);
}

//==============================================================================
juce::AudioProcessorEditor* OpenSpatialDelayProcessor::createEditor()
{
    return new OpenSpatialDelayEditor (*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpenSpatialDelayProcessor();
}
