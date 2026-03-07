# OpenSpatialDelay — Version History

## v0.1 (2026-03-05)
**Stereo Binaural Spatial Delay**

First release. Spatial delay effect where each delay object is positioned in 3D space,
rendered to binaural stereo via ITD+ILD head model.

### Features
- 12 spatial delay objects with azimuth, elevation, distance
- 3 spatialization algorithms: Direct Binaural, VBAP (16-speaker virtual array), Ambisonics (3rd-order HOA)
- 5 HRTF profile presets (simplified Woodworth ITD + broadband ILD)
- Tempo sync with note divisions (Notes, Triplet, Dotted, 16th)
- Cumulative pitch shifting per object
- Mono feedback with LP/HP filters and soft clipping
- Ableton 12-inspired dark UI with 2D spatial map
- ROYGBIV HSB gradient object colors

### Files
Frozen snapshot in `Source/v0.1/`:
- `PluginProcessor_v0.1.h`
- `PluginProcessor_v0.1.cpp`
- `PluginEditor_v0.1.h`
- `PluginEditor_v0.1.cpp`

### Output
- Stereo only (binaural)

---

## v0.2 (in progress)
**Multi-Channel I/O + Enhanced Algorithms**

Adds dynamic bus-aware multi-channel output. The plugin queries the host track's
channel configuration and spatializes directly to those channels.

### Planned Features
- Dynamic output format detection: Stereo, Quad, 5.1, 7.1, 7.1.4, 9.1.6
- Algorithms compute gains for actual output bus channels (not virtual speakers)
- 2D VBAP for flat layouts, 3D VBAP for height layouts
- Ambisonics encode → per-layout decode matrix (pseudo-inverse)
- VBIP (Vector Base Intensity Panning) algorithm
- KNN (K-Nearest Neighbor) panning algorithm
- LFE generation for surround formats
- Context-sensitive UI (Direct Binaural disabled for surround tracks)

### Files
Development on main source files:
- `Source/PluginProcessor.h`
- `Source/PluginProcessor.cpp`
- `Source/PluginEditor.h`
- `Source/PluginEditor.cpp`
