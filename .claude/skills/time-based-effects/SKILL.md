---
name: time-based-effects
description: Use when implementing chorus, flanger, phaser, pitch shifting, vibrato, tremolo, or comb filtering in JUCE C++ plugins. Covers modulated delay lines, allpass chains, and interpolation. NOT for spatial audio, reverb, or synthesis.
---

# Time-Based Audio Effects

## Which Effect?

| Effect | Delay Range | Feedback | Key Character |
|--------|-------------|----------|---------------|
| **Chorus** | 10-30ms | None/low | Thick, detuned doubling |
| **Flanger** | 0-10ms | High (±0.95) | Metallic, sweeping comb |
| **Phaser** | N/A (allpass) | Medium | Organic, vocal notches |
| **Vibrato** | 1-10ms | None | Pitch wobble (wet only) |
| **Tremolo** | N/A | N/A | Volume modulation |

## Modulated Delay Line (Chorus/Flanger)

Core building block for chorus, flanger, and vibrato.

```cpp
class ModulatedDelay {
public:
    void prepare(float sampleRate, float maxDelayMs) {
        sr = sampleRate;
        int maxSamples = static_cast<int>(maxDelayMs * 0.001f * sr) + 4;
        buffer.resize(maxSamples, 0.0f);
        writePos = 0;
        lfoPhase = 0.0f;
    }

    // baseDelayMs: center delay, depthMs: LFO sweep amount, rateHz: LFO speed
    // feedback: -0.95 to 0.95 (negative = hollow character)
    float processSample(float input, float baseDelayMs, float depthMs,
                        float rateHz, float feedback) noexcept {
        // LFO
        float lfo = std::sin(2.0f * juce::MathConstants<float>::pi * lfoPhase);
        lfoPhase += rateHz / sr;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // Modulated delay in samples
        float delaySamples = (baseDelayMs + depthMs * lfo) * 0.001f * sr;

        // Read with cubic interpolation
        float readPos = writePos - delaySamples;
        if (readPos < 0.0f) readPos += buffer.size();
        float delayed = hermiteInterp(readPos);

        // Write with feedback
        buffer[writePos] = input + delayed * feedback;
        writePos = (writePos + 1) % buffer.size();

        return delayed;
    }

    void reset() { std::fill(buffer.begin(), buffer.end(), 0.0f); }

private:
    std::vector<float> buffer;
    int writePos = 0;
    float lfoPhase = 0.0f;
    float sr = 44100.0f;

    float hermiteInterp(float pos) const noexcept {
        int i = static_cast<int>(pos);
        float f = pos - i;
        int n = static_cast<int>(buffer.size());
        float y0 = buffer[(i - 1 + n) % n];
        float y1 = buffer[i % n];
        float y2 = buffer[(i + 1) % n];
        float y3 = buffer[(i + 2) % n];
        float c0 = y1, c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + c0;
    }
};

// Usage:
// Chorus: baseDelay=20ms, depth=3ms, rate=1Hz, feedback=0.0
// Flanger: baseDelay=2ms,  depth=2ms, rate=0.3Hz, feedback=0.7
// Vibrato: baseDelay=5ms,  depth=3ms, rate=5Hz, feedback=0.0 (wet only!)
```

## Phaser (Allpass Chain)

Non-harmonically-spaced notches — more organic than flanger.

```cpp
class Phaser {
public:
    static constexpr int NumStages = 6;  // 6 stages = 3 notches

    void prepare(float sampleRate) { sr = sampleRate; }

    float processSample(float input, float minFreq, float maxFreq,
                        float rateHz, float feedback) noexcept {
        // Log-scale LFO for perceptual uniformity
        float lfo = 0.5f * (1.0f + std::sin(2.0f * 3.14159f * lfoPhase));
        lfoPhase += rateHz / sr;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        float fc = minFreq * std::pow(maxFreq / minFreq, lfo);

        // Allpass coefficient
        float w = 3.14159f * fc / sr;
        float a = (std::tan(w) - 1.0f) / (std::tan(w) + 1.0f);

        // Chain of first-order allpass filters
        float sample = input + fbState * feedback;
        for (int i = 0; i < NumStages; ++i) {
            float yn = a * sample + apState[i];
            apState[i] = sample - a * yn;
            sample = yn;
        }
        fbState = sample;

        return input + sample;  // Dry + allpass output = notch filter
    }

    void reset() {
        std::fill(apState, apState + NumStages, 0.0f);
        fbState = 0.0f;
        lfoPhase = 0.0f;
    }

private:
    float apState[NumStages] = {};
    float fbState = 0.0f;
    float lfoPhase = 0.0f;
    float sr = 44100.0f;
};
```

## Interpolation Quick Reference

| Method | Points | Quality | Use Case |
|--------|--------|---------|----------|
| Linear | 2 | Poor (-3dB at fs/4) | Prototyping only |
| **Cubic Hermite** | 4 | Good (flat to ~0.8×Nyquist) | **Recommended minimum** |
| Lagrange 5th | 6 | Very Good | High quality effects |
| Sinc (8-pt) | 8 | Excellent | Highest quality |

**Always use cubic Hermite minimum for modulated delays.** The CPU cost difference vs linear is negligible.

## Common Mistakes

| Mistake | Fix |
|---|---|
| Linear interpolation on delay line — audible aliasing | Use cubic Hermite (Catmull-Rom) minimum |
| Feedback > 1.0 — runaway oscillation | Clamp `abs(feedback) < 0.95`; soft-clip in feedback path |
| Not scaling delays for sample rate — wrong pitch at 96kHz | Store delays in ms, convert to samples: `ms * 0.001f * sampleRate` |
| Chorus with feedback — sounds like flanger | Chorus typically has zero or very low feedback |
| Phaser with linear LFO — uneven sweep | Use logarithmic frequency sweep for perceptual uniformity |
| Vibrato mixed with dry — becomes chorus | Vibrato is wet-only; adding dry signal creates chorus effect |

---

See **[REFERENCE.md](REFERENCE.md)** for full theory: BBD modeling characteristics, through-zero flanging, barber-pole phaser, all 8 pitch shifting methods (OLA/SOLA/PSOLA/WSOLA/phase vocoder/granular/dual-head/formant-preserving) with comparison table, comb filter equations, rotary speaker simulation, historical timeline (tape era through modern DSP), key patents and papers, and implementation recommendations.
