# OpenSpatialVocoder

**One-line:** Channel vocoder with each frequency band positioned in 3D space
**Type:** Effect
**Complexity:** High
**In Roadmap:** No
**Template:** spatialcore-effect

## Concept

A channel vocoder where the frequency bands are spatialized as individual 3D objects, creating a voice that is physically distributed across space. Low formant bands can anchor at the center while high-frequency sibilance scatters outward, or bands can be mapped to positions that trace the spectral shape in 3D. Supports external sidechain carrier input or internal oscillators (saw, square, noise, chord). Up to 12 spatial objects represent band groups from the analysis filterbank.

## What SpatialCore Provides

- Per-band spatialization (12 band groups mapped to 12 spatial objects)
- HRTF convolution with 6 profiles for binaural vocoder rendering
- ADM-OSC Receive/Send for external band position control
- Trajectory system for animated band positions (frequency sweep through space)
- 22 output formats
- Distance attenuation per band group
- Output limiter

## Plugin-Specific DSP

- Analysis filterbank (24-48 bands, 4th-order Butterworth bandpass)
- Envelope followers per band (attack, release, sensitivity)
- Synthesis filterbank (matched to analysis, driven by carrier)
- Internal carrier oscillators: sawtooth, square, white noise, chord (multiple detuned saws)
- External sidechain carrier input routing
- Band-to-object mapping (12 objects representing grouped bands across spectrum)
- Freeze function (hold current spectral envelope)
- Sibilance detection and preservation (unprocessed high-frequency pass-through)

## Parameters

**Global (effect-specific):** numBands, freqRange, envSpeed, carrierType, sidechain, internalCarrier (saw/square/noise/chord), dryWet
**Per-object (x12):** band frequency range, gain (each object represents a group of adjacent bands)

## Market Reference

Waves Morphoder, TAL-Vocoder, and iZotope VocalSynth are all stereo vocoders. Ableton's Vocoder device outputs stereo. No vocoder spatializes individual bands in 3D. The spatial distribution of vocoder bands creates an unprecedented effect where the timbral character of speech or instruments is mapped to physical positions, producing voices that literally fill the room.

## Skills Needed

- `dsp-cookbook` — filterbank design, envelope followers, bandpass filters
- `synthesis-techniques` — carrier oscillators, spectral analysis
- `spatial-audio-dsp` — per-band spatialization, frequency-to-position mapping
