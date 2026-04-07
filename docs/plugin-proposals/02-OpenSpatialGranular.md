# OpenSpatialGranular

**One-line:** Granular processor where each grain is an independent spatial object, scattering across 3D space
**Type:** Effect
**Complexity:** High
**In Roadmap:** Yes
**Template:** spatialcore-effect

## Concept

A granular audio processor that treats each active grain as an independent spatial object, scattering grains across a 3D field with controllable spread. Grains can be frozen, scrubbed, and randomized in both time and space, creating immersive textures that evolve spatially. The combination of granular time-domain manipulation with per-grain HRTF spatialization produces a uniquely enveloping effect impossible to achieve with stereo granular tools.

## What SpatialCore Provides

- Per-grain spatialization (up to 12 simultaneous grains as spatial objects)
- HRTF convolution with 6 profiles for binaural rendering
- Trajectory system for grain position animation (orbits, scatter patterns)
- ADM-OSC Receive/Send for external grain position control
- 22 output formats including surround and Ambisonics
- Distance attenuation and air absorption per grain
- Output limiter for dense grain clouds

## Plugin-Specific DSP

- Grain engine with configurable window shapes (Hann, Gaussian, trapezoid, rectangular)
- Grain size control (1-500ms) with density scheduling
- Pitch randomization per grain with range control
- Position randomization with spatial spread envelope
- Audio buffer management for freeze and scrub modes
- Grain scheduling (synchronous, asynchronous, quasi-synchronous)
- Overlap control (1-16x) for texture density
- Scrub position with jitter for textural variation

## Parameters

**Global (effect-specific):** grainSize, density, pitchRandom, positionSpread, scrubPosition, freeze, overlap, windowShape, dryWet
**Per-object (x12):** grain azimuth/elevation/distance spread range (defines scatter volume per grain slot)

## Market Reference

Sound Particles offers spatial granular but is a standalone renderer, not a real-time plugin. Borderlands (iOS) has spatial grains but no surround/binaural output. Arturia Quanta and Output Portal are stereo-only granular effects. No existing plugin combines real-time granular processing with HRTF binaural and multi-format surround output.

## Skills Needed

- `synthesis-techniques` — granular engine architecture, window functions, scheduling
- `spatial-audio-dsp` — per-grain spatialization, HRTF integration
- `dsp-cookbook` — buffer management, interpolation, parameter smoothing
