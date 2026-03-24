#include "WSOLAPitcher.h"

//==============================================================================
float WSOLAPitcher::process (int objectIndex, float inputSample, float semitones)
{
    auto& ws = states_[objectIndex];

    // Write incoming sample into circular buffer
    ws.buffer[ws.writePos & State::kBufMask] = inputSample;
    ws.writePos++;

    // Cold-start bypass: pass through unpitched signal until buffer has enough
    // real audio data for a full grain. Without this, pitch UP races readPhase
    // into zero-filled regions, producing ~21ms silence after init/preset load.
    if (ws.writePos <= State::kGrainSize)
    {
        ws.readPhase = 0.0f;
        return inputSample;
    }

    // Normalize writePos to prevent float precision decay over long sessions.
    // After ~87s @ 48kHz, writePos > 2^22 and readPhase (float) starts losing
    // sub-sample precision needed for Catmull-Rom interpolation and drift detection.
    constexpr int kNormThreshold = 1 << 22;
    if (ws.writePos > kNormThreshold)
    {
        int excess = ws.writePos & ~State::kBufMask;
        ws.writePos -= excess;
        ws.readPhase -= static_cast<float> (excess);
        if (ws.crossfadeRemaining > 0)
            ws.fadingPhase -= static_cast<float> (excess);
    }

    const float ratio = std::pow (2.0f, semitones / 12.0f);

    // Advance read phase by pitch ratio
    ws.readPhase += ratio;

    // Catmull-Rom interpolated read from WSOLA buffer
    auto readBuf = [&] (float phase) -> float {
        float wrapped = std::fmod (phase, static_cast<float> (State::kBufSize));
        if (wrapped < 0.0f)
            wrapped += static_cast<float> (State::kBufSize);

        int   i1 = static_cast<int> (wrapped);
        float f  = wrapped - static_cast<float> (i1);

        int i0 = (i1 - 1) & State::kBufMask;
        int i2 = (i1 + 1) & State::kBufMask;
        int i3 = (i1 + 2) & State::kBufMask;
        i1 = i1 & State::kBufMask;

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
                       / static_cast<float> (State::kCrossfadeLen);
        float halfPi = juce::MathConstants<float>::halfPi;
        float gainNew = std::sin (t * halfPi);
        float gainOld = std::cos (t * halfPi);

        ws.crossfadeRemaining--;
        return primary * gainNew + fading * gainOld;
    }

    // Check if read-write drift exceeds grain size → initiate crossfade
    float drift = ws.readPhase - static_cast<float> (ws.writePos);
    if (std::abs (drift) > static_cast<float> (State::kGrainSize))
    {
        ws.fadingPhase = ws.readPhase;
        ws.readPhase = static_cast<float> (ws.writePos) - static_cast<float> (State::kGrainSize);
        ws.crossfadeRemaining = State::kCrossfadeLen;

        float fading = readBuf (ws.fadingPhase);
        ws.fadingPhase += ratio;
        ws.crossfadeRemaining--;
        return fading;
    }

    return readBuf (ws.readPhase);
}

//==============================================================================
bool WSOLAPitcher::updateGate (int objectIndex, float userPitch)
{
    float absPitch = std::abs (userPitch);
    if (! gateOpen_[objectIndex])
        gateOpen_[objectIndex] = (absPitch >= 0.05f);
    else if (absPitch < 0.001f)
        gateOpen_[objectIndex] = false;
    return gateOpen_[objectIndex];
}

//==============================================================================
void WSOLAPitcher::bypass (int objectIndex, float inputSample)
{
    auto& ws = states_[objectIndex];
    ws.buffer[ws.writePos & State::kBufMask] = inputSample;
    ws.writePos++;
    ws.readPhase = static_cast<float> (ws.writePos);

    constexpr int kNormThreshold = 1 << 22;
    if (ws.writePos > kNormThreshold)
    {
        int excess = ws.writePos & ~State::kBufMask;
        ws.writePos -= excess;
        ws.readPhase = static_cast<float> (ws.writePos);
    }
}

//==============================================================================
void WSOLAPitcher::reset (int objectIndex)
{
    auto& ws = states_[objectIndex];
    std::fill (std::begin (ws.buffer), std::end (ws.buffer), 0.0f);
    ws.writePos = 0;
    ws.readPhase = 0.0f;
    ws.fadingPhase = 0.0f;
    ws.crossfadeRemaining = 0;
    gateOpen_[objectIndex] = false;
}

//==============================================================================
void WSOLAPitcher::resetAll()
{
    for (int i = 0; i < kMaxObjects; ++i)
        reset (i);
}
