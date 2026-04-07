# OpenSpatialSynthesizer

**One-line:** Polyphonic spatial synth where each voice is an independent 3D object with MPE-to-position mapping
**Type:** Instrument
**Complexity:** Very High
**In Roadmap:** No
**Template:** spatialcore-instrument

## Concept

A polyphonic synthesizer where each of 12 voices is spatialized as an independent 3D object. Play a chord and each note emanates from a different position in space. Supports four synthesis modes (subtractive, FM, wavetable, physical modeling), multi-topology filters, a full modulation matrix that can route any source to spatial position parameters, and MPE support mapping slide to azimuth and pressure to elevation. The first open-source spatial synthesizer with HRTF binaural rendering and multi-algorithm surround output.

## What SpatialCore Provides

- Per-voice spatialization (12 voices as spatial objects)
- HRTF convolution with 6 profiles for binaural synthesis
- All 7 spatialization algorithms for surround output
- 22 output formats including Ambisonics up to 6OA
- Trajectory system per voice for animated voice positions
- ADM-OSC Receive/Send for external voice position control
- MPE-to-position mapping (slide to azimuth, pressure to elevation)
- Distance attenuation, Doppler, and air absorption per voice
- Output limiter

## Plugin-Specific DSP

- Multi-mode oscillators:
  - Subtractive: PolyBLEP anti-aliased saw, square, triangle, pulse (PWM)
  - FM: 2-operator and 6-operator with configurable algorithms
  - Wavetable: morphable wavetables with mipmap anti-aliasing
  - Physical modeling: Karplus-Strong with extensions (damping, body filter)
- Multi-topology filters: Moog ladder (4-pole LP), SVF (multi-mode), ZDF (zero-delay feedback)
- Per-voice ADSR envelopes: amplitude, filter, pitch (independent rates)
- 2-4 LFOs with sync, key trigger, and free-running modes
- Modulation matrix (8-16 slots): any source (LFO, envelope, velocity, key, aftertouch, MPE) to any destination (including azimuth, elevation, distance)
- 12-voice polyphony with configurable voice allocation
- Unison mode (Super Saw detuning, up to 7 sub-voices per voice)
- Portamento (constant time or constant rate, legato/always)
- MPE support: per-note pitch bend, slide (CC74), pressure (channel aftertouch)

## Parameters

**Global (effect-specific):** oscType, oscParams (per-mode), filterType, cutoff, resonance, envADSR (amp/filter/pitch), lfoRate/shape, modMatrix, unisonVoices, unisonDetune, portamento
**Per-object (x12):** voice position auto-managed by voice allocator (or manually overridden via mod matrix / MPE mapping)

## Market Reference

Sound Particles SkyDust 3D is the only spatial synthesizer, but it is Ambisonics-only, expensive, and closed-source. Mainstream synths (Serum, Vital, Diva, Phase Plant) are all stereo. No synthesizer offers per-voice HRTF binaural rendering, multi-algorithm surround panning, or MPE-to-spatial-position mapping. This would be the first open-source spatial synth and the first with direct binaural HRTF per voice.

## Skills Needed

- `synthesis-techniques` — oscillator design, filter topologies, modulation matrix, voice management
- `plugin-architecture-patterns` — MIDI/MPE handling, complex state management, preset system
- `spatial-audio-dsp` — per-voice spatialization, MPE-to-position mapping, voice allocation with spatial distribution
- `dsp-cookbook` — PolyBLEP, oversampling, envelope generators, LFO design
- `juce-best-practices` — real-time safety with complex voice architecture, memory management
