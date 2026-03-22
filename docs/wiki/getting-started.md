# Getting Started

OpenSpatialDelay is a spatial delay plugin that places up to 12 independently positioned delay taps in 3D space. It supports binaural monitoring, multichannel surround, and Ambisonics output up to 6th order.

## System Requirements

| Platform | Minimum OS | Architecture | Formats |
|----------|-----------|--------------|---------|
| macOS | 12.0+ | Apple Silicon (arm64) | AU, VST3 |
| Windows | 10+ | x64 | VST3 |

A VST3 or AU compatible DAW is required (e.g., Logic Pro, Reaper, Nuendo, Ableton Live, Pro Tools).

## Installation Paths

**macOS Audio Unit:**
```
~/Library/Audio/Plug-Ins/Components/OpenSpatialDelay v0.6.component
```

**macOS VST3:**
```
~/Library/Audio/Plug-Ins/VST3/OpenSpatialDelay v0.6.vst3
```

**Windows VST3:**
```
C:\Program Files\Common Files\VST3\OpenSpatialDelay v0.6.vst3
```

After placing the plugin in the appropriate directory, rescan plugins in your DAW if it does not appear automatically.

## First Launch

When loaded for the first time, the plugin initializes with the **Default** preset:

- **4 delay taps enabled** in a diagonal cross pattern at -45, +45, -135, and +135 degrees azimuth
- **Algorithm:** VBAP (Vector Base Amplitude Panning)
- **Output format:** Binaural
- **Delay time:** 500 ms
- **Feedback:** 30%
- **Dry/Wet:** 50%
- All taps at 0 degrees elevation and 0.5 normalized distance

## Quick Workflow

1. **Insert** OpenSpatialDelay on an audio track in your DAW.
2. **Play audio** through the track. You should hear the default 4-tap spatial delay.
3. **Drag objects** on the spatial map (center of the UI) to reposition delay taps in azimuth and distance.
4. **Adjust delay and feedback** using the knobs on the right panel.
5. **Try presets** using the preset browser in the header bar -- click the dropdown or use the Prev/Next arrows to browse factory and user presets.
6. **Change the output format** via the Output Format dropdown to match your DAW bus configuration (Binaural, Stereo, Surround, or Ambisonics).

## Next Steps

- [Interface Overview](ui-overview.md) -- learn the full layout and controls
- [Presets](presets.md) -- explore factory presets and create your own
