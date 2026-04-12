# Architecture Reference

## HRTF Rendering Signal Flow

`renderDirectBinauralHRTF()` uses a 3-pass architecture:

1. **Pass 1:** Per-sample delay engine → per-source mono accumulation
2. **Pass 2:** Per-block HRTF convolution via `BinauralRenderer::renderSourceBuffers()`
3. **Pass 3:** Per-sample dry/wet mix + output gain

## PartitionedConvolver

Uses spectral envelope EMA smoothing: magnitude and phase smoothed separately per frequency bin. This prevents comb filtering from phase-misaligned time-domain blending.

## Phase Vocoder Pitch Shifter

Replaces WSOLA-Lite. Uses STFT (2048-point FFT, 4x overlap) with:
- Laroche-Dolson phase locking for tonal content
- Röbel-style spectral flux transient detection with adaptive median threshold
- Phase reset on transient frames preserves attack sharpness
- Latency: 2048 samples (reported to DAW via `setLatencySamples`)
- Range: ±12 semitones (combined with Doppler)

## Dry Path Latency Compensation

The phase vocoder adds 2048 samples of latency to the wet path. The dry signal must be delayed by the same amount so the DAW's plugin delay compensation (PDC) is correct at all dry/wet settings.

Implementation: stereo circular `dryDelayLineL/R` buffers that pre-fill `dryCompBufferL/R` from raw DAW input at the start of each `processBlock`.

The dry/wet mix happens in a single post-render stage in `processBlock` — render paths output raw wet signal only. This ensures the dry signal truly bypasses the entire plugin (Input selector only affects the wet path). Equal-power crossfade (cos/sin) replaces linear (1-dw/dw) for constant perceived loudness at all mix settings.

## ITD Delay Line

For MIT KEMAR SOFA file, ITD values are always 0 (embedded in HRIR waveform). The ITD delay line is effectively a pass-through for this dataset.
