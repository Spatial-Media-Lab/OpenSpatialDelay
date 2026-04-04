#pragma once

#include <JuceHeader.h>
#include "SharedFFTCache.h"
#include <cmath>
#include <cstring>
#include <algorithm>

/**
 * Phase vocoder pitch shifter with Laroche-Dolson phase locking
 * and Röbel-style transient-aware phase reset.
 *
 * Processes one sample at a time (internal FIFO wraps block-based STFT).
 * Supports +/-12 semitones.
 *
 * Transient handling:
 *   - Spectral flux (half-wave rectified) with adaptive median threshold
 *   - On transient frames, output phases are reset to analysis phases
 *     instead of propagated, preserving temporal sharpness
 *   - A hold counter extends phase reset across 2 post-transient frames
 *     to cover the full 4x overlap region
 */
class PhaseVocoderPitchShifter
{
public:
    static constexpr int kFFTOrder  = 11;                    // 2^11 = 2048
    static constexpr int kFFTSize   = 1 << kFFTOrder;        // 2048
    static constexpr int kOverlap   = 4;
    static constexpr int kHopSize   = kFFTSize / kOverlap;   // 512
    static constexpr int kNumBins   = kFFTSize / 2 + 1;      // 1025

    PhaseVocoderPitchShifter()
    {
        fft = getSharedFFTCache().getOrCreate (kFFTOrder);
        // v1.0.8: Generate periodic Hann window (N denominator, not N-1).
        // JUCE's WindowingFunction::hann uses symmetric (N-1), which creates a
        // ~-66 dB COLA ripple at the hop rate (93.75 Hz @ 48kHz/512 hop).
        // Periodic Hann sums to exactly 1.5 under 4x overlap — zero ripple.
        for (int i = 0; i < kFFTSize; ++i)
            windowData[i] = 0.5f * (1.0f - std::cos (kTwoPi * static_cast<float> (i)
                                                       / static_cast<float> (kFFTSize)));
        reset();
    }

    void reset()
    {
        std::memset (inputRing,       0, sizeof (inputRing));
        std::memset (outputAccum,     0, sizeof (outputAccum));
        std::memset (lastInputPhase,  0, sizeof (lastInputPhase));
        std::memset (lastOutputPhase, 0, sizeof (lastOutputPhase));

        inputWritePos   = 0;
        inputCount      = 0;
        hopCounter      = 0;
        outputReadPos   = 0;
        outputWritePos  = 0;
        filledLatency   = false;

        // Transient detection state
        std::memset (prevAnalysisMag, 0, sizeof (prevAnalysisMag));
        std::memset (fluxHistory,     0, sizeof (fluxHistory));
        fluxWritePos     = 0;
        transientHold    = 0;

        // Bypass hysteresis state
        bypassed_    = true;
        bypassFade_  = 1.0f;
    }

    /** Process one sample with pitch shift in semitones (-12 to +12).
     *  Matches WSOLAPitchShifter::process() API for drop-in replacement.
     */
    float process (float inputSample, float semitones)
    {
        // Write input to ring buffer
        inputRing[inputWritePos] = inputSample;
        inputWritePos = (inputWritePos + 1) % kFFTSize;
        inputCount++;

        // Hysteresis bypass: enter at |s| < 0.005, exit at |s| > 0.02
        // Prevents oscillation near zero crossing from toggling PV on/off
        float absSemi = std::abs (semitones);
        if (bypassed_)
        {
            if (absSemi > kBypassExitThreshold)
                bypassed_ = false;
        }
        else
        {
            if (absSemi < kBypassEnterThreshold)
                bypassed_ = true;
        }

        if (bypassed_)
        {
            // Fade toward dry — NO FFT processing, just drain the accumulator
            if (bypassFade_ < 1.0f)
                bypassFade_ = std::min (bypassFade_ + kBypassFadeStep, 1.0f);

            // Keep hop counter in sync (no processOneFrame — too expensive)
            hopCounter++;
            if (hopCounter >= kHopSize)
                hopCounter = 0;

            // Read bypass (latency-compensated dry)
            int readPos = (inputWritePos - kFFTSize + kFFTSize) % kFFTSize;
            float dry = (inputCount < kFFTSize) ? 0.0f : inputRing[readPos];

            // Drain PV accumulator during crossfade to prevent stale data buildup
            if (filledLatency && bypassFade_ < 1.0f)
            {
                float pvOut = outputAccum[outputReadPos];
                outputAccum[outputReadPos] = 0.0f;
                outputReadPos = (outputReadPos + 1) % kOutputSize;
                return dry * bypassFade_ + pvOut * (1.0f - bypassFade_);
            }

            // Fully bypassed — just return latency-compensated dry
            if (! filledLatency && inputCount >= kFFTSize + kHopSize)
                filledLatency = true;

            return dry;
        }

        // Active PV: fade in PV, fade out dry
        if (bypassFade_ > 0.0f)
            bypassFade_ = std::max (bypassFade_ - kBypassFadeStep, 0.0f);

        // Accumulate until we have a hop's worth
        hopCounter++;

        if (hopCounter >= kHopSize && inputCount >= kFFTSize)
        {
            hopCounter = 0;

            float pitchRatio = std::pow (2.0f, semitones / 12.0f);
            processOneFrame (pitchRatio);
        }

        // During initial fill, output silence
        if (! filledLatency)
        {
            if (inputCount >= kFFTSize + kHopSize)
                filledLatency = true;
            else
                return 0.0f;
        }

        // Pop one sample from output accumulator
        float pvOut = outputAccum[outputReadPos];
        outputAccum[outputReadPos] = 0.0f;  // Clear for next overlap-add cycle
        outputReadPos = (outputReadPos + 1) % kOutputSize;

        // If still crossfading from bypass, blend with dry
        if (bypassFade_ > 0.0f)
        {
            int readPos = (inputWritePos - kFFTSize + kFFTSize) % kFFTSize;
            float dry = inputRing[readPos];
            return dry * bypassFade_ + pvOut * (1.0f - bypassFade_);
        }

        return pvOut;
    }

    static constexpr int getLatency() { return kFFTSize; }

private:
    static constexpr int   kOutputSize = kFFTSize * 2;
    static constexpr float kPi         = 3.14159265358979323846f;
    static constexpr float kTwoPi      = 2.0f * kPi;

    std::shared_ptr<juce::dsp::FFT> fft;  // Shared via process-global FFT cache (issue #131)
    float windowData[kFFTSize] = {};  // Periodic Hann window (generated in constructor)

    // Input ring buffer (stores last kFFTSize samples)
    float inputRing[kFFTSize] = {};
    int   inputWritePos = 0;
    int   inputCount    = 0;

    // Output overlap-add accumulator (circular)
    float outputAccum[kOutputSize] = {};
    int   outputReadPos  = 0;
    int   outputWritePos = 0;

    // Hop counter
    int hopCounter = 0;

    // Latency fill flag
    bool filledLatency = false;

    // Phase vocoder state (per bin)
    float lastInputPhase[kNumBins]  = {};
    float lastOutputPhase[kNumBins] = {};

    // Bypass hysteresis state
    bool  bypassed_    = true;   // Start bypassed (no pitch shift initially)
    float bypassFade_  = 1.0f;   // 1.0 = full bypass/dry, 0.0 = full PV
    static constexpr float kBypassEnterThreshold = 0.005f;  // Enter bypass below this
    static constexpr float kBypassExitThreshold  = 0.02f;   // Exit bypass above this
    static constexpr float kBypassFadeStep       = 1.0f / 128.0f;  // ~2.7ms crossfade at 48kHz

    // Transient detection state
    static constexpr int kFluxHistorySize = 16;
    float prevAnalysisMag[kNumBins]          = {};
    float fluxHistory[kFluxHistorySize]      = {};
    int   fluxWritePos                       = 0;
    int   transientHold                      = 0;
    static constexpr int   kTransientHoldFrames  = 2;
    static constexpr float kTransientSensitivity = 2.0f;

    // Working buffers for one STFT frame
    float fftData[kFFTSize * 2]       = {};  // JUCE FFT uses 2x for real-only transforms
    float analysisMag[kNumBins]       = {};
    float analysisFreq[kNumBins]      = {};
    float synthesisMag[kNumBins]      = {};
    float synthesisFreq[kNumBins]     = {};
    float synthesisPhase[kNumBins]    = {};
    int   peakBins[kNumBins]          = {};  // Indices of spectral peaks
    int   binToPeak[kNumBins]         = {};  // Maps each bin to its nearest peak

    /** Half-wave rectified spectral flux with adaptive median threshold.
     *  Must be called AFTER analysisMag[] is populated.
     *  Updates prevAnalysisMag[] and fluxHistory[] as side effects.
     */
    bool detectTransient()
    {
        // Half-wave rectified spectral flux: sum of positive magnitude increases
        float flux = 0.0f;
        for (int k = 0; k < kNumBins; ++k)
        {
            float diff = analysisMag[k] - prevAnalysisMag[k];
            if (diff > 0.0f)
                flux += diff;
        }

        // Store previous magnitudes for next frame
        std::memcpy (prevAnalysisMag, analysisMag, sizeof (prevAnalysisMag));

        // Store flux in history ring buffer
        fluxHistory[fluxWritePos] = flux;
        fluxWritePos = (fluxWritePos + 1) % kFluxHistorySize;

        // Compute median of recent flux values for adaptive threshold
        float sorted[kFluxHistorySize];
        std::memcpy (sorted, fluxHistory, sizeof (sorted));
        std::sort (sorted, sorted + kFluxHistorySize);
        float median = sorted[kFluxHistorySize / 2];

        return (flux > kTransientSensitivity * median + 1e-6f);
    }

    /** Process one STFT frame: analysis, pitch shift, phase lock, synthesis. */
    void processOneFrame (float pitchRatio)
    {
        // =====================================================================
        // 1. Extract input frame from ring buffer and apply analysis window
        // =====================================================================
        for (int i = 0; i < kFFTSize; ++i)
        {
            int readIdx = (inputWritePos - kFFTSize + i + kFFTSize) % kFFTSize;
            fftData[i] = inputRing[readIdx];
        }

        juce::FloatVectorOperations::multiply (fftData, windowData, kFFTSize);

        // Zero the second half (JUCE FFT requires 2*N buffer for real-only transform)
        std::memset (fftData + kFFTSize, 0, sizeof (float) * static_cast<size_t> (kFFTSize));

        // =====================================================================
        // 2. Forward FFT
        // =====================================================================
        fft->performRealOnlyForwardTransform (fftData);

        // =====================================================================
        // 3. Analysis: compute magnitude and instantaneous frequency per bin
        // =====================================================================
        float expectedPhaseDiff = kTwoPi * static_cast<float> (kHopSize) / static_cast<float> (kFFTSize);

        for (int k = 0; k < kNumBins; ++k)
        {
            float re = fftData[k * 2];
            float im = fftData[k * 2 + 1];

            float mag   = std::sqrt (re * re + im * im);
            float phase = std::atan2 (im, re);

            // Phase difference from last frame
            float phaseDiff = phase - lastInputPhase[k];
            lastInputPhase[k] = phase;

            // Subtract expected phase advance for this bin
            phaseDiff -= static_cast<float> (k) * expectedPhaseDiff;

            // Wrap to [-pi, pi]
            phaseDiff = phaseDiff - kTwoPi * std::round (phaseDiff / kTwoPi);

            // True frequency (in bin units): bin index + deviation
            float trueFreq = static_cast<float> (k) + phaseDiff / expectedPhaseDiff;

            analysisMag[k]  = mag;
            analysisFreq[k] = trueFreq;
        }

        // =====================================================================
        // 3b. Transient detection (spectral flux with adaptive threshold)
        // =====================================================================
        bool transientDetected = detectTransient();
        if (transientDetected)
            transientHold = kTransientHoldFrames;

        bool usePhaseReset = (transientHold > 0);
        if (transientHold > 0)
            transientHold--;

        // =====================================================================
        // 4. Pitch shifting: shift bins by pitch ratio
        // =====================================================================
        std::memset (synthesisMag,  0, sizeof (synthesisMag));
        std::memset (synthesisFreq, 0, sizeof (synthesisFreq));

        for (int k = 0; k < kNumBins; ++k)
        {
            int targetBin = static_cast<int> (std::round (static_cast<float> (k) * pitchRatio));

            if (targetBin >= 0 && targetBin < kNumBins)
            {
                // If multiple source bins map to same target, keep the loudest
                if (analysisMag[k] > synthesisMag[targetBin])
                {
                    synthesisMag[targetBin]  = analysisMag[k];
                    synthesisFreq[targetBin] = analysisFreq[k] * pitchRatio;
                }
            }
        }

        // =====================================================================
        // 5. Phase computation: Röbel phase reset OR Laroche-Dolson
        // =====================================================================
        if (usePhaseReset)
        {
            // TRANSIENT: reset output phase to scaled input phase.
            // Transients are broadband — their perceptual quality comes from
            // timing, not inter-bin phase coherence. Resetting preserves sharpness.
            for (int k = 0; k < kNumBins; ++k)
            {
                int sourceBin = static_cast<int> (std::round (static_cast<float> (k) / pitchRatio));
                sourceBin = std::clamp (sourceBin, 0, kNumBins - 1);
                synthesisPhase[k] = lastInputPhase[sourceBin];
            }
        }
        else
        {
            // STEADY-STATE: Laroche-Dolson phase locking

            // 5a. Find spectral peaks (local maxima in synthesisMag)
            int numPeaks = 0;

            for (int k = 1; k < kNumBins - 1; ++k)
            {
                if (synthesisMag[k] > synthesisMag[k - 1] &&
                    synthesisMag[k] >= synthesisMag[k + 1] &&
                    synthesisMag[k] > 1e-10f)
                {
                    peakBins[numPeaks++] = k;
                }
            }

            // Handle edge case: bin 0 can be a peak
            if (kNumBins > 1 && synthesisMag[0] >= synthesisMag[1] && synthesisMag[0] > 1e-10f)
            {
                // Shift peaks right and insert at front
                for (int i = numPeaks; i > 0; --i)
                    peakBins[i] = peakBins[i - 1];
                peakBins[0] = 0;
                numPeaks++;
            }

            // 5b. Assign each bin to its nearest peak (region of influence)
            if (numPeaks > 0)
            {
                int peakIdx = 0;
                for (int k = 0; k < kNumBins; ++k)
                {
                    // Move to next peak if this bin is closer to it
                    if (peakIdx < numPeaks - 1)
                    {
                        int distCurrent = std::abs (k - peakBins[peakIdx]);
                        int distNext    = std::abs (k - peakBins[peakIdx + 1]);
                        if (distNext < distCurrent)
                            peakIdx++;
                    }
                    binToPeak[k] = peakBins[peakIdx];
                }

                // 5c. Compute phase for peak bins, then lock neighbors
                // First pass: compute peak phases using standard phase propagation
                for (int p = 0; p < numPeaks; ++p)
                {
                    int k = peakBins[p];
                    float phaseAdvance = synthesisFreq[k] * expectedPhaseDiff;
                    synthesisPhase[k] = lastOutputPhase[k] + phaseAdvance;
                }

                // Second pass: lock non-peak bins to their nearest peak
                for (int k = 0; k < kNumBins; ++k)
                {
                    int peak = binToPeak[k];
                    if (k == peak)
                        continue;  // Already computed

                    // Phase rotation relative to the peak
                    // Identity phase locking: preserve the phase relationship from analysis
                    int sourceBin = static_cast<int> (std::round (static_cast<float> (k) / pitchRatio));
                    int sourcePeak = static_cast<int> (std::round (static_cast<float> (peak) / pitchRatio));

                    sourceBin  = std::clamp (sourceBin,  0, kNumBins - 1);
                    sourcePeak = std::clamp (sourcePeak, 0, kNumBins - 1);

                    float phaseDiffFromPeak = lastInputPhase[sourceBin] - lastInputPhase[sourcePeak];
                    synthesisPhase[k] = synthesisPhase[peak] + phaseDiffFromPeak;
                }
            }
            else
            {
                // No peaks found — standard phase propagation for all bins
                for (int k = 0; k < kNumBins; ++k)
                {
                    float phaseAdvance = synthesisFreq[k] * expectedPhaseDiff;
                    synthesisPhase[k] = lastOutputPhase[k] + phaseAdvance;
                }
            }
        }

        // Update output phase memory
        for (int k = 0; k < kNumBins; ++k)
            lastOutputPhase[k] = synthesisPhase[k];

        // =====================================================================
        // 6. Synthesis: reconstruct complex spectrum
        // =====================================================================
        std::memset (fftData, 0, sizeof (float) * static_cast<size_t> (kFFTSize * 2));

        for (int k = 0; k < kNumBins; ++k)
        {
            fftData[k * 2]     = synthesisMag[k] * std::cos (synthesisPhase[k]);
            fftData[k * 2 + 1] = synthesisMag[k] * std::sin (synthesisPhase[k]);
        }

        // =====================================================================
        // 7. Inverse FFT
        // =====================================================================
        fft->performRealOnlyInverseTransform (fftData);

        // =====================================================================
        // 8. Apply synthesis window and overlap-add
        // =====================================================================
        juce::FloatVectorOperations::multiply (fftData, windowData, kFFTSize);

        // COLA normalization for Hann window with 4x overlap:
        // Sum of squared Hann windows at hop=N/4 is 1.5, so divide by 1.5
        float normFactor = 1.0f / (static_cast<float> (kOverlap) * 0.375f);

        for (int i = 0; i < kFFTSize; ++i)
        {
            int pos = (outputWritePos + i) % kOutputSize;
            outputAccum[pos] += fftData[i] * normFactor;
        }

        outputWritePos = (outputWritePos + kHopSize) % kOutputSize;
    }
};
