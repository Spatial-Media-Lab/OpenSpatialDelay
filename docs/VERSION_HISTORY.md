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

---

## v0.3 — FROZEN
**Direct Binaural Rendering + Architecture Refinement**

### Direct Binaural Rendering
- Per-source HRTF convolution: each tap's 3D position (azimuth + elevation) maps directly to an HRTF lookup via SOFA file
- No intermediate virtual speaker layout — eliminates the monitoring format dropdown
- 12 per-source PartitionedConvolvers replace 16 virtual speaker convolvers
- Realtime-safe HRIR updates at block boundaries via libmysofa KD-tree lookup (~1° threshold)
- Full height rendering via SOFA sphere measurements (superior to fixed virtual speaker elevations)
- Cross-profile normalization preserved for per-source HRIRs

### Output-Format-Aware Algorithm Selection
- Binaural (2ch bus): algorithm locked to "Direct Binaural", dropdown greyed out
- Surround (>2ch bus): full algorithm menu active (VBAP, VBIP, KNN, Ambisonics)
- Clean separation: binaural and surround are peer rendering paths, not layered

### Architecture Cleanup
- Removed: virtual speaker convolver bank (speakerConvL/R[16]), SH convolver bank (shConvL/R[16])
- Removed: renderSpeakerBuffers(), renderSHBuffers(), computeSHProjectedHRIRs()
- Removed: MonitoringFormat enum, MonitoringLayoutState, monitoring format APVTS parameter + UI
- Removed: updateSpeakerBinauralCache(), speaker-layout VBAP triplets and Ambisonics decode matrices
- Simplified processBlock binaural branching from 3 paths to 2

### Known Limitations

**Missing Air Absorption Distance Filter (Spec Section 4.3)**

The spec calls for a "6 dB/doubling distance high-shelf filter" to simulate frequency-dependent
air absorption. The current implementation applies level attenuation only (`distGain = 1/(d*4+0.25)`)
with no frequency-dependent distance filtering. This will be addressed in a future version
alongside enhanced distance modeling.

---

## v0.4 (2026-03-10) — FROZEN
**Enhanced DSP: DBAP, Doppler, Air Absorption**

Major feature release adding DBAP algorithm, per-object Doppler effect, air absorption
distance filtering, and comprehensive UI reorganization with design hierarchy.

### DBAP Algorithm (Distance-Based Amplitude Panning)
- New `DBAPAlgorithm` class implementing Lossius et al. (ICMC 2009)
- Computes speaker gains from Euclidean distances in Cartesian space
- Inverse-distance-squared weighting (`a=2`, 6 dB/doubling rolloff) with constant-power normalization
- Ideal for irregular/non-standard speaker layouts where VBAP triangulation fails
- Added as 6th algorithm (5 user-facing): Ambisonics, DBAP, KNN, VBAP, VBIP

### Doppler Effect (Per-Object)
- Per-object Doppler amount knob (`object{N}_dopplerAmount` parameter, 0=off, >0=on at that intensity)
- No global Doppler control — each tap controls its own Doppler independently
- Per-block velocity tracking: Cartesian position delta / block duration
- Exponential moving average smoothing (alpha=0.1) prevents clicks from sudden parameter jumps
- Pitch shift: `semitones = 12 * log2(c / (c + v * amount))`, clamped to +-12 semitones
- Speed of sound: 343 m/s at 20C, distance mapped to 0-10m physical scale
- Doppler only applies to direct tap output, NOT to the feedback path

### Air Absorption Distance Filter (Global)
- Global `airAbsorption` toggle parameter (enabled by default)
- Per-object IIR low-pass filter driven by each tap's distance
- Cutoff formula: `cutoff = 20000 * exp(-4 * distance)` Hz
- At distance=0: full bandwidth (20kHz). At distance=1: ~366 Hz cutoff
- Filter coefficients update once per block (safe for IIR stability)
- Only applies to direct tap output, NOT to the feedback path
- Resolves v0.3 Known Limitation: "Missing Air Absorption Distance Filter"

### UI Reorganization
- **Design hierarchy established:** Right panel = global, Bottom panel = per-object, Header = routing
- **Right panel sections:** DELAY → TONE (with AIR toggle) → MIX (signal flow order)
- **Bottom panel per-object controls:** ON/OFF, AZIMUTH, ELEV, DIST, DOPPLER
- DBAP added to algorithm dropdown (5 user-facing algorithms)
- Per-object DOPPLER amount knob (amber accent, 0=off)
- AIR absorption toggle right-aligned in TONE section header (cyan accent)
- Algorithm dropdown hidden entirely for Binaural output (was greyed out in v0.3)
- Algorithm and HRTF Profile dropdowns share same header position (swap based on output format)
- Title bar updated to "OpenSpatialDelay v0.4"

### Version Bump Checklist (Standard Practice)
Every version bump updates: CMakeLists.txt (VERSION, PLUGIN_CODE, PRODUCT_NAME),
install_plugins.sh, PluginEditor.cpp title bar, VERSION_HISTORY.md, CLAUDE.md

### Files
Frozen snapshot in `Archive/v0.4/`:
- `PluginProcessor_v0.4.h`
- `PluginProcessor_v0.4.cpp`
- `PluginEditor_v0.4.h`
- `PluginEditor_v0.4.cpp`

---

## v0.5 (2026-03-11) — FROZEN
**Stereo Variants, Output Reordering, MDAP Algorithm, Bug Fixes, Code Optimization, Framework Foundations**

Major feature release adding 5 stereo output variants (mic simulations), MDAP algorithm,
reordering all output formats by category then channel count, refactoring processBlock into
dedicated render methods, comprehensive code optimization, build warning elimination, and
trajectory/ADM-OSC parameter stubs for v0.6+.

### Stereo Variant Rendering (5 modes)
- New `renderStereoVariant()` — 5th rendering path alongside HRTF, Woodworth, Ambisonics, Surround
- **Stereo:** Equal-power pan law from azimuth
- **Stereo (VBAP):** 2-speaker VBAP at ±30° virtual speakers
- **Stereo (XY):** Coincident cardioid pair at ±45°
- **Stereo (MS):** Mid-Side encoding (Mid=cos(az), Side=sin(az), L=M+S, R=M−S)
- **Stereo (Blumlein):** Crossed figure-8 pair at ±45°
- Elevation contributes only to distance attenuation (real mic pairs don't capture height in L/R)
- Stereo gains pre-computed per block (block-rate), per-sample delay engine shared with other paths

### MDAP Algorithm (Multiple-Direction Amplitude Panning)
- New `MDAPAlgorithm` class — VBAP with 8 auxiliary sub-sources on a spread ring
- Produces wider, more stable spatial images than point-source VBAP
- 7th algorithm total (6 user-facing): Ambisonics, DBAP, KNN, MDAP, VBAP, VBIP

### Output Format Reordering (22 total)
- 22 output formats ordered by category, then ascending channel count:
  - Binaural (1): Binaural (default)
  - Stereo (1): Stereo (5 sub-modes via algorithm parameter)
  - Surround (13): Quad, 5.0, 5.1, 7.0, 5.1.2, 7.1, Oct, 7.0.2, 5.1.4, 7.1.2, 7.1.4, 7.1.6, 9.1.6
  - Ambisonics (6): FOA, SOA, HOA, 4OA, 5OA, 6OA
- `OutputFormatInfo` struct gains `isStereoVariant` field
- Parameter migration in `setStateInformation` maps v0.4 indices (17 formats) → v0.5 indices (21 formats)
- `pluginStateVersion` property added to state XML for version detection

### Bug Fixes
- **VBAP 3D L/R swap fix:** `computeVBAPGains3D()` used max gain sum to select among overlapping
  brute-force triangles, which always picked the widest triangle spanning the L/R axis. Fixed by
  changing to min gain sum criterion (tightest enclosing triangle). Corrected all height-channel
  format rendering (5.1.2 through 9.1.6) with VBAP, VBIP, and MDAP algorithms.

### processBlock Refactoring
- Extracted 5 rendering paths into dedicated private methods:
  - `renderDirectBinauralHRTF()` — 3-pass HRTF convolution
  - `renderSimpleBinauralWoodworth()` — Woodworth ITD+ILD
  - `renderAmbisonicsOutput()` — SH encode + NFC-HOA shelf filters
  - `renderDiscreteSurround()` — speaker gains + LFE
  - `renderStereoVariant()` — mic simulation (new)
- Shared inline helpers: `readObjectSample()`, `processFeedbackSample()`
- processBlock is now a thin dispatcher: parameter reads → object state → dispatch

### Code Optimization
- **Parameter pointer caching:** Per-object `std::atomic<float>*` arrays cached at construction,
  eliminating 72+ string allocations per audio block
- **Doppler/air absorption gating:** Skip expensive sqrt/log2/trig/filter recalculation when
  object positions are static (threshold >1e-5)
- **Feedback filter gating:** Skip `makeLowPass()`/`makeHighPass()` when frequency unchanged (>0.1 Hz)
- **Pitch window caching:** Pre-computed `sampleRate * 0.250f` in prepareToPlay
- **Build warnings eliminated:** 0 warnings from project source (56 sign-conversion fixes,
  unused param/variable cleanup, dead code removal, JUCE infinity warning suppression)
- **Dead code removed:** Unused `noteDivisionRatios[]`, `kFontName`, `sinEl3`,
  dead `AmbisonicsAlgorithm::computeBinauralGains()` override

### Build Optimizations
- Power-of-2 delay buffer with bitmask indexing (eliminates integer division in hot path)
- `-ffast-math` (macOS) / `/fp:fast /arch:AVX2` (Windows) compiler flags
- Contiguous `sourceAccumBufStorage` allocation replaces 12 separate vectors

### Framework Documentation
- Added `# SPATIAL MEDIA LIBRARY` section boundary comments throughout PluginProcessor.h and .cpp
- Clear `SPATIAL FRAMEWORK` vs `DELAY-SPECIFIC` vs `MIXED` labels on all major code sections
- Guides future plugin authors on what to reuse (~68% framework) vs replace (~32% delay-specific)

### UI Updates
- Output format dropdown shows 22 formats (auto-populated from registry)
- Stereo variants: both Algorithm and HRTF Profile dropdowns hidden
- Binaural: HRTF Profile visible, Algorithm hidden (unchanged)
- Surround: Algorithm visible and enabled (unchanged)
- Ambisonics: Algorithm visible but disabled, shows "Ambisonics Encode" (unchanged)

### Trajectory + ADM-OSC Foundation (Stubs)
- `juce_osc` module linked in CMakeLists.txt
- Per-object trajectory parameters (inactive): `trajectoryShape` (6 shapes), `trajectorySpeed`
- Global `admOscEnabled` parameter (inactive)
- Parameter IDs established so presets saved in v0.5 carry forward to v0.6+

### Files
Frozen snapshot in `Archive/v0.5/`:
- `PluginProcessor_v0.5.h`
- `PluginProcessor_v0.5.cpp`
- `PluginEditor_v0.5.h`
- `PluginEditor_v0.5.cpp`
- `CMakeLists_v0.5.txt`

### Output
- 22 output formats (1 binaural + 1 stereo + 13 surround + 6 Ambisonics)

---

## v0.6 (2026-03-13)
**ADM-OSC Receive, Trajectory Animation, Elevation Visualization, Preset System**

Major feature release activating the ADM-OSC receiver and trajectory animation stubs from v0.5,
adding IEM-standard elevation visualization to the spatial map, and introducing a full preset
system with 8 factory presets and user-saveable JSON presets.

### ADM-OSC Receive
- `juce::OSCReceiver` with `MessageLoopCallback` listener on port 4002 (editable, range 1024-65535)
- `/adm/obj/N/` namespace (N = 1-12, 1-based ADM-OSC object IDs mapped to 0-based internal indices)
- 8 message types:
  - `/azim` (float, degrees)
  - `/elev` (float, degrees)
  - `/dist` (float, 0-1)
  - `/aed` (3 floats: azimuth degrees, elevation degrees, distance 0-1)
  - `/xyz` (3 floats: Cartesian x, y, z)
  - `/x` (float, individual Cartesian axis — accumulated with cached `/y`, `/z`)
  - `/y` (float, individual Cartesian axis — accumulated with cached `/x`, `/z`)
  - `/z` (float, individual Cartesian axis — accumulated with cached `/x`, `/y`)
- Cartesian-to-Polar conversion per ITU-R BS.2127-0:
  `az = atan2(-x, y)`, `el = atan2(z, sqrt(x^2 + y^2))`, `dist = clamp(sqrt(x^2 + y^2 + z^2), 0, 1)`
- Per-object OSC override flag: receiving any OSC message sets `oscOverrideActive[obj]`, which pauses
  trajectory animation for that object; 500 ms timeout after last receive releases the override
  and resumes trajectory
- `admOscEnabled` global parameter controls receiver (edge-detected: OFF-to-ON connects, ON-to-OFF
  disconnects and clears all overrides)

### Trajectory Animation
- 6 shapes: None, Spiral, Orbit, Bounce, Figure-8, Random
- Per-object `trajectoryShape` (`AudioParameterChoice`, 6 choices) and
  `trajectorySpeed` (`AudioParameterFloat`, 0.0-10.0, default 1.0)
- Phase accumulation at ~60 Hz timer rate: `phase += speed * dt` (dt = 1/60), wraps at 1.0
- Base position (azimuth, elevation, distance) captured when shape transitions from None to active
  or when shape changes
- Shape details (from `computeTrajectory()`):
  - **Spiral:** 360-degree azimuth sweep, +/-45-degree elevation sine, distance pulse 0.3-1.0
    (`dist = 0.3 + 0.7 * (0.5 + 0.5 * cos(phase * 2pi))`)
  - **Orbit:** 360-degree azimuth sweep at constant base elevation and distance
  - **Bounce:** triangle wave +/-90-degree azimuth, +/-30-degree elevation, constant distance
  - **Figure-8:** Lissajous 1:2 ratio, +/-90-degree azimuth, +/-45-degree elevation, constant distance
  - **Random:** irrational-frequency sine sums (e, pi, sqrt(2), sqrt(3), sqrt(5) as frequency
    multipliers) — pseudo-random but deterministic and smooth
- Output azimuth wrapped to -180..+180, elevation clamped to -90..+90, distance clamped to 0..1

### Elevation Visualization (IEM-Standard)
- Dot size encodes elevation: `baseDiam + 3 * sin(elevRad)` — above horizon = larger, below = smaller
- Base diameter: 18 px selected, 14 px unselected
- Hemisphere-dependent opacity: 1.0 above ear level, 0.3 below
- Selection halo alpha: 0.3 above, 0.15 below
- Dot outline always drawn at full colour (visible regardless of hemisphere)
- Elevation degree label shown for selected object when |elevation| > 1-degree
  (positioned above dot if positive, below if negative)
- Number label colour adapts to dot brightness: black text above horizon, object colour below
- No stems — matches IEM StereoEncoder / Nuendo / Pro Tools industry standard

### Preset System
- 8 factory presets:
  1. Default — 4 taps in diagonal cross pattern
  2. Stereo Ping-Pong — 2 taps at +/-90-degree
  3. Circle (Quad) — 4 taps in equidistant ring
  4. Surround 5.1 — 5 taps at standard 5.1 positions
  5. Surround 7.1 — 7 taps at standard 7.1 positions
  6. Atmos 7.1.4 — 11 taps (ear-level 7.1 + 4 height at 45-degree elevation)
  7. Rising Spiral — 8 taps spiraling upward with Orbit trajectory (speeds 1.5-3.2)
  8. Falling Cascade — 6 taps descending with pitch drop (-1 st)
- User presets: individual JSON files in `~/Library/Application Support/OpenSpatialDelay/Presets/`
- Preset data captures all global + per-tap parameters (delayTime, tempoSync, noteDivision,
  syncMode, feedback, filterLP, filterHP, pitchShift, dryWet, inputGain, outputGain,
  airAbsorption, algorithm, hrtfProfile, and per-tap enabled/azimuth/elevation/distance/
  dopplerAmount/trajectoryShape/trajectorySpeed)
- Presets do NOT store: `outputFormat`, `admOscEnabled`, `oscReceivePort`
- JSON serialization via `juce::JSON` (DynamicObject tree with "taps" array)
- User presets loaded from disk at startup, sorted alphabetically

### Preset Browser (UI)
- Header bar: ComboBox listing all factory + user presets
- Prev/Next buttons (`<` / `>`) for sequential preset stepping (wraps around)
- Save button opens AlertWindow with text editor for preset name input
- ComboBox auto-refreshes after save

### Per-Object Parameters (New in v0.6)
- `object{N}_trajectoryShape` (Choice: None / Spiral / Orbit / Bounce / Figure-8 / Random)
- `object{N}_trajectorySpeed` (Float 0.0-10.0, default 1.0)
- Parameter IDs established in v0.5 stubs, now fully active

### UI Updates
- Header bar: Preset browser (ComboBox + Prev/Next buttons + Save button)
- Header bar: OSC toggle button (green when active) + editable port label (double-click to edit,
  valid range 1024-65535, reverts on invalid input)
- Spatial map: OSC override indicator ("OSC" label in cyan on actively-controlled objects,
  collision-aware positioning relative to elevation label)
- Per-object panel: Trajectory Shape dropdown + Speed slider

### State Format
- `pluginStateVersion = 10`
- Persists `oscReceivePort` and `currentPresetIndex` in state XML alongside APVTS tree
- v0.4-to-v0.5 output format migration preserved in `setStateInformation`

### Files
- v0.6 is current HEAD (no archived snapshot yet)
- `scripts/adm_osc_test.py` — ADM-OSC test tool for sending OSC messages to the plugin

### Output
- 21 output formats (1 binaural + 1 stereo + 13 surround + 6 Ambisonics)
