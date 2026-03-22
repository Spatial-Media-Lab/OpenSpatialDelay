#pragma once

#include <juce_core/juce_core.h>

static constexpr int PRESET_MAX_OBJECTS = 12;

//==============================================================================
// PresetData — standalone preset struct for shared use by plugin and CLI tools
//==============================================================================
struct PresetData
{
    juce::String name;
    juce::String category;  // v0.9: preset category for browser grouping
    // Global params
    float delayTime = 500.0f;
    bool  tempoSync = false;
    float noteDivision = 4.0f;
    int   syncMode = 0;
    float feedback = 0.3f;
    float filterLP = 20000.0f;
    float filterHP = 20.0f;
    float filterLPQ = 0.707f;    // v0.9: LP resonance (Q), default Butterworth
    float filterHPQ = 0.707f;    // v0.9: HP resonance (Q), default Butterworth
    float dryWet = 0.5f;
    float inputGain = 0.0f;
    float outputGain = 0.0f;
    bool  airAbsorption = false;
    bool  filterEnabled = false; // v0.9: filter on/off (persisted in presets)
    bool  wobbleEnabled = false; // v0.8: Wobble modulation enable toggle
    float wobbleAmount = 0.0f;   // v0.8: Wobble modulation depth (0..100)
    float wobbleMorph = 0.0f;    // v0.8: Wobble waveform morph (0..100)
    int   algorithm = 4;       // VBAP
    int   hrtfProfile = 0;
    // Per-tap data
    struct TapData
    {
        bool  enabled = false;
        float azimuthDeg = 0.0f;
        float elevationDeg = 0.0f;
        float distance = 0.5f;
        float dopplerAmount = 0.0f;
        float pitchShift = 0.0f;      // per-tap pitch shift (semitones, 0=no shift)
        int   trajectoryShape = 0;
        float trajectorySpeed = 1.0f;
        int   trajectoryDirection = 0;  // v0.8: 0=Forward, 1=Reverse
        int   inputChannel = 0;           // v0.8: 0=L+R, 1=L, 2=R
    };
    TapData taps[PRESET_MAX_OBJECTS] = {};
    bool isFactory = false;  // v0.9: true for factory presets written to disk
};

//==============================================================================
// v0.9: Trajectory string ID ↔ index mapping (for preset serialization)
//==============================================================================
int          trajectoryStringToIndex (const juce::String& id);
juce::String trajectoryIndexToString (int index);
int          trajectoryLegacyToNewIndex (int oldIndex);

//==============================================================================
// Free function declarations
//==============================================================================
juce::String serializePresetToJson (const PresetData& pd);
PresetData   parsePresetJson (const juce::String& json);

//==============================================================================
// Extern declarations for factory preset data
//==============================================================================
extern const PresetData factoryPresets[];
extern const int NUM_FACTORY_PRESETS;
extern const int NUM_PRESET_CATEGORIES;
extern const char* const presetCategoryNames[];
