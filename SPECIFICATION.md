# OpenSpatialDelay — Project Specification

**Document Version:** 2.0
**Date:** 2026-04-21 (consolidated against v1.0.0 shipped code)
**Author:** Andrew Rahman (Product Vision) & Claude (Technical Architecture)

> This document is the technical specification for OpenSpatialDelay v1.0.0. It was consolidated on 2026-04-21 to reflect the shipped codebase. Features deferred post-v1.0 are tracked as GitHub issues — see §12 — and have been removed from this document.

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Product Vision](#2-product-vision)
3. [Technical Architecture](#3-technical-architecture)
4. [Feature Specification (v1.0.0)](#4-feature-specification)
5. [Signal Flow](#5-signal-flow)
6. [Spatialization Engine](#6-spatialization-engine)
7. [HRTF Implementation](#7-hrtf-implementation)
8. [UI Design](#8-ui-design)
9. [Preset System](#9-preset-system)
10. [ADM-OSC Integration](#10-adm-osc-integration)
11. [Build, CI/CD & Distribution](#11-build-cicd--distribution)
12. [Release History & Post-v1.0 Backlog](#12-release-history--post-v10-backlog)
13. [Appendices](#13-appendices)

---

## 1. Executive Summary

**OpenSpatialDelay** is an open-source spatial delay effect where each delay tap is positioned in 3D space and rendered to binaural headphones, Ambisonics, or discrete surround speakers (Quad through 9.1.6 Atmos). The output format is determined by the DAW's track I/O bus, independent from the spatialization algorithm. The plugin targets music producers, live sound engineers, post-production professionals, and installation/research users.

**Key differentiators:**
- User-selectable spatialization algorithm — 7 options (Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP)
- 6 binauralization options: 5 curated HRTF profiles from academically validated, freely licensed databases + 1 CPU-lite option (Woodworth ITD+ILD approximation, not an HRTF)
- Up to 12 manually placeable delay taps, each with its own pitch shift, Doppler, and animated trajectory
- 13 trajectory shapes for per-tap animation (Orbit, Figure-8, Spiral, Helix, Heart, Bounce, etc.), plus "None" for taps that stay fixed
- 23 output formats (2 stereo, 15 surround, 6 Ambisonics orders)
- ADM-OSC receive + send for interoperability with spatial audio renderers, plus an `/osd/` namespace for full parameter control
- Stereo input with per-tap L / R / L+R channel selection
- Open-source, licensed under GPL-3.0
- macOS (Apple Silicon) + Windows (x64), VST3 + AU

**Release status:** v1.0.0 shipped 2026-04-17. See §12 for history and the post-v1.0 backlog (all tracked as GitHub issues).

---

## 2. Product Vision

### 2.1 One-Sentence Pitch
A spatial delay plugin where each repeat occurs at a different, definable position in 3D space, rendered to headphones via binaural HRTF processing.

### 2.2 Target Users
| User Type | Primary Use Case |
|-----------|-----------------|
| Music Producers | Creative spatial effects in DAW mixing |
| Live Sound Engineers | Real-time spatial delay for immersive live performance |
| Post-Production / Film Audio | Spatial sound design for immersive cinema formats |
| Installation Artists / Researchers | Experimental spatial audio in custom configurations |

### 2.3 Reference Products
- **Sound Particles inDelay** — Primary feature and workflow reference. A spatial delay with per-tap positioning, trajectory presets, and multi-format output. Supports Channels, Taps, and Particles modes with up to 100 delays. Features include air simulation, a "Craziness" parameter, and 30+ output formats. UI features a 2D top-down spatial visualizer with colored tap indicators above a horizontal time editor.
- **SnappySnap by Electric Smudge** — UI design reference. Dark void-black aesthetic with neon cyan/purple accents, glassmorphism panels, circular snapshot layouts, and smooth animations. Demonstrates how a deeply technical plugin can feel modern and visually striking.

### 2.4 Platform Targets
| Platform | Architecture | Plugin Formats |
|----------|-------------|----------------|
| macOS | Apple Silicon (arm64) | VST3, AU |
| Windows | x64 | VST3 |

### 2.5 Brand Identity
- **Suite name:** Spatial Media Library
- **First plugin:** OpenSpatialDelay
- **License:** GPL-3.0
- **Distribution:** GitHub (initial), potential website sales later

---

## 3. Technical Architecture

### 3.1 Framework & Language
- **Framework:** JUCE 8.0.3 (C++17)
- **Build system:** CMake 3.22+
- **Spatial audio HRTF parsing:** libmysofa v1.3.2 (BSD-3-Clause — compatible with GPL-3.0 distribution)
- **OSC transport:** JUCE `juce_osc` module (built-in `OSCSender` / `OSCReceiver` over UDP) — no external dependencies required

### 3.2 Module Architecture

The v1.0 codebase is a single-plugin flat layout. All DSP, UI, and integration code lives in `Source/`. There is no `SpatialMediaLab/Core/` shared library in v1.0 — that remains the long-term ambition for the plugin suite and is tracked in issue [#221](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/221).

```
OpenSpatialDelay/
|-- Source/                        # Plugin implementation (flat)
|   |-- PluginProcessor.h / .cpp       # ~175KB — DSP engine, APVTS, spatialization, HRTF,
|   |                                  # delay taps, feedback, OSC, preset state
|   |-- PluginEditor.h / .cpp          # UI, Colours_OSD palette, spatial map, controls
|   |-- PhaseVocoderPitchShifter.h     # STFT pitch shifter (2048 FFT, 4x overlap, ±12 st)
|   |-- TrajectoryEngine.h / .cpp      # 13 trajectory shapes for animated taps (+ "None" disabled state)
|   |-- FilterBank.h / .cpp            # Feedback-path LP + HP biquads with resonance
|   |-- DopplerVelocity.h / .cpp       # Per-tap Doppler pitch from position velocity
|   |-- PresetData.h / .cpp            # 70 factory presets (C++ static structs)
|   |-- SharedFFTCache.h               # Global vDSP FFT setup reuse (multi-instance)
|-- HRTF/                          # 5 embedded SOFA files
|   |-- bernschuetz_ku100.sofa             # CC BY 3.0
|   |-- cipic_subject_003.sofa             # Public Domain
|   |-- hutubs_pp2.sofa                    # CC BY 4.0
|   |-- mit_kemar_large_pinna.sofa         # MIT license
|   `-- sadie_d2_ku100.sofa                # Apache 2.0
|-- fonts/                         # Embedded UI fonts
|   |-- DM_Sans-*.ttf                      # SIL OFL 1.1 (Regular, Medium, SemiBold, Bold)
|   |-- JetBrains_Mono-*.ttf               # SIL OFL 1.1 (Regular, Medium, Bold)
|   `-- Roboto-Medium.ttf                  # Apache 2.0
|-- Tests/                         # Catch2 suite
|   |-- DSPUnitTests.cpp
|   |-- IntegrationTests.cpp
|   |-- SurroundOutputTests.cpp
|   |-- ConvolverGlitchTests.cpp
|   |-- OscTests.cpp
|   |-- ParameterCountTests.cpp
|   |-- PreReleaseTests.cpp
|   `-- TrajectoryTests.cpp
|-- JUCE/                          # JUCE 8.0.3 (git submodule)
|-- scripts/                       # build_version.sh, install helpers, video tooling
|-- docs/                          # User manual PDF, legal notices, version history
`-- CMakeLists.txt                 # juce_add_plugin target; PLUGIN_CODE Os10

### 3.3 Core Design Principles

1. **Lock-free audio processing:** The `processBlock` callback never allocates, acquires locks, or performs I/O. Parameter changes flow via `std::atomic` loads from APVTS or lock-free FIFOs.
2. **SIMD optimization:** JUCE's `FloatVectorOperations` (ARM NEON on Apple Silicon, SSE/AVX on x64) handles bulk channel operations — gain scaling, buffer copies, accumulation.
3. **Algorithm abstraction:** All 7 spatialization algorithms inherit from a common interface (`computeGains(SourcePosition, SpeakerLayout) → float[]`), enabling runtime algorithm switching.
4. **Shared FFT setup:** Multi-instance plugin scenarios share a global vDSP FFT setup via `SharedFFTCache` to avoid per-instance allocation spikes.

### 3.4 I/O Architecture

**Core design principle:** The output format is **independent from the spatialization algorithm**. The algorithm computes object-position-to-speaker-gains for whatever speaker layout is active. The output format is determined solely by the DAW's track I/O bus configuration, not by user selection.

#### 3.4.1 Input Format

| Format | Channels | Behavior |
|--------|----------|----------|
| Mono | 1 | Direct pass-through to the delay engine |
| Stereo | 2 | Independent L and R delay lines; each tap chooses L, R, or L+R via the per-tap `inputChannel` parameter |

#### 3.4.2 Output Format

The plugin ships **23 output formats** (`NUM_OUTPUT_FORMATS` in `Source/PluginProcessor.h`). It auto-detects the active format from the host's track channel count and selects the matching entry from the `outputFormatRegistry`. All speaker positions follow ITU-R BS.775 (ear-level) and BS.2051 (height) standards. Convention: 0° = front, positive azimuth = left.

**Stereo (2 formats)**

| Output Format | Channels | Render Path |
|---|---|---|
| Binaural | 2 | HRTF convolution for each tap |
| Stereo (encoded) | 2 | One of 5 mic-simulation modes (Equal Power, Stereo VBAP, XY, MS, Blumlein) selected via the algorithm parameter |

**Surround (15 formats)**

| Output Format | Channels | LFE | Height |
|---|---|---|---|
| Quadraphonic | 4 | No | No |
| 5.0 Surround | 5 | No | No |
| 5.1 Surround | 6 | Yes | No |
| 7.0 Surround | 7 | No | No |
| 7.1 Surround | 8 | Yes | No |
| 9.1 Surround (ITU-R BS.2051 System H — ear-level) | 10 | Yes | No |
| Octaphonic | 8 | No | No |
| 5.1.2 | 8 | Yes | 2 |
| 5.1.4 | 10 | Yes | 4 |
| 7.1.2 | 10 | Yes | 2 |
| 7.1.4 (Atmos bed) | 12 | Yes | 4 |
| 7.1.6 | 14 | Yes | 6 |
| 9.1.4 | 14 | Yes | 4 |
| 9.1.6 (Atmos full) | 16 | Yes | 6 |
| SML 13.1 (Spatial Media Lab Multi-Use Room) | 14 | Yes | 4 |

**Ambisonics (6 formats, AmbiX ACN/SN3D)**

| Output Format | Channels | Order |
|---|---|---|
| Ambisonics FOA | 4 | 1st |
| Ambisonics SOA | 9 | 2nd |
| Ambisonics HOA | 16 | 3rd |
| Ambisonics 4OA | 25 | 4th |
| Ambisonics 5OA | 36 | 5th |
| Ambisonics 6OA | 49 | 6th |

**Algorithm × Output Format compatibility:**

| Algorithm | Binaural (2ch) | Discrete Surround (4–16ch) | Ambisonics |
|-----------|----------------|----------------------------|------------|
| VBAP | Yes (via virtual speaker layout) | Yes (2D pair-wise, 3D triplet-wise for height) | Via SH encode |
| VBIP | Yes (via virtual speakers) | Yes (intensity-weighted VBAP) | Via SH encode |
| KNN | Yes (via virtual speakers) | Yes (k-nearest-neighbor interpolation) | Via SH encode |
| Ambisonics (HOA) | Yes (SH → binaural decode) | Yes (SH → Tikhonov-regularized speaker decode) | Native |
| DBAP | Yes (via virtual speakers) | Yes (no sweet-spot assumption) | Via SH encode |
| MDAP | Yes (via virtual speakers) | Yes (multi-direction amplitude panning) | Via SH encode |
| ConstantPower | Yes (stereo path) | Limited (pair-wise constant-power between two nearest speakers) | Via SH encode |

A dedicated Direct Binaural rendering path (`renderDirectBinauralHRTF`) is used for stereo/binaural output regardless of algorithm selection — each tap is convolved with the HRTF for its position directly, without going through a virtual speaker intermediary.

**Rendering paths:**
- **Binaural (2ch, default for stereo bus):** Each active tap is convolved with the selected HRTF profile for its azimuth/elevation via a partitioned FFT convolver. Dry signal follows a stereo path with 2048-sample PDC to match the phase-vocoder latency.
- **Stereo encoded (2ch, opt-in):** Taps are routed through one of 5 stereo encoding modes (Equal Power, Stereo VBAP, XY, MS, Blumlein) when the user selects a stereo mode instead of Binaural.
- **Discrete Surround (4–16ch):** Algorithms compute gains directly for the physical speaker layout. LFE is derived from a mono sum of spatialized objects (120 Hz 2nd-order Butterworth LP, −10 dB). Dry signal is routed to L/R only.
- **Ambisonics (4–49ch):** Tap positions are encoded into spherical harmonics up to 6th order. No speaker decode occurs inside the plugin — the Ambisonics bus is the output.

#### 3.4.3 Bus Layout Negotiation

The plugin queries the host via JUCE's `BusesLayout` system:
- **Default configuration:** Mono input → Stereo output
- **Supported inputs:** Mono, Stereo
- **Supported outputs:** Stereo, Quadraphonic, 5.1, 7.1, 7.1.4, 9.1.6
- **Internal bus:** Up to 16 discrete output channels. Actual channel count determined by the track the plugin is placed on.
- **Detection flow:** `prepareToPlay()` calls `detectOutputFormat(getTotalNumOutputChannels())` → `activateLayout(format)`, which configures the `SpeakerLayout` struct, computes the Ambisonics decode matrix (Tikhonov-regularized pseudo-inverse), and builds VBAP triplets for height layouts.
- **UI adaptation:** The editor polls the active output format each timer tick. On stereo bus (binaural), the HRTF profile selector is visible and the output-format dropdown exposes the stereo-encoding modes (Equal Power, Stereo VBAP, XY, MS, Blumlein). On surround or Ambisonics buses, the HRTF selector is hidden and the algorithm selector drives all output channels.

---

## 4. Feature Specification

Ranges and defaults are sourced from `Source/PluginProcessor.cpp::createParameterLayout`. The plugin exposes **143 APVTS parameters** total (23 global + 10 per-tap × 12 taps) — verified by `Tests/ParameterCountTests.cpp`.

### 4.1 Global Parameters (23)

**Delay**

| Parameter | Range | Default | Notes |
|---|---|---|---|
| Delay Time | 1 – 2000 ms | 500 ms | Master free-run delay time. Overridden by sync division when tempo sync is on. |
| Tempo Sync | On/Off | Off | When on, delay time snaps to a note division. |
| Note Division | 0.5 – 32 sixteenths (continuous float) | 4.0 (1/4) | Active only when Tempo Sync is on. |
| Sync Mode | Straight / Dotted / Triplet | Straight | Modifier for note division. |
| Feedback | 0 – 100% | 30% | Feedback into the pre-spatial mono feedback path. |
| Dry/Wet | 0 – 100% | 50% | Equal-power (cos/sin) crossfade at the post-render stage. |

**Feedback filter**

| Parameter | Range | Default | Notes |
|---|---|---|---|
| Filter On | On/Off | On | Master bypass for the feedback-path filter bank. |
| Filter HP Frequency | 20 – 5000 Hz | 50 Hz | High-pass biquad on the feedback path. |
| Filter HP Q | 0.1 – 4.0 | 0.707 | HP resonance. |
| Filter LP Frequency | 200 – 20000 Hz | 5000 Hz | Low-pass biquad on the feedback path. |
| Filter LP Q | 0.1 – 4.0 | 0.707 | LP resonance. |

**Modulation & air**

| Parameter | Range | Default | Notes |
|---|---|---|---|
| Wobble On | On/Off | Off | Master bypass for delay-time wobble modulation. |
| Wobble Amount | 0 – 100% | 0% | Depth of wobble on delay time. |
| Wobble Morph | 0 – 100% | 0% | Morphs the LFO waveform from sine (0%) to square (100%). |
| Air Absorption | On/Off | On | Enables distance-driven high-frequency attenuation. |

**I/O gain**

| Parameter | Range | Default | Notes |
|---|---|---|---|
| Input Gain | −100 dB – +40 dB | 0 dB | Applied before the delay engine. |
| Output Gain | −100 dB – +12 dB | 0 dB | Applied after dry/wet mix. |

**Global tap offsets** (additive on top of each enabled tap's own value)

| Parameter | Range | Default | Notes |
|---|---|---|---|
| Global Tap Azimuth | −180° – +180° | 0° | Rotates all taps by a common offset. |
| Global Tap Elevation | −90° – +90° | 0° | Tilts all taps by a common offset. |
| Global Tap Distance | 0.0 – 1.0 | 0.0 | Pushes all taps radially outward. |
| Global Tap Pitch | −12 st – +12 st | 0 st | Shifts every tap's pitch by a common offset. |
| Global Tap Doppler | 0.0 – 1.0 | 0.0 | Scales every tap's Doppler amount. |
| Global Tap Speed | 0.0 – 1.0 | 0.0 | Scales every tap's trajectory speed. |

### 4.2 Per-Tap Parameters (10 × 12 taps = 120)

The plugin has 12 delay taps. Each tap has 10 APVTS parameters:

| Parameter | Range | Notes |
|---|---|---|
| On | On/Off | Whether the tap contributes to the output. |
| Azimuth | −180° – +180° | 0° = front, positive = left. |
| Elevation | −90° – +90° | Polar elevation. |
| Distance | 0.0 – 1.0 | Normalized; drives distance attenuation + air absorption. |
| Doppler Amount | 0.0 – 1.0 | Per-tap Doppler pitch from position velocity. |
| Pitch Shift | −12 st – +12 st | Per-tap phase-vocoder pitch shift, additive with Global Tap Pitch. |
| Trajectory Shape | None (disabled) + 13 shapes: Bounce, Circle, Cross, Figure-8, Heart, Helix, Infinity, Line, Orbit, Random, Spiral, Square, Triangle | "None" is the off state; selecting it disables trajectory animation for the tap. |
| Trajectory Speed | 0.0 – 1.0 | Rate (multiplied by Global Tap Speed). |
| Trajectory Direction | Forward / Reverse | Trajectory animation direction. |
| Input Channel | L+R / L / R | Which input channel feeds this tap's delay line. |

### 4.3 Spatial Rendering

| Feature | Specification |
|---|---|
| Output formats | 23 total: 2 stereo (Binaural + encoded), 15 surround (Quad through 9.1.6 Atmos including the SML 13.1 layout), 6 Ambisonics (1st through 6th order). See §3.4.2. |
| Spatialization algorithm | User-selectable: Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP (7 options). |
| Binauralization options | 6 total — 5 HRTF profiles (Immersive, Natural, Precise, Spatial, Studio Reference) plus 1 CPU-lite "Simple (Low CPU)" Woodworth ITD+ILD option (not an HRTF). |
| HRTF convolution | Partitioned FFT convolution with spectral-envelope EMA smoothing (magnitude + phase per bin) to prevent phase-misalignment comb filtering. |
| Distance attenuation | Inverse-distance law applied automatically. |
| Air absorption | High-frequency roll-off proportional to distance; toggleable via the Air Absorption global. |
| LFE generation | 120 Hz 2nd-order Butterworth LP, −10 dB, derived from a mono sum of spatialized signals; applied on surround formats that carry LFE. |

### 4.4 DAW Integration

| Feature | Specification |
|---|---|
| Plugin formats | VST3 (macOS + Windows), AU (macOS only) |
| Sample rates | Whatever the host reports via `prepareToPlay`; no hardcoded rates. Verified in tests at 44.1, 48, 88.2, 96 kHz. |
| Buffer sizes | 64 – 4096 samples |
| Parameter automation | All 143 APVTS parameters are host-automatable. A subset of 64 is marked as automation-discoverable for Ableton auto-populate. |
| State save/restore | Full APVTS state + preset metadata. |
| Tempo sync | Reads BPM and transport position from host. |

---

## 5. Signal Flow

```
                          +------------------+
                          |   MONO INPUT     |
                          |   (from DAW)     |
                          +--------+---------+
                                   |
                                   v
                          +--------+---------+
                          |   Input Gain     |
                          +--------+---------+
                                   |
                                   v
     +-----------+        +--------+---------+
     |  FEEDBACK |------->| Soft Clip & Mix  |
     |  SAMPLE   |  *fb   | (input + fb)     |
     +-----------+        +--------+---------+
          ^                        |
          |                        v
          |               +--------+---------+
          |               | WRITE to Delay   |
          |               |    Buffer        |
          |               +--------+---------+
          |                        |
          |      +-----------------+-----------------+
          |      |                 |                 |
          |      v                 v                 v
          |  +---+---+        +---+---+        +---+---+
          |  | READ  |        | READ  |        | READ  |  (up to 12)
          |  | 1*Base|        | 2*Base|        | N*Base|
          |  +---+---+        +---+---+        +---+---+
          |      |                 |                 |
          |      v                 v                 v
          |  +---+---+        +---+---+        +---+---+
          |  | Pitch |        | Pitch |        | Pitch |
          |  | P_1   |        | P_2   |        | P_N   |  (per-tap value + global offset)
          |  +---+---+        +---+---+        +---+---+
          |      |                 |                 |
          |      |  (each tap's mono signal + 3D position)
          |      |                 |                 |
          |      v                 v                 v
          |  +---+-----------------+-----------------+---+
          |  |     SPATIALIZATION ALGORITHM (per tap)     |
          |  |  User-selectable (7 options): Ambisonics, |
          |  |  ConstantPower, DBAP, KNN, MDAP, VBAP,    |
          |  |  VBIP (see §6.2)                           |
          |  |                                            |
          |  |  Input:  tap mono + position (az, el, dst) |
          |  |  Output: speaker gains[] or SH coefficients|
          |  +---+-----------------+-----------------+---+
          |      |                 |                 |
          |      v                 v                 v
          |  +---+-----------------+-----------------+---+
          |  |        OUTPUT RENDERER (by bus)           |
          |  |  Binaural (2ch): per-tap HRTF convolution |
          |  |  Stereo encoded: 5 mic-sim modes          |
          |  |  Surround: gains → physical speakers + LFE|
          |  |  Ambisonics: SH coefficients → N channels |
          |  +---+-----------------------------+---------+
          |      |                             |
          |      v                             v
          |  +---+---+                    +----+--+
          |  |Sum L  |                    |Sum R  |
          |  +---+---+                    +---+---+
          |      |           DRY               |
          |      |           PATH              |
          |      v            |                v
          |  +---+----+  +---+---+     +------+---+
          |  | Wet L  |  |Dry*   |     | Wet R    |
          |  +---+----+  |(1-dw) |     +-----+----+
          |      |       +---+---+           |
          |      v           v               v
          |  +---+-----------+---------------+---+
          |  |        DRY/WET MIX → Output Gain  |
          |  +---+---------------------------+---+
          |      |                           |
          |      v                           v
          |  +---+---+                  +----+--+
          |  |OUT L  |                  |OUT R  |
          |  +-------+                  +-------+
          |
          |  ======= FEEDBACK PATH (MONO, PRE-SPATIAL) =========
          |
          |  +---+---+
          +--| READ  |<-- Read from END of sequence: N * baseDelay
             | N*Base|    (N = highest enabled tap index + 1)
             +---+---+
                 |
                 v
             +---+---+
             | Pitch |    Feedback-path pitch shifter
             | P_fb  |
             +---+---+
                 |
                 v
             +---+---+
             |  LP   |    Feedback tone filters
             |  HP   |
             +---+---+
                 |
                 v
             +---+------+
             |Soft Clip  |   Gain makeup: 1.0 + (fb² × 0.2)
             |+ Makeup   |   Musical self-oscillation
             +---+-------+
                 |
                 v
             +-----------+
             |  FEEDBACK |---> (back to top, mixed with next input)
             |  SAMPLE   |
             +-----------+
```

### Spatial Ping-Pong Concept

This delay works like a traditional ping-pong delay, but instead of bouncing between just Left and Right, the signal "travels" through up to 12 spatial positions in 3D space. Each tap is a sequential station along one shared delay chain. The signal enters the delay line, appears at Tap 1's spatial position after 1× base delay, at Tap 2's position after 2× base delay, and so on. When it reaches the last enabled tap, the feedback path reads that signal, filters and pitch-shifts it, and feeds it back to the delay input — completing the round trip so the whole spatial sequence repeats. Each tap applies its own per-tap pitch value (plus the Global Tap Pitch offset); pitch is **not** multiplied by tap index.

### Two-Stage Spatialization Architecture

The rendering of each tap to the output is a **two-stage process** designed for modularity across the entire Spatial Media Library plugin suite:

**Stage A — Spatialization Algorithm (user-selectable, 7 options):**
Each tap's mono signal and 3D position are fed into the selected algorithm, which computes gain coefficients for the active speaker layout. See §6.2 for the full list.

**Stage B — Output Renderer (output-format-dependent):**
Determined by the DAW's track I/O bus (see §3.4.2):
- **Stereo / Binaural (2ch):** `renderDirectBinauralHRTF` convolves each tap with the HRTF for its position. This path bypasses the virtual speaker array entirely. Stereo-encoded modes (Equal Power, Stereo VBAP, XY, MS, Blumlein) are an alternative when the user selects one instead of Binaural.
- **Discrete Surround (4–16ch):** Algorithm gains are routed to the physical output channels defined by the active `SpeakerLayout`. LFE is derived from a mono sum of spatialized signals (120 Hz LP, −10 dB). Dry signal is routed to L/R only.
- **Ambisonics (4–49ch):** Tap positions are encoded into 1st–6th order spherical harmonics with no speaker decode inside the plugin.

The algorithm stage is reusable across all output formats. Binaural output always uses direct per-tap HRTF convolution — it is not a Stage A algorithm choice.

### Feedback Path

**Critical: the feedback path is MONO and pre-spatial.** Spatialization is applied only in the output rendering path. The feedback loop stays in the mono domain, reading from a single point at the end of the tap chain. This ensures clean, stable feedback without spatial rendering artifacts accumulating in the loop.

### Implementation Notes (v1.0.0)

- **Binaural path:** `renderDirectBinauralHRTF()` convolves each tap with the selected HRTF profile for its position via partitioned FFT convolution. The 16-speaker virtual layout is used only for surround algorithm gains, not for binaural routing.
- **All 7 algorithms shipped:** Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP — all concrete classes wired in `PluginProcessor.cpp`.
- **Dry path latency compensation:** Dry signal flows through a stereo `dryDelayLineL/R` sized to `PhaseVocoderPitchShifter::kFFTSize` (2048) samples, matching the phase vocoder's report to `setLatencySamples`. Changing FFT size or overlap requires updating both paths together.
- **Shared FFT:** Global vDSP FFT setup is reused across instances via `SharedFFTCache` to avoid multi-instance allocation spikes.

---

## 6. Spatialization Engine

### 6.1 Algorithm Abstraction

All spatialization algorithms implement a common interface. The algorithm operates on whatever `SpeakerLayout` is active — a 16-speaker virtual layout for binaural rendering, or the physical speaker layout for discrete surround output (see Section 3.4.2). The algorithm does not need to know the output format.

```cpp
class SpatializationAlgorithm {
public:
    virtual ~SpatializationAlgorithm() = default;

    // Compute per-speaker gain coefficients for a source at the given position
    virtual void computeGains(
        const SourcePosition& source,
        const SpeakerLayout& layout,
        float* outputGains,           // array of size layout.numSpeakers
        int numSpeakers
    ) const = 0;

    virtual juce::String getName() const = 0;
};
```

### 6.2 Algorithms Shipped in v1.0.0

All seven algorithms below are concrete classes in `Source/PluginProcessor.cpp` and wired into the `algorithms[]` dispatch table.

| Algorithm | Academic Reference | Notes |
|-----------|--------------------|-------|
| **VBAP** | Pulkki, "Virtual Sound Source Positioning Using Vector Base Amplitude Panning," JAES 1997 | Pair-wise (2D) for flat layouts, triplet-wise (3D) for height layouts. |
| **VBIP** | Intensity-weighted variant of VBAP | Squared gains, renormalized. Better energy preservation for off-centre sources. |
| **Ambisonics (HOA)** | Daniel, "Représentation de champs acoustiques," PhD Thesis, 2000 | Encode to spherical harmonics (1st–6th order), decode to speaker layout via Tikhonov-regularized pseudo-inverse. |
| **KNN** | Nearest-neighbor interpolation over the speaker layout | Use k nearest speaker positions and interpolate. |
| **DBAP** | Lossius, Baltazar, de la Hogue, ICMC 2009 | No sweet-spot assumption; suited to irregular layouts. |
| **MDAP** | Pulkki, "Multiple-Direction Amplitude Panning," AES 1999 | Spreads energy across multiple directions around the source; softer than VBAP for moving sources. |
| **ConstantPower** | Standard equal-power pan law | Pair-wise constant-power panning between the two nearest speakers. |

A dedicated **Direct Binaural** render path (`renderDirectBinauralHRTF`) convolves each tap with the selected HRTF for its exact position — this is always used for the stereo/binaural output bus, independent of the algorithm selection.

### 6.3 Binaural Rendering (v1.0.0)

For stereo/binaural output, `renderDirectBinauralHRTF()` executes a 3-pass architecture:

1. **Pass 1 — Per-sample delay engine → per-source mono accumulation.** Each active tap reads its delayed signal, applies per-tap pitch shift (phase-vocoder STFT), Doppler, and distance attenuation.
2. **Pass 2 — Per-block HRTF convolution via `BinauralRenderer::renderSourceBuffers()`.** Each tap's mono signal is convolved with the HRTF for its current azimuth/elevation. Partitioned overlap-save FFT convolution with the partition size matched to the host buffer.
3. **Pass 3 — Per-sample dry/wet mix + output gain.** Equal-power (cos/sin) crossfade between the stereo dry path (latency-compensated to match the phase vocoder's 2048-sample delay) and the wet sum.

**HRTF lookup:** For positions between measured HRTF directions, `libmysofa`'s KD-tree lookup returns the nearest measured position. HRIR updates at block boundaries with ~1° threshold for realtime-safe position-driven reloading.

**Spectral envelope smoothing:** The `PartitionedConvolver` applies EMA smoothing to magnitude and phase separately per frequency bin, avoiding comb filtering that would result from phase-misaligned time-domain IR blending.

**ITD handling:** SOFA files that bake ITD into the HRIR waveform (e.g. MIT KEMAR) use an onset-detection-driven ITD delay line that is effectively a pass-through; other profiles use the explicit delay values in the SOFA file.

See `agent_docs/architecture.md` for deeper implementation detail.

---

## 7. HRTF Implementation

### 7.1 Binauralization Options

The plugin exposes 6 binauralization options in the HRTF profile selector. Index 0 is a CPU-lite analytical model (no SOFA file); indices 1–5 are measured HRTFs loaded from SOFA files embedded in the plugin binary. Index order matches `hrtfProfileNames[]` in `Source/PluginProcessor.cpp`.

| Index | User-Facing Label | Source | Head Type | License |
|---|---|---|---|---|
| 0 | **Simple (Low CPU)** | Woodworth ITD + broadband ILD model — analytical, no SOFA file | — | — |
| 1 | **Immersive** | SADIE II (York), Subject D2 | Neumann KU100 dummy | Apache 2.0 |
| 2 | **Natural** | CIPIC (UC Davis), Subject 003 | Human | Public Domain |
| 3 | **Precise** | HUTUBS (TU Berlin), Subject PP2 | Human | CC BY 4.0 |
| 4 | **Spatial** | Bernschütz (TH Köln), HRIR_FULL2DEG | Neumann KU100 dummy | CC BY 3.0 |
| 5 | **Studio Reference** | MIT KEMAR, large pinna (DB-065) | KEMAR dummy head | MIT |

**Profile character descriptions (for UI tooltips):**
- **Simple (Low CPU)** — Analytical ITD + ILD. Lowest CPU cost. Not an HRTF — no pinna / torso cues.
- **Immersive** — Rich spatial image with strong elevation cues. Slightly warmer tonality.
- **Natural** — Measured from a human subject. Transparent lateral imaging for average head geometry.
- **Precise** — Cross-validated with numerical simulation. Strong phase accuracy. Analytically wide image.
- **Spatial** — Ultra-high-resolution 2° grid. Smoothest spatial transitions.
- **Studio Reference** — The most widely used reference HRTF in spatial audio. Neutral, predictable imaging.

### 7.2 HRTF File Format & Loading

- **File format:** SOFA (Spatially Oriented Format for Acoustics) — the AES69 standard.
- **Parser library:** `libmysofa` (LGPL 2.1+) — lightweight C library for reading SOFA files. Compiles natively on macOS and Windows.
- **Bundle strategy:** All 5 SOFA files are embedded in the plugin binary as binary resources via JUCE's `BinaryData` system. No external file dependencies.
- **Memory footprint:** Estimated 5-15 MB total for all 5 profiles (depending on spatial resolution). Only the active profile is loaded into the convolution engine at runtime.

### 7.3 Post-v1.0 HRTF Backlog

Tracked as GitHub issues — not in scope for this specification:

- Custom SOFA file import — issue [#218](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/218)
- Companion iPhone app for HRTF personalization via LiDAR ear scanning — issue [#220](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/220)

---

## 8. UI Design

The canonical visual reference for the shipped UI is the annotated user manual at `docs/OpenSpatialDelay_Manual_v1.0.pdf`. This section describes the structural contract only; colour and typography values are sourced from code.

### 8.1 Design Tokens

**Window:** 820 × 580 px, resizable. Implementation in `Source/PluginEditor.h/.cpp`.

**Typography** (embedded via `BinaryData`, declared in `CMakeLists.txt`):

| Family | Weights | License |
|---|---|---|
| DM Sans | Regular, Medium, SemiBold, Bold | SIL OFL 1.1 |
| JetBrains Mono | Regular, Medium, Bold | SIL OFL 1.1 |
| Roboto | Medium | Apache 2.0 |

**Colour palette** (`Colours_OSD` namespace, `Source/PluginEditor.cpp`). Colours are near-black surfaces with six functional accents and 12 per-tap object colours.

| Token | Value | Purpose |
|---|---|---|
| `bgVoid` | `#03060b` | Page background |
| `bgPanel` | `#0a0d12` | Panel surface |
| `bgHeader` | `#06090f` | Header bar |
| `bgRecessed` | `#010205` | Recessed wells (filter graph, spatial map) |
| `bgWell` | `#020307` | Slider / knob wells |
| `borderSubtle` | `#252930` | Panel borders |
| `borderDim` | `#171b20` | Divider lines |
| `accentStellar` | `#80d8ff` | Delay section |
| `accentViolet` | `#7457d1` | Tone / filter section |
| `accentAmber` | `#f0a646` | Mix / output section |
| `accentGreen` | `#3bce6c` | OSC section |
| `accentRose` | `#e467a6` | Modulation section |
| `accentSync` | `#e1c34b` | Tempo-sync indicator |
| `textPrimary` | `#e1e5ea` | Primary text |
| `textSecondary` | `#9fa5ae` | Secondary text |

In addition, `Colours_OSD` defines 12 per-tap object colours (a red → rose spectrum) used across the spatial map, per-tap timeline, and tap-specific knobs.

### 8.2 Layout Regions

Current (v1.0.0) editor structure:

- **Header bar** — preset browser (dropdown + prev / next / save / menu), input format indicator, output format indicator, ADM-OSC status, plugin name/version.
- **Spatial map (centre)** — 2D top-down view with listener at origin, 12 colour-coded draggable tap handles, distance rings, azimuth markings, and live trajectory-animation trails.
- **Global Tap Drawer (left edge, collapsible)** — 6 offset knobs (Azimuth, Elevation, Distance, Pitch, Doppler, Speed) that adjust all enabled taps simultaneously while preserving their relative arrangement.
- **Right panel** — algorithm selector, HRTF profile selector, filter section (HP + LP with Q, filter-graph visualization, Filter On bypass), wobble modulation section (amount + morph + on/off), OSC configuration (enable, send/receive toggles, IP/port), dry/wet, input / output gain.
- **Bottom panel** — per-tap controls for the currently-selected tap: delay-tap index selector, on/off, azimuth, elevation, distance, doppler amount, pitch shift, trajectory shape + speed + direction, input channel.

### 8.3 Interaction Patterns

- Drag tap handles on the spatial map to reposition in azimuth/distance; double-click a handle to select.
- Right-click any knob or slider for context menu (reset, enter value, link to MIDI, link to OSC).
- Shift-drag on knobs for fine precision; double-click to reset.
- Undo/redo via DAW standard shortcuts maps to internal APVTS undo stack; preset name updates on undo/redo.

---

## 9. Preset System

### 9.1 Preset Format

Factory presets are defined as C++ `static const` structs in `Source/PresetData.cpp` and compiled into the plugin binary. At plugin install time, a post-build script writes each factory preset to `~/Library/Audio/Presets/OpenSpatialDelay/<Category>/<Name>.json` (macOS) so users can see, copy, and edit them with any text editor.

Preset JSON carries the full APVTS state (23 globals + 10 × 12 per-tap params) plus metadata (name, category, description, version). User-saved presets live in the same directory and are indistinguishable to the loader from factory presets.

### 9.2 Factory Presets

v1.0.0 ships **70 factory presets** across **8 curated categories**, plus a **User** category for user-created presets (empty at ship):

| Category | Focus |
|---|---|
| Classic Delays | Familiar delay archetypes (ping-pong, slap, dotted eighth, etc.) |
| Spatial Movement | Orbiting, spiralling, and trajectory-led presets |
| Ambient + Texture | Long-tail, pad-friendly spatial washes |
| Height + 3D | Presets exercising elevation and 3D spatial layouts |
| Surround Production | Quad / 5.1 / 7.1 / 7.1.4 / 9.1.6 mix-ready configurations |
| Wobble + Modulated | Tape-style and LFO-driven wobble presets |
| Creative + Experimental | Non-standard designs (gated, chaotic, glitch) |
| Rhythmic | Tempo-synced grooves and polymetric patterns |
| **User** (ships empty) | Users can save new presets from the plugin header preset menu at any time. User presets are saved as JSON under `~/Library/Audio/Presets/OpenSpatialDelay/User/` on macOS and are indistinguishable from factory presets to the loader. Users can also create their own custom categories. |

### 9.3 User Preset Authoring

- Save: header menu → **Save As** → name + category (pick an existing category or type a new one).
- Delete / rename: edit the JSON file on disk, or use the header menu entries.
- Share: user preset JSON files are plain text and portable between OSD installs.

---

## 10. ADM-OSC Integration

### 10.1 Overview

**ADM-OSC** is an industry standard for transmitting object-based audio positioning data in real time, developed by L-Acoustics, FLUX:: Immersive, and Radio France, with contributions from BBC, Dolby, d&b audiotechnik, DiGiCo, Lawo, Magix, Merging Technologies, Meyer Sound, Sound Particles, and Steinberg. It implements the ITU-R BS.2076 Audio Definition Model (ADM) over the Open Sound Control (OSC) protocol.

By implementing ADM-OSC, OpenSpatialDelay (and all future Spatial Media Library plugins) can interoperate with the professional spatial audio ecosystem — sending tap positions to external renderers (L-ISA, SPAT Revolution, Dolby Atmos Renderer) and receiving position data from external controllers, consoles, or automation systems.

**Status in v1.0.0:** Full send + receive are live. In addition to the ADM-OSC namespace, OSD exposes its own `/osd/` namespace for controlling every plugin parameter via OSC (both inbound and outbound).

### 10.2 ADM-OSC Message Specification

The plugin will implement the ADM-OSC v1.0 namespace. All messages use the form `/adm/obj/N/...` where `N` is the object number (1-indexed, corresponding to tap numbers 1–12).

#### Position Messages (Polar)

| OSC Address | Type | Unit | Range | Description |
|-------------|------|------|-------|-------------|
| `/adm/obj/N/azim` | float | degrees | -180 to +180 | Azimuth (0 = front, positive = left) |
| `/adm/obj/N/elev` | float | degrees | -90 to +90 | Elevation (0 = horizon, +90 = zenith) |
| `/adm/obj/N/dist` | float | normalized | 0 to 1 | Distance from listener |
| `/adm/obj/N/aed` | float float float | see above | see above | Combined azimuth, elevation, distance |

#### Position Messages (Cartesian)

| OSC Address | Type | Unit | Range | Description |
|-------------|------|------|-------|-------------|
| `/adm/obj/N/x` | float | normalized | -1 to +1 | Left-right (1 = right) |
| `/adm/obj/N/y` | float | normalized | -1 to +1 | Front-back (1 = front) |
| `/adm/obj/N/z` | float | normalized | -1 to +1 | Up-down (1 = up) |
| `/adm/obj/N/xyz` | float float float | normalized | -1 to +1 | Combined x, y, z |

#### Object Property Messages

The ADM-OSC `/gain` and `/w` (width) property messages are defined by the spec but **not implemented in v1.0.0**. Tracked in issue [#222](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/222).

#### `/osd/` Namespace (OSD-specific)

In addition to ADM-OSC, OSD exposes all plugin parameters via an `/osd/` namespace — receive and send. Examples:

- `/osd/obj/N/enabled`, `/osd/obj/N/pitch`, `/osd/obj/N/doppler`, `/osd/obj/N/trajectory`, `/osd/obj/N/speed`, `/osd/obj/N/direction`, `/osd/obj/N/input`
- `/osd/obj/N/azim` / `/elev` / `/dist` / `/aed` / `/xyz` (aliases mirroring the ADM-OSC object position messages)
- `/osd/global/delaytime`, `/osd/global/feedback`, `/osd/global/filterlp`, `/osd/global/filterhp`, `/osd/global/wobble`, `/osd/global/wobbleamount`, `/osd/global/wobblemorph`, `/osd/global/drywet`, `/osd/global/inputgain`, `/osd/global/outputgain`, `/osd/global/air`, `/osd/global/temposync`, `/osd/global/notedivision`, `/osd/global/syncmode`, `/osd/global/filterenabled`, `/osd/global/filterlpq`, `/osd/global/filterhpq`, `/osd/global/tapazimuth`, `/osd/global/tapelevation`, `/osd/global/tapdistance`, `/osd/global/tappitch`, `/osd/global/tapdoppler`, `/osd/global/tapspeed`

All `/osd/` messages support both send and receive.

#### Coordinate System

- **Polar:** 0° azimuth = front; positive azimuth = left; +90° elevation = directly above
- **Cartesian:** Normalized -1 to +1; x=+1 right, y=+1 front, z=+1 up
- **Conversion:** Per ITU-R BS.2127-0 Section 10.1

### 10.3 Transport Layer

| Parameter | Value | Notes |
|-----------|-------|-------|
| Protocol | UDP | Standard for real-time OSC |
| Default send port | 4003 | Editable in the plugin header |
| Default receive port | 4002 | Editable in the plugin header |
| IP address | User-configurable | Default: `127.0.0.1` (localhost) for same-machine use |

### 10.4 Behavioral Modes

**Send:** The plugin broadcasts the spatial position of each active delay tap as an ADM-OSC object (`/adm/obj/N/aed`) and mirrors plugin parameters on `/osd/global/...` and `/osd/obj/N/...`. Each tap maps to an ADM-OSC object (Tap 1 = `/adm/obj/1/...`, etc.). Updates are rate-limited to avoid UDP flooding.

**Receive:** The plugin listens for ADM-OSC position messages and `/osd/` parameter messages and updates internal state via a lock-free FIFO to the audio thread. Enables external controllers (tablets, consoles, motion capture) to drive tap positions and any plugin parameter.

Both send and receive can be independently enabled / disabled from the plugin header.

### 10.5 Implementation via JUCE

JUCE provides a native `juce_osc` module with `OSCSender` and `OSCReceiver` classes:

```cpp
// Sending tap position via ADM-OSC
juce::OSCSender sender;
sender.connect("127.0.0.1", 4003);

// For each active tap, send its position
for (int i = 0; i < numActiveTaps; ++i)
{
    auto& tap = taps[i];
    juce::OSCMessage msg("/adm/obj/" + juce::String(i + 1) + "/aed");
    msg.addFloat32(tap.azimuthDeg);
    msg.addFloat32(tap.elevationDeg);
    msg.addFloat32(tap.distance);
    sender.send(msg);
}
```

```cpp
// Receiving tap positions from external controller
juce::OSCReceiver receiver;
receiver.connect(4002);
receiver.addListener(this);  // Implements OSCReceiver::Listener

void oscMessageReceived(const juce::OSCMessage& message) override
{
    // Parse /adm/obj/N/aed or /adm/obj/N/xyz messages
    // Update tap positions via lock-free FIFO to audio thread
}
```

**Threading considerations:** OSC messages arrive on the network thread. Position updates must be communicated to the audio thread via a lock-free FIFO (e.g., `juce::AbstractFifo` or `std::atomic` position structs), never by direct mutation — consistent with the project's lock-free `processBlock` requirement.

### 10.6 UI Integration

ADM-OSC and `/osd/` namespace configuration lives in the plugin header:

- **Enable** toggles for send and receive (independent).
- **Send IP** and **Send Port** fields (default `127.0.0.1:4003`).
- **Receive Port** field (default `4002`).
- Status indicators for current send/receive state.

See the user manual (`docs/OpenSpatialDelay_Manual_v1.0.pdf`) for annotated screenshots.

### 10.7 Compatible Software & Hardware

The following tools already support ADM-OSC and will interoperate with OpenSpatialDelay:

| Tool | Vendor | Role |
|------|--------|------|
| SPAT Revolution | FLUX:: Immersive | Spatial audio renderer / mixer |
| L-ISA Controller | L-Acoustics | Live spatial audio system |
| Nuendo | Steinberg | DAW with ADM-OSC support |
| SpaceMap Go | Meyer Sound | Spatial sound design tool |
| QLAB 5 | Figure 53 | Show control (theatre, live events) |
| Ovation | Merging Technologies | Immersive audio workstation |
| Space Controller | Sound Particles | Spatial audio controller |
| Modulo Kinetic | Modulo Pi | Media server with spatial audio |

---

## 11. Build, CI/CD & Distribution

### 11.1 Native Build (Development)

**macOS (Apple Silicon):**
```bash
cmake -B build -G "Xcode" -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release
```

**Windows (x64, via MSVC):**
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### 11.2 CI/CD Pipeline (GitHub Actions)

```
Trigger: Push to main or PR

Jobs:
  macOS:
    Runner: macos-14 (M1 Apple Silicon)
    Steps:
      - Checkout
      - Install dependencies (libmysofa via Homebrew)
      - CMake configure (Xcode generator, arm64)
      - CMake build (Release)
      - Run unit tests
      - Run analyze_spatial.py validation
      - Package: .vst3 bundle + .component (AU) bundle
      - Upload artifacts

  Windows:
    Runner: windows-latest
    Steps:
      - Checkout
      - Install dependencies (vcpkg: libmysofa)
      - CMake configure (VS 2022, x64)
      - CMake build (Release)
      - Run unit tests
      - Package: .vst3 bundle
      - Upload artifacts

```

A tag-push release job (automatic GitHub Release creation) is not part of v1.0.0 CI — releases are cut manually via `gh release create`. Automating this is tracked in issue [#223](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/223).

### 11.3 Distribution

| Channel | Method |
|---|---|
| Releases | [github.com/Spatial-Media-Lab/OpenSpatialDelay/releases](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases) — free, open-source |
| Copy protection | None (honour system) |

### 11.4 Code Signing

Not shipping in v1.0.0. macOS binaries require the user to run `xattr -cr` after download; Windows binaries trigger SmartScreen. Both macOS notarization (Apple Developer Program) and Windows EV code signing are tracked in issue [#217](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/217). The SignPath workflow template in `.github/workflows/build-windows.yml` is present but commented out, ready to activate once the certificate is procured.

---

## 12. Release History & Post-v1.0 Backlog

### 12.1 Release History

For the per-patch changelog, see [`docs/VERSION_HISTORY.md`](docs/VERSION_HISTORY.md). Summary of major milestones:

| Version | Released | Headline |
|---|---|---|
| v0.1 | 2026-03-31 | First installable plugin — binaural spatial delay, HRTF convolution, 12 taps, basic UI |
| v0.2 | Q2 2026 | Surround output (Quad / 5.1 / 7.1 / 7.1.4 / 9.1.6), VBIP + KNN algorithms, Ambisonics decode, LFE, stereo input |
| v0.3 | Q2 2026 | Direct binaural rendering path (per-tap HRTF lookup); virtual-speaker cleanup; DBAP |
| v0.4 | Q3 2026 | Animated trajectories, trajectory presets, tempo-synced animation, Doppler, ADM-OSC Send |
| v0.5 – v0.9 | Q3 2026 – Apr 2026 | Stereo encoding modes, MDAP + ConstantPower algorithms, Ambisonics 1st–6th order, wobble modulation, phase-vocoder pitch, Global Tap Offsets, 70 factory presets, user manual |
| **v1.0.0** | **2026-04-17** | Public release under the Spatial Media Lab org; ADM-OSC Receive + full `/osd/` namespace; production documentation |

### 12.2 Post-v1.0 Backlog

Features deferred past v1.0.0 are tracked as GitHub issues on the `Spatial-Media-Lab/OpenSpatialDelay` repo. They are **not** described in detail in this specification.

| Issue | Title |
|---|---|
| [#217](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/217) | Code signing — macOS notarization + Windows EV certificate |
| [#218](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/218) | Custom SOFA file import — user-loaded HRTF profiles |
| [#219](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/219) | AAX plugin format — Pro Tools compatibility |
| [#220](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/220) | Companion iPhone app for personalized HRTF via ear scanning |
| [#221](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/221) | Spatial Media Library plugin suite — Reverb, Granular, Panner, Chorus (requires shared-core refactor) |
| [#222](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/222) | ADM-OSC `/gain` and `/w` message support |
| [#223](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/223) | Automated GitHub Release creation on tag push |

---

## 13. Appendices

### Appendix A: Academic References

| Topic | Reference |
|-------|-----------|
| VBAP | Pulkki, V. (1997). "Virtual Sound Source Positioning Using Vector Base Amplitude Panning." *Journal of the AES*, 45(6), 456-466. |
| DBAP | Lossius, T., Baltazar, P., de la Hogue, T. (2009). "DBAP — Distance-Based Amplitude Panning." *Proc. ICMC*, Montreal. |
| Ambisonics | Daniel, J. (2000). "Representation de champs acoustiques." PhD Thesis, Universite Paris 6. |
| HRTF Interpolation | Gamper, H. (2013). "Head-related transfer function interpolation in azimuth, elevation, and distance." *JASA*, 134(6). |
| SOFA Format | AES69-2015. "AES Standard for File Exchange — Spatial Acoustic Data File Format." |
| ADM | ITU-R BS.2076-2. "Audio Definition Model." International Telecommunication Union, 2019. |
| ADM-OSC | Music Unit, L-Acoustics, FLUX:: Immersive, Radio France et al. "ADM-OSC: An industry initiative for communicating object-based audio data." *Proc. I3DA*, 2023. GitHub: immersive-audio-live/ADM-OSC |
| OSC | Wright, M. (2002). "Open Sound Control 1.0 Specification." CNMAT, UC Berkeley. |

### Appendix B: HRTF License Summary

| Dataset | License | Attribution Required | Commercial Use | Modification |
|---------|---------|---------------------|---------------|-------------|
| MIT KEMAR | MIT-style | Yes (Gardner & Martin) | Yes | Yes |
| SADIE II D2 | Apache 2.0 | Yes (Armstrong et al.) | Yes | Yes |
| CIPIC S003 | Public Domain | Courtesy citation | Yes | Yes |
| HUTUBS PP2 | CC BY 4.0 | Yes (Brinkmann et al.) | Yes | Yes |
| Bernschuetz KU100 | CC BY 3.0 | Yes (Bernschuetz) | Yes | Yes |

### Appendix C: v1.0.0 Release Metadata

v1.0.0 shipped on **2026-04-17** under the [`Spatial-Media-Lab/OpenSpatialDelay`](https://github.com/Spatial-Media-Lab/OpenSpatialDelay) organization, licensed GPL-3.0. The original v1.0.0 tag was re-cut from `768c248` to the post-#198 / post-#196 fix commit on 2026-04-17. The pre-retag commit is reachable at tag `v1.0.0-dev-baseline`. The archived personal repo at `AndrewRahman/OpenSpatialDelay` serves as a historical reference and URL redirect.

For version-by-version change history see [`docs/VERSION_HISTORY.md`](docs/VERSION_HISTORY.md). For the per-patch plugin-code registry used during active development see [`agent_docs/version_registry.md`](agent_docs/version_registry.md).

---

*End of Specification Document*
