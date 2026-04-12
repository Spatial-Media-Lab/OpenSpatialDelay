# OpenSpatialReverb

**One-line:** Algorithmic reverb where early reflections are individually positioned in 3D space, diffuse tail spread across spatial field
**Type:** Effect
**Complexity:** High
**In Roadmap:** Yes
**Template:** spatialcore-effect

## Concept

An algorithmic reverb that spatializes individual early reflections using image-source modeling, placing each reflection at its computed 3D position in the room. The diffuse tail is spread across the spatial field using decorrelated FDN outputs mapped to spatial positions. This creates a physically-informed sense of space that adapts to room geometry parameters, far beyond what static stereo or quad reverbs can achieve.

## What SpatialCore Provides

- All 7 spatialization algorithms (VBAP, VBIP, KNN, DBAP, MDAP, Ambisonics, Direct Binaural)
- HRTF convolution with 6 profiles (5 SOFA + Woodworth)
- 22 output formats (Binaural, Stereo, 13 Surround, 6 Ambisonics)
- ADM-OSC Receive/Send for external object control
- Trajectory system for animated reflection positions
- Per-object spatialization pipeline (up to 12 objects)
- Output limiter, distance attenuation, air absorption

## Plugin-Specific DSP

- Feedback Delay Network (FDN) with Hadamard mixing matrices for diffuse tail
- Early reflection engine using image-source method (up to 12 reflections as spatial objects)
- Room shape controls: size, width, height, wall damping per surface
- Pre-delay with tempo sync option
- Diffusion network (allpass chains for density)
- Decay time with frequency-dependent damping (low/high ratio)
- Modulation on FDN delay lines for chorus-like movement
- Late reverb decorrelation for spatial spread

## Parameters

**Global (effect-specific):** roomSize, width, height, preDelay, decayTime, diffusion, damping, erLevel, tailLevel, dryWet
**Per-object (x12):** reflection position auto-computed from room geometry (azimuth, elevation, distance derived from wall positions and source location)

## Market Reference

Dear Reality dearVR PRO offers binaural room simulation but uses fixed reflection patterns without per-reflection spatial control. Waves B360 provides Ambisonics reverb but lacks object-based early reflections. No existing plugin offers per-reflection 3D positioning combined with user-selectable spatialization algorithms and multi-format output.

## Skills Needed

- `reverb-algorithms` — FDN, Schroeder, image-source, diffusion networks
- `spatial-audio-dsp` — per-reflection spatialization, HRTF, algorithm integration
- `dsp-cookbook` — filters, allpass chains, parameter smoothing
