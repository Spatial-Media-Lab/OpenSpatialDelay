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

// --- SPATIAL MEDIA LIBRARY: Table-driven speaker layout definitions ----------
// ITU-R BS.775 / BS.2051 / SMPTE ST 2098-1 standard positions
// Convention: 0° = front, positive azimuth = left, negative = right
// LFE is tracked but excluded from spatialization
// Each entry: { azimuth°, elevation°, channelIndex }
//==============================================================================
struct SpeakerDef { float azDeg; float elDeg; int chIdx; };
struct LayoutDef { int numSpeakers; int lfeIdx; int totalChs; SpeakerDef speakers[16]; };

enum LayoutID { Quad, S5_0, S5_1, S7_0, S7_1, S5_1_2, S5_1_4, S7_0_2, S7_1_2, S7_1_4, S7_1_6, S9_1_6, Octaphonic, NUM_LAYOUT_DEFS };

static const LayoutDef layoutDefs[NUM_LAYOUT_DEFS] = {
    // Quad (4.0)
    { 4, -1, 4, {{ 30,0,0}, {-30,0,1}, { 110,0,2}, {-110,0,3}} },
    // 5.0
    { 5, -1, 5, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 110,0,3}, {-110,0,4}} },
    // 5.1 (LFE=ch3)
    { 5,  3, 6, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 110,0,4}, {-110,0,5}} },
    // 7.0
    { 7, -1, 7, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,3}, {-90,0,4}, { 135,0,5}, {-135,0,6}} },
    // 7.1 (LFE=ch3)
    { 7,  3, 8, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}} },
    // 5.1.2 (LFE=ch3)
    { 7,  3, 8, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 110,0,4}, {-110,0,5}, { 90,45,6}, {-90,45,7}} },
    // 5.1.4 (LFE=ch3)
    { 9,  3, 10, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 110,0,4}, {-110,0,5}, { 45,45,6}, {-45,45,7}, { 135,45,8}, {-135,45,9}} },
    // 7.0.2
    { 9, -1, 9, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,3}, {-90,0,4}, { 135,0,5}, {-135,0,6}, { 90,45,7}, {-90,45,8}} },
    // 7.1.2 (LFE=ch3)
    { 9,  3, 10, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 90,45,8}, {-90,45,9}} },
    // 7.1.4 (LFE=ch3)
    { 11, 3, 12, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 45,45,8}, {-45,45,9}, { 135,45,10}, {-135,45,11}} },
    // 7.1.6 (LFE=ch3)
    { 13, 3, 14, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 45,45,8}, {-45,45,9}, { 135,45,10}, {-135,45,11}, { 90,45,12}, {-90,45,13}} },
    // 9.1.6 (LFE=ch3)
    { 15, 3, 16, {{ 30,0,0}, {-30,0,1}, {0,0,2}, { 90,0,4}, {-90,0,5}, { 135,0,6}, {-135,0,7}, { 60,0,8}, {-60,0,9}, { 45,45,10}, {-45,45,11}, { 90,45,12}, {-90,45,13}, { 135,45,14}, {-135,45,15}} },
    // Octaphonic (8.0, no LFE) — "Center" configuration, 45° intervals
    { 8, -1, 8, {{0,0,0}, {-45,0,1}, {-90,0,2}, {-135,0,3}, { 180,0,4}, { 135,0,5}, { 90,0,6}, { 45,0,7}} },
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
// Delay-specific: delayTime, feedback, filterLP/HP, pitchShift, dryWet, gains
// #############################################################################

//==============================================================================
// Parameter layout
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
    OpenSpatialDelayProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Shared string-from-value formatters (DRY)
    auto fmtDbInf = [](float value, int) {
        if (value <= -99.9f)
            return juce::String (juce::CharPointer_UTF8 ("-\xe2\x88\x9e dB"));
        return juce::String (value, 1) + " dB";
    };
    auto fmtPct01 = [](float value, int) {
        return juce::String (juce::roundToInt (value * 100.0f)) + "%";
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
    auto fmtDeg = [](float value, int) {
        return juce::String (value, 1) + juce::String::charToString (0x00B0);
    };

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

    // Note division: value = number of 16th notes. Range: 0.5 (1/32) to 32 (2/1)
    // Steps at musical intervals: 0.5, 1, 2, 4, 8, 16, 32
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("noteDivision", 1), "Note Division",
        juce::NormalisableRange<float> (0.5f, 32.0f, 0.5f), 4.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) {
                // value = number of 16th notes
                float sixteenths = value;
                if (std::abs (sixteenths - 32.0f)  < 0.01f) return juce::String ("2/1");
                if (std::abs (sixteenths - 16.0f)  < 0.01f) return juce::String ("1/1");
                if (std::abs (sixteenths - 8.0f)   < 0.01f) return juce::String ("1/2");
                if (std::abs (sixteenths - 4.0f)   < 0.01f) return juce::String ("1/4");
                if (std::abs (sixteenths - 2.0f)   < 0.01f) return juce::String ("1/8");
                if (std::abs (sixteenths - 1.0f)   < 0.01f) return juce::String ("1/16");
                if (std::abs (sixteenths - 0.5f)   < 0.01f) return juce::String ("1/32");
                // For other values, show as fraction of 16ths
                if (sixteenths >= 1.0f)
                    return juce::String (juce::roundToInt (sixteenths)) + "/16";
                return juce::String (sixteenths, 1) + "/16";
            })));

    // v0.7: Reworked to 3-state: Straight (0), Dotted (1), Triplet (2)
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("syncMode", 7), "Sync Mode",
        juce::StringArray { "Straight", "Dotted", "Triplet" }, 0));


    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("feedback", 1), "Feedback",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.3f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (fmtPct01)));

    // v0.9: Defaults changed from LP=20000/HP=20 to LP=5000/HP=50. Version bumped to 9
    // so DAWs discard stale saved state and use new defaults on fresh instances.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterLP", 9), "Low-Pass Filter",
        juce::NormalisableRange<float> (200.0f, 20000.0f, 1.0f, 0.3f), 5000.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtFreq)));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterHP", 9), "High-Pass Filter",
        juce::NormalisableRange<float> (20.0f, 5000.0f, 1.0f, 0.3f), 50.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtFreq)));

    // v0.7: Separate HP and LP resonance (Q). Range 0.5 (gentle) to 8.0 (sharp peak). Default: 0.707 (Butterworth).
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterHPQ", 7), "HP Resonance",
        juce::NormalisableRange<float> (0.5f, 8.0f, 0.01f, 0.4f), 0.707f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 2); })));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterLPQ", 7), "LP Resonance",
        juce::NormalisableRange<float> (0.5f, 8.0f, 0.01f, 0.4f), 0.707f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (value, 2); })));

    // v0.9: Dedicated filter enabled toggle (default OFF — filter bypassed on fresh load)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("filterEnabled", 9), "Filter Enabled",
        juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f));

    // v0.7: Global pitch shift in cents (±200 ct). Per-tap override in semitones is separate.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("pitchShift", 7), "Pitch Shift",
        juce::NormalisableRange<float> (-200.0f, 200.0f, 1.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [](float value, int) { return juce::String (juce::roundToInt (value)) + " ct"; })));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("dryWet", 1), "Dry/Wet",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (fmtPct01)));

    // Input gain: -100 dB (shown as -∞) to +40 dB
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("inputGain", 1), "Input Gain",
        juce::NormalisableRange<float> (-100.0f, 40.0f, 0.1f, 3.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtDbInf)));

    // Output gain: -100 dB (shown as -∞) to +12 dB
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("outputGain", 1), "Output Gain",
        juce::NormalisableRange<float> (-100.0f, 12.0f, 0.1f, 3.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtDbInf)));



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

    // v0.8: Wobble modulation (delay time LFO)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("wobbleAmount", 8), "Wobble Amount",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (fmtPct100)));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("wobbleMorph", 8), "Wobble Morph",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (fmtPct100)));

    // v0.8: Wobble modulation enable toggle
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("wobbleEnabled", 8), "Wobble Enabled", false));

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
        // v0.9: Default azimuth is 0° (center front) so double-click resets to 0°.
        // Initial spatial spread is applied in constructor after APVTS creation.
        float defaultAz = 0.0f;

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            id ("enabled"), name ("Enabled"), defaultEnabled));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("time"), name ("Time"),
            juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.3f), defaultTime));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("azimuth"), name ("Azimuth"),
            juce::NormalisableRange<float> (-180.0f, 180.0f, 0.1f), defaultAz,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtDeg)));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("elevation"), name ("Elevation"),
            juce::NormalisableRange<float> (-90.0f, 90.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (fmtDeg)));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("distance"), name ("Distance"),
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

        // v0.4: Per-object Doppler amount (0=off, >0=on at that intensity)
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("dopplerAmount"), name ("Doppler Amount"),
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (fmtPct01)));

        // v0.8: Per-object pitch shift offset (semitones, integer ±24, additive on top of global cumulative)
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("pitchShift"), name ("Pitch Shift"),
            juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [](float value, int) { return juce::String (juce::roundToInt (value)) + " st"; })));

        // v0.6: Per-object trajectory (shape + speed)
        // v0.9: Expanded from 6 to 14 shapes, alphabetized (None at top)
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            id ("trajectoryShape"), name ("Trajectory Shape"),
            juce::StringArray { "None", "Bounce", "Circle", "Cross", "Figure-8", "Heart", "Helix",
                                "Infinity", "Line", "Orbit", "Random", "Spiral", "Square", "Triangle" }, 0));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id ("trajectorySpeed"), name ("Trajectory Speed"),
            juce::NormalisableRange<float> (0.0f, 5.0f, 0.01f), 0.3f,
            juce::AudioParameterFloatAttributes().withLabel ("Hz").withStringFromValueFunction (
                [](float value, int) { return juce::String (value, 2) + " Hz"; })));

        // v0.8: Per-object trajectory direction (0=Forward/CCW, 1=Reverse/CW)
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            id ("trajectoryDirection"), name ("Trajectory Direction"),
            juce::StringArray { "Forward", "Reverse" }, 0));

        // v0.8: Per-object input channel selection (only active when Input Format = Stereo)
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            id ("inputChannel"), name ("Input Channel"),
            juce::StringArray { "L+R", "L", "R" }, 0));
    }

    // v0.7: Input format (Mono / Stereo)
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("inputFormat", 7), "Input Format",
        juce::StringArray { "Mono", "Stereo" }, 0));

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

    // --- v0.9: Origin-point trajectory animation engine ---
    // Knobs = live origin. Trajectory computes animated position stored in internal arrays.
    // processBlock reads trajectoryFinalAz/El/Dist instead of APVTS when active.
    // Knobs are never overwritten — user can move the origin while trajectory runs.
    {
        constexpr float dt = 1.0f / 60.0f;  // Timer runs at ~60Hz

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            int shape = juce::roundToInt (cachedParam_trajectoryShape[t] != nullptr
                                          ? cachedParam_trajectoryShape[t]->load() : 0.0f);

            if (shape == 0)
            {
                // None — no trajectory active, processBlock reads from APVTS directly
                trajectoryActive[t].store (false, std::memory_order_relaxed);
                prevTrajectoryShape[t] = 0;
                continue;
            }

            // Skip animation if OSC is controlling this object (but keep trajectoryActive true
            // so processBlock reads the last computed position, and OSC updates the origin)
            if (oscOverrideActive[t].load (std::memory_order_relaxed))
                continue;

            float speed = cachedParam_trajectorySpeed[t] != nullptr
                          ? cachedParam_trajectorySpeed[t]->load() : 1.0f;

            // Detect shape change: reset phase (origin is always live from APVTS knobs)
            if (prevTrajectoryShape[t] == 0 || prevTrajectoryShape[t] != shape)
            {
                trajectoryPhase[t] = 0.0f;
                randomNoise[t].initialized = false;  // re-randomize on shape change
                randomTime[t] = 0.0f;
                // Capture base for getTrajectoryState() visualization (used by editor)
                baseAzimuth[t]   = cachedObj[t].azimuth->load();
                baseElevation[t] = cachedObj[t].elevation->load();
                baseDistance[t]   = cachedObj[t].distance->load();
            }
            prevTrajectoryShape[t] = shape;

            // Advance phase for this object
            // Spiral (shape 11), Random (shape 10), and Square (shape 12) run at half base speed
            float effectiveSpeed = (shape == 10 || shape == 11 || shape == 12) ? speed * 0.5f : speed;
            trajectoryPhase[t] += effectiveSpeed * dt;
            if (trajectoryPhase[t] >= 1.0f)
                trajectoryPhase[t] -= std::floor (trajectoryPhase[t]);

            // v0.8: Read trajectory direction (0=Forward, 1=Reverse)
            // Invert: natural phase progression is CCW on map; Forward should be CW
            bool reverse = (cachedParam_trajectoryDirection[t] == nullptr
                            || cachedParam_trajectoryDirection[t]->load() < 0.5f);

            // Read LIVE origin from APVTS knobs (not captured base)
            float originAz   = cachedObj[t].azimuth->load();
            float originEl   = cachedObj[t].elevation->load();
            float originDist = cachedObj[t].distance->load();

            if (shape == 10)  // Random — multi-sine noise with randomized parameters (Issue #9)
            {
                auto& rn = randomNoise[t];
                if (! rn.initialized)
                {
                    // Generate unique frequencies, phases, and amplitudes per instance
                    auto randSign = [&]() { return randomRng.nextBool() ? 1.0f : -1.0f; };
                    float azAmps[]   = { 50.0f, 27.0f, 19.0f, 11.0f };
                    float elAmps[]   = { 30.0f, 16.0f, 12.0f, 8.0f };
                    float distAmps[] = { 0.50f, 0.30f, 0.20f };
                    for (int k = 0; k < 4; ++k)
                    {
                        rn.freqAz[k]  = 0.15f + randomRng.nextFloat() * 1.75f;
                        rn.phaseAz[k] = randomRng.nextFloat() * juce::MathConstants<float>::twoPi;
                        rn.ampAz[k]   = azAmps[k] * randSign();
                        rn.freqEl[k]  = 0.15f + randomRng.nextFloat() * 1.75f;
                        rn.phaseEl[k] = randomRng.nextFloat() * juce::MathConstants<float>::twoPi;
                        rn.ampEl[k]   = elAmps[k] * randSign();
                    }
                    for (int k = 0; k < 3; ++k)
                    {
                        rn.freqDist[k]  = 0.15f + randomRng.nextFloat() * 1.25f;
                        rn.phaseDist[k] = randomRng.nextFloat() * juce::MathConstants<float>::twoPi;
                        rn.ampDist[k]   = distAmps[k] * randSign();
                    }
                    rn.initialized = true;
                }

                // Use ever-increasing time (not wrapping phase) so pattern never repeats
                randomTime[t] += effectiveSpeed * dt;
                float p = randomTime[t] * juce::MathConstants<float>::twoPi;
                float az = 0.0f, el = 0.0f, dist = 0.0f;
                for (int k = 0; k < 4; ++k)
                {
                    az += rn.ampAz[k] * std::sin (p * rn.freqAz[k] + rn.phaseAz[k]);
                    el += rn.ampEl[k] * std::sin (p * rn.freqEl[k] + rn.phaseEl[k]);
                }
                for (int k = 0; k < 3; ++k)
                    dist += rn.ampDist[k] * std::sin (p * rn.freqDist[k] + rn.phaseDist[k]);

                trajectoryFinalAz[t] = originAz + az;
                trajectoryFinalEl[t] = juce::jlimit (-90.0f, 90.0f, originEl + el);
                float distScaleR = 1.0f - originDist;
                trajectoryFinalDist[t] = juce::jlimit (0.0f, 1.0f, originDist + dist * distScaleR);

                while (trajectoryFinalAz[t] > 180.0f)  trajectoryFinalAz[t] -= 360.0f;
                while (trajectoryFinalAz[t] < -180.0f) trajectoryFinalAz[t] += 360.0f;
            }
            else
            {
                auto result = computeTrajectory (shape, trajectoryPhase[t],
                                                 originAz, originEl, originDist,
                                                 reverse);

                trajectoryFinalAz[t]   = result.azDeg;
                trajectoryFinalEl[t]   = result.elDeg;
                trajectoryFinalDist[t] = result.dist;
            }

            trajectoryActive[t].store (true, std::memory_order_relaxed);

            // Update base for getTrajectoryState() visualization
            baseAzimuth[t]   = originAz;
            baseElevation[t] = originEl;
            baseDistance[t]   = originDist;
        }
    }

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
            if (trajectoryActive[t].load (std::memory_order_relaxed))
            {
                az   = trajectoryFinalAz[t];
                el   = trajectoryFinalEl[t];
                dist = trajectoryFinalDist[t];
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
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::discreteChannels (50), true)),
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
    cachedParam_hrtfProfile     = apvts.getRawParameterValue ("hrtfProfile");
    cachedParam_globalPitchShift = apvts.getRawParameterValue ("pitchShift");
    cachedParam_airAbsorption   = apvts.getRawParameterValue ("airAbsorption");
    cachedParam_wobbleEnabled   = apvts.getRawParameterValue ("wobbleEnabled");
    cachedParam_wobbleAmount    = apvts.getRawParameterValue ("wobbleAmount");
    cachedParam_wobbleMorph     = apvts.getRawParameterValue ("wobbleMorph");
    cachedParam_inputFormat     = apvts.getRawParameterValue ("inputFormat");
    cachedParam_delayTime       = apvts.getRawParameterValue ("delayTime");
    cachedParam_dryWet          = apvts.getRawParameterValue ("dryWet");
    cachedParam_feedback        = apvts.getRawParameterValue ("feedback");
    cachedParam_inputGain       = apvts.getRawParameterValue ("inputGain");
    cachedParam_outputGain      = apvts.getRawParameterValue ("outputGain");
    cachedParam_algorithm       = apvts.getRawParameterValue ("algorithm");
    cachedParam_admOscEnabled   = apvts.getRawParameterValue ("admOscEnabled");

    // Cache RangedAudioParameter* for trajectory writes and pre-build OSC addresses
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        auto prefix = "object" + juce::String (i + 1) + "_";
        trajParam_azimuth[i]   = apvts.getParameter (prefix + "azimuth");
        trajParam_elevation[i] = apvts.getParameter (prefix + "elevation");
        trajParam_distance[i]  = apvts.getParameter (prefix + "distance");
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

    // v0.9: Load presets from disk (factory presets installed at build time by install_presets tool)
    loadAllPresetsFromDisk();
}

OpenSpatialDelayProcessor::~OpenSpatialDelayProcessor()
{
    // v0.6: Disconnect OSC receiver before destruction
    oscReceiver.disconnect();
    oscReceiver.removeListener (this);
    // v0.7: Disconnect OSC sender
    oscSender.disconnect();
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

//==============================================================================
// v0.7: OSC Send control methods
//==============================================================================
void OpenSpatialDelayProcessor::setOscSendEnabled (bool enabled)
{
    if (enabled == oscSendEnabled)
        return;

    oscSendEnabled = enabled;
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
    setFloat  ("pitchShift",   preset->pitchShift);
    setFloat  ("dryWet",       preset->dryWet);
    setFloat  ("inputGain",    preset->inputGain);
    setFloat  ("outputGain",   preset->outputGain);
    setBool   ("airAbsorption", preset->airAbsorption);
    setBool   ("filterEnabled", preset->filterEnabled);
    setBool   ("wobbleEnabled", preset->wobbleEnabled);
    setFloat  ("wobbleAmount", preset->wobbleAmount);
    setFloat  ("wobbleMorph",  preset->wobbleMorph);
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
        setFloat  (prefix + "pitchShift",      tap.pitchShift);
        setChoice (prefix + "trajectoryShape", tap.trajectoryShape);
        setFloat  (prefix + "trajectorySpeed", tap.trajectorySpeed);
        setChoice (prefix + "trajectoryDirection", tap.trajectoryDirection);
        setChoice (prefix + "inputChannel",        tap.inputChannel);
    }

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
    pd.pitchShift   = apvts.getRawParameterValue ("pitchShift")->load();
    pd.dryWet       = apvts.getRawParameterValue ("dryWet")->load();
    pd.inputGain    = apvts.getRawParameterValue ("inputGain")->load();
    pd.outputGain   = apvts.getRawParameterValue ("outputGain")->load();
    pd.airAbsorption = apvts.getRawParameterValue ("airAbsorption")->load() > 0.5f;
    pd.filterEnabled = apvts.getRawParameterValue ("filterEnabled")->load() > 0.5f;
    pd.wobbleEnabled = apvts.getRawParameterValue ("wobbleEnabled")->load() > 0.5f;
    pd.wobbleAmount  = apvts.getRawParameterValue ("wobbleAmount")->load();
    pd.wobbleMorph   = apvts.getRawParameterValue ("wobbleMorph")->load();
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
    loadAllPresetsFromDisk();
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

void OpenSpatialDelayProcessor::loadAllPresetsFromDisk()
{
    allPresets.clear();

    auto dir = getPresetDirectory();
    if (! dir.isDirectory())
        return;

    // v0.9: Ensure User subfolder exists for user preset storage (Issue #2)
    dir.getChildFile ("User").createDirectory();

    // Pass 1: Root-level files (legacy .json presets)
    auto rootFiles = dir.findChildFiles (juce::File::findFiles, false, "*.json");
    rootFiles.sort();
    for (const auto& file : rootFiles)
    {
        auto json = file.loadFileAsString();
        if (json.isNotEmpty())
        {
            auto pd = parsePresetJson (json);
            if (pd.name.isEmpty())
                pd.name = file.getFileNameWithoutExtension();
            if (pd.category.isEmpty())
                pd.category = "User";
            allPresets.push_back (pd);
        }
    }

    // Pass 2: Category subfolders — scan .osdpreset and .json
    auto subdirs = dir.findChildFiles (juce::File::findDirectories, false);
    subdirs.sort();
    for (const auto& subdir : subdirs)
    {
        juce::String categoryName = subdir.getFileName();

        // Scan both .osdpreset and .json
        auto files = subdir.findChildFiles (juce::File::findFiles, false, "*.osdpreset");
        auto jsonFiles = subdir.findChildFiles (juce::File::findFiles, false, "*.json");
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
                pd.category = categoryName;  // folder name = category
                allPresets.push_back (pd);
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
    if (outputSet == juce::AudioChannelSet::discreteChannels (26)) return true;  // v0.8: 4OA stereo-pair (13×2)
    if (outputSet == juce::AudioChannelSet::discreteChannels (36)) return true;  // 5th order
    if (outputSet == juce::AudioChannelSet::discreteChannels (49)) return true;  // 6th order
    if (outputSet == juce::AudioChannelSet::discreteChannels (50)) return true;  // v0.8: 6OA stereo-pair (25×2)

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
        case OutputFormat::Quad:          buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::Quad]);       break;
        case OutputFormat::Surround5_0:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S5_0]);       break;
        case OutputFormat::Surround5_1:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S5_1]);       break;
        case OutputFormat::Surround7_0:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_0]);       break;
        case OutputFormat::Surround7_1:   buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_1]);       break;
        case OutputFormat::Surround5_1_2: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S5_1_2]);     break;
        case OutputFormat::Surround5_1_4: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S5_1_4]);     break;
        case OutputFormat::Surround7_0_2: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_0_2]);     break;
        case OutputFormat::Surround7_1_2: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_1_2]);     break;
        case OutputFormat::Surround7_1_4: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_1_4]);     break;
        case OutputFormat::Surround7_1_6: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S7_1_6]);     break;
        case OutputFormat::Surround9_1_6: buf.layout = makeLayoutFromDef (layoutDefs[LayoutID::S9_1_6]);     break;
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

    // v0.9: Reset wobble modulation state (4-layer tape emulation)
    std::fill (std::begin (wobblePhases), std::end (wobblePhases), 0.0f);
    blockWobbleAmount = 0.0f;
    blockWobbleMorph = 0.0f;

    // Reset all varispeed states
    for (auto& vs : varispeedState)
    {
        vs.drift = 0.0f;
        vs.fadingDrift = 0.0f;
        vs.crossfadeRemaining = 0;
        vs.crossfadeLength = 0;
    }

    // v0.9: Reset all WSOLA-lite per-tap pitch states
    for (auto& ws : wsolaState)
    {
        std::fill (std::begin (ws.buffer), std::end (ws.buffer), 0.0f);
        ws.writePos = 0;
        ws.readPhase = 0.0f;
        ws.fadingPhase = 0.0f;
        ws.crossfadeRemaining = 0;
    }

    // Pre-allocate input buffers
    monoInputBuffer.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    inputBufferL.resize (static_cast<size_t> (samplesPerBlock), 0.0f);
    inputBufferR.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

    // Initialize filters
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
    feedbackLPFilter.prepare (spec);
    feedbackHPFilter.prepare (spec);
    feedbackLPFilter.reset();
    feedbackHPFilter.reset();
    for (int t = 0; t < MAX_OBJECTS; ++t)
    {
        tapLPFilter[t].prepare (spec);
        tapHPFilter[t].prepare (spec);
        tapLPFilter[t].reset();
        tapHPFilter[t].reset();
    }
    cachedFeedbackLPFreq = -1.0f;  // Force recalculation on first block
    cachedFeedbackHPFreq = -1.0f;

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
    // v1.0: Pre-compute transparent coefficients to avoid heap allocation in processBlock
    airTransparentCoeffs = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 20000.0f);
    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        airAbsorptionFilter[i].prepare (spec);
        airAbsorptionFilter[i].reset();
        *airAbsorptionFilter[i].coefficients = airTransparentCoeffs;
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
    delayBufferL.clear();
    delayBufferR.clear();
    monoInputBuffer.clear();
    inputBufferL.clear();
    inputBufferR.clear();
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

//==============================================================================
// Varispeed Pitch Shifter — Tape-Speed with Short Crossfade
// Single active read head drifting at playback ratio. When drift exceeds max,
// a short (~10ms) equal-power crossfade resets to nominal position.
// Used for global cumulative pitch and feedback pitch.
//==============================================================================
float OpenSpatialDelayProcessor::readVarispeed (float delaySamples,
                                                  float semitones, int phaseIndex,
                                                  DelayChannel ch)
{
    // v0.8: Channel-aware delay line read dispatch
    auto readDL = [this, ch](float pos) -> float {
        switch (ch) {
            case DelayChannel::Left:  return readDelayLineL (pos);
            case DelayChannel::Right: return readDelayLineR (pos);
            case DelayChannel::Mono:  return readDelayLineMono (pos);
        }
        return readDelayLineMono (pos);
    };

    if (std::abs (semitones) < 0.001f)
        return readDL (delaySamples);

    auto& vs = varispeedState[phaseIndex];
    const float ratio = std::pow (2.0f, semitones / 12.0f);
    const float driftInc = ratio - 1.0f;

    // Advance drift: read pointer moves at 'ratio' speed relative to write
    vs.drift += driftInc;

    // Max drift before crossfade: cap at ~83ms (4000 samples) or 40% of delay
    float maxDrift = juce::jlimit (480.0f, 4000.0f, delaySamples * 0.4f);

    // During crossfade: blend fading head with new primary head
    if (vs.crossfadeRemaining > 0)
    {
        float primaryPos = delaySamples - vs.drift;
        float primary = readDL (primaryPos);

        float fadingPos = delaySamples - vs.fadingDrift;
        float fading = readDL (fadingPos);
        vs.fadingDrift += driftInc;

        float t = 1.0f - static_cast<float> (vs.crossfadeRemaining)
                       / static_cast<float> (vs.crossfadeLength);
        float halfPi = juce::MathConstants<float>::halfPi;
        float gainNew = std::sin (t * halfPi);
        float gainOld = std::cos (t * halfPi);

        vs.crossfadeRemaining--;
        return primary * gainNew + fading * gainOld;
    }

    // Check if drift exceeded max → initiate crossfade to nominal position
    if (std::abs (vs.drift) > maxDrift)
    {
        vs.fadingDrift = vs.drift;
        vs.drift = 0.0f;
        vs.crossfadeLength = 480;   // ~10ms @ 48kHz
        vs.crossfadeRemaining = vs.crossfadeLength;

        // First crossfade sample: fully on the fading (old) head
        float fadingPos = delaySamples - vs.fadingDrift;
        float fading = readDL (fadingPos);
        vs.fadingDrift += driftInc;

        vs.crossfadeRemaining--;
        return fading;  // gainOld=1, gainNew=0 at t=0
    }

    // Normal operation: single head reading at drifted position
    return readDL (delaySamples - vs.drift);
}

//==============================================================================
// v0.9: WSOLA-Lite Per-Tap Pitch Shifter — Timing-Preserving
// Reads from a small per-tap circular buffer at the pitch ratio rate.
// When read-write drift exceeds grain size, Hann crossfade resets to nominal.
// Global cumulative pitch + doppler stay on varispeed (tape character).
// Per-tap pitch uses this path to avoid timing drift.
//==============================================================================
float OpenSpatialDelayProcessor::wsolaProcess (int objectIndex, float inputSample,
                                                 float perTapSemitones)
{
    auto& ws = wsolaState[objectIndex];

    // Write incoming sample into circular buffer
    ws.buffer[ws.writePos & WSOLAState::kBufMask] = inputSample;
    ws.writePos++;

    const float ratio = std::pow (2.0f, perTapSemitones / 12.0f);

    // Advance read phase by pitch ratio
    ws.readPhase += ratio;

    // Catmull-Rom interpolated read from WSOLA buffer
    auto readBuf = [&](float phase) -> float {
        float wrapped = std::fmod (phase, static_cast<float> (WSOLAState::kBufSize));
        if (wrapped < 0.0f) wrapped += static_cast<float> (WSOLAState::kBufSize);

        int   i1 = static_cast<int> (wrapped);
        float f  = wrapped - static_cast<float> (i1);

        int i0 = (i1 - 1) & WSOLAState::kBufMask;
        int i2 = (i1 + 1) & WSOLAState::kBufMask;
        int i3 = (i1 + 2) & WSOLAState::kBufMask;
        i1 = i1 & WSOLAState::kBufMask;

        float y0 = ws.buffer[i0], y1 = ws.buffer[i1];
        float y2 = ws.buffer[i2], y3 = ws.buffer[i3];

        return y1 + 0.5f * f * (y2 - y0 + f * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3
                                                  + f * (3.0f * (y1 - y2) + y3 - y0)));
    };

    // During crossfade: blend fading grain with new primary grain
    if (ws.crossfadeRemaining > 0)
    {
        float primary = readBuf (ws.readPhase);
        float fading  = readBuf (ws.fadingPhase);
        ws.fadingPhase += ratio;

        float t = 1.0f - static_cast<float> (ws.crossfadeRemaining)
                       / static_cast<float> (WSOLAState::kCrossfadeLen);
        float halfPi = juce::MathConstants<float>::halfPi;
        float gainNew = std::sin (t * halfPi);
        float gainOld = std::cos (t * halfPi);

        ws.crossfadeRemaining--;
        return primary * gainNew + fading * gainOld;
    }

    // Check if read-write drift exceeds grain size → initiate crossfade
    float drift = ws.readPhase - static_cast<float> (ws.writePos);
    if (std::abs (drift) > static_cast<float> (WSOLAState::kGrainSize))
    {
        ws.fadingPhase = ws.readPhase;
        // Reset read phase to just behind write position (nominal latency ~1 grain)
        ws.readPhase = static_cast<float> (ws.writePos) - static_cast<float> (WSOLAState::kGrainSize);
        ws.crossfadeRemaining = WSOLAState::kCrossfadeLen;

        // First crossfade sample: fully on fading head
        float fading = readBuf (ws.fadingPhase);
        ws.fadingPhase += ratio;
        ws.crossfadeRemaining--;
        return fading;
    }

    return readBuf (ws.readPhase);
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

    // v0.9: When trajectory is active, return the animated position (not the origin/knob values)
    if (trajectoryActive[objectIndex].load (std::memory_order_relaxed))
    {
        state.azimuthDeg   = trajectoryFinalAz[objectIndex];
        state.elevationDeg = trajectoryFinalEl[objectIndex];
        state.distance     = trajectoryFinalDist[objectIndex];
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
    TrajectoryState ts;
    if (objectIndex < 0 || objectIndex >= MAX_OBJECTS)
        return ts;

    ts.originAzDeg = baseAzimuth[objectIndex];
    ts.originElDeg = baseElevation[objectIndex];
    ts.originDist  = baseDistance[objectIndex];
    ts.shape       = prevTrajectoryShape[objectIndex];
    ts.phase       = trajectoryPhase[objectIndex];
    ts.reverse     = (cachedParam_trajectoryDirection[objectIndex] == nullptr
                      || cachedParam_trajectoryDirection[objectIndex]->load() < 0.5f);
    ts.randomTime  = randomTime[objectIndex];
    return ts;
}

OpenSpatialDelayProcessor::RandomPosition
OpenSpatialDelayProcessor::evaluateRandomNoise (int objectIndex, float time) const
{
    RandomPosition rp { 0.0f, 0.0f, 0.0f };
    if (objectIndex < 0 || objectIndex >= MAX_OBJECTS || ! randomNoise[objectIndex].initialized)
        return rp;

    auto& rn = randomNoise[objectIndex];
    float p = time * juce::MathConstants<float>::twoPi;

    for (int k = 0; k < 4; ++k)
    {
        rp.azDeg += rn.ampAz[k] * std::sin (p * rn.freqAz[k] + rn.phaseAz[k]);
        rp.elDeg += rn.ampEl[k] * std::sin (p * rn.freqEl[k] + rn.phaseEl[k]);
    }
    for (int k = 0; k < 3; ++k)
        rp.dist += rn.ampDist[k] * std::sin (p * rn.freqDist[k] + rn.phaseDist[k]);

    return rp;
}

// #############################################################################
// DELAY-SPECIFIC — processBlock, render methods, and DSP helpers
// processBlock dispatches to 5 rendering paths. The render methods combine
// the delay engine (above) with the spatial framework (algorithms, HRTF).
// Other SML plugins would replace everything below with their own processBlock
// and render logic, calling the spatial algorithms via computeGains().
// #############################################################################

// --- DELAY-SPECIFIC: Wobble modulation (v0.9) --- tape wow/flutter emulation
// 4 incommensurate sinusoids spanning wow (1.5 Hz) to flutter (8.1 Hz).
// Morph crossfades weight distribution: wow-heavy at 0%, flutter-heavy at 100%.
static constexpr int   kWobbleLayers = 4;
static constexpr float kWobbleRates[kWobbleLayers]    = { 1.5f, 3.7f, 5.9f, 8.1f };
static constexpr float kWowWeights[kWobbleLayers]     = { 0.55f, 0.30f, 0.10f, 0.05f };
static constexpr float kFlutterWeights[kWobbleLayers] = { 0.05f, 0.10f, 0.30f, 0.55f };

inline float OpenSpatialDelayProcessor::applyWobble (float baseDelaySamples)
{
    if (blockWobbleAmount <= 0.0f)
        return baseDelaySamples;

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

    constexpr float maxDeviation = 0.006f;  // ±0.6% of delay time at max amount
    return baseDelaySamples * (1.0f + blockWobbleAmount * modulation * maxDeviation);
}

//==============================================================================
// v0.5: Shared inline helpers for render methods
//==============================================================================
float OpenSpatialDelayProcessor::readObjectSample (int objectIndex, float baseDelaySamples,
                                                    float pitchSemitones)
{
    float objDelaySamples = static_cast<float> (objectIndex + 1) * baseDelaySamples;
    objDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), objDelaySamples);

    // v0.9: Split pitch into global (varispeed, tape-speed) and per-tap (WSOLA-lite, timing-preserving).
    // Global cumulative pitch + doppler → varispeed (preserves tape character, all taps drift together).
    // Per-tap additive pitch → WSOLA-lite (preserves rhythmic timing, no drift between taps).
    float perTapPitch = cachedObj[objectIndex].pitchShift->load (std::memory_order_relaxed);
    float globalPitch = static_cast<float> (objectIndex + 1) * pitchSemitones
                      + dopplerSemitones[objectIndex];

    // v0.8: Per-tap input channel routing
    int inputCh = static_cast<int> (cachedObj[objectIndex].inputChannel->load (std::memory_order_relaxed));
    DelayChannel ch = (inputCh == 1) ? DelayChannel::Left : (inputCh == 2) ? DelayChannel::Right : DelayChannel::Mono;

    float objMono = readVarispeed (objDelaySamples, globalPitch, objectIndex, ch);

    // v0.9: Per-tap pitch via WSOLA-lite (timing-preserving) — only when non-zero
    if (std::abs (perTapPitch) >= 0.001f)
        objMono = wsolaProcess (objectIndex, objMono, perTapPitch);
    // v0.9: True bypass — skip filter entirely when AIR is off (saves 12 IIR evals/sample)
    float result = airAbsorptionActive
                 ? airAbsorptionFilter[objectIndex].processSample (objMono)
                 : objMono;

    // v0.7: Per-tap output filter (same coefficients as feedback filter)
    // Ensures first cycle of taps is filtered, not just feedback repeats
    if (! filterBypassed)
        result = tapHPFilter[objectIndex].processSample (
                     tapLPFilter[objectIndex].processSample (result));

    // v0.7: Accumulate per-tap peak for UI activity glow
    float absVal = std::abs (result);
    if (absVal > tapPeakAccum[objectIndex])
        tapPeakAccum[objectIndex] = absVal;

    return result;
}

void OpenSpatialDelayProcessor::processFeedbackSample (float currentLoopMult,
                                                        float baseDelaySamples,
                                                        float pitchSemitones, float fb)
{
    float fbDelaySamples = currentLoopMult * baseDelaySamples;
    fbDelaySamples = juce::jlimit (1.0f, static_cast<float> (delayBufferSize - 2), fbDelaySamples);
    float fbPitchShiftAmount = currentLoopMult * pitchSemitones;

    float feedbackRaw = readVarispeed (fbDelaySamples, fbPitchShiftAmount, MAX_OBJECTS, DelayChannel::Mono);
    float filtered = filterBypassed ? feedbackRaw
        : feedbackHPFilter.processSample (feedbackLPFilter.processSample (feedbackRaw));
    const float makeupGain = 1.0f + (fb * fb * kMakeupGainCoeff);
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

    if (delayBufferL.empty() || numSamples <= 0)
        return;

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
    filterBypassed  = cachedParam_filterEnabled->load() < 0.5f;
    int   profileIndex    = static_cast<int> (cachedParam_hrtfProfile->load());
    // v0.7: Global pitch is in cents (±200), convert to semitones for DSP
    float pitchSemitones  = cachedParam_globalPitchShift->load() / 100.0f;

    // v0.4: Air absorption — true bypass with edge detection for immediate toggle response
    airAbsorptionActive = cachedParam_airAbsorption->load() > 0.5f;
    bool airStateChanged = (airAbsorptionActive != prevAirAbsorptionActive);
    prevAirAbsorptionActive = airAbsorptionActive;

    // v0.8: Wobble modulation (block-rate parameter reads, gated by wobbleEnabled)
    {
        bool wobbleEnabled = cachedParam_wobbleEnabled->load() > 0.5f;
        blockWobbleAmount = wobbleEnabled ? (cachedParam_wobbleAmount->load() / 100.0f) : 0.0f;
        blockWobbleMorph  = cachedParam_wobbleMorph->load();
    }

    // v0.8: Input format (0=Mono, 1=Stereo) — selects whether dual delay lines receive L/R or summed mono
    int inputFormat = static_cast<int> (cachedParam_inputFormat->load());

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

    // --- Update feedback filters (only when frequency or Q changes) -----------
    bool lpQChanged = std::abs (filterLPQ - cachedFilterLPQ) > 0.001f;
    bool hpQChanged = std::abs (filterHPQ - cachedFilterHPQ) > 0.001f;
    bool lpChanged = std::abs (lpFreq - cachedFeedbackLPFreq) > 0.1f || lpQChanged;
    bool hpChanged = std::abs (hpFreq - cachedFeedbackHPFreq) > 0.1f || hpQChanged;
    if (lpChanged)
    {
        auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, lpFreq, filterLPQ);
        feedbackLPFilter.coefficients = lpCoeffs;
        for (int t = 0; t < MAX_OBJECTS; ++t)
            tapLPFilter[t].coefficients = lpCoeffs;
        cachedFeedbackLPFreq = lpFreq;
    }
    if (hpChanged)
    {
        auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (currentSampleRate, hpFreq, filterHPQ);
        feedbackHPFilter.coefficients = hpCoeffs;
        for (int t = 0; t < MAX_OBJECTS; ++t)
            tapHPFilter[t].coefficients = hpCoeffs;
        cachedFeedbackHPFreq = hpFreq;
    }
    if (lpQChanged) cachedFilterLPQ = filterLPQ;
    if (hpQChanged) cachedFilterHPQ = filterHPQ;

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
    int algorithmIndex = static_cast<int> (cachedParam_algorithm->load());
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
        objects[t].enabled      = cachedObj[t].enabled->load()   > 0.5f;

        // v0.9: When trajectory is active, read animated position from internal arrays
        // (the knobs/APVTS hold the origin; the internal arrays hold origin + trajectory offset)
        if (trajectoryActive[t].load (std::memory_order_relaxed))
        {
            objects[t].azimuthDeg   = trajectoryFinalAz[t];
            objects[t].elevationDeg = trajectoryFinalEl[t];
            objects[t].distance     = trajectoryFinalDist[t];
        }
        else
        {
            objects[t].azimuthDeg   = cachedObj[t].azimuth->load();
            objects[t].elevationDeg = cachedObj[t].elevation->load();
            objects[t].distance     = cachedObj[t].distance->load();
        }

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
        constexpr float emaAlpha = 0.2f;          // Smoothing factor for velocity
        constexpr float maxDopplerSemitones = 12.0f;

        for (int t = 0; t < MAX_OBJECTS; ++t)
        {
            if (! objects[t].enabled)
            {
                dopplerSemitones[t] = 0.0f;
                // v1.0: Use pre-computed transparent coefficients (no heap allocation)
                *airAbsorptionFilter[t].coefficients = airTransparentCoeffs;
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
            float objDopplerAmount = cachedObj[t].dopplerAmount->load();

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

                    // Virtual ear Doppler — offset listener position for audible
                    // binaural Doppler from orbiting/tangentially moving sources.
                    // Anatomical ear offset is ~0.085m; exaggerated for creative effect.
                    constexpr float earOffset = 2.5f;   // virtual ear x-offset (meters, exaggerated for creative effect)
                    float dxC = cx - earOffset;
                    float dxP = px - earOffset;
                    float distToEarCur  = std::sqrt (dxC * dxC + cy * cy + cz * cz);
                    float distToEarPrev = std::sqrt (dxP * dxP + py * py + pz * pz);
                    float rawRadialVelocity = (distToEarCur - distToEarPrev) / blockDuration;

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

            // --- Air absorption filter coefficient update ---
            // v0.9: Update on position change OR AIR toggle state change (true bypass fix).
            // When AIR is OFF, readObjectSample() skips the filter entirely (zero CPU).
            if (airAbsorptionActive && (positionChanged || airStateChanged))
            {
                // Quadratic distance mapping: gentle near (0–0.5 ≈ 0–5m), steep far (0.5–1.0 ≈ 5–20m)
                // dist² compresses the near range and expands the far range perceptually.
                float mappedDist = dist * dist;
                constexpr float absorbCoeff = 3.1f;
                float cutoff = 20000.0f * std::exp (-absorbCoeff * mappedDist);
                cutoff = juce::jlimit (500.0f, 20000.0f, cutoff);
                *airAbsorptionFilter[t].coefficients =
                    *juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, cutoff);
            }
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
        const int ambiOrder = outputFormatRegistry[static_cast<size_t> (fmtEnumIdx)].ambiOrder;
        renderAmbisonicsOutput (buffer, numSamples, objects, objDistGain, pitchSemitones, ambiOrder);
    }
    else
    {
        renderDiscreteSurround (buffer, numSamples, objects, objChannelGains,
                                objDistGain, pitchSemitones, surLayout);
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
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples);  // v0.9: Tape wobble
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain = smoothedInputGain.getNextValue();
        /* dw */       smoothedDryWet.getNextValue();
        float fb     = smoothedFeedback.getNextValue();
        /* outGain */  smoothedOutputGain.getNextValue();

        // STAGE 1: WRITE to delay line (dual L/R)
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);
        // rawInput not needed here — PASS 3 reads monoInputBuffer directly

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

        outL[s] = outputLimiter ((rawInput * (1.0f - dw) + wetBufL[static_cast<size_t> (s)] * dw) * outGain);
        outR[s] = outputLimiter ((rawInput * (1.0f - dw) + wetBufR[static_cast<size_t> (s)] * dw) * outGain);
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
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples);  // v0.9: Tape wobble
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float dw      = smoothedDryWet.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();

        // === STAGE 1: WRITE (dual L/R) ===
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);
        float rawInput = monoInputBuffer[static_cast<size_t> (s)];  // for dry mix (no inGain — INPUT only scales delay input)

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
        outL[s] = outputLimiter ((rawInput * (1.0f - dw) + wetL * dw) * outGain);
        outR[s] = outputLimiter ((rawInput * (1.0f - dw) + wetR * dw) * outGain);
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
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples);  // v0.9: Tape wobble
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float dw      = smoothedDryWet.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();

        // === STAGE 1: WRITE (dual L/R) ===
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);
        float rawInput = monoInputBuffer[static_cast<size_t> (s)];  // for dry mix (no inGain — INPUT only scales delay input)

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
        outL[s] = outputLimiter ((rawInput * (1.0f - dw) + wetL * dw) * outGain);
        outR[s] = outputLimiter ((rawInput * (1.0f - dw) + wetR * dw) * outGain);
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
    // v0.7: Cached — only recomputed when ambiOrder changes (rare: output format switch)
    if (ambiOrder != cachedMaxrEOrder)
    {
        const float maxrEDenom = 2.0f * static_cast<float> (ambiOrder) + 2.0f;
        for (int n = 0; n <= ambiOrder; ++n)
            cachedMaxrE[n] = std::cos (static_cast<float> (n) * juce::MathConstants<float>::pi / maxrEDenom);
        cachedMaxrEOrder = ambiOrder;
    }

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
            objSHCoeffs[t][c] = evalSH (c, azRad, elRad) * cachedMaxrE[acnToOrder (c)];
    }

    // Get output channel write pointers
    float* outChannels[MAX_AMBI_CHANNELS] = {};
    int numOutCh = buffer.getNumChannels();
    for (int ch = 0; ch < numOutCh && ch < MAX_AMBI_CHANNELS; ++ch)
        outChannels[ch] = buffer.getWritePointer (ch);

    for (int s = 0; s < numSamples; ++s)
    {
        float currentDelayMs = smoothedDelayTime.getNextValue();
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples);  // v0.9: Tape wobble
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float dw      = smoothedDryWet.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();

        // === STAGE 1: WRITE (dual L/R) ===
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);
        float rawInput = monoInputBuffer[static_cast<size_t> (s)];  // for dry mix (no inGain — INPUT only scales delay input)

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
                outChannels[c][s] = outputLimiter (ambiAccum[c] * dw * outGain);
        }

        // Dry: omnidirectional (W channel = ACN 0 only)
        if (outChannels[0] != nullptr)
            outChannels[0][s] = outputLimiter (outChannels[0][s] + rawInput * (1.0f - dw) * outGain);
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
        float baseDelaySamples = currentDelayMs * blockMsToSamples;
        baseDelaySamples = applyWobble (baseDelaySamples);  // v0.9: Tape wobble
        float currentLoopMult = smoothedLoopMultiplier.getNextValue();

        float inGain  = smoothedInputGain.getNextValue();
        float dw      = smoothedDryWet.getNextValue();
        float fb      = smoothedFeedback.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();

        // === STAGE 1: WRITE (dual L/R) ===
        float rawL = inputBufferL[static_cast<size_t> (s)] * inGain;
        float rawR = inputBufferR[static_cast<size_t> (s)] * inGain;
        float delayInputL = softClip ((rawL + feedbackSample * fb) * kFeedbackInputHeadroom);
        float delayInputR = softClip ((rawR + feedbackSample * fb) * kFeedbackInputHeadroom);
        writeDelayLine (delayInputL, delayInputR);
        float rawInput = monoInputBuffer[static_cast<size_t> (s)];  // for dry mix (no inGain — INPUT only scales delay input)

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
                outChannels[ch][s] = outputLimiter (channelAccum[sp] * dw * outGain);
        }

        // Dry signal → L and R only (channels 0 and 1)
        float drySignal = rawInput * (1.0f - dw) * outGain;
        if (outChannels[0] != nullptr) outChannels[0][s] = outputLimiter (outChannels[0][s] + drySignal);
        if (outChannels[1] != nullptr) outChannels[1][s] = outputLimiter (outChannels[1][s] + drySignal);

        // LFE generation — low-pass filtered mono sum at −10 dB
        if (lfeIdx >= 0 && lfeIdx < numOutCh && outChannels[lfeIdx] != nullptr)
        {
            float lfeSig = outputLimiter (lfeFilter.processSample (wetMono) * 0.316f * dw * outGain);  // −10 dB ≈ 0.316
            outChannels[lfeIdx][s] = lfeSig;
        }
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
                           cachedObj[objIdx].elevation->load(),
                           cachedObj[objIdx].distance->load());
    }
    else if (property == "/elev" && message.size() >= 1 && message[0].isFloat32())
    {
        float elDeg = message[0].getFloat32();
        handleOSCPosition (objIdx,
                           cachedObj[objIdx].azimuth->load(),
                           elDeg,
                           cachedObj[objIdx].distance->load());
    }
    else if (property == "/dist" && message.size() >= 1 && message[0].isFloat32())
    {
        float dist = message[0].getFloat32();
        handleOSCPosition (objIdx,
                           cachedObj[objIdx].azimuth->load(),
                           cachedObj[objIdx].elevation->load(),
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
    if (! trajectoryActive[objIdx].load (std::memory_order_relaxed))
    {
        oscOverrideActive[objIdx].store (true, std::memory_order_relaxed);
        oscLastReceiveTime[objIdx] = juce::Time::getMillisecondCounterHiRes();
    }
}

// #############################################################################
// DELAY-SPECIFIC: Trajectory Animation
// Per-plugin modulation — each SML plugin implements its own trajectory shapes.
// #############################################################################

//==============================================================================
// Trajectory shape computation — pure function, no side effects
//==============================================================================
OpenSpatialDelayProcessor::TrajectoryResult
OpenSpatialDelayProcessor::computeTrajectory (int shape, float phase,
                                               float baseAz, float baseEl, float baseDist,
                                               bool reverse)
{
    // v0.8: Reverse direction — flip phase so trajectory runs backwards
    if (reverse)
        phase = 1.0f - phase;

    TrajectoryResult r;
    r.controlsAz = false;
    r.controlsEl = false;
    r.controlsDist = false;

    // Issue #3: distance amplitude scales inversely with origin distance
    const float distScale = 1.0f - baseDist;

    switch (shape)
    {
        case 1: // Bounce — azimuth ping-pongs ±90°, elevation bounces ±30°
        {
            float tri = 1.0f - std::abs (2.0f * phase - 1.0f);
            r.azDeg = baseAz - 90.0f * (2.0f * tri - 1.0f);
            r.elDeg = baseEl - 30.0f * (2.0f * tri - 1.0f);
            r.dist  = baseDist;
            r.controlsAz = r.controlsEl = true;
            break;
        }

        case 2: // Circle — circular path around origin (Cartesian→polar)
        {
            const float circleR = 1.0f * distScale;
            float p = phase * juce::MathConstants<float>::twoPi;
            float localX = circleR * std::sin (p);
            float localY = circleR * std::cos (p);
            // Rotate by baseAz and offset by baseDist
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 3: // Cross — plus-sign pattern, 4 cardinal sweeps (az ↔ el)
        {
            float p = phase * 4.0f;
            int segment = juce::jlimit (0, 3, static_cast<int> (p));
            float t = p - static_cast<float> (segment);
            float tri = 1.0f - std::abs (2.0f * t - 1.0f); // 0→1→0 within segment
            if (segment == 0)      { r.azDeg = baseAz;                       r.elDeg = baseEl + 60.0f * tri; }
            else if (segment == 1) { r.azDeg = baseAz + 90.0f * tri;         r.elDeg = baseEl; }
            else if (segment == 2) { r.azDeg = baseAz;                       r.elDeg = baseEl - 60.0f * tri; }
            else                   { r.azDeg = baseAz - 90.0f * tri;         r.elDeg = baseEl; }
            r.dist = baseDist;
            r.controlsAz = r.controlsEl = true;
            break;
        }

        case 4: // Figure-8 — front CCW, back CW, origin-relative (Cartesian→polar)
        {
            const float loopR = 0.5f * distScale;
            constexpr float halfPi = juce::MathConstants<float>::halfPi;
            constexpr float twoPi  = juce::MathConstants<float>::twoPi;
            // Invert phase for Figure-8 so Forward button = correct traversal direction (#13)
            float fig8Phase = 1.0f - phase;
            float localX, localY;
            if (fig8Phase < 0.5f)
            {
                float t = fig8Phase * 2.0f;
                float theta = -halfPi + twoPi * t;
                localX = loopR * std::cos (theta);
                localY = loopR + loopR * std::sin (theta);
            }
            else
            {
                float t = (fig8Phase - 0.5f) * 2.0f;
                float theta = halfPi - twoPi * t;
                localX = loopR * std::cos (theta);
                localY = -loopR + loopR * std::sin (theta);
            }
            // Rotate local shape by baseAz and offset by baseDist (like Line shape)
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            // Rotate local coords by baseAz
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float rotX = localX * cosA + localY * sinA;
            float rotY = -localX * sinA + localY * cosA;
            float mapX = baseCx + rotX;
            float mapY = baseCy + rotY;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsEl = r.controlsDist = true;
            break;
        }

        case 5: // Heart — parametric heart curve (Cartesian→polar), origin-relative
        {
            float p = phase * juce::MathConstants<float>::twoPi;
            // Standard parametric heart: x = 16sin³(t), y = 13cos(t) - 5cos(2t) - 2cos(3t) - cos(4t)
            float sinP = std::sin (p);
            float hx = 16.0f * sinP * sinP * sinP;
            float hy = 13.0f * std::cos (p) - 5.0f * std::cos (2.0f * p)
                      - 2.0f * std::cos (3.0f * p) - std::cos (4.0f * p);
            // Normalize to ~0.35 radius (heart spans roughly -16..16 x, -17..15 y)
            const float scale = (1.0f / 17.0f) * distScale;
            float localX = hx * scale;
            float localY = hy * scale;  // point-down orientation (Y+ = front)
            // Rotate by baseAz and offset by baseDist
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 6: // Helix — orbit with cosine-eased elevation ramp (3D corkscrew)
        {
            r.azDeg = baseAz + 360.0f * phase;  // positive = CW on spatial map (with Forward→reverse phase inversion)
            // Cosine easing: slows at -90° and +90° peaks (ease in/out)
            float easedPhase = 0.5f * (1.0f - std::cos (phase * juce::MathConstants<float>::pi));
            r.elDeg = 90.0f - 180.0f * easedPhase;  // inverted: bottom-to-top in Forward (#14)
            r.dist  = baseDist;
            r.controlsAz = r.controlsEl = true;
            break;
        }

        case 7: // Infinity — Bernoulli lemniscate (Cartesian→polar), origin-relative
        {
            float p = phase * juce::MathConstants<float>::twoPi;
            // Lemniscate of Bernoulli: x = a*cos(t)/(1+sin²(t)), y = a*sin(t)*cos(t)/(1+sin²(t))
            const float a = 1.0f * distScale;
            float sinP = std::sin (p);
            float cosP = std::cos (p);
            float denom = 1.0f + sinP * sinP;
            float localX = a * cosP / denom;
            float localY = a * sinP * cosP / denom;
            // Rotate by baseAz and offset by baseDist
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 8: // Line — sweep rotated by baseAz (Cartesian→polar)
        {
            const float amplitude = 1.0f * distScale;
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            // Local sweep along X axis, then rotate by baseAz
            float p = phase * juce::MathConstants<float>::twoPi;
            float localX = amplitude * std::cos (p);
            float localY = 0.0f;
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 9: // Orbit — circular orbit (azimuth only)
            r.azDeg = baseAz + 360.0f * phase;
            r.elDeg = baseEl;
            r.dist  = baseDist;
            r.controlsAz = true;
            break;

        case 10: // Random — multi-layer irrational-frequency oscillators (all axes, half-speed)
        {
            float p = phase * juce::MathConstants<float>::twoPi;
            r.azDeg = baseAz + 50.0f * std::sin (p * 0.5f)
                             + 35.0f * std::sin (p * 1.3592f + 0.7f)
                             + 20.0f * std::sin (p * 2.3346f + 2.1f)
                             + 12.0f * std::sin (p * 3.6946f + 4.3f);
            r.elDeg = baseEl + 30.0f * std::sin (p * 0.7071f + 1.1f)
                             + 20.0f * std::sin (p * 1.5708f + 3.5f)
                             + 12.0f * std::sin (p * 2.9299f + 0.3f)
                             +  8.0f * std::sin (p * 4.2699f + 5.7f);
            r.dist  = juce::jlimit (0.0f, 1.0f,
                                    baseDist + 0.50f * distScale * std::sin (p * 0.8661f + 2.3f)
                                             + 0.30f * distScale * std::sin (p * 1.9365f + 4.9f)
                                             + 0.20f * distScale * std::sin (p * 3.1416f + 1.6f));
            r.controlsAz = r.controlsEl = r.controlsDist = true;
            break;
        }

        case 11: // Spiral — Archimedean spiral from origin outward (Cartesian→polar)
        {
            constexpr float numTurns = 1.75f;
            const float maxRadius = 1.0f * distScale;
            // Phase 0→1 spirals outward (origin→edge), positive theta = CW on spatial map
            float theta = juce::MathConstants<float>::twoPi * numTurns * (1.0f - phase);
            float rLocal = maxRadius * (1.0f - phase);  // inverted: origin→outward (#15)
            float localX = rLocal * std::cos (theta);
            float localY = rLocal * std::sin (theta);
            // Rotate by baseAz and offset by baseDist
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 12: // Square — 4-corner rectangular path, origin-relative (Cartesian→polar)
        {
            const float halfSide = 0.7071f * distScale;
            const float lcx[4] = { -halfSide,  halfSide,  halfSide, -halfSide };
            const float lcy[4] = {  halfSide,  halfSide, -halfSide, -halfSide };
            float p = phase * 4.0f;
            int edge = juce::jlimit (0, 3, static_cast<int> (p));
            float t = p - static_cast<float> (edge);
            float ease = 0.5f * (1.0f - std::cos (t * juce::MathConstants<float>::pi));
            int next = (edge + 1) & 3;
            float localX = lcx[edge] + (lcx[next] - lcx[edge]) * ease;
            float localY = lcy[edge] + (lcy[next] - lcy[edge]) * ease;
            // Rotate by baseAz and offset by baseDist
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsEl = r.controlsDist = true;
            break;
        }

        case 13: // Triangle — equilateral triangle, origin-relative (Cartesian→polar)
        {
            const float circumR = 1.0f * distScale;
            const float lvx[3] = { 0.0f,  circumR * 0.8660254f, -circumR * 0.8660254f };
            const float lvy[3] = { circumR, -circumR * 0.5f, -circumR * 0.5f };
            float p = phase * 3.0f;
            int side = juce::jlimit (0, 2, static_cast<int> (p));
            float t = p - static_cast<float> (side);
            float ease = 0.5f * (1.0f - std::cos (t * juce::MathConstants<float>::pi));
            int next = (side + 1) % 3;
            float localX = lvx[side] + (lvx[next] - lvx[side]) * ease;
            float localY = lvy[side] + (lvy[next] - lvy[side]) * ease;
            // Rotate by baseAz and offset by baseDist
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsEl = r.controlsDist = true;
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
    state.setProperty ("pluginStateVersion", 15, nullptr);  // v0.9 state format (15 = Circle shape + 14 trajectories)
    state.setProperty ("oscReceivePort", oscReceivePort, nullptr);  // v0.6: persist OSC port
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

    // v0.6: Restore OSC receive port (non-APVTS property)
    oscReceivePort = static_cast<int> (tree.getProperty ("oscReceivePort", 4002));

    // v0.6: Restore preset index (non-APVTS property)
    currentPresetIndex = static_cast<int> (tree.getProperty ("currentPresetIndex", 0));

    // v0.7: Restore OSC Send settings
    oscSendPort = static_cast<int> (tree.getProperty ("oscSendPort", 4003));
    oscSendIP   = tree.getProperty ("oscSendIP", "127.0.0.1").toString();
    bool savedSendEnabled = static_cast<bool> (tree.getProperty ("oscSendEnabled", false));
    if (savedSendEnabled)
        setOscSendEnabled (true);

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
