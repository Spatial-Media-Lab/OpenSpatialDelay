# OpenSpatialChorus

**One-line:** Chorus/flanger/ensemble where each detuned voice is at a different 3D position
**Type:** Effect
**Complexity:** Medium
**In Roadmap:** Yes
**Template:** spatialcore-effect

## Concept

A multi-voice chorus effect where each detuned voice is spatialized at a unique 3D position, creating an enveloping shimmer that surrounds the listener. Supports classic chorus, flanger, and BBD-style ensemble modes. Voice positions can be auto-spread evenly across the spatial field or individually placed with trajectory animation, turning a familiar effect into a fully immersive spatial experience. This is the first spatial chorus ever built.

## What SpatialCore Provides

- Per-voice spatialization (2-12 voices as spatial objects)
- HRTF convolution with 6 profiles for binaural chorus
- ADM-OSC Receive/Send for external voice position control
- Trajectory system for animated voice movement (orbits, figure-8)
- 22 output formats including surround and Ambisonics
- Distance attenuation and Doppler per voice
- Output limiter

## Plugin-Specific DSP

- Multi-voice chorus engine (2-12 simultaneous voices)
- Per-voice detuning with independent LFO phase offsets
- LFO engine with phase-offset distribution (even, random, custom)
- Flanger mode (shorter delay times, feedback, comb filtering)
- Ensemble mode (BBD-style bucket brigade modeling with clock noise)
- Voice spread control (auto-distribute or manual per-voice placement)
- Feedback with per-voice or global routing
- Stereo input handling with per-voice channel selection

## Parameters

**Global (effect-specific):** numVoices, detuneAmount, rate, depth, feedback, mode (chorus/flanger/ensemble), voiceSpread, dryWet
**Per-object (x12):** auto-spread based on voice index (azimuth evenly distributed, elevation and distance follow spread curve)

## Market Reference

No spatial chorus exists in any format. All existing chorus plugins (Valhalla SpaceModulator, TAL-Chorus-LX, Arturia Chorus JUN-6) are mono or stereo. Dimension D emulators create pseudo-spatial width but use simple L/R decorrelation. This would be the first chorus with true 3D positioning per voice, binaural HRTF rendering, and surround output.

## Skills Needed

- `time-based-effects` — chorus/flanger architecture, BBD modeling, LFO design
- `dsp-cookbook` — modulated delay lines, interpolation, feedback
- `spatial-audio-dsp` — per-voice spatialization, auto-spread algorithms
