# OpenSpatialSampler

**One-line:** Sample-based spatial instrument where each pad/MIDI note occupies a 3D position
**Type:** Instrument
**Complexity:** Medium
**In Roadmap:** No
**Template:** spatialcore-instrument

## Concept

A sample-based instrument where each of 12 pads is mapped to a MIDI note and positioned at a unique location in 3D space. Load drum samples, foley hits, or melodic one-shots, then trigger them from positions distributed around the listener. A kick drum from below-center, hi-hats orbiting overhead, snare at ear level. Each pad has independent ADSR, velocity sensitivity, and round-robin for realistic playback. Trajectory animation can move pads during playback, creating percussion that travels through space.

## What SpatialCore Provides

- Per-pad spatialization (12 pads as spatial objects)
- HRTF convolution with 6 profiles for binaural drum/sample rendering
- Trajectory system for animated pad positions during playback
- ADM-OSC Receive/Send for external pad position control
- 22 output formats for surround and Ambisonics percussion
- Distance attenuation and air absorption per pad
- Output limiter

## Plugin-Specific DSP

- Sample loading engine (WAV, AIFF, FLAC file support)
- Per-pad ADSR envelope (attack, decay, sustain, release)
- One-shot and loop playback modes per pad
- MIDI note mapping (configurable note-to-pad assignment)
- Velocity sensitivity with adjustable velocity curves (linear, exponential, logarithmic)
- Round-robin sample layers (up to 8 per pad)
- Voice stealing (oldest, quietest, same-note priority)
- Sample start/end point editing with zero-crossing snap
- Pitch control per pad (coarse semitones + fine cents)

## Parameters

**Global (effect-specific):** global gain, master filter (LP cutoff for overall darkness control)
**Per-object (x12):** sample file, ADSR (attack/decay/sustain/release), MIDI note, velocity curve, loop mode (off/forward/ping-pong)

## Market Reference

NI Battery and Ableton Drum Rack are the industry standard samplers but output stereo (or discrete multi-out without spatialization). NI Kontakt supports surround output but has no per-sample HRTF or spatial panning built in. No sampler positions individual samples in 3D space with binaural HRTF rendering and multi-algorithm surround panning.

## Skills Needed

- `plugin-architecture-patterns` — MIDI handling, sample management, voice allocation
- `juce-best-practices` — file I/O, audio buffer management, thread-safe sample loading
- `spatial-audio-dsp` — per-pad spatialization, MIDI-triggered spatial events
