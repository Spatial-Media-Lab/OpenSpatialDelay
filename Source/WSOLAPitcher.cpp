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
    // into zero-filled regions, producing ~42ms silence after init/preset load.
    if (ws.writePos <= State::kGrainSize)
    {
        // v1.0.3: Center readPhase in filled region (issue #53, fix 1).
        // Placing at half-grain gives symmetric drift headroom for both
        // pitch-up and pitch-down on first grain after cold-start.
        int halfGrain = State::kGrainSize / 2;
        ws.readPhase = (ws.writePos > halfGrain)
                     ? static_cast<float> (ws.writePos - halfGrain)
                     : 0.0f;
        return inputSample;
    }

    // v1.0.3: Frequent normalization to prevent float precision decay (issue #53).
    // IEEE 754 float32 ULP grows with magnitude: at writePos ~1M (21s @ 48kHz),
    // the +1st ratio increment (0.0595) is below ULP and gets rounded to 0,
    // making small pitch shifts silently fail. Subtract exactly kBufSize when
    // writePos exceeds kBufSize + kGrainSize, keeping writePos in [kGrainSize+1,
    // kBufSize + kGrainSize] where ULP ≈ 0.0005 — full precision for all pitches.
    // Threshold must stay ABOVE kGrainSize to avoid triggering cold-start bypass.
    constexpr int kNormThreshold = State::kBufSize + State::kGrainSize;
    if (ws.writePos > kNormThreshold)
    {
        ws.writePos -= State::kBufSize;
        ws.readPhase -= static_cast<float> (State::kBufSize);
        if (ws.crossfadeRemaining > 0)
            ws.fadingPhase -= static_cast<float> (State::kBufSize);
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

        // v1.0.3: Hann crossfade replacing sin/cos (issue #53, fix 2).
        // sin/cos gains sum to sqrt(2) at midpoint (+3dB), creating periodic
        // amplitude bumps heard as clicks on correlated grains. Hann guarantees
        // gainNew + gainOld = 1.0 at all points (constant amplitude).
        float t = 1.0f - static_cast<float> (ws.crossfadeRemaining)
                       / static_cast<float> (State::kCrossfadeLen);
        float gainNew = 0.5f * (1.0f - std::cos (t * juce::MathConstants<float>::pi));
        float gainOld = 1.0f - gainNew;

        ws.crossfadeRemaining--;
        return primary * gainNew + fading * gainOld;
    }

    // Check if read-write drift exceeds grain size → initiate crossfade
    float drift = ws.readPhase - static_cast<float> (ws.writePos);
    if (std::abs (drift) > static_cast<float> (State::kGrainSize))
    {
        ws.fadingPhase = ws.readPhase;

        // v1.0.3: Ratio-aware reset offset for pitch-down (issue #53, fix 3).
        // For ratio < 1.0, fixed kGrainSize offset causes post-reset drift to
        // ALWAYS exceed threshold immediately (drift = -kGrainSize + kCrossfadeLen*(ratio-1)),
        // trapping the algorithm in perpetual back-to-back crossfading.
        // Scaling by ratio ensures post-reset drift is within bounds.
        float grainOffset = (ratio < 1.0f)
                          ? static_cast<float> (State::kGrainSize) * ratio
                          : static_cast<float> (State::kGrainSize);
        ws.readPhase = static_cast<float> (ws.writePos) - grainOffset;
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

    // v1.0.3: Position readPhase at half-grain behind writePos (issue #53, fix 4).
    // Previously set readPhase = writePos, placing it at the write boundary.
    // For small pitch shifts (+1st), this caused >1s warm-up delay before pitch
    // became audible because drift accumulated too slowly from zero offset.
    // Half-grain offset ensures immediate pitch audibility on gate open.
    ws.readPhase = static_cast<float> (ws.writePos - State::kGrainSize / 2);

    // v1.0.3: Clear crossfade state on bypass (issue #53, fix 5).
    // If gate closed during an active crossfade, stale fadingPhase persisted.
    // On gate reopen, process() would enter crossfade block reading from
    // buffer positions written before bypass — completely stale audio.
    ws.crossfadeRemaining = 0;
    ws.fadingPhase = ws.readPhase;

    // v1.0.3: Same frequent normalization as process() (issue #53).
    constexpr int kNormThreshold = State::kBufSize + State::kGrainSize;
    if (ws.writePos > kNormThreshold)
    {
        ws.writePos -= State::kBufSize;
        ws.readPhase = static_cast<float> (ws.writePos - State::kGrainSize / 2);
        ws.fadingPhase = ws.readPhase;
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
