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
- Observatory v6 dark UI with 2D spatial map
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
  - Surround (13): Quad, 5.0, 5.1, 7.0, 7.1, Oct, 5.1.2, 5.1.4, 7.1.2, 7.1.4, 7.1.6, 9.1.4, 9.1.6
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

## v0.6 (2026-03-13) — FROZEN
**ADM-OSC Receive + Trajectory Animation + Preset System + UI Polish**

Major feature release adding ADM-OSC Receive for external position control, per-object
trajectory animation (6 shapes), a complete preset system with factory presets and user
save/load, a 6th HRTF profile, IEM-faithful elevation visualization, and comprehensive
UI polish across 10 targeted fixes.

### ADM-OSC Receive
- `juce::OSCReceiver` with `MessageLoopCallback` listener (message thread, safe for APVTS updates)
- Parses `/adm/obj/N/` namespace: `azim`, `elev`, `dist`, `aed`, `xyz`, `x`, `y`, `z`
- Cartesian→Polar conversion per ITU-R BS.2127-0: `azDeg = atan2(-x,y)*(180/pi)`, `elDeg = atan2(z,sqrt(x²+y²))*(180/pi)`
- 500ms override timeout: OSC takes priority over trajectory, auto-releases after timeout
- Partial Cartesian accumulation for individual `/x`, `/y`, `/z` messages
- Default port 4002, persisted in state XML as non-APVTS ValueTree property
- Connection managed by `admOscEnabled` toggle parameter
- Edge-detect enable/disable transitions in timer callback for auto-reconnect

### Trajectory Animation
- 6 trajectory shapes per object: None, Spiral, Orbit, Bounce, Figure-8, Random
- `computeTrajectory()` pure static function (stateless, testable)
- 60Hz message-thread timer advances phase, writes to APVTS via `setValueNotifyingHost()`
- Base position captured on shape change (None→active transition)
- OSC override takes priority — trajectory paused during external control
- Per-object `trajectoryShape` (0-5) and `trajectorySpeed` (0.1-10.0x) APVTS parameters

### Preset System
- 8 factory presets: Default plus 7 specialized spatial delay configurations
- `PresetData` struct with 28 parameters (global + per-tap × 12)
- User presets: save/load from `~/Library/Application Support/OpenSpatialDelay/Presets/` (JSON)
- API: `loadPreset()`, `saveUserPreset()`, `loadNextPreset()`, `loadPreviousPreset()`
- Header UI: ComboBox dropdown + prev/next navigation buttons + Save button
- Current preset index persisted in state XML

### 6th HRTF Profile
- Bernschuetz KU100 Full2Deg (CC BY 3.0) — "Spatial" preset
- 6 HRTF profiles total: Simple (Woodworth), MIT KEMAR, SADIE II D2, CIPIC Subject003, HUTUBS PP2, Bernschuetz KU100

### IEM-Faithful Elevation Visualization
- Hemisphere alpha: objects above horizon render solid/opaque, below render transparent (0.3 alpha)
- Selection halo: translucent ring around selected object, alpha varies by hemisphere
- Asymmetric dot scaling: +5px upward (z > 0), -3px downward (z < 0) — preserves 11px minimum at -90°
- Path-based faux bold number labels: `GlyphArrangement::createPath()` + `fillPath()` + `strokePath(0.8f)` for guaranteed visual weight regardless of font system
- Pixel-grid snapping for HiDPI: `std::round()` on dot positions + `juce::roundToInt()` on all compass labels
- Object outline stroke (1.2px) at full colour for visibility in both hemispheres

### UI Polish (10 Fixes)
- OSC section relocated from header bar to right panel below MIX (new "OSC" section with `drawSectionHeader`)
- ELEV slider height matched to adjacent rotary knobs (50×80px, shifted up for alignment)
- Bottom panel spacing tightened: inter-control gaps compressed (90→84, 70→64, 50→46) to prevent speed knob overflow
- Preset dropdown height matched to Output Format/Algorithm dropdowns (full 24px `boxH`)
- HiDPI pixel snapping with `juce::roundToInt()` replacing `(int)` casts throughout
- OSC port field styled with background (`#151525`) and border (`#334155`) matching ComboBox appearance
- Elevation label "+90°" offset increased from -10px to -14px for clearance from halo
- Redundant OSC connection status dot removed (button colour already indicates state)
- Compass labels (F/B/L/R) pixel-snapped for HiDPI clarity
- "Port:" label painted between OSC toggle and editable port field in right panel

### Testing Tool
- `scripts/adm_osc_test.py` — Python CLI using `python-osc` library
- 7 test modes: `--manual`, `--orbit`, `--sweep`, `--multi`, `--xyz`, `--aed`, `--axes`
- Configurable port (default 4002), send rate (default 60 Hz), object selection
- Real-time animation modes with Ctrl+C interruption

### Timer Consolidation
- Single 60Hz message-thread timer handles 4 responsibilities:
  1. HRTF profile loading (background → double-buffered renderer)
  2. OSC connection management (edge-detect enable, reconnect on port change)
  3. OSC override timeout (per-object 500ms release)
  4. Trajectory animation (phase advance, APVTS update)

### State Persistence
- `pluginStateVersion = 10` (v0.5 was version 9)
- State migration from v0.5 in `setStateInformation()`: v0.5 output format indices mapped to v0.6
- `oscReceivePort` persisted as non-APVTS ValueTree property
- `currentPresetIndex` persisted in state XML

### Files
Frozen snapshot in `Archive/v0.6/`:
- `PluginProcessor_v0.6.h`
- `PluginProcessor_v0.6.cpp`
- `PluginEditor_v0.6.h`
- `PluginEditor_v0.6.cpp`
- `CMakeLists_v0.6.txt`

### Output
- 21 output formats unchanged (1 binaural + 1 stereo + 13 surround + 6 Ambisonics)

---

## v0.7 (2026-03-16) — FROZEN
**ADM-OSC Send + Output Limiter + Doppler Fix + UI Refinement**

Feature release adding ADM-OSC Send for broadcasting tap positions to external renderers,
an output limiter to prevent DAW speaker protection muting during self-oscillation, Doppler
effect fix for orbit trajectories, and continued UI polish.

### ADM-OSC Send
- `juce::OSCSender` broadcasting `/adm/obj/N/aed` messages to external renderers (SPAT Revolution, L-ISA, Dolby Atmos Renderer)
- New parameters: `admOscSendEnabled` (bool toggle), non-APVTS: `oscSendIP` (string, default `"127.0.0.1"`), `oscSendPort` (int, default 4003)
- 30Hz send rate (every 2nd tick of 60Hz timer) per ADM-OSC best practice
- Position-change gating: only sends when position changes (>0.1° azimuth/elevation, >0.001 distance threshold)
- UI: SEND toggle + IP field + send port field in OSC section of right panel
- State persistence: IP and port persisted in state XML

### Output Limiter
- Musical +2dB ceiling (`outputLimiter()`) prevents DAW speaker protection muting during self-oscillation with high filter resonance
- Rational approximation soft saturator: linear passthrough below threshold, soft saturation above
- Soft saturator output protection approach
- Applied in all 5 rendering paths: Direct Binaural HRTF, Simple Binaural Woodworth, Stereo Variants, Ambisonics Output, Discrete Surround

### Doppler Effect Fix
- **Virtual ear offset:** Replaced scalar 3D distance (which is constant during orbit due to trigonometric identity) with distance-to-virtual-ear calculation. 2.5m x-axis offset breaks the symmetry for audible binaural Doppler from orbiting sources
- **Varispeed threshold lowered:** `readVarispeed()` early-return threshold reduced from 0.05 semitones (~5 cents) to 0.001 (~0.1 cents), eliminating silent gaps at zero-crossings during orbit oscillation
- **EMA alpha increased:** Exponential moving average smoothing alpha increased from 0.1 to 0.2 for 2x faster velocity tracking of 1Hz orbit oscillation

### Testing Tool Update
- `scripts/adm_osc_test.py` updated to v0.7 with new `--listen` mode
- Receives and prints incoming ADM-OSC messages from plugin's OSC Send
- Configurable listen port (default 4003)
- Handler for `/adm/obj/*/aed` with formatted output (object ID, azimuth, elevation, distance)
- Default handler for all other ADM-OSC messages
- 8 test modes total: `--manual`, `--orbit`, `--sweep`, `--multi`, `--xyz`, `--aed`, `--axes`, `--listen`

### UI Refinements
- OSC section expanded with SEND toggle, IP field, and send port field
- Custom font system with DM Sans + JetBrains Mono binary resources
- Continued UI polish from v0.6 design review

### State Persistence
- `pluginStateVersion = 11` (v0.6 was version 10)
- `oscSendIP` and `oscSendPort` persisted as non-APVTS ValueTree properties
- `admOscSendEnabled` persisted as APVTS parameter

### Files
Frozen snapshot in `Archive/v0.7/`:
- `PluginProcessor_v0.7.h`
- `PluginProcessor_v0.7.cpp`
- `PluginEditor_v0.7.h`
- `PluginEditor_v0.7.cpp`
- `CMakeLists_v0.7.txt`

### Output
- 21 output formats unchanged (1 binaural + 1 stereo + 13 surround + 6 Ambisonics)

---

## v0.8 (2026-03-17) — FROZEN
**Stereo Input + Wobble Modulation + Per-Tap Pitch + UI Redesign**

Feature release adding stereo input routing with per-tap channel selection, wobble
modulation (delay-time LFO), per-tap additive pitch shift, trajectory direction
control, and comprehensive UI redesign with new button components and color system.

### Stereo Input Routing
- Dual delay lines (`delayBufferL`, `delayBufferR`) for independent L/R input processing
- Per-tap input channel selection: L+R (summed), L only, R only
- New APVTS parameter: `object{N}_inputChannel` (choice: 0=L+R, 1=L, 2=R)
- INPUT button in bottom panel cycles through modes with color coding (white=L+R, blue=L, red=R)

### Wobble Modulation
- Delay time LFO with morphable waveform (sine → triangle → square)
- 3 new APVTS parameters: `wobbleEnabled` (bool), `wobbleAmount` (0–100), `wobbleMorph` (0–100)
- LFO frequency auto-syncs to delay time for musical modulation
- MOD section in right panel with enable toggle + AMOUNT and MORPH knobs
- Rose/pink accent color for modulation controls

### Per-Tap Pitch Shift (Additive)
- Changed from override mode (v0.7) to additive: per-tap pitch adds on top of global cumulative pitch
- Formula: `totalSemitones = (k × globalPitch / 100) + perTapPitch + dopplerSemitones`
- Existing `object{N}_pitchShift` parameter, behavior changed

### Trajectory Direction Control
- New `object{N}_trajectoryDirection` parameter (choice: 0=Forward, 1=Reverse)
- Phase flip in `computeTrajectory()` for reverse playback
- Forward/Reverse arrow buttons (← →) in bottom panel trajectory section

### UI: New Button Components
- `IndicatorToggle` — reusable toggle pill with 5px indicator dot + uppercase label
- `StyledButton` — centred-text button with no indicator dot, same visual language
- Applied to: tap ON/OFF, tempo sync, OSC toggles, air absorption, FLT/MOD headers, input channel, trajectory direction

### UI: Color System Overhaul
- Lifted backgrounds (12–16% lightness) replacing flat black
- Increased border visibility with subtle warm-grey strokes
- Gold sync accent for tempo sync buttons
- Rose/pink accent for modulation controls
- L/R channel colors: blue (left), red (right) for input channel button
- Consistent `Colours_OSD` namespace throughout

### UI: Spatial Map Refinements
- Thinner distance rings (0.7f stroke matching center reticle weight)
- Brighter crosshair dashed lines (0.6f alpha matching rings)
- Centered F/B cardinal labels using float-precision bounding rectangles

### State Persistence
- `pluginStateVersion = 12` (v0.7 was version 11)
- New parameters: `wobbleEnabled`, `wobbleAmount`, `wobbleMorph`, `object{N}_trajectoryDirection`, `object{N}_inputChannel`

### Files
Frozen snapshot in `Archive/v0.8/`:
- `PluginProcessor_v0.8.h`
- `PluginProcessor_v0.8.cpp`
- `PluginEditor_v0.8.h`
- `PluginEditor_v0.8.cpp`
- `CMakeLists_v0.8.txt`

### Output
- 21 output formats unchanged (1 binaural + 1 stereo + 13 surround + 6 Ambisonics)

## v0.9 (2026-03-19) — FROZEN
**Trajectory System Rewrite + Preset Overhaul + WSOLA Per-Tap Pitch + SML Branding + 22 Issues Closed**

Major release: complete trajectory system rewrite (13 shapes with origin-point architecture),
60-preset factory suite with build-time installer, WSOLA-lite per-tap pitch shifting,
Spatial Media Lab branding, spatial map distance labels, and 22 GitHub issues resolved.
Also includes UI polish, recurring bug fixes, and preset save overlay.

### Bug Fixes
- **Input gain leak (recurring):** Fixed `inputGain` applying to both dry and wet paths — now only applies to delay write (Stage 1). Root cause documented in `docs/bug-reports/RECURRING_INPUT_GAIN_LEAK.md`
- **Filter defaults:** Changed HP default from 20Hz to 50Hz, LP default from 20kHz to 5kHz, resonance defaults to 0.71. Disabled filter visualization now matches enabled state (WYSIWYG)
- **Filter toggle default:** Filter now defaults to OFF (was incorrectly defaulting to ON)

### WSOLA-lite Per-Tap Pitch Shifting
- New `WSOLAProcessor` struct: per-tap time-domain pitch shifting that preserves delay timing
- Split architecture: varispeed for global pitch (warm character), WSOLA-lite for per-tap pitch (rhythm-preserving)
- Eliminates the timing disruption that occurred with the previous dual-head crossfade approach for per-tap pitch
- Each of 12 taps has an independent WSOLA processor instance

### Preset Save Overlay
- Replaced `juce::AlertWindow` (separate OS window) with `PresetSaveOverlay` custom component
- In-plugin modal overlay: semi-transparent backdrop + centered card, cannot float to other screens
- Observatory v6 design: `bgPanel` card, `borderSubtle` outline, 6px rounded corners
- Smart name pre-fill: user presets pre-fill the name field, factory presets show empty field
- Save button: filled cyan (`accentStellar`), Cancel button: unfilled red-tinted with red text
- Keyboard: Return to save, Escape to cancel, backdrop click to dismiss
- Both buttons have proper hover (+12% brightness) and click (+15% brightness) states

### UI: Text Input Field Styling
- Knob text editors: thin 1px rounded-rectangle outline in section accent color when editing
- OSC input fields: thin 1px sharp-rectangle outline in cyan when editing
- Text selection highlight: accent-matched per section (cyan for DELAY/OSC, rose for MOD, amber for MIX, violet for TONE)
- Consistent text centering across all input fields

### UI: Dropdown Fixes
- Fixed text alignment regression: restored `getLabelBorderSize()` padding in custom `drawLabel` override (was drawing text edge-to-edge after sharp-rect outline change)
- Added `positionComboBoxText` override: reserves 20px for arrow (not JUCE default 30px), fixing "Figure-8" truncation in trajectory dropdown
- All dropdown text now has proper left padding matching v0.8 appearance

### Output Limiter
- Ceiling remains at +2dB rational approximation soft saturator
- Fixed interaction with input gain to prevent exceeding limiter ceiling during self-oscillation

### State Persistence
- `pluginStateVersion = 13` (v0.8 was version 12)
- Filter defaults updated: HP 50Hz, LP 5kHz, Res 0.71
- Filter enabled default: OFF

### Preset System Overhaul (v0.9, 2026-03-18)
- **60 factory presets** across 9 categories: Classic Delays, Spatial Movement, Ambient + Texture, Height + 3D, Surround Production, Wobble + Modulated, Creative + Experimental, Rhythmic, User
- **PresetData extracted** to shared `PresetData.h`/`PresetData.cpp` for use by both plugin and build-time `install_presets` CLI tool
- **Build-time preset installation:** `install_presets` CLI binary generates `.osdpreset` JSON files from C++ source. CMake `add_dependencies(OpenSpatialDelay_VST3 install_presets)` ensures correct build ordering. Factory presets always overwritten from source; user presets untouched.
- **Industry-standard preset location:** `~/Library/Audio/Presets/OpenSpatialDelay/` (was `~/Library/Application Support/OpenSpatialDelay/Presets/`)
- **User folder** created automatically for user-generated presets
- **Filter resonance in presets:** `filterLPQ` and `filterHPQ` fields added to PresetData struct. Backward-compatible JSON parsing (defaults to 0.707f Butterworth if missing from old files).
- **SMPTE channel ordering** for surround presets: 5.1 (L,R,C,Ls,Rs), 7.1 (L,R,C,Lss,Rss,Lrs,Rrs), 7.1.4 Atmos (adds Tfl,Tfr,Trl,Trr at +45° elevation)
- **Per-tap pitch presets:** Ascending Staircase (+1/+2/+3st), Falling Cascade (-1 to -5st), Fifth Ghost (tap3 -5st)
- **Bug fix:** `writeFactoryPresetsToDisk()` had `if (file.existsAsFile()) continue;` that prevented source changes from reaching disk. Rhythmic presets (new in v0.9) worked while older presets had stale data. Root cause: disk caching, not source values.

### Preset Dropdown Styling Fix (v0.9, 2026-03-18)
- **Root cause:** `showPresetMenu()` never called `setLookAndFeel()` on the PopupMenu, causing it to use the default system LookAndFeel instead of `OSDLookAndFeel`
- **Fix:** Added `mainMenu.setLookAndFeel(&osdLookAndFeel)` before `showMenuAsync()`
- Custom `drawPopupMenuItem()` override ensures consistent rendering: 24px item height, DM Sans Regular 13px, compact 4×6px submenu arrows matching ComboBox dropdown arrow size

### AIR Absorption True Bypass + Perceptual Curve (v0.9, 2026-03-18)
- **Bug fix:** Position-change gating prevented AIR toggle from taking immediate effect on static taps. Loading a preset with AIR enabled showed no effect; turning AIR off left the effect active.
- **True bypass:** `readObjectSample()` now skips the filter entirely when AIR is off (zero CPU cost vs previous always-running 20kHz passthrough)
- **Edge detection:** `airStateChanged` flag detects toggle transitions, forcing coefficient update even on static taps
- **Perceptual distance curve:** Replaced linear `dist` with quadratic `dist²` mapping (coeff=3.1). 0.5 ≈ 5m (8.6kHz, natural warmth), 1.0 ≈ 20m (900Hz, very dark). Minimum cutoff 500Hz.

### Trajectory System Rewrite (v0.9, 2026-03-18/19)
- **13 shapes:** None, Bounce, Cross, Figure-8, Heart, Helix, Infinity, Line, Orbit, Random, Spiral, Square, Triangle
- **Origin-point architecture:** Knobs = live origin position, trajectory computes animated position stored in internal `trajectoryFinalAz/El/Dist[]` arrays. `processBlock` reads from arrays when `trajectoryActive[]` is true. Knobs never overwritten — user can reposition running trajectories.
- **Cartesian shapes** (Figure-8, Square, Triangle) rotate by baseAz and offset by baseDist
- **Spiral/Heart** use origin-relative distance scaling
- **Random:** Multi-sine noise with randomized frequencies/phases/signs per instance, non-wrapping time accumulator (never repeats). Runs at half base speed.
- **Direction control:** Forward/Reverse per object via `object{N}_trajectoryDirection`
- **OSC integration:** OSC Receive sets origin when trajectory active; OSC Send broadcasts animated position

### Trajectory Visualization (v0.9, 2026-03-18/19)
- Glow trail on spatial map for selected tap — brightens near animated dot, dims away
- Elevation encoded as opacity (0.3–1.0) + line thickness (1.0–5.5px)
- Crosshair origin marker at knob position when trajectory active
- Spiral skips wrap-back segment for clean visual
- Random uses 2s look-ahead trail via `evaluateRandomNoise()`
- **Known regression:** Glow trail visual quality degraded during trajectory rewrite; deferred to future version. See `docs/bug-reports/GLOW_TRAIL_REGRESSION.md`.

### Test Infrastructure (v0.9, 2026-03-18)
- Catch2 v3.7.1 via CMake FetchContent
- 33 unit tests, 102 assertions
- Covers all 13 trajectory shapes, origin-relative behavior, control flags, wrapping, clamping, reverse mode
- Run: `cmake --build build --target OpenSpatialDelayTests && ./build/OpenSpatialDelayTests`

### SML Branding (v0.9, 2026-03-19)
- SML badge button in header bar linking to spatialmedialab.org
- Custom SVG icon (spatial node graph) at 9px
- Roboto Medium 11.5f font, button height 17px matching title visual weight
- Width computed from actual content (icon + gap + text + matched horizontal padding)

### Spatial Map: Distance Labels (v0.9, 2026-03-19)
- Meter distance labels (1m, 2m, 5m, 10m, 20m) drawn on distance rings
- Styled with `textDim` color, JetBrains Mono 9px

### GitHub Issues Resolved (22 total, all closed)
- **#1:** Azimuth knob double-click reset to default instead of 0°
- **#2:** User Presets folder missing after build
- **#3:** Trajectories scale inversely with distance from origin
- **#4:** Distance labels added to spatial map rings
- **#5–#8:** Figure-8, Heart, Infinity, Spiral shape corrections
- **#9:** Random trajectory made truly random (was deterministic)
- **#10:** Line trajectory amplitude corrected (0.75 both sides)
- **#11:** Trajectory path visualization realtime redraw
- **#12:** SML button icon added
- **#13:** Figure-8 direction fix
- **#14:** Helix clockwise direction fix
- **#15:** Spiral outward direction fix
- **#16:** Square trajectory speed halved
- **#17:** SML button text/icon scaling to match title height
- **#18:** Circle shape request (closed — Orbit covers this)
- **#19:** Glow trail visual quality regression (deferred)
- **#20:** Line trajectory azimuth rotation
- **#21:** Spiral clockwise direction
- **#22:** Bounce left-to-right default direction

### Files
Active source in `Source/`:
- `PluginProcessor.h` (~33 KB)
- `PluginProcessor.cpp` (~175 KB)
- `PluginEditor.h` (~14 KB)
- `PluginEditor.cpp` (~45 KB)
- `PresetData.h` (shared preset struct, ~2 KB)
- `PresetData.cpp` (60 factory presets, serialization, install function, ~40 KB)
- `Tests/TrajectoryTests.cpp` (Catch2 test suite, 33 tests)

### Output
- 21 output formats unchanged (1 binaural + 1 stereo + 13 surround + 6 Ambisonics)

---

## v1.0 (2026-03-19) — Active Development
**Real-World Testing Release**

Starting point for real-world testing. Carries forward all v0.9 features.
Plugin identity: `Os10` (PLUGIN_CODE).

### New Features
- **SpatialMediaLab 13.1 output format:** Custom 13-speaker room layout (8 ear-level + 4 height + 1 zenith + LFE). Derived from IEM AllRADecoder config. 14-channel discrete bus support added.
- **Wobble modulation reverted to v0.8 design:** Single-oscillator with morphable waveform (sine→triangle→rounded square→irregular). LFO rate tied to delay time for natural pitch coupling.
- **Quad layout corrected:** Speakers now at symmetric 90° spacing (±45°/±135°) instead of 30°/110°.
- **Default preset selection:** Fresh instances start on "Default" preset by name (not alphabetical first).
- **Full OSC control (Issue #32):** All parameters controllable via OSC using hybrid namespace. ADM-OSC `/adm/obj/N/` for position (standard interop). Custom `/osd/obj/N/` for per-tap params (enabled, doppler, pitch, trajectory, speed, direction, input) + position aliases. `/osd/global/` for all global params (delayTime, feedback, filters, dryWet, algorithm, etc.). Send broadcasts all changed values with change-gating. `handleOSCParam()` generic helper for APVTS parameter setting from denormalized OSC values.
- **Dead code cleanup:** Removed unused `trajParam_elevation`/`trajParam_distance` member variables, unused `CMAKE_POLICY_VERSION_MINIMUM` CMake variable. Clarified discarded smoothing calls in binaural render path.
- **Music notation sync icons (Issue #26):** Replaced Unicode text labels (`♪.` / `♪³`) on dotted/triplet sync mode buttons with proper music notation SVG icons — dotted eighth note and beamed eighth note triplet. Added icon-only rendering path to `StyledButton::paintButton()`. Icons tinted by existing accent color logic (gold when active, dim when inactive).
- **Global tap controls (Issue #25):** Collapsible mini-drawer on left edge of spatial map with 6 global offset knobs (AZIM, ELEV, DIST, DOPPLER, PITCH, SPEED). Turning a global knob offsets every enabled tap's matching parameter by the same delta, preserving the spatial arrangement (IEM MultiEncoder-style). Azimuth wraps at ±180°, all others clamp. Value readouts with unit suffixes (°, st, Hz). Silver/ice accent color. Drawer open/close state persisted in DAW session via ValueTree. Preset load resets all offsets to 0. OSC control via 6 new `/osd/global/tap*` addresses (receive with clamping + change-gated send at 30Hz). UI-only knobs — not APVTS parameters, not DAW-automatable. 7 new Catch2 tests (12 assertions).

### Bug Fixes
- **Issue #24 (7.1/7.1.4 rear speakers):** Investigated with diagnostic instrumentation — confirmed plugin computes correct gains. Root cause was Reaper project routing configuration. Closed.
- **Height speaker elevation routing guard:** Added defensive guard preventing 2D VBAP fallback on 3D layouts with height speakers. If VBAP triplets are ever empty at runtime on a height layout, uses 3D nearest-speaker fallback instead of 2D azimuth-only panning (which would incorrectly route signal to height speakers). Guards applied to VBAP, VBIP, and MDAP (4 dispatch points). Diagnostic `jassert` in `activateLayout()` catches height layouts with empty triplets in debug builds. Issue #23 (SML 13.1) closed.
- **Global pitch removal cleanup (Issue #30 follow-up):** Deleted 60 stale on-disk factory preset files that contained ghost `"pitchShift"` keys from the old `install_presets` tool. 9 presets had non-zero values (e.g., Shimmer: 12.0). Files were inert (v1.0 loads from compiled array) but confusing. Removed dead `tools/install_presets.cpp`.
- **Doppler transient on preset load:** `loadPreset()` now resets Doppler tracking arrays (`prevAzimuth`, `prevElevation`, `prevDistance`, `dopplerSemitones`, `smoothedRadialVelocity`) to match the new preset positions, eliminating a spurious pitch artifact on the first audio block after switching presets.
- **HRTF binaural glitch fix (Issue #36):** Three-layer fix for audio clicking/popping during rapid position changes (e.g., Global AZIM/ELEV knob sweeps): (1) per-sample gain interpolation in all 5 rendering paths eliminates block-boundary amplitude discontinuities, (2) non-restarting dual-convolver crossfade ensures equal-power fade always completes, (3) ITD-free HRIR interpolation removes inter-aural time difference before loading into convolver, with ITD applied separately as smoothly-interpolated fractional-sample delay.
- **Metal GPU crash fix (Issue #37):** Fixed Reaper crash on macOS when using Metal GPU rendering by disabling OpenGL context in plugin editor.

### Infrastructure
- **22 output formats** — added SpatialMediaLab 13.1 (was 21 in v0.9)
- **14-channel discrete bus** added to `isBusesLayoutSupported()` for SML 13.1 / 7.1.6
- **162 Catch2 tests** — trajectory shapes (40), surround output (54), OSC receive/send (47), convolver glitch (21). Includes 8 elevation/height isolation tests.
- **State migration v15→v16:** Handles outputFormat parameter shift for SML 13.1 insertion
- **Diagnostic cleanup:** Removed all #24 diagnostic instrumentation from production code

### Files
Active source in `Source/`:
- `PluginProcessor.h` (~51 KB)
- `PluginProcessor.cpp` (~276 KB)
- `PluginEditor.h` (~30 KB)
- `PluginEditor.cpp` (~170 KB)
- `PresetData.h` (shared preset struct, ~3 KB)
- `PresetData.cpp` (70 factory presets, serialization, ~61 KB)
- `Tests/TrajectoryTests.cpp` (40 Catch2 trajectory tests)
- `Tests/SurroundOutputTests.cpp` (54 Catch2 surround output tests)
- `Tests/OscTests.cpp` (47 Catch2 OSC receive/send tests)
- `Tests/ConvolverGlitchTests.cpp` (21 Catch2 convolver glitch tests)

### Output
- 23 output formats (1 binaural + 1 stereo + 15 surround + 6 Ambisonics)

---

### Post-Baseline Updates (April 2026)

After the v1.0 real-world testing release, the following features and fixes were merged:

#### New Features
- **Constant Power panning algorithm (Issue #141):** 8th spatialization algorithm, now the default for surround output. Uses cosine-distance weighting to all speakers within 90 degrees for smooth, natural panning. Added to algorithm dropdown, glossary, controls reference, and output formats documentation.
- **Low-frequency bypass for HRTF profiles (Issue #89):** Automatic bass preservation for HRTF measurements that lack low-frequency content. Crossover at 200 Hz routes bass directly to output, bypassing the HRTF convolution. Also adds great-circle threshold for HRIR update near elevation poles and onset-detection ITD alignment for zero-delay datasets.
- **9.1 Surround output format (Issue #88):** ITU-R BS.2051 System H. 23rd output format. AU bus layout expanded for 10-channel discrete. Reordered before Octaphonic in format list.

#### UI Improvements
- **Ambisonics algorithm dropdown (Issue #143):** Greyed out and dropdown arrow hidden when Ambisonics output is selected (algorithm is always Ambisonics for Ambisonics formats). Full label width used when arrow is hidden.

#### Bug Fixes
- **Wobble modulation fix (Issue #92):** 4-layer tape wobble emulation. LFO rate decoupled from delay time, depth reduced for subtler effect.
- **Transport fade-in (Issue #103):** 5 ms fade-in after transport start prevents click/pop artifacts during scrub and seek operations.
- **Ableton compatibility (Issues #120, #122):** Fixed crash on plugin deletion, re-signed plugin bundles after plist patching for VST3 visibility, reduced automatable parameters to 64 for auto-populate compatibility, fixed automation reset.
- **Direction toggle (Issue #100):** Fixed Bounce (diagonal flip instead of phase offset), Line, and Random trajectory direction behavior.
- **Preset chirp (Issue #99):** Reset phase vocoder pitch shifters during preset crossfade to eliminate chirp artifact.
- **Dry signal attenuation (Issue #97):** Fixed dry signal being attenuated at 0% wet by moving output limiter to wet path only.
- **HRTF profile switch clipping (Issue #90):** Renderer-level crossfade eliminates click during HRTF profile changes.
- **Multi-instance buzzing (Issues #96, #137):** Moved HRTFDatabase into BinauralRenderer; fixed heap corruption in PartitionedConvolver::prepare().
- **Global knob persistence (Issue #95):** Fixed global tap drawer knobs resetting on UI close/reopen.
- **Knob text input (Issue #119):** Added valueFromString parsers for all knobs.
- **Negative zero display (Issue #126):** Fixed -0.00 display on global drawer knobs and DIST knob.
- **Random trail (Issue #110):** Fixed distance scaling and glow to use symmetric proximity-based rendering.
- **VST3 multichannel bus (Issues #111, #122):** Conditional bus layout and negotiation for dynamic channel detection.
- **Algorithm dropdown (Issue #93):** Stopped preset loading from overwriting algorithm selection.
- **Global pitch range (Issue #101):** Fixed Global Pitch knob range from +/-24 st to +/-12 st.

#### Infrastructure
- **Shared FFT cache (Issue #131):** Thread-safe singleton FFT cache for multi-instance vDSP stability. Shared LookAndFeel and visibility throttle for CoreGraphics crash prevention.
- **23 output formats** (was 22 at baseline, added 9.1 Surround)
- **8 spatialization algorithms** (was 7, added Constant Power as default)
