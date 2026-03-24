#pragma once

#include <juce_dsp/juce_dsp.h>

//==============================================================================
// FilterBank — Feedback, Per-Tap, and Air Absorption Filter Management
// Extracted from PluginProcessor for independent testability and reuse.
// Manages IIR filter state, EMA-smoothed coefficient updates, and bypass logic.
//==============================================================================
class FilterBank
{
public:
    static constexpr int kMaxObjects = 12;

    /** Prepare all filters for the given sample rate and block size. */
    void prepare (double sampleRate, int samplesPerBlock);

    /** Block-rate: update feedback/tap filter coefficients with EMA smoothing.
        Only regenerates IIR coefficients when the smoothed values change enough. */
    void updateCoefficients (double sampleRate, float lpFreq, float hpFreq,
                             float lpQ, float hpQ, bool filterEnabled);

    /** Block-rate: update air absorption cutoff for one object based on distance.
        Uses EMA smoothing to prevent IIR coefficient transients. */
    void updateAirAbsorption (int objectIndex, double sampleRate, float dist,
                              bool airActive, bool positionChanged, bool stateChanged);

    /** Clear air absorption for a disabled object (set to transparent). */
    void clearAirAbsorption (int objectIndex);

    /** Per-sample: process through tap LP/HP filters. */
    float processTapSample (int objectIndex, float input);

    /** Per-sample: process through feedback LP/HP filters. */
    float processFeedbackSample (float input);

    /** Per-sample: process through air absorption filter. */
    float processAirSample (int objectIndex, float input);

    /** Reset all filter states (e.g., after preset change). */
    void resetAll();

    /** Reset a single object's tap + air filters. */
    void resetObject (int objectIndex);

    /** Force coefficient recalculation on next updateCoefficients() call. */
    void invalidateCoefficients();

    /** Set smoothed filter frequencies directly (e.g., from preset load). */
    void setSmoothedFrequencies (float lp, float hp, float lpQ, float hpQ);

    bool isFilterBypassed() const { return bypassed_; }
    bool isAirActive() const { return airActive_; }

private:
    // Feedback path filters
    juce::dsp::IIR::Filter<float> fbLP_;
    juce::dsp::IIR::Filter<float> fbHP_;

    // Per-tap output filters (same coefficients as feedback, independent state)
    juce::dsp::IIR::Filter<float> tapLP_[kMaxObjects];
    juce::dsp::IIR::Filter<float> tapHP_[kMaxObjects];

    // Per-object air absorption filters
    juce::dsp::IIR::Filter<float> airFilter_[kMaxObjects];
    juce::dsp::IIR::Coefficients<float> airTransparentCoeffs_;
    float smoothedAirCutoff_[kMaxObjects] = {};

    // EMA-smoothed control state
    float smoothedLP_ = 20000.0f;
    float smoothedHP_ = 20.0f;
    float smoothedLPQ_ = 0.707f;
    float smoothedHPQ_ = 0.707f;

    // Cached for change detection
    float cachedLP_ = -1.0f;
    float cachedHP_ = -1.0f;
    float cachedLPQ_ = -1.0f;
    float cachedHPQ_ = -1.0f;

    bool bypassed_ = true;
    bool airActive_ = false;
};
