# OpenSpatialDistortion

**One-line:** Multi-band distortion where each frequency band is positioned in 3D space
**Type:** Effect
**Complexity:** Medium
**In Roadmap:** No
**Template:** spatialcore-effect

## Concept

A multi-band distortion processor that splits the input into 2-6 frequency bands, applies independent saturation to each band, then spatializes each band as a separate 3D object. Low frequencies can rumble from below, midrange grit can sit at ear level, and high-frequency fizz can shimmer above -- creating a distortion that physically occupies space. Each band gets its own saturation character, from warm tube to harsh bitcrush, with the spatial separation preventing the mud that plagues traditional multi-band distortion.

## What SpatialCore Provides

- Per-band spatialization (2-6 bands mapped to spatial objects)
- HRTF convolution with 6 profiles for binaural rendering
- All 7 spatialization algorithms for surround output
- 22 output formats
- Trajectory system for animated band positions
- Distance attenuation and air absorption per band
- Output limiter (critical for distortion effects)

## Plugin-Specific DSP

- Crossover filters (Linkwitz-Riley 4th order, phase-coherent reconstruction)
- 2-6 band splitting with adjustable crossover frequencies
- Per-band saturation types: tube (asymmetric soft clip), tape (hysteresis), fuzz (hard clip), bitcrush (sample rate/bit depth reduction), wavefold (wavefolding)
- Oversampling 2x/4x for alias-free saturation
- Per-band drive, output gain, and wet/dry mix
- Pre/post EQ per band
- Input gain staging with VU metering

## Parameters

**Global (effect-specific):** numBands, crossoverFreqs[], satType[], drive[], mix[], oversampling, inputGain, dryWet
**Per-object (x12):** band frequency range mapped to object position (low bands low elevation, high bands high elevation by default)

## Market Reference

FabFilter Saturn 2 is the gold standard for multi-band distortion but is stereo-only. iZotope Trash 2, Soundtoys Decapitator, and Guitarix are all mono/stereo. No distortion plugin offers spatial band separation. The ability to physically separate frequency bands in 3D space is a novel approach to taming multi-band saturation while creating an immersive effect.

## Skills Needed

- `dsp-cookbook` — crossover filters, saturation algorithms, oversampling
- `spatial-audio-dsp` — per-band spatialization, frequency-to-position mapping
