#include "DopplerVelocity.h"
#include <juce_core/juce_core.h>

//==============================================================================
void DopplerVelocity::update (int objectIndex, float azRad, float elRad, float dist,
                               float dopplerAmount, float blockDuration)
{
    // Check if position changed since last block
    posChanged_[objectIndex] = (std::abs (azRad - prevAz_[objectIndex]) > 1e-5f)
                            || (std::abs (elRad - prevEl_[objectIndex]) > 1e-5f)
                            || (std::abs (dist  - prevDist_[objectIndex]) > 1e-5f);

    if (dopplerAmount > 0.001f && blockDuration > 0.0f)
    {
        if (posChanged_[objectIndex])
        {
            // Convert to Cartesian (physical scale: 0..10m)
            float physDist     = dist * 10.0f;
            float prevPhysDist = prevDist_[objectIndex] * 10.0f;

            float cx = physDist     * std::cos (elRad) * std::sin (azRad);
            float cy = physDist     * std::cos (elRad) * std::cos (azRad);
            float cz = physDist     * std::sin (elRad);
            float px = prevPhysDist * std::cos (prevEl_[objectIndex]) * std::sin (prevAz_[objectIndex]);
            float py = prevPhysDist * std::cos (prevEl_[objectIndex]) * std::cos (prevAz_[objectIndex]);
            float pz = prevPhysDist * std::sin (prevEl_[objectIndex]);

            // Virtual ear Doppler — offset listener position
            float dxC = cx - kEarOffset;
            float dxP = px - kEarOffset;
            float distToEarCur  = std::sqrt (dxC * dxC + cy * cy + cz * cz);
            float distToEarPrev = std::sqrt (dxP * dxP + py * py + pz * pz);
            float rawVelocity = (distToEarCur - distToEarPrev) / blockDuration;

            // Clamp velocity to prevent extreme pitch transients
            rawVelocity = juce::jlimit (-50.0f, 50.0f, rawVelocity);

            // EMA smoothing
            smoothedVelocity_[objectIndex] = kVelocityAlpha * rawVelocity
                                           + (1.0f - kVelocityAlpha) * smoothedVelocity_[objectIndex];

            // Doppler pitch: semitones = 12 * log2(c / (c + v * amount))
            float v = smoothedVelocity_[objectIndex] * dopplerAmount;
            float denominator = kSpeedOfSound + v;
            if (denominator > 1.0f)
            {
                rawSemitones_[objectIndex] = 12.0f * std::log2 (kSpeedOfSound / denominator);
                rawSemitones_[objectIndex] = juce::jlimit (-kMaxSemitones, kMaxSemitones,
                                                            rawSemitones_[objectIndex]);
            }
            else
            {
                rawSemitones_[objectIndex] = -kMaxSemitones;
            }
        }
        else
        {
            // Position static: decay velocity toward zero
            smoothedVelocity_[objectIndex] *= (1.0f - kVelocityAlpha);
            if (std::abs (smoothedVelocity_[objectIndex]) < 1e-6f)
            {
                smoothedVelocity_[objectIndex] = 0.0f;
                rawSemitones_[objectIndex] = 0.0f;
            }
            else
            {
                float v = smoothedVelocity_[objectIndex] * dopplerAmount;
                float denominator = kSpeedOfSound + v;
                if (denominator > 1.0f)
                {
                    rawSemitones_[objectIndex] = 12.0f * std::log2 (kSpeedOfSound / denominator);
                    rawSemitones_[objectIndex] = juce::jlimit (-kMaxSemitones, kMaxSemitones,
                                                                rawSemitones_[objectIndex]);
                }
                else
                    rawSemitones_[objectIndex] = -kMaxSemitones;
            }
        }
    }
    else
    {
        rawSemitones_[objectIndex] = 0.0f;
        smoothedVelocity_[objectIndex] = 0.0f;
    }

    // Store current position for next block
    prevAz_[objectIndex]   = azRad;
    prevEl_[objectIndex]   = elRad;
    prevDist_[objectIndex] = dist;
}

//==============================================================================
void DopplerVelocity::clearDisabled (int objectIndex)
{
    rawSemitones_[objectIndex] = 0.0f;
}

//==============================================================================
void DopplerVelocity::smooth (int objectIndex)
{
    // Store previous smoothed value before updating (for per-sample interpolation)
    prevSmoothedSemitones_[objectIndex] = smoothedSemitones_[objectIndex];

    // Two-pole (cascaded) EMA: continuous first derivative, eliminates
    // staircase artifacts that cause hop-rate buzz in the phase vocoder.
    // Stage 1: raw → intermediate
    intermediateSmoothed_[objectIndex] += kSmoothAlpha
        * (rawSemitones_[objectIndex] - intermediateSmoothed_[objectIndex]);
    // Stage 2: intermediate → output
    smoothedSemitones_[objectIndex] += kSmoothAlpha
        * (intermediateSmoothed_[objectIndex] - smoothedSemitones_[objectIndex]);
}

//==============================================================================
void DopplerVelocity::reset (int objectIndex, float azRad, float elRad, float dist)
{
    prevAz_[objectIndex]   = azRad;
    prevEl_[objectIndex]   = elRad;
    prevDist_[objectIndex] = dist;
    rawSemitones_[objectIndex] = 0.0f;
    smoothedSemitones_[objectIndex] = 0.0f;
    prevSmoothedSemitones_[objectIndex] = 0.0f;
    intermediateSmoothed_[objectIndex] = 0.0f;
    smoothedVelocity_[objectIndex] = 0.0f;
    posChanged_[objectIndex] = false;
}

//==============================================================================
void DopplerVelocity::resetAll()
{
    for (int i = 0; i < kMaxObjects; ++i)
        reset (i, 0.0f, 0.0f, 0.5f);
}
