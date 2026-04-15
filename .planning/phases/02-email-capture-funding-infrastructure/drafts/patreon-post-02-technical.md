# Seed Post 2 — Inside OpenSpatialDelay: HRTF, Phase Vocoder, and 3D Trajectory

**Visibility:** PATRON-ONLY ($3+ Supporter tier, per D-28)
**Publish order:** 2nd
**Public teaser (shows above the paywall):** A look at the DSP architecture behind OpenSpatialDelay
— for patrons who want to know how it actually works.

---

OpenSpatialDelay is a spatial delay plugin where every echo lives at a distinct 3D position — the
first tool in the **Spatial Media Library** pipeline, with source code maintained under
**Spatial Media Lab**. This post walks through the DSP building blocks that make it possible.

## 1. HRTF Convolution — How Each Echo Lands at a Specific Position

An echo gets its spatial location by convolving the tap's audio with a pair of HRIRs (head-related
impulse responses) — one for the left ear, one for the right. The plugin ships with 5 HRTF
profiles loaded from SOFA files, sourced from public research datasets (KU100, CIPIC, HUTUBS,
MIT KEMAR, SADIE). Each profile is a multi-dimensional array of impulse responses indexed by
azimuth and elevation.

The convolution runs through a **partitioned convolver** — the impulse response is split into
equally-sized blocks, FFT'd up-front, and the incoming audio is convolved block-by-block in the
frequency domain. This keeps per-buffer CPU bounded for IRs that can be thousands of samples long.
Spectral envelope smoothing is applied per frequency bin (magnitude and phase smoothed separately)
to prevent comb-filtering artifacts when the position interpolates between HRIR snapshots.

## 2. Phase Vocoder — Pitch-Independent Time-Stretching

The Doppler effect in OpenSpatialDelay needs to stretch or compress the echo's playback rate
without changing pitch. Pure resampling couples the two — faster = higher pitch. The plugin uses
a **phase vocoder** built on a 2048-point STFT with 4× overlap:

- Laroche-Dolson phase locking keeps tonal content coherent across frames
- A Röbel-style spectral flux detector with an adaptive median threshold flags transients
- Phase reset on transient frames preserves attack sharpness instead of smearing it
- Range: ±12 semitones (combined with the Doppler engine)
- Latency: 2048 samples, reported to the DAW so PDC works correctly

The dry path carries a matching 2048-sample latency compensation so wet/dry mixes stay phase-
aligned. If you set the plugin to 100% dry, the output is still latency-compensated. The dry/wet
mix uses an equal-power crossfade (cos/sin) instead of linear, so perceived loudness stays
constant at all mix settings.

## 3. Trajectory Engine — Per-Echo 3D Position

Each echo has an independent position in 3D space. The trajectory engine lets you define a path
(line, circle, orbit, hand-drawn spline) that all echoes follow with configurable delay offsets
between them. The engine drives the HRTF convolver's azimuth/elevation interpolation frame-by-
frame, and separately feeds the Doppler engine an instantaneous velocity for pitch-free time-
stretch.

The **ITD/ILD model** adds interaural time and level differences on top of HRTF convolution for
close-range source simulation. Note: for the MIT KEMAR profile, ITD is baked into the HRIR
waveform itself and the ITD delay line runs as a pass-through.

## 4. What This Architecture Enables Next

The DSP kernel here — partitioned convolution, phase vocoder, trajectory engine — isn't delay-
specific. It's a toolkit for a whole pipeline of spatial effects: spatial reverbs, convolution-
based placement processors, binaural chorus, moving-source simulations. Subsequent Spatial Media
Library tools will share these primitives.

That's the technical case for the pipeline framing: we build the kernel once, then ship N tools
on top of it.

— Andrew
