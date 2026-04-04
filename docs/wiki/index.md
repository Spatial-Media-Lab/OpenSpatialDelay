# OpenSpatialDelay Documentation

OpenSpatialDelay is a spatial delay plugin where each echo lives in 3D space. Think of a classic stereo ping-pong delay, but instead of bouncing between left and right, the signal travels through up to 12 positions arranged anywhere around the listener.

Each delay tap has its own 3D position (azimuth, elevation, distance), pitch shift, Doppler amount, trajectory animation, and input channel routing. The output is rendered through one of five rendering paths -- from binaural HRTF convolution for headphones to discrete surround speaker panning for Dolby Atmos stages.

OpenSpatialDelay is the first plugin in the **Spatial Media Library**, an open-source suite of spatial audio tools by [Spatial Media Lab](https://spatialmedialab.org).

## Key Features

- **12 independent delay taps** positioned anywhere in 3D space
- **22 output formats** -- Binaural, Stereo, Surround (Quad through 9.1.6), and Ambisonics (1st through 6th order)
- **6 HRTF profiles** from measured SOFA files for realistic headphone spatialization
- **8 spatialization algorithms** for surround rendering (Constant Power, VBAP, VBIP, MDAP, KNN, DBAP, Ambisonics, Direct Binaural)
- **13 trajectory shapes** per tap for automated spatial movement
- **Tempo-synced or free-running delay** with cumulative pitch shifting
- **Per-tap pitch shift** that preserves timing, layered on top of global pitch
- **Wobble modulation** with morphable waveform (sine to square)
- **ADM-OSC** receive and send for integration with spatial audio workstations
- **60 factory presets** across 9 categories, plus user preset save/load
- **Stereo input routing** with per-tap L/R channel selection
- **Doppler effect** with distance-based air absorption

## Available Formats

- **VST3** -- macOS (Apple Silicon) and Windows (x64)
- **AU** -- macOS only

## Documentation Pages

| Page | Description |
|------|-------------|
| [Getting Started](getting-started.md) | Installation, loading in your DAW, first sound in 2 minutes |
| [Spatial Map](spatial-map.md) | The spatial map visualization and interaction |
| [Output Formats](output-formats.md) | All 22 output formats, rendering paths, and algorithm selection |
| [Controls Reference](controls-reference.md) | Complete parameter reference organized by UI section |
| [Trajectories](trajectories.md) | All 13 trajectory shapes, speed, direction, and visualization |
| [Presets](presets.md) | Factory presets, saving, loading, and file format |
| [ADM-OSC Integration](adm-osc.md) | External position control via ADM-OSC protocol |
| [Creative Tips](creative-tips.md) | Sound design recipes and production techniques |
| [Troubleshooting](troubleshooting.md) | Common issues and solutions |
| [Glossary](glossary.md) | Spatial audio terminology reference |

## System Requirements

| | Minimum |
|---|---|
| **macOS** | Apple Silicon (arm64), macOS 12+ |
| **Windows** | x64, Windows 10+ |
| **Formats** | VST3, AU (macOS only) |
| **DAWs** | Any VST3/AU host -- tested in Reaper; expected to work in Logic Pro, Ableton Live, Cubase, Bitwig, and others |

## Links

- [Spatial Media Lab](https://spatialmedialab.org)
- [GitHub Repository](https://github.com/AndrewRahman/OpenSpatialDelay)
- [Releases / Download](https://github.com/AndrewRahman/OpenSpatialDelay/releases)
