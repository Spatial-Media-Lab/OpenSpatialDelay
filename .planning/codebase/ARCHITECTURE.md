# Architecture

**Analysis Date:** 2026-04-14

## Pattern Overview

**Overall:** Modular spatial audio plugin with a 5-stage signal flow combining per-object delay processing, HRTF binaural rendering, and spatialization algorithms. The architecture separates reusable spatial infrastructure (algorithms, HRTF convolution, speaker layouts) from delay-specific DSP, enabling extraction to future Spatial Media Library plugins.

**Key Characteristics:**
- Per-object delay + pitch processing (up to 12 delays)
- Real-time HRTF binaural convolution with dual-slot crossfade (issue #50)
- 7 spatialization algorithms (VBAP, Ambisonics, DBAP, KNN, MDAP, VBIP, Constant Power)
- Lock-free audio thread reads via double-buffering for output layouts
- Block-boundary position updates (azimuth/elevation/distance) with smooth interpolation
- Per-sample Doppler pitch tracking + Phase Vocoder pitch shifting (±12 semitones)
- 6 HRTF profiles (1 Woodworth ITD+ILD, 5 convolution-based)

## Layers

**Spatial Media Library (Reusable):**
- Purpose: Abstracts spatial audio algorithms and HRTF infrastructure for multi-plugin suite
- Location: `Source/PluginProcessor.h` (lines 31–550: structs, algorithms, HRTFDatabase, BinauralRenderer)
- Contains: `SpatializationAlgorithm` base class + 7 implementations, `HRTFDatabase` (libmysofa wrapper), `PartitionedConvolver`, `BinauralRenderer`, speaker layouts, output format registry
- Depends on: JUCE, libmysofa
- Used by: Plugin processor via `BinauralRenderer` instance + algorithm dispatch

**Plugin-Specific DSP:**
- Purpose: Delay line processing, feedback network, air absorption, trajectory animation, Doppler tracking
- Location: `Source/PluginProcessor.cpp/.h` (lines 603–end), `Source/TrajectoryEngine.cpp/.h`, `Source/DopplerVelocity.cpp/.h`, `Source/FilterBank.cpp/.h`, `Source/PhaseVocoderPitchShifter.h`
- Contains: Circular delay buffers, feedback convolver, output mixing, parameter caching, trajectory state
- Depends on: Spatial library, JUCE audio processing
- Used by: `processBlock()` for sample-by-sample and block-rate updates

**Editor Layer:**
- Purpose: Real-time parameter binding, spatial map visualization, trajectory preview, OSC configuration
- Location: `Source/PluginEditor.cpp/.h` (31,715 bytes)
- Contains: `SpatialMapComponent` (2D top-down view), slider components, parameter listeners, trajectory visualization
- Depends on: PluginProcessor (state access), JUCE GUI
- Used by: Host DAW for UI rendering and parameter interaction

**Testing Harness:**
- Purpose: Unit and integration tests for trajectory math, HRTF rendering, convolvers, parameter counts
- Location: `Tests/` (7 test files, 162+ tests via Catch2)
- Contains: Signal path tests, glitch detection, OSC tests, surround output validation, pre-release verification
- Depends on: PluginProcessor (shared source compilation), Catch2, TestUtilities.h helpers
- Used by: CI/CD and manual verification (CMake target `OpenSpatialDelayTests`)

## Data Flow

**Block-Level Signal Path:**

1. **Input**: DAW feeds stereo audio buffer to `processBlock()`
2. **Trajectory Update** (audio thread):
   - Read animated azimuth/elevation/distance from `TrajectoryEngine` (updated by timer thread at ~60Hz)
   - Detect position change vs. previous block (within 1° tolerance)
3. **Per-Object Delay Processing** (sample-by-sample):
   - Read input sample at per-object delay offset (circular delay line)
   - Apply Doppler pitch shift via Phase Vocoder (if enabled)
   - Apply output gain, air absorption filter, feedback convolver output
   - Accumulate into per-object mono buffer
4. **HRTF Convolution** (block-level):
   - Update source HRIRs if position changed: lookup via `BinauralRenderer::updateSourceHRIR()`
   - Call `renderDirectBinauralHRTF()`: routes per-object buffers through convolver pairs (L/R)
   - Apply ITD smoothing (fractional-sample delay per ear)
   - Produce binaural L/R output
5. **Output Routing**:
   - Algorithm dispatch: compute speaker gains for each active source (VBAP/Ambisonics/etc.)
   - Render to output format: binaural, stereo variant, surround, or Ambisonics
   - Dry path: bypass entire plugin, apply compensating 2048-sample delay to match PDC
   - Dry/wet mix: equal-power crossfade (cos/sin) at output stage
   - Write to DAW buffer

**State Management:**
- **Parameter Tree** (`apvts`): 143 params (64 automatable in Ableton, all in other DAWs)
- **Per-Object State**: Cached parameter pointers (stable for APVTS lifetime, no string lookups in audio thread)
- **Output Layout**: Double-buffered (active + pending) for lock-free format changes
- **Phase Vocoder**: Ring buffer + FFT workspace (allocated at prepare-time)
- **HRTF Convolution**: Dual-slot architecture — old IR in slot 0, new IR loads into slot 1 during crossfade

## Key Abstractions

**SpatializationAlgorithm:**
- Purpose: Abstract panning algorithm interface with dual methods (speaker-domain + binaural-direct)
- Location: `Source/PluginProcessor.h` lines 156–184
- Examples: `DirectBinauralAlgorithm`, `VBAPAlgorithm`, `AmbisonicsAlgorithm`, `VBIPAlgorithm`, `KNNAlgorithm`, `DBAPAlgorithm`, `MDAPAlgorithm`, `ConstantPowerAlgorithm`
- Pattern: Virtual method dispatch + compile-time algorithm registry; compute gains from source position

**BinauralRenderer:**
- Purpose: Manages HRTF database per renderer (issue #96: eliminates shared-state race), coordinates source convolver updates and binaural output mixing
- Location: `Source/PluginProcessor.h` lines 434–537
- Pattern: Holds HRTFDatabase instance, pre-computes normalization gain per profile, maintains dual convolvers per source (L/R), updates HRIRs on block boundaries when position changes

**PartitionedConvolver:**
- Purpose: Real-time FFT overlap-save convolution with dual-slot crossfade
- Location: `Source/PluginProcessor.h` lines 349–428
- Pattern: Two independent convolution slots (ConvSlot) run during crossfades to avoid boundary discontinuities (issue #50); per-sample gain interpolation (cos/sin) for glitch-free transitions

**TrajectoryEngine:**
- Purpose: Compute animated spatial positions from base position + shape (circle, figure-8, random, lissajous, spiral)
- Location: `Source/TrajectoryEngine.h` lines 29–115
- Pattern: Per-object state machines; `tick()` advances phase at 60Hz, `computeTrajectory()` is a pure function for editor preview
- Uses: Azimuth wrapping helpers (`wrapAzimuth`, `unwrapAzimuthDelta`) for discontinuity detection

**DopplerVelocity:**
- Purpose: Per-block velocity tracking and pitch shift computation from spatial movement
- Location: `Source/DopplerVelocity.h` lines 11–66
- Pattern: Track previous position, compute radial velocity to virtual ear offset, smooth with two-pole EMA (issue #76), clamp to ±12 semitones

**FilterBank:**
- Purpose: Centralized IIR filter state management for feedback path, per-tap output, and per-object air absorption
- Location: `Source/FilterBank.h` lines 11–86
- Pattern: EMA-smoothed coefficient updates; bypass logic; separate feedback convolver output stream

**HRTFDatabase:**
- Purpose: SOFA file loading via libmysofa, HRIR interpolation, onset detection, minimum-phase conversion
- Location: `Source/PluginProcessor.h` lines 285–343
- Pattern: Wraps libmysofa; provides both raw and ITD-aligned HRIR pairs; static utilities for minimum-phase and low-frequency correction

## Entry Points

**PluginProcessor::prepareToPlay():**
- Location: `Source/PluginProcessor.cpp` (lines ~1500–1600)
- Triggers: Host calls when changing sample rate or block size
- Responsibilities: Allocate DSP buffers, prepare phase vocoder, prepare HRTF convolver slots, cache parameter pointers, start OSC listener, start 60Hz timer for trajectory updates

**PluginProcessor::processBlock():**
- Location: `Source/PluginProcessor.cpp` lines 4293–5050
- Triggers: Every audio block from DAW
- Responsibilities:
  - Update trajectory positions (audio thread tick)
  - Loop over 12 objects: delay processing, Doppler tracking, feedback convolution
  - Update source HRIRs if position changed
  - Call render method (5 paths: Direct Binaural HRTF, Binaural Simple, Surround, Ambisonics, Stereo)
  - Apply dry/wet mix and output gain
  - Write to output buffer

**RenderDirectBinauralHRTF():**
- Location: `Source/PluginProcessor.cpp` lines 4830–5040
- Purpose: 3-pass HRTF rendering
  - Pass 1: Per-sample delay engine, accumulate into per-source mono buffers
  - Pass 2: Block-level HRTF convolution via `BinauralRenderer::renderSourceBuffers()`
  - Pass 3: Per-sample dry/wet mix, apply output gain

**PluginEditor::resized():**
- Location: `Source/PluginEditor.cpp`
- Triggers: Window resize
- Responsibilities: Layout sliders, spatial map, spectrum/trajectory visualizers

**Timer Callback (60Hz):**
- Triggers: Every ~16ms via JUCE Timer
- Location: `Source/PluginProcessor.cpp` (timerCallback method)
- Responsibilities: Advance `TrajectoryEngine::tick()`, update `SpatialMapComponent` with new positions, trigger HRTF profile load if pending

## Error Handling

**Strategy:** Defensive programming with fallback modes; no exceptions thrown in audio thread

**Patterns:**
- **HRTF Load Failure**: If SOFA file fails to load, remain in Simple (Woodworth) mode; return from `setProfile()` early with no state change
- **NaN Recovery**: FilterBank includes `resetFeedback()` to clear stale IIR state without disrupting delay lines
- **Buffer Overrun**: Circular delay lines check write position modulo buffer size; pre-allocated with max delay overhead
- **OSC Parse Error**: OSC listener silently ignores malformed messages; no throw in OSC callback
- **Parameter Value Clamp**: JUCE RangedAudioParameter handles all bounds checks; values guaranteed in range

## Cross-Cutting Concerns

**Logging:** Sparse; only console output for version/profile loads. No file logging. Issues reported via assertions in debug builds and graceful fallback in release.

**Validation:** Parameter ranges enforced by RangedAudioParameter; spatial positions validated in separate getters (objectIndex bounds check, azimuth wrapping); HRTF interpolation clamps to dataset bounds.

**Authentication:** N/A (local plugin)

**Synchronization:**
- **Audio/Timer Thread**: Position updates via atomic bools, final positions read with `memory_order_relaxed` (temporal coherence OK within 16ms)
- **Output Format Changes**: Double-buffered layout with `activeLayoutIndex` atomic; timer thread updates pending, audio thread reads active
- **Parameter Changes**: APVTS handles atomicity; gesture boundaries captured by undo manager

**Pitch Processing:** Phase Vocoder runs at 48 kHz reference; Doppler combines with pitch shifter range (+/−12 semitones + Doppler ≈ ±1–2 additional semitones), clamped to ±12 total.

**Dry Path Compensation:** Stereo circular delay buffers (`dryDelayLineL/R`) pre-fill compensation buffer at block start; delay line size matches phase vocoder latency (2048 samples @ 48 kHz). If FFT size changes, must update both paths or PDC breaks at all dry/wet ratios.

---

*Architecture analysis: 2026-04-14*
