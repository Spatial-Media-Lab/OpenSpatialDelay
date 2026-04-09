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
    "Immersive",          // 1: SADIE II D2 KU100
    "Natural",            // 2: CIPIC Subject003
    "Precise",            // 3: HUTUBS PP2
    "Spatial",            // 4: Bernschuetz KU100
    "Studio Reference"    // 5: MIT KEMAR
};

//==============================================================================
// Binaural profiles: simplified head models for ITD + ILD rendering
// (Used by profile 0 "Simple" — Woodworth fallback)
//==============================================================================
const std::array<BinauralProfile, 5> OpenSpatialDelayProcessor::binauralProfiles = {{
    { 0.0920f, 1.3f, 1200.0f, "Immersive" },           // KU100-inspired (wider)
    { 0.0850f, 0.8f, 1800.0f, "Natural" },             // Human subject, subtler
    { 0.0900f, 1.1f, 1400.0f, "Precise" },             // Cross-validated, balanced
    { 0.0875f, 1.5f, 1100.0f, "Spatial" },             // High-res, exaggerated cues
    { 0.0875f, 1.0f, 1500.0f, "Studio Reference" },    // MIT KEMAR-inspired
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
    { OutputFormat::Surround7_1,    "7.1 Surround",     "7.1",    8, true,  false, false, 0, false },
    // --- 9.1 Surround (ITU-R BS.2051 System H — ear level only, no height) ---
    { OutputFormat::Surround9_1,    "9.1 Surround",     "9.1",   10, true,  false, false, 0, false },
    // --- Octaphonic ---
    { OutputFormat::Octaphonic,     "Octaphonic",       "Oct",    8, false, false, false, 0, false },
    // --- Atmos / Immersive (ascending channel count) ---
    { OutputFormat::Surround5_1_2,  "5.1.2 Atmos",      "5.1.2",  8, true,  true,  false, 0, false },
    { OutputFormat::Surround5_1_4,  "5.1.4 Atmos",      "5.1.4", 10, true,  true,  false, 0, false },
    { OutputFormat::Surround7_1_2,  "7.1.2 Atmos",      "7.1.2", 10, true,  true,  false, 0, false },
    { OutputFormat::Surround7_1_4,  "7.1.4 Atmos",      "7.1.4", 12, true,  true,  false, 0, false },
    { OutputFormat::Surround7_1_6,  "7.1.6 Atmos",      "7.1.6", 14, true,  true,  false, 0, false },
    { OutputFormat::Surround9_1_4,  "9.1.4 Atmos",      "9.1.4", 14, true,  true,  false, 0, false },
    { OutputFormat::Surround9_1_6,  "9.1.6 Atmos",      "9.1.6", 16, true,  true,  false, 0, false },
    // --- SML (Spatial Media Lab custom room) ---
    { OutputFormat::SurroundSML13_1,"SpatialMediaLab 13.1", "SML", 14, true,  true,  false, 0, false },
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

// --- SPATIAL MEDIA LIBRARY: Table-driven speaker layout definitions ----------
// ITU-R BS.775 / BS.2051 / SMPTE ST 2098-1 standard positions
// Convention: 0° = front, positive azimuth = left, negative = right
// LFE is tracked but excluded from spatialization
// Each entry: { azimuth°, elevation°, channelIndex }
//==============================================================================
struct SpeakerDef { float azDeg; float elDeg; int chIdx; };
struct LayoutDef { int numSpeakers; int lfeIdx; int totalChs; SpeakerDef speakers[16]; };

enum LayoutID { Quad, S5_0, S5_1, S7_0, S7_1, S9_1, S5_1_2, S5_1_4, S7_1_2, S7_1_4, S7_1_6, S9_1_4, S9_1_6, Octaphonic, SML13_1, NUM_LAYOUT_DEFS };

static const LayoutDef layoutDefs[NUM_LAYOUT_DEFS] = {
    // Quad (4.0) — symmetric 90° spacing
    { 4, -1, 4, {{ 45,0,0}, {-45,0,1}, { 135,0,2}, {-135,0,3}} },
    // 5.0
    { 5, -1, 5, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 110,0,3}, {-110,0,4}} },
    // 5.1 (LFE=ch3)
    { 5,  3, 6, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 110,0,4}, {-110,0,5}} },
    // 7.0
    { 7, -1, 7, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,3}, {-90,0,4}, { 135,0,5}, {-135,0,6}} },
    // 7.1 (LFE=ch3)
    { 7,  3, 8, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}} },
    // 9.1 (LFE=ch3) — ITU-R BS.2051 System H, 9 ear-level speakers + LFE, no height
    { 9, 3, 10, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 60,0,8}, {-60,0,9}} },
    // 5.1.2 (LFE=ch3)
    { 7,  3, 8, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 110,0,4}, {-110,0,5}, { 90,45,6}, {-90,45,7}} },
    // 5.1.4 (LFE=ch3)
    { 9,  3, 10, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 110,0,4}, {-110,0,5}, { 45,45,6}, {-45,45,7}, { 135,45,8}, {-135,45,9}} },
    // 7.1.2 (LFE=ch3)
    { 9,  3, 10, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 90,45,8}, {-90,45,9}} },
    // 7.1.4 (LFE=ch3)
    { 11, 3, 12, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 45,45,8}, {-45,45,9}, { 135,45,10}, {-135,45,11}} },
    // 7.1.6 (LFE=ch3)
    { 13, 3, 14, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 45,45,8}, {-45,45,9}, { 135,45,10}, {-135,45,11}, { 90,45,12}, {-90,45,13}} },
    // 9.1.4 (LFE=ch3)
    { 13, 3, 14, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 60,0,8}, {-60,0,9}, { 45,45,10}, {-45,45,11}, { 135,45,12}, {-135,45,13}} },
    // 9.1.6 (LFE=ch3)
    { 15, 3, 16, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 60,0,8}, {-60,0,9}, { 45,45,10}, {-45,45,11}, { 90,45,12}, {-90,45,13}, { 135,45,14}, {-135,45,15}} },
    // Octaphonic (8.0, no LFE) — "Center" configuration, 45° intervals
    { 8, -1, 8, {{0,0,0}, {-45,0,1}, {-90,0,2}, {-135,0,3}, { 180,0,4}, { 135,0,5}, { 90,0,6}, { 45,0,7}} },
    // SML 13.1 — Spatial Media Lab Multi-Use Room (13 speakers + LFE=ch13)
    // Ear level (8 at El=0°), Height (4 at El=45°), Zenith (1 at El=90°)
    // IEM AllRADecoder config — positive azimuth = left (matches OSD convention)
    { 13, 13, 14, {
        {   0,  0, 0},  // FC   — Front Center
        { -45,  0, 1},  // FR   — Front Right
        { -90,  0, 2},  // R    — Right
        {-135,  0, 3},  // RR   — Rear Right
        { 180,  0, 4},  // RC   — Rear Center
        { 135,  0, 5},  // RL   — Rear Left
        {  90,  0, 6},  // L    — Left
        {  45,  0, 7},  // FL   — Front Left
        { -45, 45, 8},  // UFR  — Upper Front Right
        {-135, 45, 9},  // URR  — Upper Rear Right
        { 135, 45,10},  // URL  — Upper Rear Left
        {  45, 45,11},  // UFL  — Upper Front Left
        {   0, 90,12},  // T    — Top (Zenith)
    }},
};

static SpeakerLayout makeLayoutFromDef (const LayoutDef& def)
{
    SpeakerLayout l = {};
    l.numSpeakers = def.numSpeakers;
    l.lfeChannelIndex = def.lfeIdx;
    l.totalChannels = def.totalChs;
    for (int i = 0; i < def.numSpeakers; ++i)
        l.speakers[i] = { degToRad (def.speakers[i].azDeg), degToRad (def.speakers[i].elDeg), def.speakers[i].chIdx };
    return l;
}


// #############################################################################
// MIXED — Parameter layout (spatial params are reusable; delay params are plugin-specific)
// Spatial: outputFormat, algorithm, hrtfProfile, per-object azimuth/elevation/distance
// Delay-specific: delayTime, feedback, filterLP/HP, dryWet, gains
// #############################################################################

//==============================================================================
// Parameter layout
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
    OpenSpatialDelayProcessor::createParameterLayout (bool abletonMode)
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Issue #189: Only limit automatable params to 64 when running in Ableton Live.
    // Other DAWs (REAPER, Logic, Pro Tools, Nuendo) have no such limit and should
    // see all 143 params as automatable. Mirrors the DAW-conditional bus layout
    // pattern in isBusesLayoutSupported() (issue #122).
    auto boolNonAuto = [&]() {
        return abletonMode ? juce::AudioParameterBoolAttributes().withAutomatable (false)
                           : juce::AudioParameterBoolAttributes();
    };
    auto floatNonAuto = [&]() {
        return abletonMode ? juce::AudioParameterFloatAttributes().withAutomatable (false)
                           : juce::AudioParameterFloatAttributes();
    };
    auto choiceNonAuto = [&]() {
        return abletonMode ? juce::AudioParameterChoiceAttributes().withAutomatable (false)
                           : juce::AudioParameterChoiceAttributes();
    };

    // Shared string-from-value formatters (DRY)
    auto fmtDbInf = [](float value, int) {
        if (value <= -99.9f)
            return juce::String (juce::CharPointer_UTF8 ("-\xe2\x88\x9e dB"));
        return juce::String (value, 1) + " dB";
    };
    auto parseDbInf = [](const juce::String& text) {
        if (text.containsChar (0x221E) || text.contains ("inf"))  // ∞
            return -100.0f;
        return text.getFloatValue();
    };
    auto fmtPct01 = [](float value, int) {
        return juce::String (juce::roundToInt (value * 100.0f)) + "%";
    };
    auto parsePct01 = [](const juce::String& text) {
        return text.getFloatValue() / 100.0f;
    };
    auto fmtPct100 = [](float value, int) {
        if (value < 10.0f) return juce::String (value, 1) + "%";
        return juce::String (juce::roundToInt (value)) + "%";
    };
    auto fmtFreq = [](float value, int) {
        if (value >= 1000.0f)
            return juce::String (value / 1000.0f, 1) + " kHz";
        return juce::String (juce::roundToInt (value)) + " Hz";
    };
    auto parseFreq = [](const juce::String& text) {
        if (text.containsIgnoreCase ("kHz") || text.containsIgnoreCase ("khz"))
            return text.getFloatValue() * 1000.0f;
        return text.getFloatValue();
    };
    auto fmtDeg = [](float value, int) {
        return juce::String (value, 1) + juce::String::charToString (0x00B0);
    };

    // --- Global parameters (issue #68: reordered, renamed, audited) ---------
    // Version hints: unique ascending per param for correct AU enumeration order.
    // Globals use 1–28, per-tap uses 100 + tapIndex*20 + offset. See issue #68.

    // G1: Delay Time
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("delayTime", 1), "Delay Time",
        juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.3f), 500.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction (
                [](float value, int) {
                    if (value >= 1000.0f)
                        return juce::String (value / 1000.0f, 2) + " s";
                    return juce::String (juce::roundToInt (value)) + " ms";
                })
            .withValueFromStringFunction (
                [](const juce::String& text) {
                    if (text.containsIgnoreCase ("s") && ! text.containsIgnoreCase ("ms"))
                        return text.getFloatValue() * 1000.0f;
                    return text.getFloatValue();
                })));

    // G2: Delay Sync (was "Tempo Sync")  [non-automatable in Ableton only: toggle state, issue #122 / #189]
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("tempoSync", 2), "Delay Sync", false,
        boolNonAuto()));

    // G3: Delay Division (was "Note Division")
    // Value = number of 16th notes. Range: 0.5 (1/32) to 32 (2/1)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("noteDivision", 3), "Delay Division",
        juce::NormalisableRange<float> (0.5f, 32.0f, 0.5f), 4.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                float sixteenths = value;
                if (std::abs (sixteenths - 32.0f)  < 0.01f) return juce::String ("2/1");
                if (std::abs (sixteenths - 16.0f)  < 0.01f) return juce::String ("1/1");
                if (std::abs (sixteenths - 8.0f)   < 0.01f) return juce::String ("1/2");
                if (std::abs (sixteenths - 4.0f)   < 0.01f) return juce::String ("1/4");
                if (std::abs (sixteenths - 2.0f)   < 0.01f) return juce::String ("1/8");
                if (std::abs (sixteenths - 1.0f)   < 0.01f) return juce::String ("1/16");
                if (std::abs (sixteenths - 0.5f)   < 0.01f) return juce::String ("1/32");
                if (sixteenths >= 1.0f)
                    return juce::String (juce::roundToInt (sixteenths)) + "/16";
                return juce::String (sixteenths, 1) + "/16";
            })));

    // G4: Sync Mode  [non-automatable in Ableton only: state selector, issue #122 / #189]
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("syncMode", 4), "Sync Mode",
        juce::StringArray { "Straight", "Dotted", "Triplet" }, 0,
        choiceNonAuto()));

    // G5: Feedback
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("feedback", 5), "Feedback",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.3f,
        juce::AudioParameterFloatAttributes().withLabel ("%")
            .withStringFromValueFunction (fmtPct01)
            .withValueFromStringFunction (parsePct01)));

    // G6: Filter On (was "Filter Enabled" — moved before filter params, enable→configure)
    // [non-automatable in Ableton only: toggle state, issue #122 / #189]
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterEnabled", 6), "Filter On",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
        floatNonAuto()));

    // G7: High-Pass Frequency (was "High-Pass Filter" — HP grouped before LP)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterHP", 7), "High-Pass Frequency",
        juce::NormalisableRange<float> (20.0f, 5000.0f, 1.0f, 0.3f), 50.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction (fmtFreq)
            .withValueFromStringFunction (parseFreq)));

    // G8: High-Pass Resonance (was "HP Resonance")
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterHPQ", 8), "High-Pass Resonance",
        juce::NormalisableRange<float> (0.5f, 8.0f, 0.01f, 0.4f), 0.707f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 2); })));

    // G9: Low-Pass Frequency (was "Low-Pass Filter")
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterLP", 9), "Low-Pass Frequency",
        juce::NormalisableRange<float> (200.0f, 20000.0f, 1.0f, 0.3f), 5000.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction (fmtFreq)
            .withValueFromStringFunction (parseFreq)));

    // G10: Low-Pass Resonance (was "LP Resonance")
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterLPQ", 10), "Low-Pass Resonance",
        juce::NormalisableRange<float> (0.5f, 8.0f, 0.01f, 0.4f), 0.707f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 2); })));

    // G11: Dry/Wet
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("dryWet", 11), "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f,
        juce::AudioParameterFloatAttributes().withLabel ("%")
            .withStringFromValueFunction (fmtPct01)
            .withValueFromStringFunction (parsePct01)));

    // G12: Input Gain (-100 dB shown as -∞ to +40 dB)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("inputGain", 12), "Input Gain",
        juce::NormalisableRange<float> (-100.0f, 40.0f, 0.1f, 3.0f), 0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction (fmtDbInf)
            .withValueFromStringFunction (parseDbInf)));

    // G13: Output Gain (-100 dB shown as -∞ to +12 dB)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("outputGain", 13), "Output Gain",
        juce::NormalisableRange<float> (-100.0f, 12.0f, 0.1f, 3.0f), 0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction (fmtDbInf)
            .withValueFromStringFunction (parseDbInf)));

    // Issue #68: Algorithm, HRTF Profile, Output Format, and Input Format removed from APVTS.
    // They are now stored as raw std::atomic<int> members (configAlgorithm, configHrtfProfile,
    // configOutputFormat, configInputFormat) to hide them from DAW automation lists.

    // G14: Air Absorption  [non-automatable in Ableton only: toggle state, issue #122 / #189]
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("airAbsorption", 14), "Air Absorption", false,
        boolNonAuto()));

    // G15: Wobble On (was "Wobble Enabled" — moved before wobble params, enable→configure)
    // [non-automatable in Ableton only: toggle state, issue #122 / #189]
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("wobbleEnabled", 15), "Wobble On", false,
        boolNonAuto()));

    // G16: Wobble Amount
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("wobbleAmount", 16), "Wobble Amount",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (fmtPct100)));

    // G17: Wobble Morph
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("wobbleMorph", 17), "Wobble Morph",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (fmtPct100)));

    // G18: OSC Receive moved to non-APVTS member (oscReceiveEnabled) — issue E20
    // Removing from APVTS keeps it out of the DAW undo stack.

    // G19–G24: Global Tap Offsets (promoted from OSC-only atomics to APVTS)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("globalTapAzimuth", 19), "Global Tap Azimuth",
        juce::NormalisableRange<float> (-180.0f, 180.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtDeg)));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("globalTapElevation", 20), "Global Tap Elevation",
        juce::NormalisableRange<float> (-90.0f, 90.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtDeg)));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("globalTapDistance", 21), "Global Tap Distance",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("globalTapPitch", 22), "Global Tap Pitch",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 1.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (juce::roundToInt (value)) + " st"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("globalTapDoppler", 23), "Global Tap Doppler",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f,
        floatNonAuto().withLabel ("%").withStringFromValueFunction (fmtPct100)));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("globalTapSpeed", 24), "Global Tap Speed",
        juce::NormalisableRange<float> (-5.0f, 5.0f, 0.01f), 0.0f,
        floatNonAuto().withLabel ("Hz").withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 2) + " Hz"; })));

    // --- Per-tap parameters (issue #68: "Object N" → "Tap 0N") ---------------
    // Version hints: 100 + tapIndex*20 + offset (Tap 01=100–109, Tap 02=120–129, …, Tap 12=320–329)
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        int tapBase = 100 + i * 20;
        auto id  = [&](const char* suffix, int offset) {
            return juce::ParameterID ("object" + juce::String (i + 1) + "_" + suffix, tapBase + offset);
        };
        auto name = [&](const char* suffix) {
            return "Tap " + juce::String (i + 1).paddedLeft ('0', 2) + " " + suffix;
        };

        bool defaultEnabled = (i < 4);
        // v0.9: Default azimuth is 0° (center front) so double-click resets to 0°.
        // Initial spatial spread is applied in constructor after APVTS creation.
        float defaultAz = 0.0f;

        // T1: Tap On (was "Object N Enabled")  [non-automatable in Ableton only: toggle state, issue #122 / #189]
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            id ("enabled", 0), name ("On"), defaultEnabled,
            boolNonAuto()));

        // T2: Per-tap Time — REMOVED (orphaned param, issue #68)

        // T3: Tap Azimuth
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("azimuth", 1), name ("Azimuth"),
            juce::NormalisableRange<float> (-180.0f, 180.0f, 0.1f), defaultAz,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtDeg)));

        // T4: Tap Elevation
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("elevation", 2), name ("Elevation"),
            juce::NormalisableRange<float> (-90.0f, 90.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtDeg)));

        // T5: Tap Distance
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("distance", 3), name ("Distance"),
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

        // T6: Tap Doppler Amount  [non-automatable in Ableton only: issue #122 / #189]
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("dopplerAmount", 4), name ("Doppler Amount"),
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f,
            floatNonAuto().withLabel ("%")
                .withStringFromValueFunction (fmtPct01)
                .withValueFromStringFunction (parsePct01)));

        // T7: Tap Pitch Shift
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("pitchShift", 5), name ("Pitch Shift"),
            juce::NormalisableRange<float> (-12.0f, 12.0f, 1.0f), 0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [](float value, int) { return juce::String (juce::roundToInt (value)) + " st"; })));

        // T8: Tap Trajectory Shape  [non-automatable in Ableton only: state selector, issue #122 / #189]
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            id ("trajectoryShape", 6), name ("Trajectory Shape"),
            juce::StringArray { "None", "Bounce", "Circle", "Cross", "Figure-8", "Heart", "Helix",
                                "Infinity", "Line", "Orbit", "Random", "Spiral", "Square", "Triangle" }, 0,
            choiceNonAuto()));

        // T9: Tap Trajectory Speed  [non-automatable in Ableton only: issue #122 / #189]
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("trajectorySpeed", 7), name ("Trajectory Speed"),
            juce::NormalisableRange<float> (0.0f, 5.0f, 0.01f), 0.3f,
            floatNonAuto().withLabel ("Hz").withStringFromValueFunction (
                [](float value, int) { return juce::String (value, 2) + " Hz"; })));

        // T10: Tap Trajectory Direction  [non-automatable in Ableton only: state toggle, issue #122 / #189]
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            id ("trajectoryDirection", 8), name ("Trajectory Direction"),
            juce::StringArray { "Forward", "Reverse" }, 0,
            choiceNonAuto()));

        // T11: Tap Input Channel  [non-automatable in Ableton only: state selector, issue #122 / #189]
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            id ("inputChannel", 9), name ("Input Channel"),
            juce::StringArray { "L+R", "L", "R" }, 0,
            choiceNonAuto()));
    }

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
      #if JUCE_DEBUG
        DBG ("HRTFDatabase: Failed to load SOFA data, error code: " + juce::String (err));
      #endif
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

  #if JUCE_DEBUG
    DBG ("HRTFDatabase: Loaded " + juce::String (numPositions) + " positions, "
         + "IR length = " + juce::String (irLength) + " samples");
  #endif

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

void HRTFDatabase::getAlignedHRIR (float azimuthRad, float elevationRad,
                                    float* irL, float* irR,
                                    float& delayL, float& delayR) const
{
    // First get the standard interpolated HRIR with embedded ITD
    getInterpolatedHRIR (azimuthRad, elevationRad, irL, irR, delayL, delayR);

    if (! loaded || irLength <= 0)
        return;

    // Remove ITD by shifting each HRIR backward by its delay (integer part).
    // The fractional part remains in delayL/delayR for the caller to apply
    // as a separate fractional-sample delay.
    //
    // This produces time-aligned HRIRs with coherent phase structure,
    // enabling smooth crossfading between neighboring positions without
    // comb-filtering from ITD misalignment.

    int shiftL = static_cast<int> (delayL);
    int shiftR = static_cast<int> (delayR);

    // v1.0.11 (issue #89): If SOFA reports zero delay for both channels,
    // the ITD is baked into the HRIR waveform (confirmed for MIT KEMAR,
    // likely CIPIC/HUTUBS/Bernschuetz). Detect onset from the waveform
    // itself so the dual-slot crossfade blends time-aligned HRIRs.
    // SADIE II KU100 reports non-zero delays and bypasses this fallback.
    // NOTE: Check raw float delays, not integer-truncated — SADIE reports
    // fractional delays (e.g., 0.3/0.7) that truncate to int 0 but are valid.
    if (delayL < 0.001f && delayR < 0.001f)
    {
        int onsetL = detectOnset (irL, irLength, 0.1f);
        int onsetR = detectOnset (irR, irLength, 0.1f);
        int minOnset = std::min (onsetL, onsetR);
        shiftL = onsetL - minOnset;
        shiftR = onsetR - minOnset;
        // Report detected ITD for the ITD delay line in renderSourceBuffers()
        delayL = static_cast<float> (onsetL);
        delayR = static_cast<float> (onsetR);
    }

    // Shift left channel: move samples backward by shiftL
    if (shiftL > 0 && shiftL < irLength)
    {
        for (int i = 0; i < irLength - shiftL; ++i)
            irL[i] = irL[i + shiftL];
        for (int i = irLength - shiftL; i < irLength; ++i)
            irL[i] = 0.0f;
    }

    // Shift right channel: move samples backward by shiftR
    if (shiftR > 0 && shiftR < irLength)
    {
        for (int i = 0; i < irLength - shiftR; ++i)
            irR[i] = irR[i + shiftR];
        for (int i = irLength - shiftR; i < irLength; ++i)
            irR[i] = 0.0f;
    }
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
// v1.0.4: Minimum-phase HRIR conversion via cepstral decomposition (issue #47).
// Converts a raw HRIR to its minimum-phase equivalent in-place.
// Preserves magnitude spectrum but removes excess phase, enabling smooth
// time-domain EMA interpolation between adjacent HRIRs without comb filtering.
//==============================================================================
void HRTFDatabase::convertToMinPhase (float* ir, int irLength, int fftOrder, float* workBuf)
{
    if (irLength <= 0 || fftOrder <= 0) return;

    const int N = 1 << fftOrder;  // FFT size (must be >= 2 * irLength)
    auto fftPtr = getSharedFFTCache().getOrCreate (fftOrder);

    // workBuf layout: N Complex<float> = N * 2 floats
    auto* cBuf = reinterpret_cast<std::complex<float>*> (workBuf);

    // Step 1: Copy IR into complex buffer, zero-pad
    for (int i = 0; i < N; ++i)
        cBuf[i] = (i < irLength) ? std::complex<float> (ir[i], 0.0f) : std::complex<float> (0.0f, 0.0f);

    // Step 2: Forward FFT
    fftPtr->perform (cBuf, cBuf, false);

    // Step 3: Compute log-magnitude (real cepstrum input)
    for (int k = 0; k < N; ++k)
    {
        float mag = std::abs (cBuf[k]);
        float logMag = std::log (std::max (mag, 1e-20f));  // epsilon for -ffast-math safety
        cBuf[k] = std::complex<float> (logMag, 0.0f);
    }

    // Step 4: IFFT to get real cepstrum
    fftPtr->perform (cBuf, cBuf, true);

    // Step 5: Apply minimum-phase window to cepstrum
    // c_mp[0] unchanged, c_mp[1..N/2-1] doubled, c_mp[N/2] unchanged, c_mp[N/2+1..N-1] zeroed
    int halfN = N / 2;
    for (int n = 1; n < halfN; ++n)
        cBuf[n] *= 2.0f;
    for (int n = halfN + 1; n < N; ++n)
        cBuf[n] = std::complex<float> (0.0f, 0.0f);

    // Step 6: Forward FFT
    fftPtr->perform (cBuf, cBuf, false);

    // Step 7: Exponentiate to get minimum-phase spectrum
    for (int k = 0; k < N; ++k)
    {
        float re = cBuf[k].real();
        float im = cBuf[k].imag();
        // Clamp to prevent exp() overflow under -ffast-math
        re = std::max (-80.0f, std::min (80.0f, re));
        float mag = std::exp (re);
        cBuf[k] = std::complex<float> (mag * std::cos (im), mag * std::sin (im));
    }

    // Step 8: IFFT to get minimum-phase IR
    fftPtr->perform (cBuf, cBuf, true);

    // Step 9: Copy first irLength samples back (real part only)
    for (int i = 0; i < irLength; ++i)
        ir[i] = cBuf[i].real();
}

int HRTFDatabase::detectOnset (const float* ir, int length, float thresholdFraction)
{
    if (ir == nullptr || length <= 0)
        return 0;

    // Find peak absolute value
    float peak = 0.0f;
    for (int i = 0; i < length; ++i)
    {
        float absVal = std::abs (ir[i]);
        if (absVal > peak)
            peak = absVal;
    }

    if (peak < 1e-20f)
        return 0;  // Silent IR

    // Scan forward for first sample exceeding threshold * peak
    float thresh = thresholdFraction * peak;
    for (int i = 0; i < length; ++i)
    {
        if (std::abs (ir[i]) >= thresh)
        {
            // Clamp: if onset is past halfway, IR has no clear leading edge
            if (i > length / 2)
                return 0;
            return i;
        }
    }

    return 0;
}

void HRTFDatabase::correctLowFrequency (float* ir, int irLength, int fftOrder,
                                         float sampleRate, float* workBuf,
                                         float lfCutoffHz, float hfCutoffHz)
{
    if (ir == nullptr || irLength <= 0 || fftOrder <= 0)
        return;

    const int N = 1 << fftOrder;
    juce::dsp::FFT fft (fftOrder);

    auto* cBuf = reinterpret_cast<std::complex<float>*> (workBuf);

    // Step 1: Copy IR into complex buffer, zero-pad
    for (int i = 0; i < N; ++i)
        cBuf[i] = (i < irLength) ? std::complex<float> (ir[i], 0.0f)
                                 : std::complex<float> (0.0f, 0.0f);

    // Step 2: Forward FFT
    fft.perform (cBuf, cBuf, false);

    // Step 3: Identify frequency bin ranges
    float binHz = sampleRate / static_cast<float> (N);
    int lfBin = static_cast<int> (lfCutoffHz / binHz);
    int hfBin = static_cast<int> (hfCutoffHz / binHz);

    if (lfBin < 1) lfBin = 1;
    if (hfBin <= lfBin) hfBin = lfBin + 1;
    if (hfBin > N / 2) hfBin = N / 2;

    // Step 4: Compute mean magnitude in the reference range (100-300 Hz)
    float sumMag = 0.0f;
    int magCount = 0;
    for (int k = lfBin; k < hfBin; ++k)
    {
        sumMag += std::abs (cBuf[k]);
        ++magCount;
    }
    float meanMag = (magCount > 0) ? sumMag / static_cast<float> (magCount) : 0.0f;

    // Step 5: Magnitude-only correction below lfCutoff.
    // Scale each bin's magnitude up to meanMag while preserving its original phase.
    // This avoids the phase discontinuities between adjacent positions that caused
    // glitches when full phase extrapolation was used (same issue as min-phase, #47).
    for (int k = 0; k < lfBin; ++k)
    {
        float currentMag = std::abs (cBuf[k]);
        if (currentMag < 1e-20f)
        {
            // Bin has essentially zero energy — set to meanMag with zero phase
            cBuf[k] = std::complex<float> (meanMag, 0.0f);
        }
        else
        {
            // Scale magnitude up to meanMag, preserve original phase
            float scale = meanMag / currentMag;
            cBuf[k] *= scale;
        }

        // Mirror to negative frequencies (conjugate symmetry for real output)
        if (k > 0 && (N - k) < N)
            cBuf[N - k] = std::conj (cBuf[k]);
    }

    // Step 6: Inverse FFT
    fft.perform (cBuf, cBuf, true);

    // Step 7: Copy back to IR (real part only, original length)
    for (int i = 0; i < irLength; ++i)
        ir[i] = cBuf[i].real();
}

//==============================================================================
// PartitionedConvolver implementation — overlap-save FFT convolution
//==============================================================================

// Process-global FFT cache (issue #131).  Apple's vDSP shares internal
// twiddle factor memory across FFT setups of the same order.  Destroying the
// last setup of a given order frees the shared table even while another
// thread's vDSP_fft_zrip is reading from it.  The cache creates each order
// once and holds a permanent shared_ptr so the vDSP twiddle tables are never
// freed while the process is alive.  Individual convolvers also hold
// shared_ptrs, providing redundant safety.
std::shared_ptr<juce::dsp::FFT> SharedFFTCache::getOrCreate (int fftOrder)
{
    juce::SpinLock::ScopedLockType sl (lock);
    auto it = cache.find (fftOrder);
    if (it != cache.end())
        return it->second;
    auto ptr = std::make_shared<juce::dsp::FFT> (fftOrder);
    cache[fftOrder] = ptr;
    return ptr;
}

SharedFFTCache& getSharedFFTCache()
{
    static SharedFFTCache instance;
    return instance;
}

void PartitionedConvolver::prepare (int maxBlockSize, int irLength_)
{
    irLen = irLength_;
    blockSize = maxBlockSize;

    // FFT size must be >= blockSize + irLen - 1 (linear convolution length)
    // Round up to next power of 2
    int minFFTSize = blockSize + irLen - 1;
    int newFftOrder = 1;
    while ((1 << newFftOrder) < minFFTSize)
        ++newFftOrder;

    bool orderChanged = (newFftOrder != fftOrder) || fft == nullptr;
    fftOrder = newFftOrder;
    fftSize = 1 << fftOrder;

    if (orderChanged)
        fft = getSharedFFTCache().getOrCreate (fftOrder);

    // v1.0.5: Allocate dual convolution slots (issue #50)
    for (int s = 0; s < 2; ++s)
    {
        slots[s].irFreqDomain.assign (static_cast<size_t> (fftSize * 2), 0.0f);
        slots[s].inputAccum.resize (static_cast<size_t> (fftSize * 2), 0.0f);
        slots[s].fftWorkBuf.resize (static_cast<size_t> (fftSize * 2), 0.0f);
        slots[s].overlapBuf.resize (static_cast<size_t> (fftSize), 0.0f);
        slots[s].inputAccumPos = 0;
    }

    // Work buffers for dual-slot output mixing
    slotOutputA.resize (static_cast<size_t> (maxBlockSize), 0.0f);
    slotOutputB.resize (static_cast<size_t> (maxBlockSize), 0.0f);

    // Pending IR buffer (deferred updates during transitions)
    pendingIR.resize (static_cast<size_t> (irLen), 0.0f);
    pendingIRLen = 0;
    hasPendingIR = false;

    reset();
}

void PartitionedConvolver::loadIRIntoSlot (ConvSlot& slot, const float* ir, int length)
{
    std::fill (slot.irFreqDomain.begin(), slot.irFreqDomain.end(), 0.0f);
    for (int i = 0; i < std::min (length, fftSize); ++i)
        slot.irFreqDomain[static_cast<size_t> (i)] = ir[i];
    fft->performRealOnlyForwardTransform (slot.irFreqDomain.data(), true);
}

void PartitionedConvolver::resetSlot (ConvSlot& slot)
{
    std::fill (slot.inputAccum.begin(), slot.inputAccum.end(), 0.0f);
    std::fill (slot.overlapBuf.begin(), slot.overlapBuf.end(), 0.0f);
    std::fill (slot.fftWorkBuf.begin(), slot.fftWorkBuf.end(), 0.0f);
    slot.inputAccumPos = 0;
}

void PartitionedConvolver::setIR (const float* ir, int length)
{
    if (fftSize == 0) return;

    irLen = length;

    // v1.0.5: Dual-convolver IR loading strategy (issue #50).
    // First IR ever: load directly into the active slot, no crossfade.
    if (slots[static_cast<size_t> (activeSlot)].irFreqDomain.empty() ||
        std::all_of (slots[static_cast<size_t> (activeSlot)].irFreqDomain.begin(),
                     slots[static_cast<size_t> (activeSlot)].irFreqDomain.end(),
                     [] (float v) { return v == 0.0f; }))
    {
        loadIRIntoSlot (slots[static_cast<size_t> (activeSlot)], ir, length);
        return;
    }

    // Mid-transition (Warmup or Crossfading): defer to pendingIR
    if (state != State::Idle)
    {
        for (int i = 0; i < std::min (length, static_cast<int> (pendingIR.size())); ++i)
            pendingIR[static_cast<size_t> (i)] = ir[i];
        pendingIRLen = length;
        hasPendingIR = true;
        return;
    }

    // Idle: load new IR into inactive slot, enter Warmup
    int inactiveSlot = 1 - activeSlot;
    resetSlot (slots[static_cast<size_t> (inactiveSlot)]);
    // Sync input accumulator position so both slots process in lockstep
    slots[static_cast<size_t> (inactiveSlot)].inputAccumPos =
        slots[static_cast<size_t> (activeSlot)].inputAccumPos;
    loadIRIntoSlot (slots[static_cast<size_t> (inactiveSlot)], ir, length);

    state = State::Warmup;
    stateBlockCount = 0;
}

void PartitionedConvolver::processSlot (ConvSlot& slot, const float* in, float* out, int numSamples)
{
    // Standard overlap-save convolution on a single ConvSlot
    int samplesProcessed = 0;

    while (samplesProcessed < numSamples)
    {
        int spaceInAccum = blockSize - slot.inputAccumPos;
        int samplesToAccum = std::min (spaceInAccum, numSamples - samplesProcessed);

        for (int i = 0; i < samplesToAccum; ++i)
            slot.inputAccum[static_cast<size_t> (slot.inputAccumPos + i)] = in[samplesProcessed + i];

        slot.inputAccumPos += samplesToAccum;
        samplesProcessed += samplesToAccum;

        if (slot.inputAccumPos >= blockSize)
        {
            // Copy input to work buffer, zero-pad to fftSize
            std::fill (slot.fftWorkBuf.begin(), slot.fftWorkBuf.end(), 0.0f);
            for (int i = 0; i < blockSize; ++i)
                slot.fftWorkBuf[static_cast<size_t> (i)] = slot.inputAccum[static_cast<size_t> (i)];

            // Forward FFT of input
            fft->performRealOnlyForwardTransform (slot.fftWorkBuf.data(), true);

            // Complex multiply with slot's IR spectrum
            for (int i = 0; i < fftSize * 2; i += 2)
            {
                float re1 = slot.fftWorkBuf[static_cast<size_t> (i)],     im1 = slot.fftWorkBuf[static_cast<size_t> (i + 1)];
                float re2 = slot.irFreqDomain[static_cast<size_t> (i)],   im2 = slot.irFreqDomain[static_cast<size_t> (i + 1)];
                slot.fftWorkBuf[static_cast<size_t> (i)]     = re1 * re2 - im1 * im2;
                slot.fftWorkBuf[static_cast<size_t> (i + 1)] = re1 * im2 + im1 * re2;
            }

            fft->performRealOnlyInverseTransform (slot.fftWorkBuf.data());

            int outStart = samplesProcessed - blockSize;
            int outSamples = std::min (blockSize, numSamples - outStart);

            for (int i = 0; i < outSamples; ++i)
            {
                out[outStart + i] = slot.fftWorkBuf[static_cast<size_t> (i)]
                                  + slot.overlapBuf[static_cast<size_t> (i)];
            }

            // Save overlap for next block
            int overlapLen = fftSize - blockSize;
            for (int i = 0; i < overlapLen; ++i)
                slot.overlapBuf[static_cast<size_t> (i)] = slot.fftWorkBuf[static_cast<size_t> (blockSize + i)];
            for (int i = overlapLen; i < fftSize; ++i)
                slot.overlapBuf[static_cast<size_t> (i)] = 0.0f;

            slot.inputAccumPos = 0;
        }
    }
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

    // v1.0.5: Dual-convolver state machine (issue #50).
    // Three states: Idle (single slot), Warmup (both process, output only active),
    // Crossfading (equal-power cos/sin blend with per-sample gain interpolation).
    switch (state)
    {
        case State::Idle:
        {
            // Only the active slot processes
            processSlot (slots[static_cast<size_t> (activeSlot)], in, out, numSamples);
            break;
        }

        case State::Warmup:
        {
            // Both slots process, but output only from active slot.
            // This lets the inactive slot build up its overlap buffer.
            processSlot (slots[static_cast<size_t> (activeSlot)], in, out, numSamples);
            processSlot (slots[static_cast<size_t> (1 - activeSlot)], in, slotOutputB.data(), numSamples);

            ++stateBlockCount;
            if (stateBlockCount >= kWarmupBlocks)
            {
                state = State::Crossfading;
                stateBlockCount = 0;
                // Initialize crossfade gains
                prevFadeOutGain = 1.0f;
                prevFadeInGain  = 0.0f;
            }
            break;
        }

        case State::Crossfading:
        {
            // Both slots process into separate buffers
            processSlot (slots[static_cast<size_t> (activeSlot)], in, slotOutputA.data(), numSamples);
            processSlot (slots[static_cast<size_t> (1 - activeSlot)], in, slotOutputB.data(), numSamples);

            ++stateBlockCount;

            // Compute equal-power crossfade gains for this block's END
            float progress = static_cast<float> (stateBlockCount) / static_cast<float> (kCrossfadeBlocks);
            if (progress > 1.0f) progress = 1.0f;

            constexpr float halfPi = juce::MathConstants<float>::halfPi;
            fadeOutGain = std::cos (progress * halfPi);   // 1 → 0
            fadeInGain  = std::sin (progress * halfPi);   // 0 → 1

            // Per-sample linear interpolation between previous and current gains
            float fadeOutInc = (fadeOutGain - prevFadeOutGain) / static_cast<float> (numSamples);
            float fadeInInc  = (fadeInGain  - prevFadeInGain)  / static_cast<float> (numSamples);

            float gOut = prevFadeOutGain;
            float gIn  = prevFadeInGain;

            for (int i = 0; i < numSamples; ++i)
            {
                gOut += fadeOutInc;
                gIn  += fadeInInc;
                out[i] = slotOutputA[static_cast<size_t> (i)] * gOut
                       + slotOutputB[static_cast<size_t> (i)] * gIn;
            }

            prevFadeOutGain = fadeOutGain;
            prevFadeInGain  = fadeInGain;

            // Check if crossfade is complete
            if (stateBlockCount >= kCrossfadeBlocks)
            {
                // Swap active slot to the new one
                activeSlot = 1 - activeSlot;
                state = State::Idle;
                stateBlockCount = 0;
                fadeOutGain = 1.0f;
                fadeInGain  = 0.0f;
                prevFadeOutGain = 1.0f;
                prevFadeInGain  = 0.0f;

                // Apply any pending IR that arrived during the transition
                if (hasPendingIR)
                {
                    hasPendingIR = false;
                    setIR (pendingIR.data(), pendingIRLen);
                }
            }
            break;
        }
    }
}

void PartitionedConvolver::reset()
{
    for (int s = 0; s < 2; ++s)
        resetSlot (slots[s]);

    state = State::Idle;
    activeSlot = 0;
    stateBlockCount = 0;
    fadeOutGain = 1.0f;
    fadeInGain  = 0.0f;
    prevFadeOutGain = 1.0f;
    prevFadeInGain  = 0.0f;
    hasPendingIR = false;
    pendingIRLen = 0;
}

void PartitionedConvolver::clearAll()
{
    reset();
    // Zero frequency-domain IR data so the next setIR does a direct load
    // instead of crossfading from stale IR (used during preset transitions)
    for (int s = 0; s < 2; ++s)
        std::fill (slots[s].irFreqDomain.begin(), slots[s].irFreqDomain.end(), 0.0f);
}

//==============================================================================
// BinauralRenderer implementation — manages HRTF convolver banks
//==============================================================================
void BinauralRenderer::prepare (double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    currentBlockSize = maxBlockSize;

    // Pre-allocate to max(blockSize, 512) to cover typical IR lengths
    // and avoid audio-thread allocation in renderSourceBuffers / updateSourceHRIR.
    size_t preAllocSize = static_cast<size_t> (std::max (maxBlockSize, 512));
    convTmpL.resize (preAllocSize, 0.0f);
    convTmpR.resize (preAllocSize, 0.0f);
}

void BinauralRenderer::setProfile (int profileIndex)
{
    activeProfile = profileIndex;

    if (profileIndex == 0 || ! hrtfDatabase.isLoaded())
    {
        // Simple mode — no convolution needed
        storedNormGain = 1.0f;
        storedIRLength = 0;
        itdActive = false;
        for (int i = 0; i < MAX_SOURCES; ++i)
            sourceConvReady[i] = false;
        return;
    }

    // v1.0: Enable ITD-free HRIR mode for all SOFA profiles.
    // ITD is extracted and applied separately for smooth crossfading.
    itdActive = true;

    int irLen = hrtfDatabase.getIRLength();
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
        hrtfDatabase.getInterpolatedHRIR (refDirs[d][0], refDirs[d][1],
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

        // v1.0.4: Clear stale ITD state to prevent transient on profile switch
        // (issue #90). Without this, the renderer reuses ITD buffer data from
        // its previous profile, causing a volume swell when renderSourceBuffers
        // interpolates from stale currentITD to new targetITD.
        currentITDL[i] = 0.0f;
        currentITDR[i] = 0.0f;
        targetITDL[i] = 0.0f;
        targetITDR[i] = 0.0f;
        itdWritePos[i] = 0;
        std::memset (itdBufferL[i], 0, sizeof (itdBufferL[i]));
        std::memset (itdBufferR[i], 0, sizeof (itdBufferR[i]));
    }

    // v1.0.11 (issue #89, Phase 3): Low-shelf bass compensation for MIT KEMAR.
    // KEMAR has a 24 dB deficit at 50 Hz and 6 dB at 100 Hz (measurement limitation).
    // Design a low-shelf biquad filter to boost post-convolution output.
    // Applied per-source to the convolved signal — bass remains spatialized since
    // it amplifies whatever LF the HRTF captured at each direction.
    lfShelfActive = (profileIndex == 1);  // Only MIT KEMAR needs compensation
    if (lfShelfActive)
    {
        // Low-shelf: +12 dB at 200 Hz, Q=0.7 (gentle slope)
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            currentSampleRate, 200.0f, 0.7f, juce::Decibels::decibelsToGain (12.0f));
        lfShelfB[0] = coeffs->coefficients[0];
        lfShelfB[1] = coeffs->coefficients[1];
        lfShelfB[2] = coeffs->coefficients[2];
        lfShelfA[0] = 1.0f;  // a0 normalized
        lfShelfA[1] = coeffs->coefficients[3];
        lfShelfA[2] = coeffs->coefficients[4];

        // Clear filter state
        for (int i = 0; i < MAX_SOURCES; ++i)
        {
            lfShelfStateL[i][0] = lfShelfStateL[i][1] = 0.0f;
            lfShelfStateR[i][0] = lfShelfStateR[i][1] = 0.0f;
        }
    }

  #if JUCE_DEBUG
    DBG ("BinauralRenderer: Profile " + juce::String (profileIndex)
         + " loaded — IR=" + juce::String (irLen)
         + ", normGain=" + juce::String (storedNormGain, 4)
         + ", lfShelf=" + juce::String (lfShelfActive ? "ON" : "OFF")
         + " (per-source direct binaural)");
  #endif
}

void BinauralRenderer::updateSourceHRIR (int sourceIndex, float azRad, float elRad)
{
    if (sourceIndex < 0 || sourceIndex >= MAX_SOURCES || storedIRLength <= 0)
        return;

    // v1.0.11 (issue #89): Great-circle angular distance threshold replaces independent
    // azimuth/elevation comparison. The old check treated 1° of azimuth equally at equator
    // and pole, but at elevation 89° a 1° azimuth change is only ~0.017° of actual angular
    // movement on the sphere. This caused constant HRIR switching at poles (zenith stuck
    // centered, erratic nadir jumps). Great-circle distance naturally handles the pole
    // singularity — azimuth changes near ±90° elevation produce near-zero angular distance.
    constexpr float THRESHOLD = 0.017f;  // ~1 degree in radians
    if (sourceConvReady[sourceIndex])
    {
        float cachedAz = cachedSourceAz[sourceIndex];
        float cachedEl = cachedSourceEl[sourceIndex];
        float dot = std::cos (elRad) * std::cos (cachedEl) * std::cos (azRad - cachedAz)
                  + std::sin (elRad) * std::sin (cachedEl);
        dot = juce::jlimit (-1.0f, 1.0f, dot);
        float angularDist = std::acos (dot);
        if (angularDist < THRESHOLD)
            return;
    }

    // Query HRTF at exact source direction (realtime-safe: KD-tree lookup, no malloc)
    float delayL = 0.0f, delayR = 0.0f;
    std::vector<float>& tmpL = convTmpL;  // Reuse work buffer (safe: not in render path here)
    std::vector<float>& tmpR = convTmpR;

    // Ensure work buffers are large enough for IR
    if ((int) tmpL.size() < storedIRLength)
    {
        jassertfalse;  // Audio thread allocation — should have been pre-allocated in prepare()
        tmpL.resize (static_cast<size_t> (storedIRLength));
        tmpR.resize (static_cast<size_t> (storedIRLength));
    }

    // v1.0: Use ITD-free HRIRs for smooth crossfading (no comb-filtering from ITD misalignment)
    if (itdActive)
        hrtfDatabase.getAlignedHRIR (azRad, elRad, tmpL.data(), tmpR.data(), delayL, delayR);
    else
        hrtfDatabase.getInterpolatedHRIR (azRad, elRad, tmpL.data(), tmpR.data(), delayL, delayR);

    // Apply cross-profile normalization
    for (int n = 0; n < storedIRLength; ++n)
    {
        tmpL[static_cast<size_t> (n)] *= storedNormGain;
        tmpR[static_cast<size_t> (n)] *= storedNormGain;
    }

    // Load into convolver (realtime-safe: in-place FFT in pre-allocated buffers)
    sourceConvL[sourceIndex].setIR (tmpL.data(), storedIRLength);
    sourceConvR[sourceIndex].setIR (tmpR.data(), storedIRLength);

    // v1.0: Store ITD target for smooth per-sample interpolation in renderSourceBuffers
    if (itdActive)
    {
        targetITDL[sourceIndex] = delayL;
        targetITDR[sourceIndex] = delayR;
    }

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
        jassertfalse;  // Audio thread allocation — should have been pre-allocated in prepare()
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

        // v1.0.11 (issue #89, Phase 3): Low-shelf bass boost for KEMAR.
        // Amplifies the residual LF content in the convolved output. Bass stays
        // spatialized because the boost is applied to the per-direction HRTF output.
        if (lfShelfActive)
        {
            for (int s = 0; s < numSamples; ++s)
            {
                // Transposed Direct Form II biquad — left channel
                float xL = convTmpL[static_cast<size_t> (s)];
                float yL = lfShelfB[0] * xL + lfShelfStateL[src][0];
                lfShelfStateL[src][0] = lfShelfB[1] * xL - lfShelfA[1] * yL + lfShelfStateL[src][1];
                lfShelfStateL[src][1] = lfShelfB[2] * xL - lfShelfA[2] * yL;
                convTmpL[static_cast<size_t> (s)] = yL;

                // Right channel
                float xR = convTmpR[static_cast<size_t> (s)];
                float yR = lfShelfB[0] * xR + lfShelfStateR[src][0];
                lfShelfStateR[src][0] = lfShelfB[1] * xR - lfShelfA[1] * yR + lfShelfStateR[src][1];
                lfShelfStateR[src][1] = lfShelfB[2] * xR - lfShelfA[2] * yR;
                convTmpR[static_cast<size_t> (s)] = yR;
            }
        }

        // v1.0: Apply ITD as fractional-sample delay if using aligned HRIRs.
        // ITD is smoothly interpolated per-sample from currentITD to targetITD
        // to prevent timing discontinuities during rapid position changes.
        if (itdActive)
        {
            float itdL0 = currentITDL[src];
            float itdR0 = currentITDR[src];
            float itdL1 = targetITDL[src];
            float itdR1 = targetITDR[src];
            float invN = (numSamples > 1) ? 1.0f / static_cast<float> (numSamples - 1) : 1.0f;

            int wp = itdWritePos[src];

            for (int s = 0; s < numSamples; ++s)
            {
                float frac = static_cast<float> (s) * invN;
                float delL = itdL0 + frac * (itdL1 - itdL0);
                float delR = itdR0 + frac * (itdR1 - itdR0);

                // Write to circular ITD delay buffer
                itdBufferL[src][wp] = convTmpL[static_cast<size_t> (s)];
                itdBufferR[src][wp] = convTmpR[static_cast<size_t> (s)];

                // Read with fractional delay (linear interpolation)
                int idxL = static_cast<int> (delL);
                float fracL = delL - static_cast<float> (idxL);
                int rp0L = (wp - idxL + kITDBufferSize) & (kITDBufferSize - 1);
                int rp1L = (rp0L - 1 + kITDBufferSize) & (kITDBufferSize - 1);
                float sampleL = itdBufferL[src][rp0L] * (1.0f - fracL) + itdBufferL[src][rp1L] * fracL;

                int idxR = static_cast<int> (delR);
                float fracR = delR - static_cast<float> (idxR);
                int rp0R = (wp - idxR + kITDBufferSize) & (kITDBufferSize - 1);
                int rp1R = (rp0R - 1 + kITDBufferSize) & (kITDBufferSize - 1);
                float sampleR = itdBufferR[src][rp0R] * (1.0f - fracR) + itdBufferR[src][rp1R] * fracR;

                outL[s] += sampleL;
                outR[s] += sampleR;

                wp = (wp + 1) & (kITDBufferSize - 1);
            }

            itdWritePos[src] = wp;
            currentITDL[src] = itdL1;
            currentITDR[src] = itdR1;
        }
        else
        {
            // No ITD processing — direct sum
            for (int s = 0; s < numSamples; ++s)
            {
                outL[s] += convTmpL[static_cast<size_t> (s)];
                outR[s] += convTmpR[static_cast<size_t> (s)];
            }
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
        currentITDL[i] = 0.0f;
        currentITDR[i] = 0.0f;
        targetITDL[i] = 0.0f;
        targetITDR[i] = 0.0f;
        itdWritePos[i] = 0;
        std::memset (itdBufferL[i], 0, sizeof (itdBufferL[i]));
        std::memset (itdBufferR[i], 0, sizeof (itdBufferR[i]));
        lfShelfStateL[i][0] = lfShelfStateL[i][1] = 0.0f;
        lfShelfStateR[i][0] = lfShelfStateR[i][1] = 0.0f;
    }
}

void BinauralRenderer::invalidateSources()
{
    for (int i = 0; i < MAX_SOURCES; ++i)
    {
        sourceConvL[i].clearAll();
        sourceConvR[i].clearAll();
        sourceConvReady[i] = false;
        cachedSourceAz[i] = -999.0f;
        cachedSourceEl[i] = -999.0f;
        currentITDL[i] = 0.0f;
        currentITDR[i] = 0.0f;
        targetITDL[i] = 0.0f;
        targetITDR[i] = 0.0f;
        itdWritePos[i] = 0;
        std::memset (itdBufferL[i], 0, sizeof (itdBufferL[i]));
        std::memset (itdBufferR[i], 0, sizeof (itdBufferR[i]));
        lfShelfStateL[i][0] = lfShelfStateL[i][1] = 0.0f;
        lfShelfStateR[i][0] = lfShelfStateR[i][1] = 0.0f;
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
        renderer.hrtfDatabase.unload();
        renderer.setProfile (0);
      #if JUCE_DEBUG
        DBG ("HRTF: Switched to Simple (Woodworth) profile");
      #endif
        return;
    }

    const char* sofaData = nullptr;
    int sofaSize = 0;

    switch (profileIndex)
    {
        case 1:  sofaData = HRTFData::sadie_d2_ku100_sofa;
                 sofaSize = HRTFData::sadie_d2_ku100_sofaSize;         break;
        case 2:  sofaData = HRTFData::cipic_subject_003_sofa;
                 sofaSize = HRTFData::cipic_subject_003_sofaSize;      break;
        case 3:  sofaData = HRTFData::hutubs_pp2_sofa;
                 sofaSize = HRTFData::hutubs_pp2_sofaSize;             break;
        case 4:  sofaData = HRTFData::bernschuetz_ku100_sofa;
                 sofaSize = HRTFData::bernschuetz_ku100_sofaSize;      break;
        case 5:  sofaData = HRTFData::mit_kemar_large_pinna_sofa;
                 sofaSize = HRTFData::mit_kemar_large_pinna_sofaSize;  break;
        default:
          #if JUCE_DEBUG
            DBG ("HRTF: Invalid profile index " + juce::String (profileIndex));
          #endif
            return;
    }

    float sampleRate = static_cast<float> (currentSampleRate);
    bool success = renderer.hrtfDatabase.loadFromMemory (sofaData, sofaSize, sampleRate);

    if (success)
    {
        renderer.setProfile (profileIndex);
      #if JUCE_DEBUG
        DBG ("HRTF: Loaded profile " + juce::String (profileIndex)
             + " (" + juce::String (hrtfProfileNames[profileIndex]) + ")"
             + " — " + juce::String (renderer.hrtfDatabase.getNumPositions()) + " positions"
             + ", IR=" + juce::String (renderer.hrtfDatabase.getIRLength()) + " samples");
      #endif
    }
    else
    {
      #if JUCE_DEBUG
        DBG ("HRTF: Failed to load profile " + juce::String (profileIndex)
             + ", falling back to Simple");
      #endif
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
    if (wantedProfile != loadedHRTFProfileIndex
        && ! rendererXfadeActive_.load (std::memory_order_acquire))
    {
        auto& renderer = binauralRenderers[prepareRendererIndex];
        renderer.prepare (currentSampleRate, static_cast<int> (monoInputBuffer.size()));
        loadHRTFProfileIntoRenderer (wantedProfile, renderer);
        activeRendererIndex.store (prepareRendererIndex, std::memory_order_release);
        prepareRendererIndex = 1 - prepareRendererIndex;
        loadedHRTFProfileIndex = wantedProfile;
    }

    // --- v0.6: ADM-OSC connection management (edge-detect enable/disable) ---
    // oscReceiveEnabled is a plain member (not APVTS) so it stays out of the DAW undo stack (issue E20)
    if (oscReceiveEnabled && ! prevOscReceiveEnabled)
    {
        // Transition OFF→ON: connect
        oscConnected = oscReceiver.connect (oscReceivePort);
        if (oscConnected)
            oscReceiver.addListener (this);
    }
    else if (! oscReceiveEnabled && prevOscReceiveEnabled)
    {
        // Transition ON→OFF: disconnect
        oscReceiver.disconnect();
        oscReceiver.removeListener (this);
        oscConnected = false;
        // Clear all OSC overrides
        for (int t = 0; t < MAX_OBJECTS; ++t)
            oscOverrideActive[t].store (false, std::memory_order_relaxed);
    }
    prevOscReceiveEnabled = oscReceiveEnabled;

    // --- v0.6: OSC override timeout (500ms since last receive → release override) ---
    if (oscReceiveEnabled)
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

    // v1.0.2: Trajectory tick moved to processBlock (audio thread) — see issue #77.
    // Trajectory now advances in audio-time, fixing offline render speed mismatch
    // and eliminating 60Hz staircase buzz in Doppler velocity.

    // --- SPATIAL MEDIA LIBRARY: ADM-OSC Send — broadcast object positions at 30Hz ---
    if (oscSendEnabled && oscSendConnected && ++oscSendTickCounter >= 2)
    {
        oscSendTickCounter = 0;
        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (cachedObj[t].enabled == nullptr
                || cachedObj[t].enabled->load() < 0.5f)
                continue;

            // v0.9: Send animated position when trajectory active, origin otherwise
            float az, el, dist;
            if (trajectory.isActive (t))
            {
                az   = trajectory.getFinalAz (t);
                el   = trajectory.getFinalEl (t);
                dist = trajectory.getFinalDist (t);
            }
            else
            {
                az   = cachedObj[t].azimuth->load();
                el   = cachedObj[t].elevation->load();
                dist = cachedObj[t].distance->load();
            }

            // Position-change gating: only send when position actually moved
            if (std::abs (az - oscSendPrevAz[t]) < 0.1f
                && std::abs (el - oscSendPrevEl[t]) < 0.1f
                && std::abs (dist - oscSendPrevDist[t]) < 0.001f)
                continue;

            oscSendPrevAz[t]   = az;
            oscSendPrevEl[t]   = el;
            oscSendPrevDist[t] = dist;

            // ADM-OSC: /adm/obj/N/aed azimuth elevation distance
            juce::OSCMessage msg (oscSendAddress[t]);
            msg.addFloat32 (az);
            msg.addFloat32 (el);
            msg.addFloat32 (dist);
            oscSender.send (msg);
        }

        // v1.0: /osd/obj/N/ — per-object non-position params (change-gated)
        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            auto objStr = juce::String (t + 1);
            auto sendIfChanged = [&] (const char* prop, float current, float& prev) {
                if (std::abs (current - prev) > 0.001f)
                {
                    prev = current;
                    juce::OSCMessage m ("/osd/obj/" + objStr + "/" + prop);
                    m.addFloat32 (current);
                    oscSender.send (m);
                }
            };

            if (cachedObj[t].enabled != nullptr)
                sendIfChanged ("enabled", cachedObj[t].enabled->load(), oscSendPrevEnabled[t]);
            if (cachedObj[t].dopplerAmount != nullptr)
                sendIfChanged ("doppler", cachedObj[t].dopplerAmount->load(), oscSendPrevDoppler[t]);
            if (cachedObj[t].pitchShift != nullptr)
                sendIfChanged ("pitch", cachedObj[t].pitchShift->load(), oscSendPrevObjPitch[t]);
            if (cachedObj[t].inputChannel != nullptr)
                sendIfChanged ("input", cachedObj[t].inputChannel->load(), oscSendPrevInput[t]);
            if (cachedParam_trajectoryShape[t] != nullptr)
                sendIfChanged ("trajectory", cachedParam_trajectoryShape[t]->load(), oscSendPrevTrajShape[t]);
            if (cachedParam_trajectorySpeed[t] != nullptr)
                sendIfChanged ("speed", cachedParam_trajectorySpeed[t]->load(), oscSendPrevTrajSpeed[t]);
            if (cachedParam_trajectoryDirection[t] != nullptr)
                sendIfChanged ("direction", cachedParam_trajectoryDirection[t]->load(), oscSendPrevTrajDir[t]);
        }

        // v1.0: /osd/global/ — global params (change-gated)
        {
            auto& pg = oscSendPrevGlobal;
            auto sendGlobal = [&] (const char* prop, float current, float& prev) {
                if (std::abs (current - prev) > 0.001f)
                {
                    prev = current;
                    juce::OSCMessage m (juce::String ("/osd/global/") + prop);
                    m.addFloat32 (current);
                    oscSender.send (m);
                }
            };

            if (cachedParam_delayTime)      sendGlobal ("delaytime",     cachedParam_delayTime->load(),      pg.delayTime);
            if (cachedParam_tempoSync)      sendGlobal ("temposync",     cachedParam_tempoSync->load(),      pg.tempoSync);
            if (cachedParam_noteDivision)   sendGlobal ("notedivision",  cachedParam_noteDivision->load(),   pg.noteDivision);
            if (cachedParam_feedback)       sendGlobal ("feedback",      cachedParam_feedback->load(),       pg.feedback);
            if (cachedParam_filterLP)       sendGlobal ("filterlp",      cachedParam_filterLP->load(),       pg.filterLP);
            if (cachedParam_filterHP)       sendGlobal ("filterhp",      cachedParam_filterHP->load(),       pg.filterHP);
            if (cachedParam_filterLPQ)      sendGlobal ("filterlpq",     cachedParam_filterLPQ->load(),      pg.filterLPQ);
            if (cachedParam_filterHPQ)      sendGlobal ("filterhpq",     cachedParam_filterHPQ->load(),      pg.filterHPQ);
            if (cachedParam_filterEnabled)  sendGlobal ("filterenabled", cachedParam_filterEnabled->load(),  pg.filterEnabled);
            if (cachedParam_dryWet)         sendGlobal ("drywet",        cachedParam_dryWet->load(),         pg.dryWet);
            if (cachedParam_inputGain)      sendGlobal ("inputgain",     cachedParam_inputGain->load(),      pg.inputGain);
            if (cachedParam_outputGain)     sendGlobal ("outputgain",    cachedParam_outputGain->load(),     pg.outputGain);
            // Algorithm and HRTF profile are configuration-level — not sent via OSC.
            if (cachedParam_airAbsorption)  sendGlobal ("air",           cachedParam_airAbsorption->load(),  pg.airAbsorption);
            if (cachedParam_wobbleEnabled)  sendGlobal ("wobble",        cachedParam_wobbleEnabled->load(),  pg.wobbleEnabled);
            if (cachedParam_wobbleAmount)   sendGlobal ("wobbleamount",  cachedParam_wobbleAmount->load(),   pg.wobbleAmount);
            if (cachedParam_wobbleMorph)    sendGlobal ("wobblemorph",   cachedParam_wobbleMorph->load(),    pg.wobbleMorph);

            // syncMode and outputFormat use APVTS string lookup (no cached atomic pointer)
            if (auto* syncParam = apvts.getRawParameterValue ("syncMode"))
            {
                float v = syncParam->load();
                if (std::abs (v - pg.syncMode) > 0.001f)
                {
                    pg.syncMode = v;
                    juce::OSCMessage m ("/osd/global/syncmode");
                    m.addFloat32 (v);
                    oscSender.send (m);
                }
            }
            {
                float v = static_cast<float> (configOutputFormat.load (std::memory_order_relaxed));
                if (std::abs (v - pg.outputFormat) > 0.001f)
                {
                    pg.outputFormat = v;
                    juce::OSCMessage m ("/osd/global/outputformat");
                    m.addFloat32 (v);
                    oscSender.send (m);
                }
            }

            // issue #68: Global tap offsets now from APVTS (still mirrored in atomics for editor)
            static const char* tapOffsetProps[] = { "tapazimuth", "tapelevation", "tapdistance",
                                                    "tappitch",   "tapdoppler",  "tapspeed" };
            float* tapOffsetPrevs[] = { &pg.tapAzimuth, &pg.tapElevation, &pg.tapDistance,
                                        &pg.tapPitch,   &pg.tapDoppler,   &pg.tapSpeed };
            std::atomic<float>* tapOffsetCached[] = {
                cachedParam_globalTapAzimuth, cachedParam_globalTapElevation, cachedParam_globalTapDistance,
                cachedParam_globalTapPitch,   cachedParam_globalTapDoppler,   cachedParam_globalTapSpeed };
            for (int i = 0; i < kNumGlobalTapOffsets; ++i)
                if (tapOffsetCached[i])
                    sendGlobal (tapOffsetProps[i], tapOffsetCached[i]->load(), *tapOffsetPrevs[i]);
        }
    }

    // Issue #122: Debounced updateHostDisplay — at most once per 60Hz tick
    // instead of on every UI dropdown / OSC config change (fixes Ableton automation reset)
    if (configStateDirty.exchange (false, std::memory_order_relaxed))
        updateHostDisplay (ChangeDetails().withNonParameterStateChanged (true));
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
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::discreteChannels (50), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout (juce::PluginHostType().isAbletonLive()))
{
    // Initialize polymorphic algorithm pointer array (O(1) index lookup)
    // 7 algorithms (alphabetical): Ambisonics (0), ConstPower (1), DBAP (2), KNN (3), MDAP (4), VBAP (5), VBIP (6)
    algorithms[0] = &algAmbisonics;
    algorithms[1] = &algConstantPower;
    algorithms[2] = &algDBAP;
    algorithms[3] = &algKNN;
    algorithms[4] = &algMDAP;
    algorithms[5] = &algVBAP;
    algorithms[6] = &algVBIP;

    // Cache per-object parameter pointers (stable for APVTS lifetime, avoids string lookups in processBlock)
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        cachedObj[i].enabled       = apvts.getRawParameterValue (prefix + "enabled");
        cachedObj[i].azimuth       = apvts.getRawParameterValue (prefix + "azimuth");
        cachedObj[i].elevation     = apvts.getRawParameterValue (prefix + "elevation");
        cachedObj[i].distance      = apvts.getRawParameterValue (prefix + "distance");
        cachedObj[i].dopplerAmount = apvts.getRawParameterValue (prefix + "dopplerAmount");
        cachedObj[i].pitchShift    = apvts.getRawParameterValue (prefix + "pitchShift");
        cachedParam_trajectoryShape[i] = apvts.getRawParameterValue (prefix + "trajectoryShape");
        cachedParam_trajectorySpeed[i] = apvts.getRawParameterValue (prefix + "trajectorySpeed");
        cachedParam_trajectoryDirection[i] = apvts.getRawParameterValue (prefix + "trajectoryDirection");
        cachedObj[i].inputChannel      = apvts.getRawParameterValue (prefix + "inputChannel");
    }

    // Cache global parameter pointers (stable for APVTS lifetime, avoids string lookups in processBlock)
    cachedParam_tempoSync       = apvts.getRawParameterValue ("tempoSync");
    cachedParam_noteDivision    = apvts.getRawParameterValue ("noteDivision");
    cachedParam_filterLP        = apvts.getRawParameterValue ("filterLP");
    cachedParam_filterHP        = apvts.getRawParameterValue ("filterHP");
    cachedParam_filterHPQ       = apvts.getRawParameterValue ("filterHPQ");
    cachedParam_filterLPQ       = apvts.getRawParameterValue ("filterLPQ");
    cachedParam_filterEnabled   = apvts.getRawParameterValue ("filterEnabled");
    cachedParam_airAbsorption   = apvts.getRawParameterValue ("airAbsorption");
    cachedParam_wobbleEnabled   = apvts.getRawParameterValue ("wobbleEnabled");
    cachedParam_wobbleAmount    = apvts.getRawParameterValue ("wobbleAmount");
    cachedParam_wobbleMorph     = apvts.getRawParameterValue ("wobbleMorph");
    cachedParam_delayTime       = apvts.getRawParameterValue ("delayTime");
    cachedParam_dryWet          = apvts.getRawParameterValue ("dryWet");
    cachedParam_feedback        = apvts.getRawParameterValue ("feedback");
    cachedParam_inputGain       = apvts.getRawParameterValue ("inputGain");
    cachedParam_outputGain      = apvts.getRawParameterValue ("outputGain");
    // admOscEnabled removed from APVTS — now oscReceiveEnabled member (issue E20)
    // issue #68: Cache global tap offset APVTS pointers
    cachedParam_globalTapAzimuth   = apvts.getRawParameterValue ("globalTapAzimuth");
    cachedParam_globalTapElevation = apvts.getRawParameterValue ("globalTapElevation");
    cachedParam_globalTapDistance   = apvts.getRawParameterValue ("globalTapDistance");
    cachedParam_globalTapPitch     = apvts.getRawParameterValue ("globalTapPitch");
    cachedParam_globalTapDoppler   = apvts.getRawParameterValue ("globalTapDoppler");
    cachedParam_globalTapSpeed     = apvts.getRawParameterValue ("globalTapSpeed");

    // Cache RangedAudioParameter* for trajectory writes and pre-build OSC addresses
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        trajParam_azimuth[i]   = apvts.getParameter (prefix + "azimuth");
        oscSendAddress[i] = "/adm/obj/" + juce::String (i + 1) + "/aed";
    }

    // v0.9: Set initial azimuth spread for first 12 taps (overridden by saved state/presets)
    // Parameter default is 0° so double-click resets to center front.
    {
        static const float initAz[] = {-45.0f, 45.0f, -135.0f, 135.0f,
                                         0.0f, 90.0f, -90.0f, 180.0f,
                                        -30.0f, 30.0f, -60.0f, 60.0f};
        for (int i = 0; i < MAX_OBJECTS; ++i)
            if (trajParam_azimuth[i] != nullptr)
                trajParam_azimuth[i]->setValueNotifyingHost (
                    trajParam_azimuth[i]->convertTo0to1 (initAz[i]));
    }

    // v1.0: Load factory presets from compiled-in array + user presets from disk
    loadAllPresets();

    // v1.0: Default preset selection — find "Quad Ping-Pong" by name so fresh instances
    // start on it regardless of alphabetical category ordering.
    // State restoration (setStateInformation) overwrites this for saved sessions.
    for (int i = 0; i < static_cast<int> (allPresets.size()); ++i)
    {
        if (allPresets[static_cast<size_t> (i)].name == "Quad Ping-Pong")
        {
            currentPresetIndex = i;
            break;
        }
    }
}

// Issue #189: Test constructor — allows forcing abletonMode for automated tests.
OpenSpatialDelayProcessor::OpenSpatialDelayProcessor (bool abletonMode)
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::discreteChannels (50), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout (abletonMode))
{
    algorithms[0] = &algAmbisonics;
    algorithms[1] = &algConstantPower;
    algorithms[2] = &algDBAP;
    algorithms[3] = &algKNN;
    algorithms[4] = &algMDAP;
    algorithms[5] = &algVBAP;
    algorithms[6] = &algVBIP;

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        cachedObj[i].enabled       = apvts.getRawParameterValue (prefix + "enabled");
        cachedObj[i].azimuth       = apvts.getRawParameterValue (prefix + "azimuth");
        cachedObj[i].elevation     = apvts.getRawParameterValue (prefix + "elevation");
        cachedObj[i].distance      = apvts.getRawParameterValue (prefix + "distance");
        cachedObj[i].dopplerAmount = apvts.getRawParameterValue (prefix + "dopplerAmount");
        cachedObj[i].pitchShift    = apvts.getRawParameterValue (prefix + "pitchShift");
        cachedParam_trajectoryShape[i] = apvts.getRawParameterValue (prefix + "trajectoryShape");
        cachedParam_trajectorySpeed[i] = apvts.getRawParameterValue (prefix + "trajectorySpeed");
        cachedParam_trajectoryDirection[i] = apvts.getRawParameterValue (prefix + "trajectoryDirection");
        cachedObj[i].inputChannel      = apvts.getRawParameterValue (prefix + "inputChannel");
    }

    cachedParam_tempoSync       = apvts.getRawParameterValue ("tempoSync");
    cachedParam_noteDivision    = apvts.getRawParameterValue ("noteDivision");
    cachedParam_filterLP        = apvts.getRawParameterValue ("filterLP");
    cachedParam_filterHP        = apvts.getRawParameterValue ("filterHP");
    cachedParam_filterHPQ       = apvts.getRawParameterValue ("filterHPQ");
    cachedParam_filterLPQ       = apvts.getRawParameterValue ("filterLPQ");
    cachedParam_filterEnabled   = apvts.getRawParameterValue ("filterEnabled");
    cachedParam_airAbsorption   = apvts.getRawParameterValue ("airAbsorption");
    cachedParam_wobbleEnabled   = apvts.getRawParameterValue ("wobbleEnabled");
    cachedParam_wobbleAmount    = apvts.getRawParameterValue ("wobbleAmount");
    cachedParam_wobbleMorph     = apvts.getRawParameterValue ("wobbleMorph");
    cachedParam_delayTime       = apvts.getRawParameterValue ("delayTime");
    cachedParam_dryWet          = apvts.getRawParameterValue ("dryWet");
    cachedParam_feedback        = apvts.getRawParameterValue ("feedback");
    cachedParam_inputGain       = apvts.getRawParameterValue ("inputGain");
    cachedParam_outputGain      = apvts.getRawParameterValue ("outputGain");
    cachedParam_globalTapAzimuth   = apvts.getRawParameterValue ("globalTapAzimuth");
    cachedParam_globalTapElevation = apvts.getRawParameterValue ("globalTapElevation");
    cachedParam_globalTapDistance   = apvts.getRawParameterValue ("globalTapDistance");
    cachedParam_globalTapPitch     = apvts.getRawParameterValue ("globalTapPitch");
    cachedParam_globalTapDoppler   = apvts.getRawParameterValue ("globalTapDoppler");
    cachedParam_globalTapSpeed     = apvts.getRawParameterValue ("globalTapSpeed");

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        trajParam_azimuth[i]   = apvts.getParameter (prefix + "azimuth");
        oscSendAddress[i] = "/adm/obj/" + juce::String (i + 1) + "/aed";
    }

    {
        static const float initAz[] = {-45.0f, 45.0f, -135.0f, 135.0f,
                                         0.0f, 90.0f, -90.0f, 180.0f,
                                        -30.0f, 30.0f, -60.0f, 60.0f};
        for (int i = 0; i < MAX_OBJECTS; ++i)
            if (trajParam_azimuth[i] != nullptr)
                trajParam_azimuth[i]->setValueNotifyingHost (
                    trajParam_azimuth[i]->convertTo0to1 (initAz[i]));
    }

    loadAllPresets();

    for (int i = 0; i < static_cast<int> (allPresets.size()); ++i)
    {
        if (allPresets[static_cast<size_t> (i)].name == "Quad Ping-Pong")
        {
            currentPresetIndex = i;
            break;
        }
    }
}

OpenSpatialDelayProcessor::~OpenSpatialDelayProcessor()
{
    // Issue #122: Stop 60Hz timer before any member destruction to prevent
    // use-after-free in timerCallback() during plugin deletion
    stopTimer();

    // v0.6: Disconnect OSC receiver before destruction
    oscReceiver.disconnect();
    oscReceiver.removeListener (this);
    // v0.7: Disconnect OSC sender
    oscSender.disconnect();
}

//==============================================================================
// v0.6: OSC port change — reconnect if currently connected
//==============================================================================
void OpenSpatialDelayProcessor::setOscReceiveEnabled (bool enabled)
{
    oscReceiveEnabled = enabled;
    markConfigStateDirty();  // E14: notify DAW so state is re-captured on save
    // Connection lifecycle handled in processBlock via edge-detect
}

void OpenSpatialDelayProcessor::setOscReceivePort (int port)
{
    if (port == oscReceivePort)
        return;

    oscReceivePort = port;
    markConfigStateDirty();  // E14: notify DAW so state is re-captured on save

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

//==============================================================================
// v0.7: OSC Send control methods
//==============================================================================
void OpenSpatialDelayProcessor::setOscSendEnabled (bool enabled)
{
    if (enabled == oscSendEnabled)
        return;

    oscSendEnabled = enabled;
    markConfigStateDirty();  // E14: notify DAW so state is re-captured on save
    if (enabled)
    {
        oscSendConnected = oscSender.connect (oscSendIP, oscSendPort);
    }
    else
    {
        oscSender.disconnect();
        oscSendConnected = false;
    }
}

void OpenSpatialDelayProcessor::setOscSendPort (int port)
{
    if (port == oscSendPort)
        return;
    oscSendPort = port;
    markConfigStateDirty();  // E14: notify DAW so state is re-captured on save
    if (oscSendEnabled)
    {
        oscSender.disconnect();
        oscSendConnected = oscSender.connect (oscSendIP, oscSendPort);
    }
}

void OpenSpatialDelayProcessor::setOscSendIP (const juce::String& ip)
{
    if (ip == oscSendIP)
        return;
    oscSendIP = ip;
    markConfigStateDirty();  // E14: notify DAW so state is re-captured on save
    if (oscSendEnabled)
    {
        oscSender.disconnect();
        oscSendConnected = oscSender.connect (oscSendIP, oscSendPort);
    }
}

// #############################################################################
// v0.6: Preset System — Factory presets, load/save, JSON serialization
// #############################################################################

// Factory presets moved to PresetData.cpp (v0.9)


//==============================================================================
// Preset API implementation — v0.9: all presets loaded from disk (allPresets)
//==============================================================================
int OpenSpatialDelayProcessor::getNumPresets() const
{
    return static_cast<int> (allPresets.size());
}

juce::StringArray OpenSpatialDelayProcessor::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& p : allPresets)
        names.add (p.name);
    return names;
}

void OpenSpatialDelayProcessor::loadPreset (int index)
{
    if (index < 0 || index >= static_cast<int> (allPresets.size()))
        return;

    const PresetData* preset = &allPresets[static_cast<size_t> (index)];

    // Issue E15: Begin gestures on all params we're about to set, so the host
    // groups the entire preset load into a single undo entry.
    std::vector<juce::RangedAudioParameter*> presetGestures;
    auto beginGesture = [&] (const juce::String& paramId) {
        if (auto* p = apvts.getParameter (paramId))
        {
            p->beginChangeGesture();
            presetGestures.push_back (p);
        }
    };

    // Collect all params that will be set
    for (const char* id : { "delayTime", "tempoSync", "noteDivision", "syncMode",
                            "feedback", "filterLP", "filterHP", "filterLPQ", "filterHPQ",
                            "dryWet", "inputGain", "outputGain", "airAbsorption",
                            "filterEnabled", "wobbleEnabled", "wobbleAmount", "wobbleMorph" })
        beginGesture (id);
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        for (const char* suffix : { "enabled", "azimuth", "elevation", "distance",
                                    "dopplerAmount", "pitchShift", "trajectoryShape",
                                    "trajectorySpeed", "trajectoryDirection", "inputChannel" })
            beginGesture (prefix + suffix);
    }

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
    setFloat  ("filterLPQ",    preset->filterLPQ);
    setFloat  ("filterHPQ",    preset->filterHPQ);
    setFloat  ("dryWet",       preset->dryWet);
    setFloat  ("inputGain",    preset->inputGain);
    setFloat  ("outputGain",   preset->outputGain);
    setBool   ("airAbsorption", preset->airAbsorption);
    setBool   ("filterEnabled", preset->filterEnabled);
    setBool   ("wobbleEnabled", preset->wobbleEnabled);
    setFloat  ("wobbleAmount", preset->wobbleAmount);
    setFloat  ("wobbleMorph",  preset->wobbleMorph);
    configHrtfProfile.store (preset->hrtfProfile, std::memory_order_relaxed);
    // NOTE: configAlgorithm, outputFormat, admOscEnabled, oscReceivePort are NOT modified by presets

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
        setFloat  (prefix + "pitchShift",      tap.pitchShift);
        setChoice (prefix + "trajectoryShape", tap.trajectoryShape);
        setFloat  (prefix + "trajectorySpeed", tap.trajectorySpeed);
        setChoice (prefix + "trajectoryDirection", tap.trajectoryDirection);
        setChoice (prefix + "inputChannel",        tap.inputChannel);
    }

    // Issue E15: End all gestures — host groups the entire preset load as one undo entry
    for (auto* p : presetGestures)
        p->endChangeGesture();

    // v1.0.1: Reset trajectory state FIRST so processBlock doesn't read stale
    // animated positions from the old preset while prevAzimuth already points to
    // the new preset's static positions — that mismatch caused huge fake velocities
    // and Doppler transient spikes (issue #42, bug 3).
    for (int i = 0; i < MAX_OBJECTS; ++i)
        trajectory.reset (i, preset->taps[i].azimuthDeg, preset->taps[i].elevationDeg, preset->taps[i].distance);

    // v1.0.1: Defer WSOLA/Doppler/filter reset to the audio thread via atomic flag.
    // loadPreset() runs on the message thread — writing non-atomic audio state here
    // races with processBlock(), causing lost resets and inconsistent WSOLA state
    // (issue #42, bug 3). Store the target prevAzimuth values; processBlock will
    // apply the full reset atomically on its own thread.
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        const auto& tap = preset->taps[i];
        pendingReset.prevAz[i]   = juce::degreesToRadians (tap.azimuthDeg);
        pendingReset.prevEl[i]   = juce::degreesToRadians (tap.elevationDeg);
        pendingReset.prevDist[i] = tap.distance;
    }
    presetResetPending.store (true, std::memory_order_release);

    // v1.0: Reset smoothed filter frequencies to prevent stale ramps
    // (These are only read on the audio thread after coefficient threshold check,
    // so writing from message thread is safe — no concurrent read-modify-write.)
    filters.setSmoothedFrequencies (preset->filterLP, preset->filterHP,
                                     preset->filterLPQ, preset->filterHPQ);
    filters.invalidateCoefficients();

    currentPresetIndex = index;
}

void OpenSpatialDelayProcessor::loadNextPreset()
{
    if (categorizedOrder.empty()) return;
    // Find current position in category-ordered sequence
    int pos = 0;
    for (int i = 0; i < static_cast<int> (categorizedOrder.size()); ++i)
    {
        if (categorizedOrder[static_cast<size_t> (i)] == currentPresetIndex)
        { pos = i; break; }
    }
    pos = (pos + 1) % static_cast<int> (categorizedOrder.size());
    loadPreset (categorizedOrder[static_cast<size_t> (pos)]);
}

void OpenSpatialDelayProcessor::loadPreviousPreset()
{
    if (categorizedOrder.empty()) return;
    int pos = 0;
    for (int i = 0; i < static_cast<int> (categorizedOrder.size()); ++i)
    {
        if (categorizedOrder[static_cast<size_t> (i)] == currentPresetIndex)
        { pos = i; break; }
    }
    int sz = static_cast<int> (categorizedOrder.size());
    pos = (pos - 1 + sz) % sz;
    loadPreset (categorizedOrder[static_cast<size_t> (pos)]);
}

PresetData OpenSpatialDelayProcessor::captureCurrentState() const
{
    PresetData pd;
    pd.delayTime    = apvts.getRawParameterValue ("delayTime")->load();
    pd.tempoSync    = apvts.getRawParameterValue ("tempoSync")->load() > 0.5f;
    pd.noteDivision = apvts.getRawParameterValue ("noteDivision")->load();
    pd.syncMode     = static_cast<int> (apvts.getRawParameterValue ("syncMode")->load());
    pd.feedback     = apvts.getRawParameterValue ("feedback")->load();
    pd.filterLP     = apvts.getRawParameterValue ("filterLP")->load();
    pd.filterHP     = apvts.getRawParameterValue ("filterHP")->load();
    pd.filterLPQ    = apvts.getRawParameterValue ("filterLPQ")->load();
    pd.filterHPQ    = apvts.getRawParameterValue ("filterHPQ")->load();
    pd.dryWet       = apvts.getRawParameterValue ("dryWet")->load();
    pd.inputGain    = apvts.getRawParameterValue ("inputGain")->load();
    pd.outputGain   = apvts.getRawParameterValue ("outputGain")->load();
    pd.airAbsorption = apvts.getRawParameterValue ("airAbsorption")->load() > 0.5f;
    pd.filterEnabled = apvts.getRawParameterValue ("filterEnabled")->load() > 0.5f;
    pd.wobbleEnabled = apvts.getRawParameterValue ("wobbleEnabled")->load() > 0.5f;
    pd.wobbleAmount  = apvts.getRawParameterValue ("wobbleAmount")->load();
    pd.wobbleMorph   = apvts.getRawParameterValue ("wobbleMorph")->load();
    pd.algorithm    = configAlgorithm.load (std::memory_order_relaxed);
    pd.hrtfProfile  = configHrtfProfile.load (std::memory_order_relaxed);

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        auto& tap   = pd.taps[i];
        tap.enabled         = apvts.getRawParameterValue (prefix + "enabled")->load() > 0.5f;
        tap.azimuthDeg      = apvts.getRawParameterValue (prefix + "azimuth")->load();
        tap.elevationDeg    = apvts.getRawParameterValue (prefix + "elevation")->load();
        tap.distance        = apvts.getRawParameterValue (prefix + "distance")->load();
        tap.dopplerAmount   = apvts.getRawParameterValue (prefix + "dopplerAmount")->load();
        tap.pitchShift      = apvts.getRawParameterValue (prefix + "pitchShift")->load();
        tap.trajectoryShape = static_cast<int> (apvts.getRawParameterValue (prefix + "trajectoryShape")->load());
        tap.trajectorySpeed = apvts.getRawParameterValue (prefix + "trajectorySpeed")->load();
        tap.trajectoryDirection = static_cast<int> (apvts.getRawParameterValue (prefix + "trajectoryDirection")->load());
        tap.inputChannel        = static_cast<int> (apvts.getRawParameterValue (prefix + "inputChannel")->load());
    }

    return pd;
}

// JSON serialization / deserialization moved to PresetData.cpp (v0.9)

//==============================================================================
// v0.9: Preset file I/O — all presets on disk as .osdpreset files
//==============================================================================
juce::File OpenSpatialDelayProcessor::getPresetDirectory()
{
    return juce::File::getSpecialLocation (juce::File::userHomeDirectory)
               .getChildFile ("Library")
               .getChildFile ("Audio")
               .getChildFile ("Presets")
               .getChildFile ("OpenSpatialDelay");
}

void OpenSpatialDelayProcessor::saveUserPreset (const juce::String& name)
{
    saveUserPreset (name, "User");
}

void OpenSpatialDelayProcessor::saveUserPreset (const juce::String& name, const juce::String& category)
{
    auto pd     = captureCurrentState();
    pd.name     = name;
    pd.category = category.isEmpty() ? juce::String ("User") : category;
    pd.isFactory = false;
    auto json   = serializePresetToJson (pd);

    auto dir = getPresetDirectory().getChildFile (pd.category);
    dir.createDirectory();

    auto file = dir.getChildFile (name + ".osdpreset");
    file.replaceWithText (json);

    // Refresh all presets and update index
    loadAllPresets();
    for (int i = 0; i < static_cast<int> (allPresets.size()); ++i)
    {
        if (allPresets[static_cast<size_t> (i)].name == name
            && allPresets[static_cast<size_t> (i)].category == pd.category)
        {
            currentPresetIndex = i;
            break;
        }
    }
}

void OpenSpatialDelayProcessor::loadAllPresets()
{
    allPresets.clear();

    // v1.0: Load factory presets from compiled-in array (no disk I/O)
    for (int i = 0; i < NUM_FACTORY_PRESETS; ++i)
    {
        PresetData pd = factoryPresets[i];
        pd.isFactory = true;
        allPresets.push_back (pd);
    }

    // Load user presets from disk (User/ folder + subfolders as subcategories)
    auto userDir = getPresetDirectory().getChildFile ("User");
    if (userDir.isDirectory())
    {
        // Root-level user presets → category "User"
        auto files = userDir.findChildFiles (juce::File::findFiles, false, "*.osdpreset");
        auto jsonFiles = userDir.findChildFiles (juce::File::findFiles, false, "*.json");
        files.addArray (jsonFiles);
        files.sort();

        for (const auto& file : files)
        {
            auto json = file.loadFileAsString();
            if (json.isNotEmpty())
            {
                auto pd = parsePresetJson (json);
                if (pd.name.isEmpty())
                    pd.name = file.getFileNameWithoutExtension();
                pd.category = "User";
                pd.isFactory = false;
                allPresets.push_back (pd);
            }
        }

        // User subfolders → category "User / SubfolderName"
        auto subdirs = userDir.findChildFiles (juce::File::findDirectories, false);
        subdirs.sort();
        for (const auto& subdir : subdirs)
        {
            juce::String subCat = "User / " + subdir.getFileName();
            auto subFiles = subdir.findChildFiles (juce::File::findFiles, false, "*.osdpreset");
            auto subJsonFiles = subdir.findChildFiles (juce::File::findFiles, false, "*.json");
            subFiles.addArray (subJsonFiles);
            subFiles.sort();

            for (const auto& file : subFiles)
            {
                auto json = file.loadFileAsString();
                if (json.isNotEmpty())
                {
                    auto pd = parsePresetJson (json);
                    if (pd.name.isEmpty())
                        pd.name = file.getFileNameWithoutExtension();
                    pd.category = subCat;
                    pd.isFactory = false;
                    allPresets.push_back (pd);
                }
            }
        }
    }

    rebuildCategorizedOrder();
}

//==============================================================================
// v0.9: Categorized preset helpers
//==============================================================================
std::vector<OpenSpatialDelayProcessor::CategorizedPreset>
    OpenSpatialDelayProcessor::getCategorizedPresets() const
{
    std::vector<CategorizedPreset> result;

    // Iterate categories in defined order
    for (int c = 0; c < NUM_PRESET_CATEGORIES; ++c)
    {
        juce::String cat = presetCategoryNames[c];

        // All presets in this category (factory first, then user)
        for (int i = 0; i < static_cast<int> (allPresets.size()); ++i)
        {
            if (allPresets[static_cast<size_t> (i)].category == cat
                && allPresets[static_cast<size_t> (i)].isFactory)
                result.push_back ({ cat, allPresets[static_cast<size_t> (i)].name, i, true });
        }
        for (int i = 0; i < static_cast<int> (allPresets.size()); ++i)
        {
            if (allPresets[static_cast<size_t> (i)].category == cat
                && ! allPresets[static_cast<size_t> (i)].isFactory)
                result.push_back ({ cat, allPresets[static_cast<size_t> (i)].name, i, false });
        }
    }

    // Safety net: any presets with unrecognized categories
    for (int i = 0; i < static_cast<int> (allPresets.size()); ++i)
    {
        bool found = false;
        for (int c = 0; c < NUM_PRESET_CATEGORIES; ++c)
        {
            if (allPresets[static_cast<size_t> (i)].category == presetCategoryNames[c])
            { found = true; break; }
        }
        if (! found)
            result.push_back ({ allPresets[static_cast<size_t> (i)].category,
                                allPresets[static_cast<size_t> (i)].name,
                                i, allPresets[static_cast<size_t> (i)].isFactory });
    }

    return result;
}

void OpenSpatialDelayProcessor::rebuildCategorizedOrder()
{
    categorizedOrder.clear();
    auto cats = getCategorizedPresets();
    categorizedOrder.reserve (cats.size());
    for (const auto& cp : cats)
        categorizedOrder.push_back (cp.originalIndex);
}

//==============================================================================
bool OpenSpatialDelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // VST3: accept any layout the host proposes (IEM Plugin Suite approach, issue #111).
    // VST3 hosts negotiate bus arrangements that may not match our predefined set;
    // accepting everything lets REAPER/Cubase provide the track's full channel count.
    // AU: keep explicit whitelist for Ableton AU compatibility (issue #122).
    if (wrapperType == wrapperType_VST3)
    {
        juce::ignoreUnused (layouts);
        return true;
    }

    // AU / standalone path: accept any channel count usable by at least one format.
    // The UI dropdown greys out formats whose requiredChannels > maxBusChannels,
    // so bus negotiation only needs to confirm the channel count is viable.
    // This handles REAPER's even-only track widths (e.g. 26ch enables 4th Order Ambi at 25ch).
    auto inputSet = layouts.getMainInputChannelSet();
    if (inputSet != juce::AudioChannelSet::mono() &&
        inputSet != juce::AudioChannelSet::stereo())
    {
        if (inputSet != layouts.getMainOutputChannelSet())
            return false;
    }

    auto outputSet = layouts.getMainOutputChannelSet();
    int numCh = outputSet.size();

    for (const auto& info : outputFormatRegistry)
        if (info.requiredChannels <= numCh)
            return true;

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
        case 14: return OutputFormat::Surround7_1_6;
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

// v1.0: Check if a speaker layout has height speakers (elevation > 1°)
static bool layoutHasHeight (const SpeakerLayout& layout)
{
    for (int s = 0; s < layout.numSpeakers; ++s)
        if (std::abs (layout.speakers[s].elevationRad) > 0.0175f)  // ~1 degree
            return true;
    return false;
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
        case OutputFormat::Quad:          buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::Quad]);       break;
        case OutputFormat::Surround5_0:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S5_0]);       break;
        case OutputFormat::Surround5_1:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S5_1]);       break;
        case OutputFormat::Surround7_0:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_0]);       break;
        case OutputFormat::Surround7_1:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_1]);       break;
        case OutputFormat::Surround9_1:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S9_1]);       break;
        case OutputFormat::Surround5_1_2: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S5_1_2]);     break;
        case OutputFormat::Surround5_1_4: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S5_1_4]);     break;
        case OutputFormat::Surround7_1_2: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_1_2]);     break;
        case OutputFormat::Surround7_1_4: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_1_4]);     break;
        case OutputFormat::Surround7_1_6: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_1_6]);     break;
        case OutputFormat::Surround9_1_4: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S9_1_4]);     break;
        case OutputFormat::Surround9_1_6: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S9_1_6]);     break;
        case OutputFormat::SurroundSML13_1: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::SML13_1]); break;
        case OutputFormat::Octaphonic:    buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::Octaphonic]); break;

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

    // v1.0: Diagnostic — height layouts must always have triplets
    jassert (buf.vbapTriplets.empty() == ! layoutHasHeight (buf.layout));

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
    delayBufferL.assign (static_cast<size_t> (delayBufferSize), 0.0f);
    delayBufferR.assign (static_cast<size_t> (delayBufferSize), 0.0f);
    writePosition = 0;
    feedbackSample = 0.0f;

    // v1.0.7: Reset wobble modulation state (4-layer tape emulation, issue #92)
    std::fill (std::begin (wobblePhases), std::end (wobblePhases), 0.0f);
    smoothedWobbleAmount.reset (sampleRate, 0.05);  // 50ms ramp
    smoothedWobbleAmount.setCurrentAndTargetValue (0.0f);
    blockWobbleMorph = 0.0f;

    // v1.0.6: Reset phase vocoder pitch shifters and report latency (issue #60)
    for (int i = 0; i < MAX_OBJECTS; ++i)
        pvPitchShifters[i].reset();
    setLatencySamples (PhaseVocoderPitchShifter::kFFTSize);

    // v1.0.7: Dry path latency compensation (issue #63)
    // Delay dry signal by kFFTSize samples to match the phase vocoder's wet path latency.
    // v1.0.1: Stereo dry buffers — dry path preserves stereo input (issue #73)
    dryDelayLineL.assign (static_cast<size_t> (PhaseVocoderPitchShifter::kFFTSize), 0.0f);
    dryDelayLineR.assign (static_cast<size_t> (PhaseVocoderPitchShifter::kFFTSize), 0.0f);
    dryDelayWritePos = 0;
    dryCompBufferL.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    dryCompBufferR.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    perSampleDW.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    perSampleOutGain.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

    // v1.0: Initialize tap fade envelopes
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        tapFadeGain[t] = 0.0f;
        tapFadeTarget[t] = 0.0f;
    }
    tapFadeIncrement = 1.0f / 64.0f;

    // Pre-allocate input buffers
    monoInputBuffer.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    inputBufferL.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    inputBufferR.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

    // Initialize filters
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
    filters.prepare (sampleRate, samplesPerBlock);
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        // v1.0.1: tap filters now prepared via filters.prepare() above
    }
    filters.invalidateCoefficients();

    // Initialize smoothed values — setCurrentAndTargetValue prevents stale ramps
    // after mid-session prepareToPlay calls (e.g., bus renegotiation)
    smoothedDryWet.reset     (sampleRate, 0.02);
    smoothedFeedback.reset   (sampleRate, 0.02);
    smoothedInputGain.reset  (sampleRate, 0.02);
    smoothedOutputGain.reset (sampleRate, 0.02);

    // Read current parameter values so smoothing starts at the right position
    // E18: Use tempo-synced value when tempo sync is active — the raw knob value
    // may differ from the actual delay time, causing a stale ramp on first block.
    float initDelayMs;
    bool tempoSyncInit = apvts.getRawParameterValue ("tempoSync")->load() > 0.5f;
    if (tempoSyncInit)
    {
        int ndivInit = static_cast<int> (apvts.getRawParameterValue ("noteDivision")->load());
        initDelayMs = getTempoSyncedDelayMs (ndivInit);
    }
    else
    {
        initDelayMs = apvts.getRawParameterValue ("delayTime")->load();
    }
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

    // v1.0.1: Preset transition fade — 5ms = imperceptible but click-free (issue #84)
    presetTransitionStep = 1.0f / (0.005f * static_cast<float> (sampleRate));
    presetTransitionState = PresetTransitionState::Idle;
    presetTransitionGain = 1.0f;

    // v1.0.6: Transport fade-in — same 5ms ramp for scrub/seek click prevention (issue #103)
    transportFadeStep = 1.0f / (0.005f * static_cast<float> (sampleRate));
    transportFadeGain = 1.0f;
    transportFadeActive = false;

    // Initialize modular 3D audio core
    computeAmbiDecodeMatrix();        // Pre-compute 3rd-order decode matrix for virtual speakers

    // v0.2: Resolve output format from user selection + bus constraint
    maxBusChannels = getTotalNumOutputChannels();
    int userFormatIndex = configOutputFormat.load (std::memory_order_relaxed);
    auto userFormat = outputFormatRegistry[static_cast<size_t> (juce::jlimit (0, NUM_OUTPUT_FORMATS - 1, userFormatIndex))].format;
    auto effectiveFormat = resolveEffectiveFormat (userFormat, maxBusChannels);
    activateLayout (effectiveFormat);

    // v0.2: Configure LFE low-pass filter (120 Hz, 2nd order Butterworth)
    lfeFilter.prepare (spec);
    lfeFilter.reset();
    *lfeFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 120.0f);

    // v0.4: Prepare air absorption filters (per-object LP, distance-driven cutoff)
    // v1.0: Pre-compute transparent coefficients to avoid heap allocation in processBlock
    // v1.0.1: air absorption filters now prepared via filters.prepare() above
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        // v1.0.1: air absorption filters now prepared via filters.prepare()
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
        smoothedNfcDistance[obj] = 0.0f;
    }

    // v1.0.1: Reset Doppler velocity tracking
    doppler.resetAll();
    std::memset (dopplerDelayAccum, 0, sizeof (dopplerDelayAccum));

    // HRTF convolution: prepare both renderers (double-buffered)
    binauralRenderers[0].prepare (sampleRate, samplesPerBlock);
    binauralRenderers[1].prepare (sampleRate, samplesPerBlock);

    // Pre-allocate renderer crossfade buffers (issue #131: avoid audio-thread allocation)
    xfadeWetL_.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    xfadeWetR_.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

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
    // Issue #122: Stop timer early — DAW calls this before destruction
    stopTimer();

    delayBufferL.clear();
    delayBufferR.clear();
    monoInputBuffer.clear();
    inputBufferL.clear();
    inputBufferR.clear();
}

// #############################################################################
// DELAY-SPECIFIC — Delay line, pitch shifter, tempo sync
// This section contains the delay engine DSP: circular buffer read/write,
// WSOLA-lite per-tap pitch shifter, and tempo-sync note division mapping.
// Other SML plugins would replace this with their own DSP (reverb, chorus, etc.).
// #############################################################################

//==============================================================================
// Delay line helpers
//==============================================================================
void OpenSpatialDelayProcessor::writeDelayLine (float sampleL, float sampleR)
{
    delayBufferL[static_cast<size_t> (writePosition)] = sampleL;
    delayBufferR[static_cast<size_t> (writePosition)] = sampleR;
    writePosition = (writePosition + 1) & delayBufferMask;  // v0.5: bitmask wrap
}

// v0.8: Templated helper — Cubic Hermite interpolation from a given buffer
static inline float readDelayBuffer (const std::vector<float>& buf, int writePos,
                                     int bufMask, int bufSize, float delaySamples)
{
    float readPos = static_cast<float> (writePos) - delaySamples - 1.0f;
    if (readPos < 0.0f) readPos += static_cast<float> (bufSize);

    int   i1 = static_cast<int> (readPos);
    float f  = readPos - static_cast<float> (i1);

    int i0 = (i1 - 1) & bufMask;
    int i2 = (i1 + 1) & bufMask;
    int i3 = (i1 + 2) & bufMask;
    i1 = i1 & bufMask;

    float y0 = buf[static_cast<size_t> (i0)];
    float y1 = buf[static_cast<size_t> (i1)];
    float y2 = buf[static_cast<size_t> (i2)];
    float y3 = buf[static_cast<size_t> (i3)];

    return y1 + 0.5f * f * (y2 - y0 + f * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3 + f * (3.0f * (y1 - y2) + y3 - y0)));
}

float OpenSpatialDelayProcessor::readDelayLineL (float delaySamples) const
{
    return readDelayBuffer (delayBufferL, writePosition, delayBufferMask, delayBufferSize, delaySamples);
}

float OpenSpatialDelayProcessor::readDelayLineR (float delaySamples) const
{
    return readDelayBuffer (delayBufferR, writePosition, delayBufferMask, delayBufferSize, delaySamples);
}

float OpenSpatialDelayProcessor::readDelayLineMono (float delaySamples) const
{
    return (readDelayLineL (delaySamples) + readDelayLineR (delaySamples)) * 0.5f;
}

// v1.0.6: Pitch shifting via PhaseVocoderPitchShifter (issue #60)

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

    // v0.7: Apply mode multiplier (0=Straight, 1=Dotted, 2=Triplet)
    int mode = static_cast<int> (apvts.getRawParameterValue ("syncMode")->load());
    if (mode == 1) // Dotted
        duration *= 1.5;
    else if (mode == 2) // Triplet
        duration *= (2.0 / 3.0);

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
    // C++11 guarantees thread-safe initialization of function-local statics,
    // eliminating the race condition when multiple instances call this
    // simultaneously (issue #96).
    static const std::vector<VBAPTriplet> triplets = []() {
        std::vector<VBAPTriplet> result;

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
                result.push_back (tri);
        }

        return result;
    }();

    return triplets;
}

// v1.0: 3D nearest-speaker fallback — used when triplets are empty on a 3D layout.
// Prevents 2D fallback from routing signal to height speakers for horizontal sources.
static void nearestSpeaker3DFallback (const SpeakerLayout& layout,
                                      float azimuthRad, float elevationRad,
                                      float* outGains, int numSpeakers)
{
    for (int s = 0; s < numSpeakers; ++s)
        outGains[s] = 0.0f;

    float px = std::cos (elevationRad) * std::sin (azimuthRad);
    float py = std::cos (elevationRad) * std::cos (azimuthRad);
    float pz = std::sin (elevationRad);

    float bestDot = -2.0f;
    int bestSpeaker = 0;
    for (int s = 0; s < numSpeakers; ++s)
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

//==============================================================================
// VBAPAlgorithm — Vector Base Amplitude Panning (2D or 3D)
//==============================================================================
void VBAPAlgorithm::computeGains (const SourcePosition& source, const LayoutContext& ctx,
                                   float* outputGains, int /*numSpeakers*/) const
{
    if (! ctx.triplets.empty())
        computeVBAPGains3D (ctx.layout, ctx.triplets, source.azimuthRad, source.elevationRad, outputGains);
    else if (layoutHasHeight (ctx.layout))
    {
        // v1.0: Guard — never use 2D fallback on 3D layouts (would route to height speakers)
        jassertfalse;  // Triplets should be populated for height layouts — investigate
        nearestSpeaker3DFallback (ctx.layout, source.azimuthRad, source.elevationRad,
                                  outputGains, ctx.layout.numSpeakers);
    }
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
    else if (layoutHasHeight (ctx.layout))
    {
        // v1.0: Guard — never use 2D fallback on 3D layouts
        jassertfalse;
        nearestSpeaker3DFallback (ctx.layout, source.azimuthRad, source.elevationRad,
                                  outputGains, numSpeakers);
    }
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
    // Source distance maps to radial position within speaker sphere (0..1 → 0..speakerRadius)
    constexpr float speakerRadius = 1.0f;
    float physicalDist = source.distance * speakerRadius;
    float srcX = physicalDist * std::cos (source.elevationRad) * std::sin (source.azimuthRad);
    float srcY = physicalDist * std::cos (source.elevationRad) * std::cos (source.azimuthRad);
    float srcZ = physicalDist * std::sin (source.elevationRad);

    // Compute Euclidean distance from source to each speaker
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
// ConstantPowerAlgorithm — Cosine-distance all-speaker panning
// All speakers within 90 degrees of the source receive a cosine-weighted gain,
// constant-power normalized. Produces smooth, diffuse spatial images.
//==============================================================================
void ConstantPowerAlgorithm::computeGains (const SourcePosition& source, const LayoutContext& ctx,
                                            float* outputGains, int numSpeakers) const
{
    for (int s = 0; s < numSpeakers; ++s)
        outputGains[s] = 0.0f;

    if (numSpeakers == 0) return;

    // Source direction as unit Cartesian vector
    float px = std::cos (source.elevationRad) * std::sin (source.azimuthRad);
    float py = std::cos (source.elevationRad) * std::cos (source.azimuthRad);
    float pz = std::sin (source.elevationRad);

    // Cosine-distance weighting: dot product = cos(angular distance)
    // Hemisphere cutoff: speakers beyond 90 degrees get zero gain
    float totalPower = 0.0f;

    for (int s = 0; s < numSpeakers; ++s)
    {
        float sx = std::cos (ctx.layout.speakers[s].elevationRad) * std::sin (ctx.layout.speakers[s].azimuthRad);
        float sy = std::cos (ctx.layout.speakers[s].elevationRad) * std::cos (ctx.layout.speakers[s].azimuthRad);
        float sz = std::sin (ctx.layout.speakers[s].elevationRad);

        float dot = juce::jlimit (-1.0f, 1.0f, px * sx + py * sy + pz * sz);
        float rawGain = std::max (0.0f, dot);
        outputGains[s] = rawGain;
        totalPower += rawGain * rawGain;
    }

    // Constant-power normalization
    if (totalPower > 1e-12f)
    {
        float scale = 1.0f / std::sqrt (totalPower);
        for (int s = 0; s < numSpeakers; ++s)
            outputGains[s] *= scale;
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
    else if (layoutHasHeight (ctx.layout))
    {
        // v1.0: Guard — never use 2D fallback on 3D layouts
        jassertfalse;
        nearestSpeaker3DFallback (ctx.layout, source.azimuthRad, source.elevationRad,
                                  tempGains, numSpeakers);
    }
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
        else if (layoutHasHeight (ctx.layout))
        {
            // v1.0: Guard — never use 2D fallback on 3D layouts
            jassertfalse;
            nearestSpeaker3DFallback (ctx.layout, auxAz, auxEl, tempGains, numSpeakers);
        }
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

    // v0.9: When trajectory is active, return the animated position (not the origin/knob values)
    if (trajectory.isActive (objectIndex))
    {
        state.azimuthDeg   = trajectory.getFinalAz (objectIndex);
        state.elevationDeg = trajectory.getFinalEl (objectIndex);
        state.distance     = trajectory.getFinalDist (objectIndex);
    }
    else
    {
        if (auto* p = apvts.getRawParameterValue (prefix + "azimuth"))
            state.azimuthDeg = p->load();
        if (auto* p = apvts.getRawParameterValue (prefix + "elevation"))
            state.elevationDeg = p->load();
        if (auto* p = apvts.getRawParameterValue (prefix + "distance"))
            state.distance = p->load();
    }

    return state;
}

TrajectoryState OpenSpatialDelayProcessor::getTrajectoryState (int objectIndex) const
{
    auto ts = trajectory.getState (objectIndex);
    TrajectoryState result;
    result.originAzDeg = ts.originAzDeg;
    result.originElDeg = ts.originElDeg;
    result.originDist  = ts.originDist;
    result.shape       = ts.shape;
    result.phase       = ts.phase;
    result.reverse     = (cachedParam_trajectoryDirection[objectIndex] == nullptr
                          || cachedParam_trajectoryDirection[objectIndex]->load() < 0.5f);
    result.randomTime  = ts.randomTime;
    return result;
}

OpenSpatialDelayProcessor::RandomPosition
OpenSpatialDelayProcessor::evaluateRandomNoise (int objectIndex, float time) const
{
    auto rp = trajectory.evaluateRandomNoise (objectIndex, time);
    return { rp.azDeg, rp.elDeg, rp.dist };
}

// #############################################################################
// DELAY-SPECIFIC — processBlock, render methods, and DSP helpers
// processBlock dispatches to 5 rendering paths. The render methods combine
// the delay engine (above) with the spatial framework (algorithms, HRTF).
// Other SML plugins would replace everything below with their own processBlock
// and render logic, calling the spatial algorithms via computeGains().
// #############################################################################

// --- DELAY-SPECIFIC: Wobble modulation (v1.0.7) --- tape wow/flutter emulation
// 4 incommensurate sinusoids spanning wow (1.5 Hz) to flutter (8.1 Hz).
// Morph crossfades weight distribution: wow-heavy at 0%, flutter-heavy at 100%.
// Restored from v0.9 architecture with smoothed amount + quadratic depth (issue #92).
static constexpr int   kWobbleLayers = 4;
static constexpr float kWobbleRates[kWobbleLayers]    = { 1.5f, 3.7f, 5.9f, 8.1f };
static constexpr float kWowWeights[kWobbleLayers]     = { 0.55f, 0.30f, 0.10f, 0.05f };
static constexpr float kFlutterWeights[kWobbleLayers] = { 0.05f, 0.10f, 0.30f, 0.55f };

inline float OpenSpatialDelayProcessor::applyWobble (float baseDelaySamples, float currentDelayMs)
{
    float wobbleAmt = smoothedWobbleAmount.getNextValue();
    if (wobbleAmt <= 0.0f)
        return baseDelaySamples;

    juce::ignoreUnused (currentDelayMs);

    const float morph = blockWobbleMorph / 100.0f;
    const float invSr = 1.0f / static_cast<float> (currentSampleRate);

    float modulation = 0.0f;
    for (int i = 0; i < kWobbleLayers; ++i)
    {
        wobblePhases[i] += kWobbleRates[i] * invSr;
        if (wobblePhases[i] >= 1.0f)
            wobblePhases[i] -= 1.0f;

        float weight = kWowWeights[i] + morph * (kFlutterWeights[i] - kWowWeights[i]);
        modulation += weight * std::sin (wobblePhases[i] * juce::MathConstants<float>::twoPi);
    }

    // Linear scaling — original 0.006 depth works correctly with fixed 4-layer rates
    // (v0.8's "too extreme" was caused by delay-dependent rate, not depth)
    constexpr float maxDeviation = 0.006f;  // ±0.6% of delay time at max amount
    return baseDelaySamples * (1.0f + wobbleAmt * modulation * maxDeviation);
}

//==============================================================================
// v0.5: Shared inline helpers for render methods
//==============================================================================
float OpenSpatialDelayProcessor::readObjectSample (int objectIndex, float baseDelaySamples, float blockFraction)
{
    // v1.0.2: Doppler via delay line modulation — the standard artifact-free approach.
    // Accumulate per-sample delay offset from Doppler velocity. The rate of change
    // of the read position naturally produces pitch shift without PV artifacts.
    // PV is reserved for user pitch shift only (constant ratio = no grain-rate buzz).
    float prevDoppler = doppler.getPrevSmoothedSemitones (objectIndex);
    float curDoppler  = doppler.getSmoothedSemitones (objectIndex);
    float interpDoppler = prevDoppler + blockFraction * (curDoppler - prevDoppler);

    // delayRate: how much delay changes per sample. Negative when pitch up (approaching).
    // pow(2, semitones/12) is the playback speed ratio. (1 - ratio) gives delay accumulation rate.
    float delayRate = 1.0f - std::pow (2.0f, interpDoppler / 12.0f);
    dopplerDelayAccum[objectIndex] += delayRate;

    float objDelaySamples = static_cast<float> (objectIndex + 1) * baseDelaySamples
                          + dopplerDelayAccum[objectIndex];
    objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);

    // v0.9: Per-tap pitch via WSOLA-lite (timing-preserving)
    float perTapPitch = cachedObj[objectIndex].pitchShift->load (std::memory_order_relaxed);

    // v0.8: Per-tap input channel routing
    int inputCh = static_cast<int> (cachedObj[objectIndex].inputChannel->load (std::memory_order_relaxed));
    DelayChannel ch = (inputCh == 1) ? DelayChannel::Left : (inputCh == 2) ? DelayChannel::Right : DelayChannel::Mono;

    // v0.8: Channel-aware delay line read (now with Doppler-modulated position)
    float objMono = readDelayLineMono (objDelaySamples);
    if (ch == DelayChannel::Left)       objMono = readDelayLineL (objDelaySamples);
    else if (ch == DelayChannel::Right) objMono = readDelayLineR (objDelaySamples);

    // v1.0.6: Phase vocoder pitch shift — user pitch only, no Doppler (issue #77)
    objMono = pvPitchShifters[objectIndex].process (objMono, perTapPitch);
    // v1.0.1: Air absorption + tap filters via FilterBank class
    float result = filters.processAirSample (objectIndex, objMono);
    result = filters.processTapSample (objectIndex, result);

    // v0.7: Accumulate per-tap peak for UI activity glow
    float absVal = std::abs (result);
    if (absVal > tapPeakAccum[objectIndex])
        tapPeakAccum[objectIndex] = absVal;

    return result;
}

void OpenSpatialDelayProcessor::processFeedbackSample (float currentLoopMult,
                                                        float baseDelaySamples,
                                                        float fb)
{
    float fbDelaySamples = currentLoopMult * baseDelaySamples;
    fbDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), fbDelaySamples);

    float feedbackRaw;
    // v1.0.8: Crossfade feedback between old and new positions on loop multiplier change (issue #65)
    if (fbCrossfadeProgress < 1.0f)
    {
        float oldFbDelay = prevLoopMultiplier * baseDelaySamples;
        oldFbDelay = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), oldFbDelay);

        float oldSample = readDelayLineMono (oldFbDelay);
        float newSample = readDelayLineMono (fbDelaySamples);
        feedbackRaw = oldSample + fbCrossfadeProgress * (newSample - oldSample);
        fbCrossfadeProgress = std::min (fbCrossfadeProgress + fbCrossfadeIncrement, 1.0f);
    }
    else
    {
        feedbackRaw = readDelayLineMono (fbDelaySamples);
    }
    float filtered = filters.processFeedbackSample (feedbackRaw);
    const float makeupGain = 1.0f + (fb * fb * kMakeupGainCoeff);
    float newFeedback = softClip (filtered * makeupGain);
    // v1.0.5: One-pole lowpass on feedback signal prevents the feedback loop from
    // amplifying HRTF crossfade transitions into audible pops (issue #50).
    // Alpha = 0.15 gives ~1.8ms smoothing at 48kHz — fast enough for musical delay
    // response, slow enough to attenuate the block-rate crossfade transients.
    constexpr float kFeedbackSmoothAlpha = 0.3f;
    feedbackSample += kFeedbackSmoothAlpha * (newFeedback - feedbackSample);
    if (! std::isfinite (feedbackSample))
    {
        feedbackSample = 0.0f;
        filters.resetFeedback();  // v1.0.2: only reset feedback filters, not all 38
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

    if (delayBufferL.empty() || numSamples <= 0)
        return;

    // v1.0.8: Transport-aware PV reset — clear stale phase state on transport
    // stop→play, seek, or position jump to prevent intermittent buzzing (issue #65).
    // The PV maintains phase/magnitude arrays across blocks; stale data after a
    // transport discontinuity can cause phase coherence errors that sound like buzz.
    {
        auto* playHead = getPlayHead();
        if (playHead != nullptr)
        {
            auto pos = playHead->getPosition();
            if (pos.hasValue())
            {
                bool isPlaying = pos->getIsPlaying();
                juce::int64 currentSample = pos->getTimeInSamples().orFallback (-1);

                bool transportStarted = (! wasPlaying && isPlaying);
                bool transportJumped  = (isPlaying && currentSample >= 0
                                         && std::abs (currentSample - expectedNextSample) > numSamples);

                if (transportStarted || transportJumped)
                {
                    for (int i = 0; i < MAX_OBJECTS; ++i)
                    {
                        pvPitchShifters[i].reset();
                        dopplerDelayAccum[i] = 0.0f;
                    }

                    // v1.0.6: Mute output and fade back in over 5ms to mask
                    // delay buffer discontinuity after scrub/seek (issue #103)
                    transportFadeGain = 0.0f;
                    transportFadeActive = true;

                    // E18: Snap delay time to correct value on transport start.
                    // Without this, smoothedDelayTime ramps from its stale value
                    // over 100ms, sweeping the read position through old buffer
                    // content (audible chirp at tempos other than the previous one).
                    {
                        bool tsync = cachedParam_tempoSync->load() > 0.5f;
                        if (tsync)
                        {
                            int ndiv = static_cast<int> (cachedParam_noteDivision->load());
                            smoothedDelayTime.setCurrentAndTargetValue (getTempoSyncedDelayMs (ndiv));
                        }
                        else
                        {
                            smoothedDelayTime.setCurrentAndTargetValue (cachedParam_delayTime->load());
                        }
                    }
                }

                wasPlaying = isPlaying;
                expectedNextSample = currentSample + numSamples;
            }
        }
    }

    // v1.0.1: Preset transition — fade-out/reconfigure/fade-in (issue #84).
    // Instead of resetting state immediately (which causes chirp through feedback),
    // trigger a fade-out. The actual reset happens at zero gain in the output stage.
    if (presetResetPending.load (std::memory_order_acquire)
        && presetTransitionState != PresetTransitionState::FadeOut)
    {
        presetTransitionState = PresetTransitionState::FadeOut;
    }

    // v0.7: Reset per-tap peak accumulators for this block
    for (int i = 0; i < MAX_OBJECTS; ++i)
        tapPeakAccum[i] = 0.0f;

    // --- Read parameter values (cached pointers, no string lookups) -----------
    bool  tempoSync       = cachedParam_tempoSync->load() > 0.5f;
    int   noteDivision    = static_cast<int> (cachedParam_noteDivision->load());
    float lpFreq          = cachedParam_filterLP->load();
    float hpFreq          = cachedParam_filterHP->load();
    float filterHPQ       = cachedParam_filterHPQ->load();
    float filterLPQ       = cachedParam_filterLPQ->load();
    // v0.9: Filter bypass driven by dedicated parameter (not threshold inference)
    bool filterEnabled = cachedParam_filterEnabled->load() > 0.5f;
    int   profileIndex    = configHrtfProfile.load (std::memory_order_relaxed);

    // v0.4: Air absorption — true bypass with edge detection for immediate toggle response
    airAbsorptionActive = cachedParam_airAbsorption->load() > 0.5f;
    bool airStateChanged = (airAbsorptionActive != prevAirAbsorptionActive);
    prevAirAbsorptionActive = airAbsorptionActive;

    // v0.8: Wobble modulation (block-rate parameter reads, gated by wobbleEnabled)
    {
        bool wobbleEnabled = cachedParam_wobbleEnabled->load() > 0.5f;
        smoothedWobbleAmount.setTargetValue (wobbleEnabled ? (cachedParam_wobbleAmount->load() / 100.0f) : 0.0f);
        blockWobbleMorph  = cachedParam_wobbleMorph->load();
    }

    // v0.8: Input format (0=Mono, 1=Stereo) — selects whether dual delay lines receive L/R or summed mono
    int inputFormat = configInputFormat.load (std::memory_order_relaxed);

    // (#3) Calculate target base delay
    float targetBaseDelayMs;
    if (tempoSync)
        targetBaseDelayMs = getTempoSyncedDelayMs (noteDivision);
    else
        targetBaseDelayMs = cachedParam_delayTime->load();

    smoothedDelayTime.setTargetValue (targetBaseDelayMs);

    float dryWetTarget    = cachedParam_dryWet->load();
    float fbTarget        = cachedParam_feedback->load();
    float inGainDb        = cachedParam_inputGain->load();
    float outGainDb       = cachedParam_outputGain->load();

    smoothedDryWet.setTargetValue     (dryWetTarget);
    smoothedFeedback.setTargetValue   (fbTarget);
    smoothedInputGain.setTargetValue  (juce::Decibels::decibelsToGain (inGainDb));
    smoothedOutputGain.setTargetValue (juce::Decibels::decibelsToGain (outGainDb));

    // v1.0.1: Filter coefficient update — delegated to FilterBank class
    filters.updateCoefficients (currentSampleRate, lpFreq, hpFreq, filterLPQ, filterHPQ, filterEnabled);

    // --- Block-rate constant: ms → samples conversion factor -------------------
    blockMsToSamples = 0.001f * static_cast<float> (currentSampleRate);

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
    int algorithmIndex = configAlgorithm.load (std::memory_order_relaxed);
    auto* algo = algorithms[juce::jlimit (0, NUM_ALGORITHMS - 1, algorithmIndex)];

    // For surround output, fall back if algorithm doesn't support speakers
    if (! isBinaural && ! isStereoVariant && ! algo->supportsSurround())
        algo = algorithms[5];  // Fall back to VBAP (index 5 in alphabetical order)

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

    // --- v1.0.2: Tick trajectory on audio thread (issue #77) -----------------
    // Trajectory must advance in audio-time (not wall-clock) so that:
    //   1) Offline render produces the same orbit speed as realtime
    //   2) Position updates every audio block (eliminates 60Hz staircase buzz)
    {
        float blockDt = static_cast<float> (numSamples) / static_cast<float> (currentSampleRate);
        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            TrajectoryEngine::ObjectInput input;
            input.shape     = juce::roundToInt (cachedParam_trajectoryShape[t] != nullptr
                                                ? cachedParam_trajectoryShape[t]->load() : 0.0f);
            input.speed     = cachedParam_trajectorySpeed[t] != nullptr
                              ? cachedParam_trajectorySpeed[t]->load() : 1.0f;
            input.reverse   = (cachedParam_trajectoryDirection[t] == nullptr
                               || cachedParam_trajectoryDirection[t]->load() < 0.5f);
            input.originAz   = cachedObj[t].azimuth->load();
            input.originEl   = cachedObj[t].elevation->load();
            input.originDist = cachedObj[t].distance->load();
            input.oscOverride = oscOverrideActive[t].load (std::memory_order_relaxed);

            trajectory.tick (t, input, blockDt);
        }
    }

    // --- Read object states and pre-compute spatial gains -------------------
    ObjectState objects[MAX_OBJECTS];
    BinauralGains objGains[MAX_OBJECTS];                              // Simple Woodworth binaural
    float objChannelGains[MAX_OBJECTS][16] = {};                      // Discrete surround path
    float objDistGain[MAX_OBJECTS] = {};                              // Distance attenuation
    int lastEnabledObjectIndex = -1;

    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        // Read per-object state via cached pointers (no string lookups)
        objects[t].enabled      = cachedObj[t].enabled->load()   > 0.5f;

        tapFadeTarget[t] = objects[t].enabled ? 1.0f : 0.0f;

        // v0.9: When trajectory is active, read animated position from internal arrays
        // (the knobs/APVTS hold the origin; the internal arrays hold origin + trajectory offset)
        if (trajectory.isActive (t))
        {
            objects[t].azimuthDeg   = trajectory.getFinalAz (t);
            objects[t].elevationDeg = trajectory.getFinalEl (t);
            objects[t].distance     = trajectory.getFinalDist (t);
        }
        else
        {
            objects[t].azimuthDeg   = cachedObj[t].azimuth->load();
            objects[t].elevationDeg = cachedObj[t].elevation->load();
            objects[t].distance     = cachedObj[t].distance->load();
        }

        if (objects[t].enabled || tapFadeGain[t] > 0.0f)
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

    // --- v1.0.1: Per-block Doppler velocity + air absorption filter update ---
    // Doppler tracking extracted to DopplerVelocity class for testability.
    {
        float blockDuration = static_cast<float> (numSamples) / static_cast<float> (currentSampleRate);

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled && tapFadeGain[t] <= 0.0f)
            {
                doppler.clearDisabled (t);
                dopplerDelayAccum[t] = 0.0f;
                filters.clearAirAbsorption (t);
                continue;
            }

            float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
            float elRad = juce::degreesToRadians (objects[t].elevationDeg);
            float dist  = objects[t].distance;

            float objDopplerAmount = cachedObj[t].dopplerAmount->load();
            if (objDopplerAmount < 0.001f)
                dopplerDelayAccum[t] = 0.0f;  // Reset when Doppler disabled
            doppler.update (t, azRad, elRad, dist, objDopplerAmount, blockDuration);
            doppler.smooth (t);

            bool positionChanged = doppler.positionChanged (t);

            // v1.0.1: Air absorption — delegated to FilterBank class
            filters.updateAirAbsorption (t, currentSampleRate, dist,
                                          airAbsorptionActive, positionChanged, airStateChanged);
        }
    }

    // --- Prepare input buffers ------------------------------------------------
    auto ns = static_cast<size_t> (numSamples);
    if (monoInputBuffer.size() < ns)  monoInputBuffer.resize (ns);
    if (inputBufferL.size() < ns)     inputBufferL.resize (ns);
    if (inputBufferR.size() < ns)     inputBufferR.resize (ns);

    if (numInputChannels == 1)
    {
        auto* in = buffer.getReadPointer (0);
        for (int i = 0; i < numSamples; ++i)
        {
            monoInputBuffer[static_cast<size_t> (i)] = in[i];
            inputBufferL[static_cast<size_t> (i)] = in[i];
            inputBufferR[static_cast<size_t> (i)] = in[i];
        }
    }
    else
    {
        auto* inL = buffer.getReadPointer (0);
        auto* inR = buffer.getReadPointer (1);
        if (inputFormat == 1)  // Stereo: keep L and R separate
        {
            for (int i = 0; i < numSamples; ++i)
            {
                inputBufferL[static_cast<size_t> (i)] = inL[i];
                inputBufferR[static_cast<size_t> (i)] = inR[i];
                monoInputBuffer[static_cast<size_t> (i)] = (inL[i] + inR[i]) * 0.5f;
            }
        }
        else  // Mono: sum to mono, write same signal to both
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float mono = (inL[i] + inR[i]) * 0.5f;
                monoInputBuffer[static_cast<size_t> (i)] = mono;
                inputBufferL[static_cast<size_t> (i)] = mono;
                inputBufferR[static_cast<size_t> (i)] = mono;
            }
        }
    }

    // v1.0.7: Fill latency-compensated dry buffer (issue #63)
    // The phase vocoder adds kFFTSize samples of latency to the wet path.
    // Delay the dry signal by the same amount so DAW PDC is correct at all dry/wet levels.
    // v1.0.1: Stereo dry path — reads raw DAW input, bypasses input selector (issue #73)
    // The Input selector only affects the wet (delay) path. The dry signal is a true bypass
    // of whatever the DAW sends, affected only by the Output knob.
    if (dryCompBufferL.size() < ns) dryCompBufferL.resize (ns);
    if (dryCompBufferR.size() < ns) dryCompBufferR.resize (ns);
    {
        auto* rawInL = buffer.getReadPointer (0);
        auto* rawInR = (numInputChannels > 1) ? buffer.getReadPointer (1) : buffer.getReadPointer (0);
        const int dryDelaySize = static_cast<int> (dryDelayLineL.size());
        for (int i = 0; i < numSamples; ++i)
        {
            auto si = static_cast<size_t> (i);
            auto wp = static_cast<size_t> (dryDelayWritePos);
            dryCompBufferL[si] = dryDelayLineL[wp];
            dryCompBufferR[si] = dryDelayLineR[wp];
            dryDelayLineL[wp] = rawInL[i];
            dryDelayLineR[wp] = rawInR[i];
            dryDelayWritePos = (dryDelayWritePos + 1) % dryDelaySize;
        }
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
    // v1.0.8: When the multiplier changes, start a crossfade between old and new feedback
    // read positions instead of sweeping — sweeping causes a Doppler chirp (issue #65).
    float loopMultiplierTarget = static_cast<float>(std::max (1, lastEnabledObjectIndex + 1));
    if (loopMultiplierTarget != smoothedLoopMultiplier.getTargetValue())
    {
        prevLoopMultiplier = smoothedLoopMultiplier.getCurrentValue();
        fbCrossfadeProgress = 0.0f;
    }
    smoothedLoopMultiplier.setCurrentAndTargetValue (loopMultiplierTarget);

    // v1.0.1: At zero-gain, reconfigure everything cleanly (issue #84).
    // All smoother targets and per-object state have been set above, so snapping
    // smoothers here gives correct new-preset values for the upcoming render.
    if (presetTransitionState == PresetTransitionState::FadeOut
        && presetTransitionGain <= 0.0f)
    {
        // Clear delay buffers — old audio is stale for the new preset
        std::fill (delayBufferL.begin(), delayBufferL.end(), 0.0f);
        std::fill (delayBufferR.begin(), delayBufferR.end(), 0.0f);
        feedbackSample = 0.0f;

        // Doppler reset
        for (int i = 0; i < MAX_OBJECTS; ++i)
        {
            doppler.reset (i, pendingReset.prevAz[i], pendingReset.prevEl[i], pendingReset.prevDist[i]);
            dopplerDelayAccum[i] = 0.0f;
        }

        // Filter + HRTF reset (safe at zero gain — no audible transient)
        filters.resetAll();
        auto& rend = binauralRenderers[activeRendererIndex.load (std::memory_order_acquire)];
        rend.invalidateSources();

        // Reset pitch shifters — stale phase/accumulator state causes chirp (issue #99)
        for (int i = 0; i < MAX_OBJECTS; ++i)
            pvPitchShifters[i].reset();

        // Snap ALL smoothers to target — no ramps during fade-in
        smoothedDelayTime.setCurrentAndTargetValue (smoothedDelayTime.getTargetValue());
        smoothedDryWet.setCurrentAndTargetValue (smoothedDryWet.getTargetValue());
        smoothedFeedback.setCurrentAndTargetValue (smoothedFeedback.getTargetValue());
        smoothedInputGain.setCurrentAndTargetValue (smoothedInputGain.getTargetValue());
        smoothedOutputGain.setCurrentAndTargetValue (smoothedOutputGain.getTargetValue());
        smoothedLoopMultiplier.setCurrentAndTargetValue (smoothedLoopMultiplier.getTargetValue());
        smoothedWobbleAmount.setCurrentAndTargetValue (smoothedWobbleAmount.getTargetValue());

        // Snap tap fades + feedback crossfade
        for (int t = 0; t < MAX_OBJECTS; ++t)
            tapFadeGain[t] = tapFadeTarget[t];
        fbCrossfadeProgress = 1.0f;

        presetResetPending.store (false, std::memory_order_release);
        presetTransitionState = PresetTransitionState::FadeIn;
    }

    // v1.0.1: Pre-advance dryWet / outputGain smoothers into per-sample arrays (issue #73)
    // Render paths now output raw wet signal; dry/wet mix happens once after dispatch.
    if (perSampleDW.size() < ns)       perSampleDW.resize (ns);
    if (perSampleOutGain.size() < ns)  perSampleOutGain.resize (ns);
    for (int s = 0; s < numSamples; ++s)
    {
        perSampleDW[static_cast<size_t> (s)]       = smoothedDryWet.getNextValue();
        perSampleOutGain[static_cast<size_t> (s)]  = smoothedOutputGain.getNextValue();
    }

    // --- Dispatch to appropriate render method --------------------------------
    // Render paths write RAW WET signal to the output buffer (no dry, no dw, no outGain, no limiter).
    if (isStereoVariant)
    {
        // Stereo mode from algorithm parameter: indices 6-10 → modes 0-4
        int stereoMode = juce::jlimit (7, 11, algorithmIndex) - 7;
        renderStereoVariant (buffer, numSamples, objects, objDistGain,
                             stereoMode);
    }
    else if (isBinaural && useHRTF)
    {
        renderDirectBinauralHRTF (buffer, numSamples, objects, objDistGain);
    }
    else if (isBinaural)
    {
        renderSimpleBinauralWoodworth (buffer, numSamples, objects, objGains);
    }
    else if (isAmbiOutput)
    {
        const int ambiOrder = outputFormatRegistry[static_cast<size_t> (fmtEnumIdx)].ambiOrder;
        renderAmbisonicsOutput (buffer, numSamples, objects, objDistGain, ambiOrder);
    }
    else
    {
        renderDiscreteSurround (buffer, numSamples, objects, objChannelGains,
                                objDistGain, surLayout);
    }

    // === v1.0.1: Single-point dry/wet mix with equal-power crossfade (issue #73) ===
    // The dry signal bypasses the entire plugin; only dryWet and outputGain affect it.
    // Equal-power (cos/sin) crossfade maintains constant perceived loudness at all dw settings.
    {
        const int numOutCh = buffer.getNumChannels();
        constexpr int kMaxPostRenderCh = 64;  // Covers surround (16), ambi (49), and any DAW padding
        float* outPtrs[kMaxPostRenderCh] = {};
        const int usableCh = std::min (numOutCh, kMaxPostRenderCh);
        for (int ch = 0; ch < usableCh; ++ch)
            outPtrs[ch] = buffer.getWritePointer (ch);

        for (int s = 0; s < numSamples; ++s)
        {
            auto si = static_cast<size_t> (s);

            // v1.0.1: Advance preset transition envelope (issue #84)
            float tGain = 1.0f;
            if (presetTransitionState == PresetTransitionState::FadeOut)
            {
                presetTransitionGain = std::max (0.0f, presetTransitionGain - presetTransitionStep);
                tGain = presetTransitionGain;
            }
            else if (presetTransitionState == PresetTransitionState::FadeIn)
            {
                presetTransitionGain = std::min (1.0f, presetTransitionGain + presetTransitionStep);
                tGain = presetTransitionGain;
                if (presetTransitionGain >= 1.0f)
                    presetTransitionState = PresetTransitionState::Idle;
            }

            // v1.0.6: Transport fade-in after scrub/seek (issue #103)
            if (transportFadeActive)
            {
                transportFadeGain = std::min (1.0f, transportFadeGain + transportFadeStep);
                tGain *= transportFadeGain;
                if (transportFadeGain >= 1.0f)
                    transportFadeActive = false;
            }

            float dw       = perSampleDW[si];
            float outGain  = perSampleOutGain[si];
            float dryCoeff = std::cos (dw * juce::MathConstants<float>::halfPi);
            float wetCoeff = std::sin (dw * juce::MathConstants<float>::halfPi);

            if (isAmbiOutput)
            {
                // Ambisonics: mono dry → W channel (ACN 0), wet → all SH channels
                // v1.0.1: Limiter on wet only — dry passes through at unity (issue #97)
                float dryMono = (dryCompBufferL[si] + dryCompBufferR[si]) * 0.5f;
                if (outPtrs[0])
                    outPtrs[0][s] = (outputLimiter (outPtrs[0][s] * wetCoeff * outGain) + dryMono * dryCoeff * outGain) * tGain;
                for (int ch = 1; ch < usableCh; ++ch)
                    if (outPtrs[ch])
                        outPtrs[ch][s] = outputLimiter (outPtrs[ch][s] * wetCoeff * outGain) * tGain;
            }
            else
            {
                // Binaural / Stereo / Surround: stereo dry → L/R (ch 0/1)
                // v1.0.1: Limiter on wet only — dry passes through at unity (issue #97)
                if (outPtrs[0])
                    outPtrs[0][s] = (outputLimiter (outPtrs[0][s] * wetCoeff * outGain) + dryCompBufferL[si] * dryCoeff * outGain) * tGain;
                if (usableCh > 1 && outPtrs[1])
                    outPtrs[1][s] = (outputLimiter (outPtrs[1][s] * wetCoeff * outGain) + dryCompBufferR[si] * dryCoeff * outGain) * tGain;
                // Remaining channels (surround speakers, LFE): wet only
                for (int ch = 2; ch < usableCh; ++ch)
                    if (outPtrs[ch])
                        outPtrs[ch][s] = outputLimiter (outPtrs[ch][s] * wetCoeff * outGain) * tGain;
            }
        }
    }

    // v0.7: Store per-tap peak to atomics for UI glow
    for (int i = 0; i < MAX_OBJECTS; ++i)
        tapActivityRMS[i].store (tapPeakAccum[i], std::memory_order_relaxed);
}

//==============================================================================
// v0.5: Extracted render methods (identical behavior to v0.4 processBlock paths)
//==============================================================================

void OpenSpatialDelayProcessor::renderDirectBinauralHRTF (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const float* objDistGain)
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

    int currentActiveIdx = activeRendererIndex.load (std::memory_order_acquire);
    auto& activeRenderer = binauralRenderers[currentActiveIdx];

    // v1.0.4: Detect renderer swap → start crossfade (issue #90).
    // The new convolver's overlap buffer is empty on first use, causing a
    // one-block transient overshoot. Crossfading old→new masks this ramp-up.
    if (currentActiveIdx != prevActiveRendererIdx_ && ! rendererXfading_)
    {
        rendererXfading_ = true;
        rendererXfadeActive_.store (true, std::memory_order_release);
        rendererXfadeBlockCount_ = 0;
        rendererXfadeFromIdx_ = prevActiveRendererIdx_;
        prevRxFadeOut_ = 1.0f;
        prevRxFadeIn_ = 0.0f;
        prevActiveRendererIdx_ = currentActiveIdx;
    }

    // Update per-source HRIRs at block boundary for any taps that moved
    // (new renderer only — old renderer keeps its existing HRIRs during crossfade)
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        if (objects[t].enabled)
        {
            float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
            float elRad = juce::degreesToRadians (objects[t].elevationDeg);
            activeRenderer.updateSourceHRIR (t, azRad, elRad);
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

    // v1.0: Per-sample distGain interpolation to prevent clicks on rapid position changes
    float hrtfInvN = (numSamples > 1) ? 1.0f / static_cast<float> (numSamples - 1) : 1.0f;

    // === PASS 1: Per-sample delay engine → per-source accumulation ===
    for (int s = 0; s < numSamples; ++s)
    {
        float frac = static_cast<float> (s) * hrtfInvN;

        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples, currentDelayMs);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain = smoothedInputGain.getNextValue();
        float fb     = smoothedFeedback.getNextValue();
        // v1.0.1: dryWet/outputGain smoothers pre-advanced in processBlock (issue #73)

        // STAGE 1: WRITE to delay line (dual L/R)
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);
        // rawInput not needed here — PASS 3 reads monoInputBuffer directly

        // STAGE 2: READ & ACCUMULATE (per-source mono × interpolated distance gain)
        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            // v1.0: Per-tap fade envelope — prevents clicks on enable/disable transitions
            if (tapFadeGain[t] < tapFadeTarget[t])
                tapFadeGain[t] = std::min (tapFadeGain[t] + tapFadeIncrement, 1.0f);
            else if (tapFadeGain[t] > tapFadeTarget[t])
                tapFadeGain[t] = std::max (tapFadeGain[t] - tapFadeIncrement, 0.0f);
            // v1.0.8: Always feed PV to keep it in sync — prevents chirp on tap enable (issue #65)
            float objMono = readObjectSample (t, baseDelaySamples, static_cast<float> (s) / static_cast<float> (numSamples));
            if (tapFadeGain[t] <= 0.0f) continue;
            float dist = prevDistGain[t] + frac * (objDistGain[t] - prevDistGain[t]);
            sourceAccumBufPtrs[t][s] = objMono * dist * tapFadeGain[t];
        }

        // STAGE 3: FEEDBACK (mono, pre-spatial)
        processFeedbackSample (currentLoopMult, baseDelaySamples, fb);
    }

    // Store current distGain as previous for next block
    for (int t = 0; t < MAX_OBJECTS; ++t)
        prevDistGain[t] = objDistGain[t];

    // === PASS 2: Per-block per-source HRTF convolution → wet L/R ===
    bool sourceEnabled[MAX_OBJECTS];
    for (int t = 0; t < MAX_OBJECTS; ++t)
        sourceEnabled[t] = (tapFadeGain[t] > 0.0f);

    const float* srcBufPtrs[MAX_OBJECTS];
    for (int t = 0; t < MAX_OBJECTS; ++t)
        srcBufPtrs[t] = sourceAccumBufPtrs[t];

    activeRenderer.renderSourceBuffers (srcBufPtrs, sourceEnabled, MAX_OBJECTS,
                                        numSamples, wetBufL.data(), wetBufR.data());

    // v1.0.4: Renderer-level crossfade (issue #90).
    // Blend old renderer's output (full overlap → steady state) with new renderer's
    // output (ramping up from empty overlap) using equal-power cos/sin envelope.
    if (rendererXfading_)
    {
        auto ns = static_cast<size_t> (numSamples);
        if (xfadeWetL_.size() < ns) { xfadeWetL_.resize (ns, 0.0f); xfadeWetR_.resize (ns, 0.0f); }

        auto& oldRenderer = binauralRenderers[rendererXfadeFromIdx_];
        oldRenderer.renderSourceBuffers (srcBufPtrs, sourceEnabled, MAX_OBJECTS,
                                          numSamples, xfadeWetL_.data(), xfadeWetR_.data());

        ++rendererXfadeBlockCount_;
        float progress = static_cast<float> (rendererXfadeBlockCount_)
                       / static_cast<float> (kRendererXfadeBlocks);
        if (progress > 1.0f) progress = 1.0f;

        constexpr float halfPi = juce::MathConstants<float>::halfPi;
        float fadeOutGain = std::cos (progress * halfPi);
        float fadeInGain  = std::sin (progress * halfPi);

        float fadeOutInc = (fadeOutGain - prevRxFadeOut_) / static_cast<float> (numSamples);
        float fadeInInc  = (fadeInGain  - prevRxFadeIn_)  / static_cast<float> (numSamples);
        float gOut = prevRxFadeOut_;
        float gIn  = prevRxFadeIn_;

        for (int i = 0; i < numSamples; ++i)
        {
            gOut += fadeOutInc;
            gIn  += fadeInInc;
            wetBufL[static_cast<size_t> (i)] = xfadeWetL_[static_cast<size_t> (i)] * gOut
                                             + wetBufL[static_cast<size_t> (i)]     * gIn;
            wetBufR[static_cast<size_t> (i)] = xfadeWetR_[static_cast<size_t> (i)] * gOut
                                             + wetBufR[static_cast<size_t> (i)]     * gIn;
        }

        prevRxFadeOut_ = fadeOutGain;
        prevRxFadeIn_  = fadeInGain;

        if (rendererXfadeBlockCount_ >= kRendererXfadeBlocks)
        {
            rendererXfading_ = false;
            rendererXfadeActive_.store (false, std::memory_order_release);
        }
    }

    // === PASS 3: Write raw wet signal to output (dry/wet mix handled in processBlock) ===
    std::memcpy (outL, wetBufL.data(), sizeof (float) * static_cast<size_t> (numSamples));
    std::memcpy (outR, wetBufR.data(), sizeof (float) * static_cast<size_t> (numSamples));

    // Zero remaining channels when binaural is selected on a multi-channel bus
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

//------------------------------------------------------------------------------
void OpenSpatialDelayProcessor::renderSimpleBinauralWoodworth (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const BinauralGains* objGains)
{
    // =========================================================================
    // SIMPLE (WOODWORTH) BINAURAL PATH — per-sample rendering
    // Used when profile = "Simple (Low CPU)" — no HRTF convolution
    // Always uses DirectBinauralAlgorithm (Woodworth ITD+ILD)
    // =========================================================================
    auto* outL = buffer.getWritePointer (0);
    auto* outR = buffer.getWritePointer (1);

    // v1.0: Per-sample gain interpolation to prevent clicks on rapid position changes
    float invN = (numSamples > 1) ? 1.0f / static_cast<float> (numSamples - 1) : 1.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float frac = static_cast<float> (s) * invN;

        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples, currentDelayMs);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        // v1.0.1: dryWet/outputGain smoothers pre-advanced in processBlock (issue #73)

        // === STAGE 1: WRITE (dual L/R) ===
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);

        // === STAGE 2: READ & SPATIALIZE ===
        float wetL = 0.0f, wetR = 0.0f;

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            // v1.0: Per-tap fade envelope — prevents clicks on enable/disable transitions
            if (tapFadeGain[t] < tapFadeTarget[t])
                tapFadeGain[t] = std::min (tapFadeGain[t] + tapFadeIncrement, 1.0f);
            else if (tapFadeGain[t] > tapFadeTarget[t])
                tapFadeGain[t] = std::max (tapFadeGain[t] - tapFadeIncrement, 0.0f);
            // v1.0.8: Always feed PV to keep it in sync — prevents chirp on tap enable (issue #65)
            float objMono = readObjectSample (t, baseDelaySamples, static_cast<float> (s) / static_cast<float> (numSamples));
            if (tapFadeGain[t] <= 0.0f) continue;

            // Interpolate between previous and current block gains
            float gL = prevBinauralGains[t].leftGain  + frac * (objGains[t].leftGain  - prevBinauralGains[t].leftGain);
            float gR = prevBinauralGains[t].rightGain + frac * (objGains[t].rightGain - prevBinauralGains[t].rightGain);
            wetL += objMono * gL * tapFadeGain[t];
            wetR += objMono * gR * tapFadeGain[t];
        }

        // === STAGE 3: FEEDBACK ===
        processFeedbackSample (currentLoopMult, baseDelaySamples, fb);

        // === STAGE 4: OUTPUT — raw wet (dry/wet mix handled in processBlock) ===
        outL[s] = wetL;
        outR[s] = wetR;
    }

    // Store current gains as previous for next block
    for (int t = 0; t < MAX_OBJECTS; ++t)
        prevBinauralGains[t] = objGains[t];

    // Zero remaining channels when binaural is selected on a multi-channel bus
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

//------------------------------------------------------------------------------
void OpenSpatialDelayProcessor::renderStereoVariant (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const float* objDistGain,
    int stereoMode)
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
        if (! objects[t].enabled && tapFadeGain[t] <= 0.0f) continue;

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
            case 3:  // MS Encode — Subcardioid Mid + Figure-8 Side, L = M+S, R = M-S
            {
                float mid  = (0.75f + 0.25f * std::cos (azRad)) * dG;
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

    // v1.0: Per-sample gain interpolation to prevent clicks on rapid position changes
    float invN = (numSamples > 1) ? 1.0f / static_cast<float> (numSamples - 1) : 1.0f;

    // Per-sample processing (same engine as Woodworth, with stereo gains instead of binaural)
    for (int s = 0; s < numSamples; ++s)
    {
        float frac = static_cast<float> (s) * invN;

        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples, currentDelayMs);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        // v1.0.1: dryWet/outputGain smoothers pre-advanced in processBlock (issue #73)

        // === STAGE 1: WRITE (dual L/R) ===
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);

        // === STAGE 2: READ & SPATIALIZE ===
        float wetL = 0.0f, wetR = 0.0f;

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            // v1.0: Per-tap fade envelope — prevents clicks on enable/disable transitions
            if (tapFadeGain[t] < tapFadeTarget[t])
                tapFadeGain[t] = std::min (tapFadeGain[t] + tapFadeIncrement, 1.0f);
            else if (tapFadeGain[t] > tapFadeTarget[t])
                tapFadeGain[t] = std::max (tapFadeGain[t] - tapFadeIncrement, 0.0f);
            // v1.0.8: Always feed PV to keep it in sync — prevents chirp on tap enable (issue #65)
            float objMono = readObjectSample (t, baseDelaySamples, static_cast<float> (s) / static_cast<float> (numSamples));
            if (tapFadeGain[t] <= 0.0f) continue;

            // Interpolate between previous and current block gains
            float gL = prevStereoGainL[t] + frac * (objGainL[t] - prevStereoGainL[t]);
            float gR = prevStereoGainR[t] + frac * (objGainR[t] - prevStereoGainR[t]);
            wetL += objMono * gL * tapFadeGain[t];
            wetR += objMono * gR * tapFadeGain[t];
        }

        // === STAGE 3: FEEDBACK ===
        processFeedbackSample (currentLoopMult, baseDelaySamples, fb);

        // === STAGE 4: OUTPUT — raw wet (dry/wet mix handled in processBlock) ===
        outL[s] = wetL;
        outR[s] = wetR;
    }

    // Store current gains as previous for next block
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        prevStereoGainL[t] = objGainL[t];
        prevStereoGainR[t] = objGainR[t];
    }

    // Zero remaining channels when stereo is selected on a multi-channel bus
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

//------------------------------------------------------------------------------
void OpenSpatialDelayProcessor::renderAmbisonicsOutput (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const float* objDistGain,
    int ambiOrder)
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
    // v0.7: Cached — only recomputed when ambiOrder changes (rare: output format switch)
    if (ambiOrder != cachedMaxrEOrder)
    {
        const float maxrEDenom = 2.0f * static_cast<float> (ambiOrder) + 2.0f;
        for (int n = 0; n <= ambiOrder; ++n)
            cachedMaxrE[n] = std::cos (static_cast<float> (n) * juce::MathConstants<float>::pi / maxrEDenom);
        cachedMaxrEOrder = ambiOrder;
    }

    // --- NFC-HOA: Update filter coefficients when distance changes (block-rate) ---
    // v1.0.1: EMA-smooth distance to prevent IIR coefficient transients on rapid
    // distance changes. 72 filters (12 objects × 6 orders) updating without smoothing
    // can cause audible artifacts. Alpha 0.15 gives ~3-block settling.
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        if (! objects[t].enabled && tapFadeGain[t] <= 0.0f) continue;
        float distMeters = objects[t].distance * 10.0f;  // 0..1 → 0..10m

        // Always track smoothed distance (even below coefficient update threshold)
        constexpr float nfcSmoothAlpha = 0.15f;
        smoothedNfcDistance[t] += nfcSmoothAlpha * (distMeters - smoothedNfcDistance[t]);

        // Only recompute coefficients when smoothed distance differs enough
        if (std::abs (smoothedNfcDistance[t] - prevNfcDistance[t]) > 0.01f)
        {
            for (int ord = 0; ord < ambiOrder && ord < MAX_AMBI_ORDER; ++ord)
            {
                int n = ord + 1;  // SH order (1-based)
                constexpr float c = 343.0f;  // speed of sound, m/s
                float r = std::max (0.05f, smoothedNfcDistance[t]);
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
            prevNfcDistance[t] = smoothedNfcDistance[t];
        }
    }

    // Pre-compute SH coefficients per enabled tap (block-rate — positions fixed within block)
    float objSHCoeffs[MAX_OBJECTS][MAX_AMBI_CHANNELS] = {};
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        if (! objects[t].enabled && tapFadeGain[t] <= 0.0f) continue;
        float azRad = juce::degreesToRadians (objects[t].azimuthDeg);
        float elRad = juce::degreesToRadians (objects[t].elevationDeg);
        for (int c = 0; c < numAmbiCh; ++c)
            objSHCoeffs[t][c] = evalSH (c, azRad, elRad) * cachedMaxrE[acnToOrder (c)];
    }

    // Get output channel write pointers
    float* outChannels[MAX_AMBI_CHANNELS] = {};
    int numOutCh = buffer.getNumChannels();
    for (int ch = 0; ch < numOutCh && ch < MAX_AMBI_CHANNELS; ++ch)
        outChannels[ch] = buffer.getWritePointer (ch);

    // v1.0: Per-sample SH coefficient interpolation to prevent clicks on rapid position changes
    float invN = (numSamples > 1) ? 1.0f / static_cast<float> (numSamples - 1) : 1.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float frac = static_cast<float> (s) * invN;

        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples, currentDelayMs);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        // v1.0.1: dryWet/outputGain smoothers pre-advanced in processBlock (issue #73)

        // === STAGE 1: WRITE (dual L/R) ===
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);

        // === STAGE 2: READ & SH ENCODE (with NFC-HOA) ===
        float ambiAccum[MAX_AMBI_CHANNELS] = {};

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            // v1.0: Per-tap fade envelope — prevents clicks on enable/disable transitions
            if (tapFadeGain[t] < tapFadeTarget[t])
                tapFadeGain[t] = std::min (tapFadeGain[t] + tapFadeIncrement, 1.0f);
            else if (tapFadeGain[t] > tapFadeTarget[t])
                tapFadeGain[t] = std::max (tapFadeGain[t] - tapFadeIncrement, 0.0f);
            // v1.0.8: Always feed PV to keep it in sync — prevents chirp on tap enable (issue #65)
            float objMono = readObjectSample (t, baseDelaySamples, static_cast<float> (s) / static_cast<float> (numSamples));
            if (tapFadeGain[t] <= 0.0f) continue;

            // Interpolate distance gain between previous and current block
            float dist = prevDistGain[t] + frac * (objDistGain[t] - prevDistGain[t]);
            float scaledMono = objMono * dist * tapFadeGain[t];

            // Order 0 (W channel): no NFC needed — interpolate SH coeff
            float sh0 = prevSHCoeffs[t][0] + frac * (objSHCoeffs[t][0] - prevSHCoeffs[t][0]);
            ambiAccum[0] += scaledMono * sh0;

            // Orders 1+: apply NFC-HOA per-order shelf filter — interpolate SH coeffs
            for (int ord = 0; ord < ambiOrder && ord < MAX_AMBI_ORDER; ++ord)
            {
                float nfcMono = nfcFilters[t][ord].processSample (scaledMono);
                int startACN = (ord + 1) * (ord + 1);
                int endACN = (ord + 2) * (ord + 2);
                for (int c = startACN; c < endACN && c < numAmbiCh; ++c)
                {
                    float shc = prevSHCoeffs[t][c] + frac * (objSHCoeffs[t][c] - prevSHCoeffs[t][c]);
                    ambiAccum[c] += nfcMono * shc;
                }
            }
        }

        // === STAGE 3: FEEDBACK ===
        processFeedbackSample (currentLoopMult, baseDelaySamples, fb);

        // === STAGE 4: OUTPUT — raw wet (dry/wet mix handled in processBlock) ===
        for (int c = 0; c < numAmbiCh && c < numOutCh; ++c)
        {
            if (outChannels[c] != nullptr)
                outChannels[c][s] = ambiAccum[c];
        }
    }

    // Store current SH coefficients and distance gains as previous for next block
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        for (int c = 0; c < numAmbiCh; ++c)
            prevSHCoeffs[t][c] = objSHCoeffs[t][c];
        prevDistGain[t] = objDistGain[t];
    }
}

//------------------------------------------------------------------------------
void OpenSpatialDelayProcessor::renderDiscreteSurround (
    juce::AudioBuffer<float>& buffer, int numSamples,
    const ObjectState* objects, const float (*objChannelGains)[16],
    const float* objDistGain,
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

    // v1.0: Per-sample gain interpolation to prevent clicks on rapid position changes
    float invN = (numSamples > 1) ? 1.0f / static_cast<float> (numSamples - 1) : 1.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        float frac = static_cast<float> (s) * invN;

        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples, currentDelayMs);
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        // v1.0.1: dryWet/outputGain smoothers pre-advanced in processBlock (issue #73)

        // === STAGE 1: WRITE (dual L/R) ===
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);

        // === STAGE 2: READ & SPATIALIZE ===
        float channelAccum[16] = {};
        float wetMono = 0.0f;  // For LFE generation

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            // v1.0: Per-tap fade envelope — prevents clicks on enable/disable transitions
            if (tapFadeGain[t] < tapFadeTarget[t])
                tapFadeGain[t] = std::min (tapFadeGain[t] + tapFadeIncrement, 1.0f);
            else if (tapFadeGain[t] > tapFadeTarget[t])
                tapFadeGain[t] = std::max (tapFadeGain[t] - tapFadeIncrement, 0.0f);
            // v1.0.8: Always feed PV to keep it in sync — prevents chirp on tap enable (issue #65)
            float objMono = readObjectSample (t, baseDelaySamples, static_cast<float> (s) / static_cast<float> (numSamples));
            if (tapFadeGain[t] <= 0.0f) continue;

            // Interpolate distance gain and channel gains between previous and current block
            float dist = prevDistGain[t] + frac * (objDistGain[t] - prevDistGain[t]);

            for (int sp = 0; sp < numSpeakers; ++sp)
            {
                float g = prevChannelGains[t][sp] + frac * (objChannelGains[t][sp] - prevChannelGains[t][sp]);
                channelAccum[sp] += objMono * dist * g * tapFadeGain[t];
            }

            wetMono += objMono * dist * tapFadeGain[t];
        }

        // === STAGE 3: FEEDBACK ===
        processFeedbackSample (currentLoopMult, baseDelaySamples, fb);

        // === STAGE 4: OUTPUT — raw wet (dry/wet mix handled in processBlock) ===
        for (int sp = 0; sp < numSpeakers; ++sp)
        {
            int ch = surLayout.speakers[sp].channelIndex;
            if (ch >= 0 && ch < numOutCh && outChannels[ch] != nullptr)
                outChannels[ch][s] = channelAccum[sp];
        }

        // LFE generation — low-pass filtered mono sum at −10 dB (raw, no dw/outGain)
        if (lfeIdx >= 0 && lfeIdx < numOutCh && outChannels[lfeIdx] != nullptr)
            outChannels[lfeIdx][s] = lfeFilter.processSample (wetMono) * 0.316f;
    }

    // Store current gains as previous for next block
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        for (int sp = 0; sp < numSpeakers; ++sp)
            prevChannelGains[t][sp] = objChannelGains[t][sp];
        prevDistGain[t] = objDistGain[t];
    }
}

// #############################################################################
// SPATIAL MEDIA LIBRARY: ADM-OSC Receive
// OSC message parsing (ADM-OSC namespace), position updates via APVTS,
// Cartesian→Polar conversion (ITU-R BS.2127-0).
// Reusable by any SML plugin needing ADM-OSC object position control.
// #############################################################################

//==============================================================================
// Cartesian→Polar conversion per ITU-R BS.2127-0
// Converts ADM-OSC Cartesian (x,y,z) to polar (azimuth, elevation, distance)
//==============================================================================
static inline void cartesianToPolar (float x, float y, float z,
                                     float& azDeg, float& elDeg, float& dist)
{
    azDeg = std::atan2 (-x, y) * (180.0f / juce::MathConstants<float>::pi);
    float r = std::sqrt (x * x + y * y);
    elDeg = std::atan2 (z, r) * (180.0f / juce::MathConstants<float>::pi);
    dist = juce::jlimit (0.0f, 1.0f, std::sqrt (x * x + y * y + z * z));
}

//==============================================================================
// OSC Receive — message thread callback (MessageLoopCallback)
// Accepts ADM-OSC standard (/adm/obj/N/) and OSD custom (/osd/obj/N/, /osd/global/)
//==============================================================================
void OpenSpatialDelayProcessor::oscMessageReceived (const juce::OSCMessage& message)
{
    const auto address = message.getAddressPattern().toString();

    // --- /adm/obj/N/... or /osd/obj/N/... — per-object messages ---
    if (address.startsWith ("/adm/obj/") || address.startsWith ("/osd/obj/"))
    {
        auto afterObj = address.substring (9);  // both prefixes are 9 chars
        auto slashIdx = afterObj.indexOf ("/");
        if (slashIdx < 0) return;

        int objNum = afterObj.substring (0, slashIdx).getIntValue();
        if (objNum < 1 || objNum > MAX_OBJECTS) return;
        int objIdx = objNum - 1;

        auto property = afterObj.substring (slashIdx);

        // --- Position messages (accepted on both /adm/ and /osd/) ---
        if (property == "/azim" && message.size() >= 1 && message[0].isFloat32())
        {
            handleOSCPosition (objIdx, message[0].getFloat32(),
                               cachedObj[objIdx].elevation->load(),
                               cachedObj[objIdx].distance->load());
        }
        else if (property == "/elev" && message.size() >= 1 && message[0].isFloat32())
        {
            handleOSCPosition (objIdx,
                               cachedObj[objIdx].azimuth->load(),
                               message[0].getFloat32(),
                               cachedObj[objIdx].distance->load());
        }
        else if (property == "/dist" && message.size() >= 1 && message[0].isFloat32())
        {
            handleOSCPosition (objIdx,
                               cachedObj[objIdx].azimuth->load(),
                               cachedObj[objIdx].elevation->load(),
                               message[0].getFloat32());
        }
        else if (property == "/aed" && message.size() >= 3
                 && message[0].isFloat32() && message[1].isFloat32() && message[2].isFloat32())
        {
            handleOSCPosition (objIdx,
                               message[0].getFloat32(),
                               message[1].getFloat32(),
                               message[2].getFloat32());
        }
        else if (property == "/xyz" && message.size() >= 3
                 && message[0].isFloat32() && message[1].isFloat32() && message[2].isFloat32())
        {
            float azDeg, elDeg, dist;
            cartesianToPolar (message[0].getFloat32(), message[1].getFloat32(),
                              message[2].getFloat32(), azDeg, elDeg, dist);
            handleOSCPosition (objIdx, azDeg, elDeg, dist);
        }
        else if (property == "/x" && message.size() >= 1 && message[0].isFloat32())
        {
            oscCartesianX[objIdx] = message[0].getFloat32();
            float azDeg, elDeg, dist;
            cartesianToPolar (oscCartesianX[objIdx], oscCartesianY[objIdx],
                              oscCartesianZ[objIdx], azDeg, elDeg, dist);
            handleOSCPosition (objIdx, azDeg, elDeg, dist);
        }
        else if (property == "/y" && message.size() >= 1 && message[0].isFloat32())
        {
            oscCartesianY[objIdx] = message[0].getFloat32();
            float azDeg, elDeg, dist;
            cartesianToPolar (oscCartesianX[objIdx], oscCartesianY[objIdx],
                              oscCartesianZ[objIdx], azDeg, elDeg, dist);
            handleOSCPosition (objIdx, azDeg, elDeg, dist);
        }
        else if (property == "/z" && message.size() >= 1 && message[0].isFloat32())
        {
            oscCartesianZ[objIdx] = message[0].getFloat32();
            float azDeg, elDeg, dist;
            cartesianToPolar (oscCartesianX[objIdx], oscCartesianY[objIdx],
                              oscCartesianZ[objIdx], azDeg, elDeg, dist);
            handleOSCPosition (objIdx, azDeg, elDeg, dist);
        }
        // --- Per-object non-position params (/osd/obj/N/ only) ---
        else if (property == "/enabled" && message.size() >= 1)
        {
            handleOSCParam ("object" + juce::String (objNum) + "_enabled",
                            message[0].isFloat32() ? message[0].getFloat32()
                                                   : static_cast<float> (message[0].getInt32()));
        }
        else if (property == "/doppler" && message.size() >= 1 && message[0].isFloat32())
        {
            handleOSCParam ("object" + juce::String (objNum) + "_dopplerAmount",
                            message[0].getFloat32());
        }
        else if (property == "/pitch" && message.size() >= 1 && message[0].isFloat32())
        {
            handleOSCParam ("object" + juce::String (objNum) + "_pitchShift",
                            message[0].getFloat32());
        }
        else if (property == "/trajectory" && message.size() >= 1)
        {
            handleOSCParam ("object" + juce::String (objNum) + "_trajectoryShape",
                            message[0].isFloat32() ? message[0].getFloat32()
                                                   : static_cast<float> (message[0].getInt32()));
        }
        else if (property == "/speed" && message.size() >= 1 && message[0].isFloat32())
        {
            handleOSCParam ("object" + juce::String (objNum) + "_trajectorySpeed",
                            message[0].getFloat32());
        }
        else if (property == "/direction" && message.size() >= 1)
        {
            handleOSCParam ("object" + juce::String (objNum) + "_trajectoryDirection",
                            message[0].isFloat32() ? message[0].getFloat32()
                                                   : static_cast<float> (message[0].getInt32()));
        }
        else if (property == "/input" && message.size() >= 1)
        {
            handleOSCParam ("object" + juce::String (objNum) + "_inputChannel",
                            message[0].isFloat32() ? message[0].getFloat32()
                                                   : static_cast<float> (message[0].getInt32()));
        }
    }
    // --- /osd/global/... — global parameter messages ---
    else if (address.startsWith ("/osd/global/"))
    {
        auto property = address.substring (11);  // skip "/osd/global" → "/delaytime" etc.
        if (message.size() < 1) return;
        float val = message[0].isFloat32() ? message[0].getFloat32()
                                           : static_cast<float> (message[0].getInt32());

        if      (property == "/delaytime")     handleOSCParam ("delayTime", val);
        else if (property == "/temposync")     handleOSCParam ("tempoSync", val);
        else if (property == "/notedivision")  handleOSCParam ("noteDivision", val);
        else if (property == "/syncmode")      handleOSCParam ("syncMode", val);
        else if (property == "/feedback")      handleOSCParam ("feedback", val);
        else if (property == "/filterlp")      handleOSCParam ("filterLP", val);
        else if (property == "/filterhp")      handleOSCParam ("filterHP", val);
        else if (property == "/filterlpq")     handleOSCParam ("filterLPQ", val);
        else if (property == "/filterhpq")     handleOSCParam ("filterHPQ", val);
        else if (property == "/filterenabled") handleOSCParam ("filterEnabled", val);
        else if (property == "/drywet")        handleOSCParam ("dryWet", val);
        else if (property == "/inputgain")     handleOSCParam ("inputGain", val);
        else if (property == "/outputgain")    handleOSCParam ("outputGain", val);
        // Algorithm, HRTF profile, and output format are configuration-level settings
        // — not controllable via OSC (no send or receive).
        else if (property == "/air")           handleOSCParam ("airAbsorption", val);
        else if (property == "/wobble")        handleOSCParam ("wobbleEnabled", val);
        else if (property == "/wobbleamount")  handleOSCParam ("wobbleAmount", val);
        else if (property == "/wobblemorph")   handleOSCParam ("wobbleMorph", val);
        // v1.0: Global tap offset knobs — now APVTS params (issue #68)
        else if (property == "/tapazimuth")    { handleOSCParam ("globalTapAzimuth", val); syncGlobalTapOffsetAtomic (0, val); }
        else if (property == "/tapelevation")  { handleOSCParam ("globalTapElevation", val); syncGlobalTapOffsetAtomic (1, val); }
        else if (property == "/tapdistance")   { handleOSCParam ("globalTapDistance", val); syncGlobalTapOffsetAtomic (2, val); }
        else if (property == "/tappitch")      { handleOSCParam ("globalTapPitch", val); syncGlobalTapOffsetAtomic (3, val); }
        else if (property == "/tapdoppler")    { handleOSCParam ("globalTapDoppler", val); syncGlobalTapOffsetAtomic (4, val); }
        else if (property == "/tapspeed")      { handleOSCParam ("globalTapSpeed", val); syncGlobalTapOffsetAtomic (5, val); }
    }
}

//==============================================================================
// Set any APVTS parameter from OSC — denormalized value in, normalized write out
//==============================================================================
void OpenSpatialDelayProcessor::handleOSCParam (const juce::String& paramID, float denormValue)
{
    if (auto* param = apvts.getParameter (paramID))
        param->setValueNotifyingHost (param->convertTo0to1 (denormValue));
}

// issue #68: Keep atomic mirror in sync for editor drawer when OSC writes to APVTS.
// Reads the clamped value back from the APVTS param (not the raw OSC input).
void OpenSpatialDelayProcessor::syncGlobalTapOffsetAtomic (int index, float /*rawValue*/)
{
    if (index < 0 || index >= kNumGlobalTapOffsets)
        return;
    static const char* ids[] = { "globalTapAzimuth", "globalTapElevation", "globalTapDistance",
                                 "globalTapPitch",   "globalTapDoppler",   "globalTapSpeed" };
    if (auto* p = apvts.getRawParameterValue (ids[index]))
        globalTapOffset[index].store (p->load(), std::memory_order_relaxed);
    globalTapOffsetChanged.store (true, std::memory_order_relaxed);
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

    // v0.9: When trajectory is active, OSC sets the origin (knobs/APVTS) —
    // trajectory continues running around the new origin. No override needed.
    // When NO trajectory is active, OSC overrides position directly (existing behavior).
    if (! trajectory.isActive (objIdx))
    {
        oscOverrideActive[objIdx].store (true, std::memory_order_relaxed);
        oscLastReceiveTime[objIdx] = juce::Time::getMillisecondCounterHiRes();
    }
}

// #############################################################################
// DELAY-SPECIFIC: Trajectory Animation
// Per-plugin modulation — each SML plugin implements its own trajectory shapes.
// #############################################################################

// v1.0.1: computeTrajectory() (~300 lines) moved to TrajectoryEngine.cpp


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
    state.setProperty ("pluginStateVersion", 23, nullptr);  // v1.1 state format (23 = Constant Power algorithm inserted at index 1)
    // Issue #68: Config params stored as top-level properties (not APVTS children)
    state.setProperty ("configAlgorithm", configAlgorithm.load (std::memory_order_relaxed), nullptr);
    state.setProperty ("configHrtfProfile", configHrtfProfile.load (std::memory_order_relaxed), nullptr);
    state.setProperty ("configOutputFormat", configOutputFormat.load (std::memory_order_relaxed), nullptr);
    state.setProperty ("configInputFormat", configInputFormat.load (std::memory_order_relaxed), nullptr);
    state.setProperty ("oscReceiveEnabled", oscReceiveEnabled, nullptr);  // v1.0: persist OSC receive enable (issue E20)
    state.setProperty ("oscReceivePort", oscReceivePort, nullptr);  // v0.6: persist OSC port
    state.setProperty ("globalDrawerOpen", globalDrawerOpen, nullptr);  // v1.0: persist drawer state
    state.setProperty ("currentPresetIndex", currentPresetIndex, nullptr);  // v0.6: persist preset selection
    // v0.7: persist OSC Send settings
    state.setProperty ("oscSendEnabled", oscSendEnabled, nullptr);
    state.setProperty ("oscSendPort", oscSendPort, nullptr);
    state.setProperty ("oscSendIP", oscSendIP, nullptr);
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

    // v0.9: Migrate trajectoryShape from 6-item to 13-item alphabetical ordering
    // Old (6 items): None=0, Spiral=1, Orbit=2, Bounce=3, Figure-8=4, Random=5
    // New (13 items): None=0, Bounce=1, Cross=2, Figure-8=3, Heart=4, Helix=5,
    //                 Infinity=6, Line=7, Orbit=8, Random=9, Spiral=10, Square=11, Triangle=12
    if (savedVersion < 13)
    {
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            auto paramId = child.getProperty ("id").toString();
            if (paramId.containsIgnoreCase ("trajectoryShape"))
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (6 items → indices 0..5)
                int oldIndex = juce::roundToInt (normalizedOld * 5.0f);
                oldIndex = juce::jlimit (0, 5, oldIndex);
                int newIndex = trajectoryLegacyToNewIndex (oldIndex);
                // Re-normalize using new item count (14 items → indices 0..13)
                float normalizedNew = static_cast<float> (newIndex) / 13.0f;
                child.setProperty ("value", normalizedNew, nullptr);
            }
        }

        tree.setProperty ("pluginStateVersion", 15, nullptr);
    }

    // v0.9: Migrate from intermediate 12-item to final 14-item trajectory ordering
    // Old 12: ..., 3=Figure-8(old), ..., 6=Lissajous, ...
    // New 14: ..., 4=Figure-8(new), ..., 7=Infinity(=old Figure-8), ..., 13=Triangle(=old Lissajous)
    if (savedVersion >= 13 && savedVersion < 14)
    {
        static const int v13ToV14Map[12] = {
            0, 1, 3, 7, 5, 6, 13, 8, 9, 10, 11, 12
        //  None Bounce Cross Inf  Heart Helix Tri Line Orbit Rand Spiral Square
        };

        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            auto paramId = child.getProperty ("id").toString();
            if (paramId.containsIgnoreCase ("trajectoryShape"))
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (12 items → indices 0..11)
                int oldIndex = juce::roundToInt (normalizedOld * 11.0f);
                oldIndex = juce::jlimit (0, 11, oldIndex);
                int newIndex = v13ToV14Map[oldIndex];
                // Re-normalize using new item count (14 items → indices 0..13)
                float normalizedNew = static_cast<float> (newIndex) / 13.0f;
                child.setProperty ("value", normalizedNew, nullptr);
            }
        }

        tree.setProperty ("pluginStateVersion", 15, nullptr);
    }

    // v0.9: Migrate from 13-item to 14-item trajectory ordering (Circle inserted at index 2)
    // Old 13: None=0, Bounce=1, Cross=2, Figure-8=3, Heart=4, Helix=5, Infinity=6, Line=7, Orbit=8, Random=9, Spiral=10, Square=11, Triangle=12
    // New 14: None=0, Bounce=1, Circle=2, Cross=3, Figure-8=4, Heart=5, Helix=6, Infinity=7, Line=8, Orbit=9, Random=10, Spiral=11, Square=12, Triangle=13
    if (savedVersion == 14)
    {
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            auto paramId = child.getProperty ("id").toString();
            if (paramId.containsIgnoreCase ("trajectoryShape"))
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (13 items → indices 0..12)
                int oldIndex = juce::roundToInt (normalizedOld * 12.0f);
                oldIndex = juce::jlimit (0, 12, oldIndex);
                // Shift indices ≥ 2 up by 1 (Circle inserted at index 2)
                int newIndex = (oldIndex >= 2) ? oldIndex + 1 : oldIndex;
                // Re-normalize using new item count (14 items → indices 0..13)
                float normalizedNew = static_cast<float> (newIndex) / 13.0f;
                child.setProperty ("value", normalizedNew, nullptr);
            }
        }

        tree.setProperty ("pluginStateVersion", 15, nullptr);
    }

    // v1.0: Migrate outputFormat from 21-item to 22-item (SML 13.1 inserted at index 15)
    // Old indices 0-14 stay the same. Old indices 15-20 (Ambisonics) shift to 16-21.
    if (savedVersion < 16)
    {
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            if (child.getProperty ("id").toString() == "outputFormat")
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (21 items → indices 0..20)
                int oldIndex = juce::roundToInt (normalizedOld * 20.0f);
                oldIndex = juce::jlimit (0, 20, oldIndex);
                // Shift Ambisonics indices up by 1
                int newIndex = (oldIndex >= 15) ? oldIndex + 1 : oldIndex;
                // Re-normalize using new item count (22 items → indices 0..21)
                float normalizedNew = static_cast<float> (newIndex) / 21.0f;
                child.setProperty ("value", normalizedNew, nullptr);
                break;
            }
        }

        tree.setProperty ("pluginStateVersion", 16, nullptr);
    }

    // v1.0: Migrate outputFormat from 22-item to 21-item (7.0.2 removed at old index 9)
    // Old indices 0-8 stay the same. Old index 9 (7.0.2) → 8 (Octaphonic fallback).
    // Old indices 10-21 shift down by 1 to 9-20.
    if (savedVersion < 17)
    {
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            if (child.getProperty ("id").toString() == "outputFormat")
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (22 items → indices 0..21)
                int oldIndex = juce::roundToInt (normalizedOld * 21.0f);
                oldIndex = juce::jlimit (0, 21, oldIndex);
                int newIndex;
                if (oldIndex < 9)        newIndex = oldIndex;       // 0-8 unchanged
                else if (oldIndex == 9)   newIndex = 8;             // 7.0.2 → Octaphonic fallback
                else                      newIndex = oldIndex - 1;  // 10-21 shift down
                // Re-normalize using new item count (21 items → indices 0..20)
                float normalizedNew = static_cast<float> (newIndex) / 20.0f;
                child.setProperty ("value", normalizedNew, nullptr);
                break;
            }
        }

        tree.setProperty ("pluginStateVersion", 17, nullptr);
    }

    // v1.0: Reorder outputFormat — 5.1.2/7.1/Oct rotated to 7.1/Oct/5.1.2
    // Old index 6 (5.1.2) → new 8, old 7 (7.1) → new 6, old 8 (Oct) → new 7
    if (savedVersion < 18)
    {
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            if (child.getProperty ("id").toString() == "outputFormat")
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                int oldIndex = juce::roundToInt (normalizedOld * 20.0f);  // 21 items (0..20)
                oldIndex = juce::jlimit (0, 20, oldIndex);
                int newIndex = oldIndex;
                if (oldIndex == 6)       newIndex = 8;   // 5.1.2 → Atmos group
                else if (oldIndex == 7)  newIndex = 6;   // 7.1 → Surround group
                else if (oldIndex == 8)  newIndex = 7;   // Oct → after Surround
                float normalizedNew = static_cast<float> (newIndex) / 20.0f;
                child.setProperty ("value", normalizedNew, nullptr);
                break;
            }
        }

        tree.setProperty ("pluginStateVersion", 18, nullptr);
    }

    // v1.0: Migrate outputFormat from 21-item to 22-item (9.1.4 inserted at index 13)
    // Old indices 0-12 stay the same. Old indices 13-20 (9.1.6, SML, Ambisonics) shift to 14-21.
    if (savedVersion < 19)
    {
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            if (! child.hasProperty ("id"))
                continue;

            if (child.getProperty ("id").toString() == "outputFormat")
            {
                float normalizedOld = static_cast<float> (child.getProperty ("value", 0.0f));
                // Denormalize using old item count (21 items → indices 0..20)
                int oldIndex = juce::roundToInt (normalizedOld * 20.0f);
                oldIndex = juce::jlimit (0, 20, oldIndex);
                // Shift indices >= 13 up by 1
                int newIndex = (oldIndex >= 13) ? oldIndex + 1 : oldIndex;
                // Re-normalize using new item count (22 items → indices 0..21)
                float normalizedNew = static_cast<float> (newIndex) / 21.0f;
                child.setProperty ("value", normalizedNew, nullptr);
                break;
            }
        }

        tree.setProperty ("pluginStateVersion", 19, nullptr);
    }

    // Issue #68: Migrate config params from APVTS children (old sessions) to top-level properties.
    // Old sessions (savedVersion < 20) stored algorithm/hrtfProfile/outputFormat/inputFormat
    // as normalized-float APVTS child nodes. New sessions store them as integer top-level properties.
    if (savedVersion < 20)
    {
        // Extract config param values from APVTS child nodes, then remove those children
        // so APVTS doesn't try to restore params that no longer exist in the layout.
        struct ConfigParamMigration { const char* id; int numChoices; std::atomic<int>* target; };
        ConfigParamMigration migrations[] = {
            { "algorithm",    11, &configAlgorithm },
            { "hrtfProfile",   6, &configHrtfProfile },
            { "outputFormat", 22, &configOutputFormat },
            { "inputFormat",   2, &configInputFormat },
        };

        for (auto& m : migrations)
        {
            for (int i = tree.getNumChildren() - 1; i >= 0; --i)
            {
                auto child = tree.getChild (i);
                if (child.hasProperty ("id") && child.getProperty ("id").toString() == m.id)
                {
                    float normVal = static_cast<float> (child.getProperty ("value", 0.0f));
                    int index = juce::roundToInt (normVal * static_cast<float> (m.numChoices - 1));
                    index = juce::jlimit (0, m.numChoices - 1, index);
                    m.target->store (index, std::memory_order_relaxed);
                    tree.removeChild (i, nullptr);
                    break;
                }
            }
        }

        // Store as top-level properties for future saves
        tree.setProperty ("configAlgorithm", configAlgorithm.load (std::memory_order_relaxed), nullptr);
        tree.setProperty ("configHrtfProfile", configHrtfProfile.load (std::memory_order_relaxed), nullptr);
        tree.setProperty ("configOutputFormat", configOutputFormat.load (std::memory_order_relaxed), nullptr);
        tree.setProperty ("configInputFormat", configInputFormat.load (std::memory_order_relaxed), nullptr);
        tree.setProperty ("pluginStateVersion", 20, nullptr);
    }
    else
    {
        // New session format: config params are top-level integer properties
        configAlgorithm.store (static_cast<int> (tree.getProperty ("configAlgorithm", 1)), std::memory_order_relaxed);
        configHrtfProfile.store (static_cast<int> (tree.getProperty ("configHrtfProfile", 0)), std::memory_order_relaxed);
        configOutputFormat.store (static_cast<int> (tree.getProperty ("configOutputFormat", 0)), std::memory_order_relaxed);
        configInputFormat.store (static_cast<int> (tree.getProperty ("configInputFormat", 1)), std::memory_order_relaxed);
    }

    // Issue #88: Migrate outputFormat from 22-item to 23-item (9.1 Surround inserted at index 8)
    // Old indices 0-7 stay the same. Old indices 8-21 (Atmos, SML, Ambisonics) shift to 9-22.
    if (savedVersion < 21)
    {
        int oldIdx = configOutputFormat.load (std::memory_order_relaxed);
        int newIdx = (oldIdx >= 8) ? oldIdx + 1 : oldIdx;
        configOutputFormat.store (newIdx, std::memory_order_relaxed);
        tree.setProperty ("configOutputFormat", newIdx, nullptr);
    }

    // Issue #88: Reorder 9.1 Surround (7) before Octaphonic (8) so surround formats group together
    if (savedVersion < 22)
    {
        int idx = configOutputFormat.load (std::memory_order_relaxed);
        if (idx == 7)      idx = 8;   // Octaphonic → 8
        else if (idx == 8) idx = 7;   // 9.1 Surround → 7
        configOutputFormat.store (idx, std::memory_order_relaxed);
        tree.setProperty ("configOutputFormat", idx, nullptr);
    }

    // v1.1: Migrate algorithm index for Constant Power insertion at index 1
    // Old: [0=Ambi, 1=DBAP, 2=KNN, 3=MDAP, 4=VBAP, 5=VBIP, 6..10=stereo]
    // New: [0=Ambi, 1=ConstPow, 2=DBAP, 3=KNN, 4=MDAP, 5=VBAP, 6=VBIP, 7..11=stereo]
    // All old indices >= 1 shift up by 1
    if (savedVersion < 23)
    {
        int oldAlgo = configAlgorithm.load (std::memory_order_relaxed);
        int newAlgo = (oldAlgo >= 1) ? oldAlgo + 1 : oldAlgo;
        configAlgorithm.store (newAlgo, std::memory_order_relaxed);
        tree.setProperty ("configAlgorithm", newAlgo, nullptr);
    }

    // v0.6: Restore OSC receive settings (non-APVTS properties, issue E20)
    // Only restore on first call (project load) — skip on undo/redo to keep
    // OSC receive settings out of the DAW undo stack.
    if (! oscReceiveStateLoaded)
    {
        oscReceivePort = static_cast<int> (tree.getProperty ("oscReceivePort", 4002));
        bool savedReceiveEnabled = static_cast<bool> (tree.getProperty ("oscReceiveEnabled", false));
        if (savedReceiveEnabled)
            setOscReceiveEnabled (true);
        oscReceiveStateLoaded = true;
    }
    globalDrawerOpen = static_cast<bool> (tree.getProperty ("globalDrawerOpen", false));

    // v0.6: Restore preset index (non-APVTS property)
    currentPresetIndex = static_cast<int> (tree.getProperty ("currentPresetIndex", 0));

    // v0.7: Restore OSC Send settings (E14: guard against undo restoring send settings)
    if (! oscSendStateLoaded)
    {
        oscSendPort = static_cast<int> (tree.getProperty ("oscSendPort", 4003));
        oscSendIP   = tree.getProperty ("oscSendIP", "127.0.0.1").toString();
        bool savedSendEnabled = static_cast<bool> (tree.getProperty ("oscSendEnabled", false));
        if (savedSendEnabled)
            setOscSendEnabled (true);
        oscSendStateLoaded = true;
    }

    apvts.replaceState (tree);

    // issue #164: Sync global tap offset atomics from restored APVTS values.
    // Without this, atomics are stale after undo and OSC Send uses old values.
    // Do NOT set globalTapOffsetChanged — that would trigger syncGlobalTapOffsetsFromOSC
    // which re-applies deltas to per-object params (incorrect for undo).
    {
        static const char* ids[] = { "globalTapAzimuth", "globalTapElevation", "globalTapDistance",
                                     "globalTapPitch",   "globalTapDoppler",   "globalTapSpeed" };
        for (int i = 0; i < kNumGlobalTapOffsets; ++i)
        {
            if (auto* p = apvts.getRawParameterValue (ids[i]))
                globalTapOffset[i].store (p->load(), std::memory_order_relaxed);
        }
    }
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
