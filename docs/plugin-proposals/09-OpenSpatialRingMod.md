# OpenSpatialRingMod

**One-line:** Ring modulator with upper and lower sidebands spatially separated in 3D
**Type:** Effect
**Complexity:** Medium
**In Roadmap:** No
**Template:** spatialcore-effect

## Concept

A ring modulator that uses Hilbert transform sideband separation to isolate the original signal, upper sideband, and lower sideband, then positions each as an independent 3D object. The metallic, inharmonic character of ring modulation becomes a spatial event -- upper partials shimmer above while lower partials drone below. Supports multiple carrier waveforms and an AM/RM blend control. With 12 objects, multiple carrier frequencies can run simultaneously, each with spatially separated sidebands.

## What SpatialCore Provides

- Per-sideband spatialization (original + upper + lower sidebands as spatial objects)
- HRTF convolution with 6 profiles for binaural rendering
- Trajectory system for animated sideband positions (sidebands orbiting original)
- 22 output formats
- Distance attenuation per sideband
- Output limiter

## Plugin-Specific DSP

- Ring modulation engine (carrier x modulator multiplication)
- Sideband separation via Hilbert transform (90-degree phase shift network)
- Carrier waveforms: sine, triangle, sawtooth, noise (white/pink)
- AM/RM blend control (amplitude modulation to ring modulation crossfade)
- Feedback path (output routed back to modulator input)
- Multiple carrier support (up to 4 carriers with independent frequencies)
- Carrier frequency with tempo sync option
- Sideband level controls (original, upper, lower independently)

## Parameters

**Global (effect-specific):** carrierFreq, carrierWave, amRmBlend, feedback, sidebandSeparation, dryWet
**Per-object (x12):** assigned to original/upper/lower sideband (objects grouped in triplets per carrier)

## Market Reference

All existing ring modulators (SoundToys RingShifter, Moog MF-102, AudioThing RingModulator) are mono or stereo. None offer sideband separation into independent spatial channels. The combination of Hilbert transform sideband isolation with per-sideband 3D positioning is entirely novel, turning a classic effect into an immersive spatial instrument.

## Skills Needed

- `dsp-cookbook` — ring modulation, Hilbert transform, AM/RM math
- `synthesis-techniques` — carrier oscillators, AM/ring modulation theory
- `spatial-audio-dsp` — per-sideband spatialization, sideband-to-position mapping
