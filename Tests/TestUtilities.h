#pragma once

#include <cmath>
#include <vector>
#include <juce_core/juce_core.h>

static constexpr float kPi = juce::MathConstants<float>::pi;
static constexpr double kSampleRate = 48000.0;
static constexpr int kBlockSize = 256;

// Compute RMS of a buffer
static inline float computeRMS (const float* buffer, int numSamples)
{
    float sumSq = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sumSq += buffer[i] * buffer[i];
    return std::sqrt (sumSq / static_cast<float> (numSamples));
}

// Generate a sine wave into a buffer
static inline void fillSine (float* buffer, int numSamples, float freq, float sampleRate,
                              float amplitude, int startSample)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float phase = 2.0f * kPi * freq * static_cast<float> (startSample + i) / sampleRate;
        buffer[i] = amplitude * std::sin (phase);
    }
}

// Detect clicks/pops in an audio buffer using first-derivative threshold.
// Returns the sample indices where |sample[n] - sample[n-1]| exceeds threshold.
// For a 440 Hz sine at 48 kHz, the max natural derivative ~ 0.058 (at zero crossing).
// A threshold of 0.15 catches any non-natural discontinuity while avoiding false positives.
static inline std::vector<int> detectGlitches (const float* buffer, int numSamples, float threshold = 0.15f)
{
    std::vector<int> glitchIndices;
    for (int i = 1; i < numSamples; ++i)
    {
        float diff = std::abs (buffer[i] - buffer[i - 1]);
        if (diff > threshold)
            glitchIndices.push_back (i);
    }
    return glitchIndices;
}
