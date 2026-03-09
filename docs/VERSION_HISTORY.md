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

## v0.2 (2026-03-09) — FROZEN
**Multi-Channel I/O + HRTF Convolution + Pitch Shifter + Cross-Platform Build**

Major feature release adding multi-channel surround output, true HRTF convolution
with 5 SOFA profiles, refined pitch shifting, and cross-platform Windows build support.

### Multi-Channel Output
- 7 output formats: Binaural, Quadraphonic, 5.1, 7.1, 7.1.4 Atmos, 9.1.6 Atmos, Octaphonic
- Dynamic output format detection via `detectOutputFormat()` + user override dropdown
- LFE generation: 120 Hz Butterworth LP, -10 dB, from mono sum of spatialized signals
- Octaphonic: 8-channel "center" configuration (C, RF, R, RR, Rear, RL, L, FL at 45-degree intervals)
- Context-sensitive UI: Direct Binaural disabled on surround tracks; HRTF dropdown hidden

### Spatialization Algorithms (all 5, polymorphic interface)
- `SpatializationAlgorithm` abstract base class with virtual `computeGains()` dispatch
- Polymorphic virtual methods: `supportsBinauralDirect()`, `computeBinauralGains()`, `supportsSurround()`, `supportsSHDomain()`
- No switch-dispatch — all algorithm selection goes through `algorithms[]` pointer array + virtual calls
- Ambisonics (HOA): 3rd-order SH encode, Tikhonov-regularized pseudo-inverse decode, SH-domain HRTF
- KNN: K-nearest-neighbor speaker interpolation
- VBAP: 2D pair-wise for flat layouts, 3D triplet-wise for height layouts
- VBIP: Intensity-weighted VBAP (squared gains, re-normalized)
- Direct Binaural: Simplified Woodworth ITD+ILD (internal, used for "Simple" HRTF profile)
- Algorithm dropdown reordered alphabetically (Ambisonics=0, KNN=1, VBAP=2, VBIP=3)

### Pitch Shifter
- Warm dual-head Hann crossfade with 250ms window (~4 splices/sec)
- No splice detection — natural phase misalignment creates warm, chorused character
- No pitch cap — cumulative pitch accumulates freely through feedback cycles
- Full ±90° elevation range (no clamp)
- Evaluated and rejected: H949 cross-correlation (functional but less preferred character),
  Multi-Grain 4-head overlap-add (fundamentally flawed comb filtering),
  single-head tether (broken — converged to static offset)

### HRTF Convolution
- 5 SOFA HRTF profiles loaded via libmysofa v1.3.2:
  - Simple (Woodworth ITD+ILD, low-latency monitoring)
  - MIT KEMAR Large Pinna
  - SADIE II D2 KU100
  - CIPIC Subject 003
  - HUTUBS PP2
- Partitioned FFT convolution with 16 virtual speakers
- SH-domain HRTF path for Ambisonics binaural output
- SN3D-correct quadrature weights: `(2l+1)/M` (not `4pi/M`)
- Self-calibrating cross-profile normalization: `targetRMS = 1/sqrt(irLen)`
- Double-buffered lock-free HRIR loading (no audio glitches on profile switch)

### Cross-Platform Build
- CMakeLists.txt supports both macOS (arm64, VST3+AU) and Windows (x64, VST3 only)
- MSVC flags: `/Zc:preprocessor` for JUCE 8 compatibility
- GitHub Actions CI: automatic Windows VST3 build on push to `main`
- vcpkg integration for zlib on Windows (system zlib on macOS)
- Download script: `scripts/download_windows_build.sh` pulls latest CI artifact to `build/windows/`
- Git LFS for SOFA HRTF files (58MB total)

### Git & Version Control
- GitHub repo: `github.com/AndrewRahman/OpenSpatialDelay` (private)
- JUCE as git submodule
- SOFA files tracked via Git LFS
- `gh` CLI for repo management and CI artifact download

### Files
Frozen snapshot in `Archive/v0.2/`:
- `PluginProcessor_v0.2.h`
- `PluginProcessor_v0.2.cpp`
- `PluginEditor_v0.2.h`
- `PluginEditor_v0.2.cpp`
