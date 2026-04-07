# OpenSpatialPanner

**One-line:** Object-based spatial panner with multi-source management, trajectory automation, and energy distribution
**Type:** Effect
**Complexity:** Low-Medium
**In Roadmap:** Yes
**Template:** spatialcore-effect

## Concept

A dedicated spatial panning tool for positioning up to 12 audio objects in 3D space with precise control over distance attenuation, proximity effect, and energy distribution. This is essentially SpatialCore with a thin, focused control layer -- no delay, no granular, no reverb -- just pure spatial positioning. It serves as the foundational tool for object-based mixing workflows, competing directly with expensive solutions like SPAT Revolution at a fraction of the cost (free).

## What SpatialCore Provides

- Everything -- this plugin IS SpatialCore with a thin control layer
- All 7 spatialization algorithms
- HRTF convolution with 6 profiles
- 22 output formats
- ADM-OSC Receive/Send for full interop with spatial audio DAWs
- Trajectory system for all 13 animation shapes
- Per-object spatial pipeline

## Plugin-Specific DSP

- Distance attenuation curves (inverse, inverse-square, linear, exponential, custom)
- Per-object gain, mute, solo controls
- Energy distribution visualization
- Proximity effect (bass boost at close range)
- Air absorption with frequency-dependent rolloff
- Spread control (point source to diffuse)
- Link/group objects for coordinated movement

## Parameters

**Global (effect-specific):** distanceCurve, proximityEffect, masterGain
**Per-object (x12):** gain, mute, solo (plus inherited azimuth, elevation, distance, trajectories)

## Market Reference

IEM StereoEncoder is free but limited to stereo/Ambisonics with single-object control. SPAT Revolution (FLUX::) is the industry standard but costs thousands and requires a separate application. Dear Reality dearVR MONITOR handles binaural only. No free, open-source plugin offers 12-object panning with 7 algorithms, HRTF binaural, surround, and Ambisonics in a single VST3/AU.

## Skills Needed

- `spatial-audio-dsp` — all algorithms, HRTF, distance models
- `oiloil-ui-ux-guide` — clean multi-object management UI
