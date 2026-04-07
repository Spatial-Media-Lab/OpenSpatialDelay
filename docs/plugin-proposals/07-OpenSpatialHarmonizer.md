# OpenSpatialHarmonizer

**One-line:** Pitch harmonizer where each harmony voice is positioned in 3D space
**Type:** Effect
**Complexity:** High
**In Roadmap:** No
**Template:** spatialcore-effect

## Concept

A real-time pitch harmonizer that generates up to 12 harmony voices, each positioned at a unique location in 3D space. Harmony intervals can be set chromatically, locked to a musical scale, or driven by incoming MIDI. Each voice preserves formants to maintain natural vocal quality even at extreme intervals. Spatial positioning creates the perception of a choir or ensemble spread across the room -- a solo voice becomes a spatially distributed harmony section with controllable width, depth, and height.

## What SpatialCore Provides

- Per-voice spatialization (up to 12 harmony voices as spatial objects)
- HRTF convolution with 6 profiles for binaural harmony rendering
- Trajectory system for animated voice positions
- ADM-OSC Receive/Send for external voice position control
- 22 output formats for surround harmony
- Distance attenuation per voice (farther voices feel more ambient)
- Output limiter

## Plugin-Specific DSP

- Pitch detection engine (YIN algorithm for monophonic input)
- Formant-preserving pitch shifting (phase vocoder with envelope preservation)
- Per-voice interval selection with three modes: chromatic (fixed semitones), scale-locked (diatonic/modal), MIDI-driven (live chord input)
- Latency compensation for analysis window
- Per-voice detuning for natural ensemble feel
- Per-voice level control and pan-to-position mapping
- Glide/portamento between interval changes
- Vibrato per voice (rate, depth, delay)

## Parameters

**Global (effect-specific):** numVoices, intervals[], detune[], voiceLevels[], scaleMode, key, formantPreserve, dryWet
**Per-object (x12):** interval (semitones), detune (cents), level (dB)

## Market Reference

Eventide H3000 and Harmonizer series are the gold standard but output stereo only. Antares Harmony Engine offers multi-voice harmony but is stereo. TC-Helicon VoiceLive provides live harmony but targets vocals only. No harmonizer offers per-voice 3D positioning with HRTF binaural rendering or surround output. A spatially distributed harmony section is a genuinely new creative tool.

## Skills Needed

- `time-based-effects` — pitch shifting algorithms, phase vocoder, formant preservation
- `dsp-cookbook` — pitch detection (YIN), windowing, overlap-add
- `spatial-audio-dsp` — per-voice spatialization, ensemble spread
