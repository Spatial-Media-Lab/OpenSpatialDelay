#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "HRTFData.h"

// libmysofa — SOFA file reader
extern "C" {
#include "mysofa.h"
}

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
    //                                                              ch  LFE  height ambi  order
    { OutputFormat::Binaural,       "Binaural",         "Bin",    2, false, false, false, 0 },
    { OutputFormat::Quad,           "Quadraphonic",     "Quad",   4, false, false, false, 0 },
    { OutputFormat::Surround5_1,    "5.1 Surround",     "5.1",    6, true,  false, false, 0 },
    { OutputFormat::Surround7_1,    "7.1 Surround",     "7.1",    8, true,  false, false, 0 },
    { OutputFormat::Surround7_1_4,  "7.1.4 Atmos",      "7.1.4", 12, true,  true,  false, 0 },
    { OutputFormat::Surround9_1_6,  "9.1.6 Atmos",      "9.1.6", 16, true,  true,  false, 0 },
    { OutputFormat::Octaphonic,     "Octaphonic",       "Oct",    8, false, false, false, 0 },
    // v0.3: Additional surround formats
    { OutputFormat::Surround5_0,    "5.0 Surround",     "5.0",    5, false, false, false, 0 },
    { OutputFormat::Surround7_0,    "7.0 Surround",     "7.0",    7, false, false, false, 0 },
    { OutputFormat::Surround5_1_2,  "5.1.2 Atmos",      "5.1.2",  8, true,  true,  false, 0 },
    { OutputFormat::Surround5_1_4,  "5.1.4 Atmos",      "5.1.4", 10, true,  true,  false, 0 },
    { OutputFormat::Surround7_0_2,  "7.0.2",            "7.0.2",  9, false, true,  false, 0 },
    { OutputFormat::Surround7_1_2,  "7.1.2 Atmos",      "7.1.2", 10, true,  true,  false, 0 },
    { OutputFormat::Surround7_1_6,  "7.1.6 Atmos",      "7.1.6", 14, true,  true,  false, 0 },
    // v0.3: Ambisonics output (AmbiX ACN/SN3D encoding)
    { OutputFormat::AmbisonicsFOA,  "1st Order Ambi",   "FOA",    4, false, false, true,  1 },
    { OutputFormat::AmbisonicsSOA,  "2nd Order Ambi",   "SOA",    9, false, false, true,  2 },
    { OutputFormat::AmbisonicsHOA,  "3rd Order Ambi",   "HOA",   16, false, false, true,  3 },
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
    l.lfeChannelIndex = 3;   // JUCE 7.1.4: L R C LFE Lss Rss Lsr Rsr Tfl Tfr Trl Trr
    l.totalChannels = 12;
    // Ear level (7)
    l.speakers[0]  = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1]  = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2]  = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3]  = { degToRad( 90.0f), 0.0f, 4 };  // Lss
    l.speakers[4]  = { degToRad(-90.0f), 0.0f, 5 };  // Rss
    l.speakers[5]  = { degToRad(135.0f), 0.0f, 6 };  // Lsr
    l.speakers[6]  = { degToRad(-135.0f),0.0f, 7 };  // Rsr
    // Top (4)
    l.speakers[7]  = { degToRad( 45.0f), degToRad(45.0f), 8 };   // Tfl
    l.speakers[8]  = { degToRad(-45.0f), degToRad(45.0f), 9 };   // Tfr
    l.speakers[9]  = { degToRad(135.0f), degToRad(45.0f), 10 };  // Trl
    l.speakers[10] = { degToRad(-135.0f),degToRad(45.0f), 11 };  // Trr
    return l;
}

static SpeakerLayout make9_1_6Layout()
{
    SpeakerLayout l = {};
    l.numSpeakers = 15;
    l.lfeChannelIndex = 3;   // JUCE 9.1.6: L R C LFE Lss Rss Lsr Rsr Lw Rw Tfl Tfr Tsl Tsr Trl Trr
    l.totalChannels = 16;
    // Ear level (9)
    l.speakers[0]  = { degToRad( 30.0f), 0.0f, 0 };   // L
    l.speakers[1]  = { degToRad(-30.0f), 0.0f, 1 };   // R
    l.speakers[2]  = { degToRad(  0.0f), 0.0f, 2 };   // C
    l.speakers[3]  = { degToRad( 90.0f), 0.0f, 4 };   // Lss
    l.speakers[4]  = { degToRad(-90.0f), 0.0f, 5 };   // Rss
    l.speakers[5]  = { degToRad(135.0f), 0.0f, 6 };   // Lsr
    l.speakers[6]  = { degToRad(-135.0f),0.0f, 7 };   // Rsr
    l.speakers[7]  = { degToRad( 60.0f), 0.0f, 8 };   // Lw
    l.speakers[8]  = { degToRad(-60.0f), 0.0f, 9 };   // Rw
    // Top (6)
    l.speakers[9]  = { degToRad( 45.0f), degToRad(45.0f), 10 };  // Tfl
    l.speakers[10] = { degToRad(-45.0f), degToRad(45.0f), 11 };  // Tfr
    l.speakers[11] = { degToRad( 90.0f), degToRad(45.0f), 12 };  // Tsl
    l.speakers[12] = { degToRad(-90.0f), degToRad(45.0f), 13 };  // Tsr
    l.speakers[13] = { degToRad(135.0f), degToRad(45.0f), 14 };  // Trl
    l.speakers[14] = { degToRad(-135.0f),degToRad(45.0f), 15 };  // Trr
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
// Channel indices follow JUCE AudioChannelSet ordering (enum-value-sorted)

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
    l.lfeChannelIndex = 3;   // JUCE create7point1point6(): L R C LFE Lss Rss Tfl Tfr Trl Trr Lsr Rsr Tsl Tsr
    l.totalChannels = 14;
    // Ear level (7)
    l.speakers[0]  = { degToRad( 30.0f), 0.0f, 0 };  // L
    l.speakers[1]  = { degToRad(-30.0f), 0.0f, 1 };  // R
    l.speakers[2]  = { degToRad(  0.0f), 0.0f, 2 };  // C
    l.speakers[3]  = { degToRad( 90.0f), 0.0f, 4 };  // Lss
    l.speakers[4]  = { degToRad(-90.0f), 0.0f, 5 };  // Rss
    l.speakers[5]  = { degToRad(135.0f), 0.0f, 10 }; // Lsr
    l.speakers[6]  = { degToRad(-135.0f),0.0f, 11 }; // Rsr
    // Top (6)
    l.speakers[7]  = { degToRad( 45.0f), degToRad(45.0f), 6 };   // Tfl
    l.speakers[8]  = { degToRad(-45.0f), degToRad(45.0f), 7 };   // Tfr
    l.speakers[9]  = { degToRad(135.0f), degToRad(45.0f), 8 };   // Trl
    l.speakers[10] = { degToRad(-135.0f),degToRad(45.0f), 9 };   // Trr
    l.speakers[11] = { degToRad( 90.0f), degToRad(45.0f), 12 };  // Tsl
    l.speakers[12] = { degToRad(-90.0f), degToRad(45.0f), 13 };  // Tsr
    return l;
}

//==============================================================================
// Note division ratios (multiplied by beat duration to get delay in ms)
//==============================================================================
static const float noteDivisionRatios[] = {
    4.0f,       // 1/1 (whole note)
    2.0f,       // 1/2
    3.0f,       // 1/2 dotted
    4.0f/3.0f,  // 1/2 triplet
    1.0f,       // 1/4 (quarter note)
    1.5f,       // 1/4 dotted
    2.0f/3.0f,  // 1/4 triplet
    0.5f,       // 1/8
    0.75f,      // 1/8 dotted
    1.0f/3.0f,  // 1/8 triplet
    0.25f,      // 1/16
    0.375f,     // 1/16 dotted
    1.0f/6.0f,  // 1/16 triplet
};

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



    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("algorithm", 3), "Algorithm",
        juce::StringArray { "Ambisonics (HOA)", "KNN", "VBAP", "VBIP" }, 0));

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
            juce::ParameterID ("outputFormat", 3), "Output Format", formatNames, 0));
    }

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
    }

    return { params.begin(), params.end() };
}

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
            std::memcpy (out, in, sizeof (float) * numSamples);
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
                float re1 = fftWorkBuf[i],     im1 = fftWorkBuf[i + 1];
                float re2 = irFreqDomain[i],   im2 = irFreqDomain[i + 1];
                fftWorkBuf[i]     = re1 * re2 - im1 * im2;
                fftWorkBuf[i + 1] = re1 * im2 + im1 * re2;
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
    static const float refDirs[][2] = {
        { 0.0f, 0.0f },                                           // Front
        { (float) M_PI, 0.0f },                                   // Back
        { (float) (M_PI * 0.5), 0.0f },                           // Left
        { (float) (-M_PI * 0.5), 0.0f },                          // Right
        { 0.0f, (float) (M_PI * 0.25) },                          // Above-front
        { 0.0f, (float) (-M_PI * 0.25) }                          // Below-front
    };
    static constexpr int NUM_REF_DIRS = 6;

    std::vector<float> tmpIRL (irLen), tmpIRR (irLen);
    double totalEnergy = 0.0;

    for (int d = 0; d < NUM_REF_DIRS; ++d)
    {
        float delayL = 0.0f, delayR = 0.0f;
        hrtfDb.getInterpolatedHRIR (refDirs[d][0], refDirs[d][1],
                                     tmpIRL.data(), tmpIRR.data(), delayL, delayR);
        for (int n = 0; n < irLen; ++n)
        {
            totalEnergy += (double) tmpIRL[n] * tmpIRL[n];
            totalEnergy += (double) tmpIRR[n] * tmpIRR[n];
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
        tmpL.resize (storedIRLength);
        tmpR.resize (storedIRLength);
    }

    db.getInterpolatedHRIR (azRad, elRad, tmpL.data(), tmpR.data(), delayL, delayR);

    // Apply cross-profile normalization
    for (int n = 0; n < storedIRLength; ++n)
    {
        tmpL[n] *= storedNormGain;
        tmpR[n] *= storedNormGain;
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
    std::memset (outL, 0, sizeof (float) * numSamples);
    std::memset (outR, 0, sizeof (float) * numSamples);

    if (convTmpL.size() < static_cast<size_t> (numSamples))
    {
        convTmpL.resize (numSamples);
        convTmpR.resize (numSamples);
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
            outL[s] += convTmpL[s];
            outR[s] += convTmpR[s];
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
    int wantedProfile = targetHRTFProfile.load (std::memory_order_relaxed);

    if (wantedProfile == loadedHRTFProfileIndex)
        return;

    auto& renderer = binauralRenderers[prepareRendererIndex];

    // Prepare renderer on message thread (heavy: SOFA parse + FFT setup)
    renderer.prepare (currentSampleRate, static_cast<int> (monoInputBuffer.size()));
    loadHRTFProfileIntoRenderer (wantedProfile, renderer);

    // Atomic swap: audio thread now uses the newly loaded renderer
    activeRendererIndex.store (prepareRendererIndex, std::memory_order_release);
    prepareRendererIndex = 1 - prepareRendererIndex;
    loadedHRTFProfileIndex = wantedProfile;
}

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
    // 4 algorithms (alphabetical): Ambisonics (0), KNN (1), VBAP (2), VBIP (3)
    algorithms[0] = &algAmbisonics;
    algorithms[1] = &algKNN;
    algorithms[2] = &algVBAP;
    algorithms[3] = &algVBIP;
}

OpenSpatialDelayProcessor::~OpenSpatialDelayProcessor() {}

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

//==============================================================================
// v0.2: Activate speaker layout for the detected output format
//==============================================================================
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
        {
            // Ambisonics output: no speaker layout, just channel count
            int fmtIdx = static_cast<int> (format);
            buf.layout = {};
            buf.layout.numSpeakers = 0;
            buf.layout.lfeChannelIndex = -1;
            buf.layout.totalChannels = outputFormatRegistry[fmtIdx].requiredChannels;
            buf.ambiNumSpeakers = 0;
            buf.vbapTriplets.clear();
            activeLayoutIndex.store (prepareLayoutIndex, std::memory_order_release);
            prepareLayoutIndex = 1 - prepareLayoutIndex;
            return;
        }

        case OutputFormat::Binaural:
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
    // Binaural always works — renders to L/R within any bus ≥ 2ch
    if (requested == OutputFormat::Binaural)
        return OutputFormat::Binaural;

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
    auto requested = outputFormatRegistry[formatIndex].format;
    auto effective = resolveEffectiveFormat (requested, maxBusChannels);

    if (effective != getActiveLayout().format)
        activateLayout (effective);
}

//==============================================================================
void OpenSpatialDelayProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Allocate delay buffer (24 seconds max: 12 objects × 2s)
    delayBufferSize = static_cast<int> (sampleRate * MAX_DELAY_SECONDS) + 1;
    delayBuffer.assign (delayBufferSize, 0.0f);
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
    auto userFormat = outputFormatRegistry[juce::jlimit (0, NUM_OUTPUT_FORMATS - 1, userFormatIndex)].format;
    auto effectiveFormat = resolveEffectiveFormat (userFormat, maxBusChannels);
    activateLayout (effectiveFormat);

    // v0.2: Configure LFE low-pass filter (120 Hz, 2nd order Butterworth)
    lfeFilter.prepare (spec);
    lfeFilter.reset();
    *lfeFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 120.0f);

    // HRTF convolution: prepare both renderers (double-buffered)
    binauralRenderers[0].prepare (sampleRate, samplesPerBlock);
    binauralRenderers[1].prepare (sampleRate, samplesPerBlock);

    // v0.3: Pre-allocate per-source accumulation buffers for direct binaural
    for (int src = 0; src < MAX_OBJECTS; ++src)
        sourceAccumBufs[src].resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    wetBufL.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    wetBufR.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

    // Force HRTF profile reload at new sample rate
    loadedHRTFProfileIndex = -1;

    // v0.3: Start background timer for HRTF loading (~20Hz)
    startTimer (50);
}

void OpenSpatialDelayProcessor::releaseResources()
{
    delayBuffer.clear();
    monoInputBuffer.clear();
}

//==============================================================================
// Delay line helpers
//==============================================================================
void OpenSpatialDelayProcessor::writeDelayLine (float sample)
{
    delayBuffer[writePosition] = sample;
    writePosition = (writePosition + 1) % delayBufferSize;
}

float OpenSpatialDelayProcessor::readDelayLine (float delaySamples) const
{
    // readPos is floating point index relative to write head
    float readPos = static_cast<float> (writePosition) - delaySamples - 1.0f;
    
    // Handle wrap-around
    const float bufSize = static_cast<float>(delayBufferSize);
    while (readPos < 0.0f) readPos += bufSize;
    readPos = std::fmod(readPos, bufSize);

    int   i1 = static_cast<int> (readPos);
    float f  = readPos - static_cast<float>(i1);

    // Get 4 points for Cubic Hermite Interpolation
    int i0 = (i1 - 1 + delayBufferSize) % delayBufferSize;
    int i2 = (i1 + 1) % delayBufferSize;
    int i3 = (i1 + 2) % delayBufferSize;

    float y0 = delayBuffer[static_cast<size_t>(i0)];
    float y1 = delayBuffer[static_cast<size_t>(i1)];
    float y2 = delayBuffer[static_cast<size_t>(i2)];
    float y3 = delayBuffer[static_cast<size_t>(i3)];

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

    const float windowSamples = static_cast<float>(currentSampleRate) * 0.250f;
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

        auto ci = toCart (virtualSpeakers[tri.i].azimuthRad, virtualSpeakers[tri.i].elevationRad);
        auto cj = toCart (virtualSpeakers[tri.j].azimuthRad, virtualSpeakers[tri.j].elevationRad);
        auto ck = toCart (virtualSpeakers[tri.k].azimuthRad, virtualSpeakers[tri.k].elevationRad);

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
                                   float* outputGains, int numSpeakers) const
{
    if (! ctx.triplets.empty())
        computeVBAPGains3D (ctx.layout, ctx.triplets, source.azimuthRad, source.elevationRad, outputGains);
    else
        computeVBAPGains2D (ctx.layout, source.azimuthRad, outputGains);
}

//==============================================================================
// Modular 3D Audio Core — 3rd-Order Ambisonics (ACN/SN3D)
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

    switch (acn)
    {
        // Order 0
        case 0: return 1.0f;

        // Order 1 (SN3D: no extra factor needed)
        case 1: return sinAz * cosEl;              // Y1^-1
        case 2: return sinEl;                       // Y1^0
        case 3: return cosAz * cosEl;              // Y1^1

        // Order 2 (SN3D normalization)
        case 4: return std::sqrt (3.0f) * 0.5f * sin2Az * cosEl2;                     // Y2^-2 (SN3D: √3/2)
        case 5: return std::sqrt (3.0f) * sinAz * sinEl * cosEl;                      // Y2^-1 (SN3D: √3) [= √3 * sinAz * sin(2el)/2 simplified]
        case 6: return 0.5f * (3.0f * sinEl2 - 1.0f);                                 // Y2^0
        case 7: return std::sqrt (3.0f) * cosAz * sinEl * cosEl;                      // Y2^1
        case 8: return std::sqrt (3.0f) * 0.5f * cos2Az * cosEl2;                     // Y2^2 (SN3D: √3/2)

        // Order 3 (SN3D normalization)
        case  9: return std::sqrt (5.0f / 8.0f) * sin3Az * cosEl * cosEl2;            // Y3^-3
        case 10: return std::sqrt (15.0f) * 0.5f * sin2Az * sinEl * cosEl2;           // Y3^-2 (SN3D: √15/2)
        case 11: return std::sqrt (3.0f / 8.0f) * sinAz * cosEl * (5.0f * sinEl2 - 1.0f);  // Y3^-1
        case 12: return 0.5f * sinEl * (5.0f * sinEl2 - 3.0f);                        // Y3^0
        case 13: return std::sqrt (3.0f / 8.0f) * cosAz * cosEl * (5.0f * sinEl2 - 1.0f);  // Y3^1
        case 14: return std::sqrt (15.0f) * 0.5f * cos2Az * sinEl * cosEl2;           // Y3^2 (SN3D: √15/2)
        case 15: return std::sqrt (5.0f / 8.0f) * cos3Az * cosEl * cosEl2;            // Y3^3

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
            M[c][s] = evalSH (c, virtualSpeakers[s].azimuthRad,
                                  virtualSpeakers[s].elevationRad);

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
// Direct Ambisonics-to-Binaural: compute SH decode weights via sphere sampling
// Projects the ILD binaural model onto the SH basis using Fibonacci sphere
//==============================================================================
BinauralGains AmbisonicsAlgorithm::computeBinauralGains (
    const SourcePosition& source, const BinauralContext& ctx) const
{
    // v0.3: Ambisonics binaural is unused — binaural always uses direct HRTF or Woodworth.
    // Kept as a Woodworth-style fallback for interface completeness.
    const auto& profile = ctx.profiles[juce::jlimit (0, 4, ctx.profileIndex - 1)];

    float sinAz  = std::sin (source.azimuthRad);
    float cosEl  = std::cos (source.elevationRad);
    float lateral = sinAz * cosEl;

    float itdSeconds = (profile.headRadius / 343.0f)
                     * (std::abs (lateral) + std::asin (std::abs (lateral)));
    float itdSamples = itdSeconds * static_cast<float> (ctx.sampleRate);

    float ildDb = profile.ildScale * 8.0f * std::abs (lateral);
    float farEarGain = juce::Decibels::decibelsToGain (-ildDb);
    float distGain = 1.0f / std::max (0.1f, source.distance * 4.0f + 0.25f);

    BinauralGains gains;
    if (lateral >= 0.0f)
    {
        gains.leftGain  = distGain;
        gains.rightGain = distGain * farEarGain;
        gains.leftDelaySamples  = 0.0f;
        gains.rightDelaySamples = itdSamples;
    }
    else
    {
        gains.leftGain  = distGain * farEarGain;
        gains.rightGain = distGain;
        gains.leftDelaySamples  = itdSamples;
        gains.rightDelaySamples = 0.0f;
    }

    return gains;
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

    float bestGainSum = -1.0f;
    int bestTri = -1;
    float bestG[3] = {};

    for (int t = 0; t < static_cast<int> (triplets.size()); ++t)
    {
        const auto& tri = triplets[t];

        float g0 = tri.inv[0][0] * px + tri.inv[0][1] * py + tri.inv[0][2] * pz;
        float g1 = tri.inv[1][0] * px + tri.inv[1][1] * py + tri.inv[1][2] * pz;
        float g2 = tri.inv[2][0] * px + tri.inv[2][1] * py + tri.inv[2][2] * pz;

        if (g0 >= -1e-6f && g1 >= -1e-6f && g2 >= -1e-6f)
        {
            float sum = g0 + g1 + g2;
            if (sum > bestGainSum)
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

        outGains[triplets[bestTri].i] = bestG[0] * scale;
        outGains[triplets[bestTri].j] = bestG[1] * scale;
        outGains[triplets[bestTri].k] = bestG[2] * scale;
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
// v0.1 Output Renderer: pre-compute binaural gains for virtual speakers
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

    // --- Update feedback filters (block-rate) --------------------------------
    feedbackLPFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, lpFreq);
    feedbackHPFilter.coefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, hpFreq);

    // --- Read output layout (once per block for consistent snapshot) -----------
    const auto& layoutState = getActiveLayout();
    const auto& surLayout = layoutState.layout;
    bool isBinaural = (layoutState.format == OutputFormat::Binaural);
    bool isAmbiOutput = outputFormatRegistry[static_cast<int> (layoutState.format)].isAmbisonicsOutput;

    // --- Read algorithm selection (block-rate, surround only) -----------------
    int algorithmIndex = static_cast<int> (apvts.getRawParameterValue ("algorithm")->load());
    auto* algo = algorithms[juce::jlimit (0, NUM_ALGORITHMS - 1, algorithmIndex)];

    // For surround output, fall back if algorithm doesn't support speakers
    if (! isBinaural && ! algo->supportsSurround())
        algo = algorithms[2];  // Fall back to VBAP (index 2 in alphabetical order)

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
        objects[t] = getObjectState (t);

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

    // --- Prepare mono input --------------------------------------------------
    if (monoInputBuffer.size() < static_cast<size_t> (numSamples))
        monoInputBuffer.resize (static_cast<size_t> (numSamples));

    if (numInputChannels == 1)
    {
        auto* in = buffer.getReadPointer (0);
        for (int i = 0; i < numSamples; ++i)
            monoInputBuffer[i] = in[i];
    }
    else
    {
        auto* inL = buffer.getReadPointer (0);
        auto* inR = buffer.getReadPointer (1);
        for (int i = 0; i < numSamples; ++i)
            monoInputBuffer[i] = (inL[i] + inR[i]) * 0.5f;
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

    if (isBinaural && useHRTF)
    {
        // =====================================================================
        // v0.3: DIRECT BINAURAL PATH — per-source HRTF convolution
        // 3-pass architecture:
        //   Pass 1: Per-sample delay engine → per-source mono accumulation
        //   Pass 2: Per-block HRTF convolution (per source) → wet L/R
        //   Pass 3: Per-sample dry/wet mix + output gain
        // No algorithm dispatch — HRTF at exact source position IS the rendering
        // =====================================================================
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

        // Zero per-source accumulation buffers
        for (int src = 0; src < MAX_OBJECTS; ++src)
        {
            if (sourceAccumBufs[src].size() < static_cast<size_t> (numSamples))
                sourceAccumBufs[src].resize (numSamples, 0.0f);
            std::memset (sourceAccumBufs[src].data(), 0, sizeof (float) * numSamples);
        }

        // Ensure wet buffers are sized
        if (wetBufL.size() < static_cast<size_t> (numSamples))
        {
            wetBufL.resize (numSamples, 0.0f);
            wetBufR.resize (numSamples, 0.0f);
        }

        // Capture smoothed value start positions for Pass 3 interpolation
        float dwStart      = smoothedDryWet.getCurrentValue();
        float outGainStart  = smoothedOutputGain.getCurrentValue();

        // === PASS 1: Per-sample delay engine → per-source accumulation ===
        for (int s = 0; s < numSamples; ++s)
        {
            float currentDelayMs = smoothedDelayTime.getNextValue();
            float baseDelaySamples = currentDelayMs * 0.001f * static_cast<float> (currentSampleRate);
            float currentLoopMult = smoothedLoopMultiplier.getNextValue();

            float inGain  = smoothedInputGain.getNextValue();
            /* dw, fb, outGain */ smoothedDryWet.getNextValue();
            float fb      = smoothedFeedback.getNextValue();
            /* outGain */         smoothedOutputGain.getNextValue();

            float rawInput    = monoInputBuffer[s];
            float inputSample = rawInput * inGain;

            // STAGE 1: WRITE to delay line
            float delayInput = inputSample + feedbackSample * fb;
            delayInput = softClip (delayInput * 0.98f);
            writeDelayLine (delayInput);

            // STAGE 2: READ & ACCUMULATE (per-source mono × distance gain)
            for (int t = 0; t < MAX_OBJECTS; ++t)
            {
                if (! objects[t].enabled) continue;

                float objDelaySamples = static_cast<float>(t + 1) * baseDelaySamples;
                objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);
                float objPitch = static_cast<float> (t + 1) * pitchSemitones;

                float objMono = readPitchShifted (objDelaySamples, objPitch, t);
                sourceAccumBufs[t][s] = objMono * objDistGain[t];
            }

            // STAGE 3: FEEDBACK (mono, pre-spatial — unchanged)
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

        // === PASS 2: Per-block per-source HRTF convolution → wet L/R ===
        bool sourceEnabled[MAX_OBJECTS];
        for (int t = 0; t < MAX_OBJECTS; ++t)
            sourceEnabled[t] = objects[t].enabled;

        const float* srcBufPtrs[MAX_OBJECTS];
        for (int t = 0; t < MAX_OBJECTS; ++t)
            srcBufPtrs[t] = sourceAccumBufs[t].data();

        activeRenderer.renderSourceBuffers (srcBufPtrs, sourceEnabled, MAX_OBJECTS,
                                            numSamples, wetBufL.data(), wetBufR.data());

        // === PASS 3: Per-sample dry/wet mix + output gain ===
        float dwEnd      = smoothedDryWet.getCurrentValue();
        float outGainEnd  = smoothedOutputGain.getCurrentValue();
        float invN = (numSamples > 1) ? 1.0f / static_cast<float> (numSamples - 1) : 1.0f;

        for (int s = 0; s < numSamples; ++s)
        {
            float frac    = static_cast<float> (s) * invN;
            float dw      = dwStart + frac * (dwEnd - dwStart);
            float outGain = outGainStart + frac * (outGainEnd - outGainStart);
            float rawInput = monoInputBuffer[s];

            outL[s] = (rawInput * (1.0f - dw) + wetBufL[s] * dw) * outGain;
            outR[s] = (rawInput * (1.0f - dw) + wetBufR[s] * dw) * outGain;
        }

        // Zero remaining channels when binaural is selected on a multi-channel bus
        for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
            buffer.clear (ch, 0, numSamples);
    }
    else if (isBinaural)
    {
        // =====================================================================
        // SIMPLE (WOODWORTH) BINAURAL PATH — per-sample rendering
        // Used when profile = "Simple (Low CPU)" — no HRTF convolution
        // Always uses DirectBinauralAlgorithm (Woodworth ITD+ILD)
        // =====================================================================
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

            float rawInput    = monoInputBuffer[s];
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

                float objDelaySamples = static_cast<float>(t + 1) * baseDelaySamples;
                objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);
                float objPitch = static_cast<float> (t + 1) * pitchSemitones;

                float objMono = readPitchShifted (objDelaySamples, objPitch, t);

                // Woodworth binaural gains (pre-computed at block start)
                wetL += objMono * objGains[t].leftGain;
                wetR += objMono * objGains[t].rightGain;
            }

            // === STAGE 3: FEEDBACK ===
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

            // === STAGE 4: OUTPUT MIX ===
            outL[s] = (rawInput * (1.0f - dw) + wetL * dw) * outGain;
            outR[s] = (rawInput * (1.0f - dw) + wetR * dw) * outGain;
        }

        // Zero remaining channels when binaural is selected on a multi-channel bus
        for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
            buffer.clear (ch, 0, numSamples);
    }
    else if (isAmbiOutput)
    {
        // =====================================================================
        // AMBISONICS OUTPUT PATH — SH encode per source (v0.3)
        // Writes AmbiX (ACN/SN3D) coefficients directly to output channels
        // No algorithm dispatch — encoding is pure spherical harmonic evaluation
        // =====================================================================
        const int fmtIdx = static_cast<int> (layoutState.format);
        const int ambiOrder = outputFormatRegistry[fmtIdx].ambiOrder;
        const int numAmbiCh = (ambiOrder + 1) * (ambiOrder + 1);  // 4, 9, or 16

        // Max-rE weights per SH order for perceptual quality
        static const float maxrE[4] = {
            1.0f,
            std::cos (juce::MathConstants<float>::pi / 8.0f),
            std::cos (2.0f * juce::MathConstants<float>::pi / 8.0f),
            std::cos (3.0f * juce::MathConstants<float>::pi / 8.0f)
        };
        auto acnToOrder = [](int acn) -> int {
            if (acn < 1) return 0; if (acn < 4) return 1;
            if (acn < 9) return 2; return 3;
        };

        // Pre-compute SH coefficients per enabled tap (block-rate — positions fixed within block)
        float objSHCoeffs[MAX_OBJECTS][16] = {};
        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled) continue;
            float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
            float elRad = juce::degreesToRadians (objects[t].elevationDeg);
            for (int c = 0; c < numAmbiCh; ++c)
                objSHCoeffs[t][c] = evalSH (c, azRad, elRad) * maxrE[acnToOrder (c)];
        }

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

            float rawInput    = monoInputBuffer[s];
            float inputSample = rawInput * inGain;

            // === STAGE 1: WRITE ===
            float delayInput = inputSample + feedbackSample * fb;
            delayInput = softClip (delayInput * 0.98f);
            writeDelayLine (delayInput);

            // === STAGE 2: READ & SH ENCODE ===
            float ambiAccum[16] = {};

            for (int t = 0; t < MAX_OBJECTS; ++t)
            {
                if (! objects[t].enabled) continue;

                float objDelaySamples = static_cast<float>(t + 1) * baseDelaySamples;
                objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);
                float objPitch = static_cast<float> (t + 1) * pitchSemitones;

                float objMono = readPitchShifted (objDelaySamples, objPitch, t);
                float dist = objDistGain[t];

                for (int c = 0; c < numAmbiCh; ++c)
                    ambiAccum[c] += objMono * dist * objSHCoeffs[t][c];
            }

            // === STAGE 3: FEEDBACK ===
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
    else
    {
        // =====================================================================
        // DISCRETE SURROUND PATH (multi-channel output — v0.2)
        // =====================================================================
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

            float rawInput    = monoInputBuffer[s];
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

                float objDelaySamples = static_cast<float>(t + 1) * baseDelaySamples;
                objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);
                float objPitch = static_cast<float> (t + 1) * pitchSemitones;

                float objMono = readPitchShifted (objDelaySamples, objPitch, t);
                float dist = objDistGain[t];

                for (int sp = 0; sp < numSpeakers; ++sp)
                    channelAccum[sp] += objMono * dist * objChannelGains[t][sp];

                wetMono += objMono * dist;
            }

            // === STAGE 3: FEEDBACK ===
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

}

//==============================================================================
// State save / restore
//==============================================================================
void OpenSpatialDelayProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OpenSpatialDelayProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
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
