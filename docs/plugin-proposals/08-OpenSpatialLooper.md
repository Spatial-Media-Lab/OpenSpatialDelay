# OpenSpatialLooper

**One-line:** Spatial loop station where each recorded layer occupies its own 3D position
**Type:** Effect
**Complexity:** Medium
**In Roadmap:** No
**Template:** spatialcore-effect

## Concept

A loop recording station where each of up to 12 layers is spatialized at a unique 3D position. Record a guitar loop at center, add a vocal layer floating to the left, percussion above and behind. Each layer can be independently controlled (volume, speed, reverse) and repositioned in real time. Host sync and quantization keep layers locked to the DAW timeline. The result is a spatial performance tool where loop building becomes spatial composition.

## What SpatialCore Provides

- Per-layer spatialization (up to 12 loop layers as spatial objects)
- HRTF convolution with 6 profiles for binaural loop playback
- Trajectory system for animated layer positions (orbiting loops)
- ADM-OSC Receive/Send for external layer position control
- 22 output formats
- Distance attenuation and air absorption per layer
- Output limiter for dense multi-layer stacking

## Plugin-Specific DSP

- Loop recording engine with overdub capability
- Undo/redo per layer (single level)
- Half-speed and reverse playback per layer
- Per-layer volume, pan (pre-spatial), and mute/solo
- Host tempo sync with beat quantization (loop length snaps to bars)
- Loop quantization (free, beat, bar)
- Crossfade at loop boundaries (adjustable fade time)
- Audio buffer management (efficient memory for up to 12 layers x max loop length)
- Arm/disarm per layer for recording

## Parameters

**Global (effect-specific):** loopLength, syncToHost, quantize, fadeTime
**Per-object (x12):** volume, speed (1x/0.5x), reverse (on/off), armed (record-ready)

## Market Reference

Boss RC-505 and RC-600 are the industry standard loop stations but output stereo only. Ableton Live's Looper device is mono/stereo. Augustus Loop and Mobius are powerful but stereo. No looper offers per-layer 3D spatial positioning. The spatial dimension transforms loop performance from a flat stereo stack into an immersive, spatially arranged composition.

## Skills Needed

- `juce-best-practices` — audio buffer management, real-time recording, thread safety
- `plugin-architecture-patterns` — state management for loop layers, undo/redo
- `spatial-audio-dsp` — per-layer spatialization integration
