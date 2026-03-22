# OpenSpatialDelay

**A spatial audio delay plugin for immersive mixing**

## Overview

OpenSpatialDelay is a 12-tap spatial delay where each delay object is independently positioned in 3D space (azimuth, elevation, distance). It supports 7 spatialization algorithms, 6 HRTF profiles with SOFA convolution, and 21 output formats from binaural stereo to 6th-order Ambisonics. Built with JUCE 8 for VST3 and AU.

## Key Features

- 12 spatial delay objects with independent 3D positioning
- 7 spatialization algorithms: Ambisonics HOA, DBAP, KNN, MDAP, VBAP, VBIP, Direct Binaural
- 5 stereo rendering modes: Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein
- 6 HRTF profiles via SOFA convolution (MIT KEMAR, SADIE KU100, CIPIC, HUTUBS, Bernschuetz)
- 21 output formats (Binaural, Stereo, Quad through 9.1.6 Atmos, FOA through 6OA Ambisonics)
- 6 trajectory animation shapes with per-object speed control
- ADM-OSC receive for external spatial automation (port 4002)
- 8 factory presets + user preset save/load
- Per-object Doppler effect and air absorption
- Tempo sync with note divisions (1/16 through whole note, triplet, dotted)
- Cumulative pitch shifting per delay tap (+/- 24 semitones)
- Ableton 12-inspired dark UI with interactive 2D spatial map

## Supported Platforms

| Platform  | Format   | Architecture         |
|-----------|----------|----------------------|
| macOS 12+ | VST3, AU | Apple Silicon (arm64) |
| Windows   | VST3     | x64                  |

## Build from Source

**Prerequisites:** CMake 3.22+, C++17 compiler, Git (for submodules), zlib

```bash
git clone --recursive <repo-url>
cd brisbane
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

**Dependencies (auto-fetched):**

- [JUCE 8](https://juce.com/) -- git submodule at `./JUCE`
- [libmysofa](https://github.com/hoene/libmysofa) v1.3.2 -- FetchContent from GitHub
- zlib -- system (required by libmysofa)
- 5 SOFA HRTF files -- embedded as binary resources via `juce_add_binary_data`

## Quick Start

1. Build and install the plugin (AU: `~/Library/Audio/Plug-Ins/Components/`, VST3: `~/Library/Audio/Plug-Ins/VST3/`)
2. Open your DAW and insert OpenSpatialDelay on a track
3. Four delay taps are enabled by default in a diagonal cross pattern
4. Drag objects on the spatial map to position them in space
5. Try the factory presets from the dropdown in the header bar
6. For headphone monitoring, select "Binaural" output format

## Documentation

- [Wiki Documentation](docs/wiki/) -- Getting Started, UI Overview, Parameters, Algorithms, and more
- [Version History](docs/VERSION_HISTORY.md)
- [Speaker Layouts Reference](docs/speaker-layouts.md)
- [User Manual](docs/OpenSpatialDelay_User_Manual_v0.6.docx)
- [ADM-OSC Test Script](scripts/adm_osc_test.py) -- test OSC integration with `python3 scripts/adm_osc_test.py`

## Architecture

Designed as a **Spatial Media Library** -- approximately 68% of the codebase is a reusable spatial audio framework (spatialization algorithms, HRTF convolution, output format management, speaker layouts) and 32% is delay-specific DSP. Source files are annotated with `SPATIAL FRAMEWORK` / `DELAY-SPECIFIC` / `MIXED` section comments.

## Dependencies and Credits

- [JUCE](https://juce.com/) -- Cross-platform C++ audio framework
- [libmysofa](https://github.com/hoene/libmysofa) -- SOFA/HRTF file reader (Piotr Majdak et al.)
- HRTF datasets: MIT KEMAR, SADIE II, CIPIC, HUTUBS, TH Cologne (Bernschuetz)
- Spatialization algorithms: VBAP (Pulkki 1997), DBAP (Lossius et al. 2009), MDAP (Pulkki 2000)

## License

License: TBD
