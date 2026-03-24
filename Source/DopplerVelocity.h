#pragma once

#include <cmath>
#include <algorithm>

//==============================================================================
// Doppler Velocity Tracking — Per-Object Pitch Shift from Spatial Movement
// Extracted from PluginProcessor for independent testability and reuse.
// Tracks radial velocity to a virtual ear offset and computes pitch in semitones.
//==============================================================================
class DopplerVelocity
{
public:
    static constexpr int kMaxObjects = 12;

    /** Update Doppler for a single object based on its current spatial position.
        Call once per block per enabled object. */
    void update (int objectIndex, float azRad, float elRad, float dist,
                 float dopplerAmount, float blockDuration);

    /** Clear Doppler for a disabled object (resets raw semitones to 0). */
    void clearDisabled (int objectIndex);

    /** Apply per-block EMA smoothing to raw dopplerSemitones.
        Call after all update() calls for the block. */
    void smooth (int objectIndex);

    /** Reset all state for a specific object (e.g., after preset change). */
    void reset (int objectIndex, float azRad, float elRad, float dist);

    /** Reset all objects. */
    void resetAll();

    /** Get the smoothed Doppler pitch in semitones. */
    float getSmoothedSemitones (int objectIndex) const { return smoothedSemitones_[objectIndex]; }

    /** Get the raw (unsmoothed) Doppler pitch in semitones. */
    float getRawSemitones (int objectIndex) const { return rawSemitones_[objectIndex]; }

    /** Check if position changed since last block (set during update). */
    bool positionChanged (int objectIndex) const { return posChanged_[objectIndex]; }

private:
    // Previous block positions (radians / normalized)
    float prevAz_[kMaxObjects]   = {};
    float prevEl_[kMaxObjects]   = {};
    float prevDist_[kMaxObjects] = {};

    // Velocity and pitch state
    float smoothedVelocity_[kMaxObjects] = {};
    float rawSemitones_[kMaxObjects] = {};
    float smoothedSemitones_[kMaxObjects] = {};
    bool  posChanged_[kMaxObjects] = {};

    // Constants
    static constexpr float kSpeedOfSound = 343.0f;
    static constexpr float kVelocityAlpha = 0.35f;
    static constexpr float kMaxSemitones = 12.0f;
    static constexpr float kEarOffset = 2.5f;
    static constexpr float kSmoothAlpha = 0.15f;
};
