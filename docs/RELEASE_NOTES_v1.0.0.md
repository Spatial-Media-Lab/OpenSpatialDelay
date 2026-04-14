# OpenSpatialDelay v1.0.0 — Release Notes

**A free, open-source spatial delay plugin where each echo lives in 3D space.**

12 delay taps, 23 output formats (binaural through 9.1.6 Atmos), 8 panning algorithms, 70 factory presets — VST3 + AU for macOS and Windows.

---

## Features

### Spatial Delay Engine
- **12 independent delay taps**, each positioned in 3D space (azimuth, elevation, distance)
- Per-tap pitch shifting (±12 semitones via phase vocoder)
- Per-tap Doppler amount control
- Per-tap stereo input routing (L+R / L / R)
- Tempo sync with note division and offset modes
- Filtered feedback with LP/HP controls and soft-clip saturation

### Spatialization
- **8 panning algorithms:** Constant Power (default), VBAP, VBIP, MDAP, KNN, DBAP, Ambisonics (HOA), Direct Binaural
- **6 HRTF profiles** for binaural rendering: Simple (low CPU), Studio Reference (MIT KEMAR), Immersive (SADIE II D2 KU100), Natural (CIPIC Subject 003), Precise (HUTUBS PP2), Spatial (Bernschuetz KU100)
- Low-frequency bypass for bass-deficient HRTF profiles
- Air absorption modeling

### Output Formats (23)
- **Binaural** (HRTF convolution, 2ch)
- **Stereo** (5 sub-modes including Mid-Side and XY)
- **Surround:** Quad, 5.0, 5.1, 7.0, 7.1, 9.1, Octaphonic, 5.1.2, 5.1.4, 7.1.2, 7.1.4 Atmos, 7.1.6, 9.1.4, 9.1.6, SpatialMediaLab 13.1
- **Ambisonics:** 1st through 6th order (ACN/SN3D)

### Trajectories & Animation
- **14 trajectory shapes:** Bounce, Circle, Cross, Figure-8, Heart, Helix, Infinity, Line, Orbit, Random, Spiral, Square, Triangle
- Per-tap speed and direction controls
- Global tap speed offset

### Presets
- **70 factory presets** across 9 categories: Classic Delays, Spatial Movement, Ambient + Texture, Height + 3D, Surround Production, Wobble + Modulated, Creative + Experimental, Rhythmic, Template
- User preset save/load with category organization

### Integration
- ADM-OSC support (receive + send) for external spatial audio workflows
- Plugin delay compensation (2048 samples reported to DAW)
- 144 automatable parameters; config params hidden from automation lists
- Correct bus negotiation for all output formats
- 291 automated tests (110,205 assertions) covering DSP, surround output, trajectories, OSC, convolution, input routing, and Ambisonics

### Tape Wobble
- 4-layer tape wobble emulation with amount and morph controls

---

## System Requirements

| | Minimum |
|---|---|
| **macOS** | Apple Silicon (arm64), macOS 12 Monterey or later |
| **Windows** | x64, Windows 10 or later |
| **Formats** | VST3, AU (macOS only) |
| **DAW** | Any VST3/AU-compatible host (tested in REAPER, Ableton Live) |

---

## Installation

### macOS

1. Download `OpenSpatialDelay-v1.0.0-macOS-arm64.zip`
2. Unzip and copy:
   - `OpenSpatialDelay v1.0.component` → `~/Library/Audio/Plug-Ins/Components/`
   - `OpenSpatialDelay v1.0.vst3` → `~/Library/Audio/Plug-Ins/VST3/`
3. Remove quarantine (required — not yet notarized):
   ```bash
   xattr -cr ~/Library/Audio/Plug-Ins/VST3/OpenSpatialDelay*.vst3
   xattr -cr ~/Library/Audio/Plug-Ins/Components/OpenSpatialDelay*.component
   ```
4. Restart your DAW

### Windows

1. Download `OpenSpatialDelay-v1.0.0-Windows-x64.zip`
2. Unzip and copy `OpenSpatialDelay v1.0.vst3` → `C:\Program Files\Common Files\VST3\`
3. Restart your DAW
4. If Windows SmartScreen blocks the file, click "More info" → "Run anyway"

### Uninstallation

**macOS:**
```
~/Library/Audio/Plug-Ins/Components/OpenSpatialDelay*.component
~/Library/Audio/Plug-Ins/VST3/OpenSpatialDelay*.vst3
~/Library/Audio/Presets/OpenSpatialDelay/    (user presets — optional)
```

**Windows:**
```
C:\Program Files\Common Files\VST3\OpenSpatialDelay*.vst3
%APPDATA%\OpenSpatialDelay\    (user presets — optional)
```

---

## Bug Fixes (since v0.9)

Over 30 bugs resolved during the v1.0 development cycle, including:

- **Stability:** Fixed heap corruption crash under multi-instance load (#137), CoreGraphics crash with multiple instances (#131), Ableton crash on plugin deletion (#122)
- **Audio:** Fixed Doppler buzzing on moving objects (#77), dry signal mono collapse (#73, #78), chirping on preset changes (#84, #99), pop/click after transport scrub (#103), HRTF profile switching clipping (#90)
- **DAW compatibility:** Fixed VST3 not appearing in Ableton's browser (#120), single-parameter automation limit in Ableton (#122), VST3 track channel count detection (#111)
- **UI:** Fixed azimuth knobs rotating counter-clockwise on mousewheel (#157), OSC input fields requiring double-click (#152), algorithm dropdown blank after preset change (#93), global controls resetting when UI closed (#95)
- **Presets:** Corrected tap input assignments and timing for 18 presets (#91)

---

## Known Limitations

- **macOS:** Ad-hoc code signed (not notarized). Requires `xattr -cr` quarantine removal after download.
- **Windows:** Unsigned. May trigger SmartScreen warning on first run.
- **Intel Mac:** Not supported. Apple Silicon (arm64) only.
- **AAX:** Not available. Pro Tools support is planned for a future release.
- **Custom SOFA import:** Not available in v1.0.

---

## Licensing

### License
Licensed under the [GNU General Public License v3.0 (GPL-3.0)](LICENSE).

Third-party licenses and attribution are included in the Legal Notices document bundled with the release.

---

## Credits

**Design & Production:** Andrew Rahman
**Technical Architecture:** Claude (Anthropic)
**HRTF Data:** MIT KEMAR, SADIE II, CIPIC, HUTUBS, TH Köln
**Built with:** [JUCE 8](https://juce.com), [libmysofa](https://github.com/hoene/libmysofa)

---

## Links

- **Source code:** [github.com/Spatial-Media-Lab/OpenSpatialDelay](https://github.com/Spatial-Media-Lab/OpenSpatialDelay)
- **Bug reports:** [GitHub Issues](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues)
- **Website:** [spatialmedialab.org](https://spatialmedialab.org)
