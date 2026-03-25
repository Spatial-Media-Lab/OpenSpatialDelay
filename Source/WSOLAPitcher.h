#pragma once

#include <cmath>
#include <algorithm>
#include <juce_core/juce_core.h>

//==============================================================================
// WSOLA-Lite Per-Tap Pitch Shifter — Timing-Preserving
// Extracted from PluginProcessor for independent testability and reuse.
// Reads from a small per-object circular buffer at the pitch ratio rate.
// When read-write drift exceeds grain size, Hann crossfade resets to nominal.
//==============================================================================
class WSOLAPitcher
{
public:
    static constexpr int kMaxObjects = 12;

    struct State
    {
        // v1.0.3: Doubled buffer/grain/crossfade for better WSOLA quality (issue #53).
        // Larger grains = fewer crossfade resets = cleaner pitch shifting.
        // Memory: 4096 × 4 bytes × 12 taps = 192KB (trivial).
        static constexpr int kBufSize = 4096;        // ~85ms at 48kHz, power of 2
        static constexpr int kBufMask = kBufSize - 1;
        static constexpr int kGrainSize = 2048;      // ~42ms grain
        static constexpr int kCrossfadeLen = 1024;   // ~21ms crossfade (50% overlap)

        float buffer[kBufSize] = {};
        int   writePos = 0;
        float readPhase = 0.0f;
        float fadingPhase = 0.0f;
        int   crossfadeRemaining = 0;
    };

    /** Process a single sample through WSOLA pitch shifter for the given object. */
    float process (int objectIndex, float inputSample, float semitones);

    /** Update the hysteresis gate. Uses user-only pitch (not combined with Doppler)
        so that Doppler cannot close the gate when the user has set a pitch shift.
        Returns true if the gate is now open. */
    bool updateGate (int objectIndex, float userPitch);

    /** Keep the WSOLA buffer populated during bypass (gate closed).
        This ensures fresh audio is available on reactivation. */
    void bypass (int objectIndex, float inputSample);

    /** Reset state for a single object (e.g., after preset change). */
    void reset (int objectIndex);

    /** Reset all objects. */
    void resetAll();

    bool isGateOpen (int objectIndex) const { return gateOpen_[objectIndex]; }

private:
    State states_[kMaxObjects] = {};
    bool  gateOpen_[kMaxObjects] = {};
};
