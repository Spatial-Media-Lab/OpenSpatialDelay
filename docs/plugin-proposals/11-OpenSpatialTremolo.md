# OpenSpatialTremolo

**One-line:** Multi-tap spatial tremolo with phase-offset LFOs creating spatial rhythmic patterns
**Type:** Effect
**Complexity:** Low-Medium
**In Roadmap:** No
**Template:** spatialcore-effect

## Concept

A tremolo effect with up to 12 taps, each at a different 3D position, driven by phase-offset LFOs that create spatial rhythmic patterns. As amplitude peaks move from tap to tap, the sound appears to pulse through 3D space. Supports harmonic tremolo mode (separate LP/HP amplitude modulation) and Leslie simulation with physically modeled horn and drum rotation including Doppler. The simplest spatially-aware effect in the suite, yet one of the most viscerally immersive.

## What SpatialCore Provides

- Per-tap spatialization (up to 12 tremolo taps as spatial objects)
- HRTF convolution with 6 profiles for binaural rendering
- Trajectory system for rotary orbits (natural fit for Leslie simulation)
- 22 output formats including surround and Ambisonics
- Doppler effect per tap (essential for Leslie horn simulation)
- Distance attenuation and air absorption
- Output limiter

## Plugin-Specific DSP

- Per-tap amplitude LFO with independent phase offset
- Phase offset distribution (even spacing, custom, random)
- Rate sync to host tempo with note division options
- Harmonic tremolo mode (LP and HP paths modulated with opposite phase)
- Leslie simulation: separate horn (high-pass, fast/slow rotation) and drum (low-pass, slower rotation) with speed-dependent Doppler
- LFO waveforms: sine, triangle, square, sample-and-hold
- Acceleration/deceleration curves for Leslie speed switching (fast/slow/brake)
- Per-tap depth control for uneven spatial amplitude patterns

## Parameters

**Global (effect-specific):** rate, depth, shape (sine/tri/square/S&H), phaseSpread, mode (tremolo/harmonic/Leslie), syncToHost, dryWet
**Per-object (x12):** phase offset, depth (each tap independently configurable)

## Market Reference

Soundtoys Tremolator and Goodhertz Trem Control are excellent but stereo-only. PSP L'otary 2 and IK Multimedia Leslie Collection simulate Leslie in stereo. No tremolo or Leslie plugin positions amplitude modulation taps in 3D space. The spatial phase offset approach creates rhythmic amplitude patterns that physically move around the listener.

## Skills Needed

- `time-based-effects` — tremolo, Leslie simulation, LFO design
- `dsp-cookbook` — amplitude modulation, crossover filters, Doppler
- `spatial-audio-dsp` — per-tap spatialization, rotation-to-position mapping
