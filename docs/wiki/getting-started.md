# Getting Started

This guide covers installation, loading OpenSpatialDelay in your DAW, and getting your first spatial delay sound within two minutes.

## Installation

### macOS (Apple Silicon)

**From release download:**

1. Download the latest release from the [Releases page](https://github.com/AndrewRahman/OpenSpatialDelay/releases)
2. Copy the AU plugin to: `~/Library/Audio/Plug-Ins/Components/`
3. Copy the VST3 plugin to: `~/Library/Audio/Plug-Ins/VST3/`
4. Factory presets are installed automatically to: `~/Library/Audio/Presets/OpenSpatialDelay/`

**From source:**

```bash
git clone --recursive https://github.com/AndrewRahman/OpenSpatialDelay.git
cd OpenSpatialDelay
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The post-build script automatically copies both AU and VST3 plugins to the correct locations and installs factory presets.

> **Note:** Building from source requires CMake 3.22+, a C++17 compiler, and zlib. JUCE 8 and libmysofa are fetched automatically.

### Windows (x64)

**From release download:**

1. Download the VST3 from the [Releases page](https://github.com/AndrewRahman/OpenSpatialDelay/releases)
2. Copy the `.vst3` file to: `C:\Program Files\Common Files\VST3\`
3. Factory presets are installed on first launch to: `%APPDATA%\OpenSpatialDelay\Presets\`

**From source:**

```bash
git clone --recursive https://github.com/AndrewRahman/OpenSpatialDelay.git
cd OpenSpatialDelay
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

> **Note:** Windows builds require Visual Studio 2022 and vcpkg for zlib.

### Verifying Installation

After installation, rescan plugins in your DAW:

- **Reaper:** Options > Preferences > Plug-ins > VST (or AU) > Re-scan
- **Logic Pro:** AU plugins are detected automatically on restart
- **Ableton Live:** Preferences > Plug-ins > Rescan
- **Cubase:** Rescan on restart or via Plug-in Manager

OpenSpatialDelay will appear under the "Spatial Media Lab" or "OpenSpatialDelay" manufacturer name, depending on your DAW.

## Loading in Your DAW

Insert OpenSpatialDelay as an effect on any audio or bus track. It processes audio in real time -- send any signal through it to hear the spatial delay.

### Channel Configuration

OpenSpatialDelay adapts to the track's channel configuration:

- **Stereo track (2 channels):** Binaural or Stereo output modes are available
- **Multi-channel track (4+ channels):** Surround and Ambisonics output modes become available

> **Tip:** If you want surround output, make sure the track is configured for the correct channel count before loading the plugin. In Reaper, set the track channel count via the routing dialog.

## First Sound in 2 Minutes

1. **Load the plugin** on a track with audio (vocals, guitar, drums -- anything works)
2. **Select an output format** from the dropdown in the header bar. Choose **Binaural** if you are monitoring on headphones, or **Stereo** for speakers
3. **Press play** in your DAW. You will hear four delay taps spread across the spatial field -- this is the Default preset
4. **Drag a tap** on the spatial map to move it. The dot represents a delay tap's position in space. Click and drag to reposition
5. **Adjust the delay time** using the TIME knob in the right panel. All taps share the same base delay interval -- tap 1 plays at 1x the delay time, tap 2 at 2x, and so on
6. **Increase feedback** with the FEEDBACK knob to hear more repeats
7. **Try a preset** -- click the preset name in the header bar and browse through categories like "Spatial Movement" or "Ambient + Texture"

## Output Format Quick Pick

Choose based on your monitoring setup:

| Monitoring Setup | Recommended Format | Notes |
|---|---|---|
| Headphones | Binaural | Full 3D with HRTF -- most immersive on headphones |
| Stereo speakers | Stereo | 5 mic simulation modes (Equal Power, VBAP, XY, MS, Blumlein) |
| 5.1 speakers | 5.1 Surround | Standard film/broadcast surround |
| 7.1 speakers | 7.1 Surround | Extended surround |
| 7.1.4 Atmos | 7.1.4 Atmos | Dolby Atmos with height speakers |
| Ambisonics pipeline | 1st-6th Order Ambi | For ambisonics-based workflows (e.g., VR, 360 video) |

> **Tip:** Output format is intentionally not saved in presets. This lets you load any preset regardless of your monitoring setup -- the spatial positions translate across formats.

## Next Steps

- Learn about the [Spatial Map](spatial-map.md) to understand how positions work
- Explore the [Controls Reference](controls-reference.md) for every parameter
- Try [Trajectories](trajectories.md) to animate your delay taps
- Browse [Creative Tips](creative-tips.md) for sound design recipes
