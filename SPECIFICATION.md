# Spatial Media Library — Project Specification
## OpenSpatialDelay v0.1

**Document Version:** 1.1
**Date:** 2026-03-03
**Author:** Andrew Rahman (Product Vision) & Claude (Technical Architecture)

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Product Vision](#2-product-vision)
3. [Technical Architecture](#3-technical-architecture)
4. [v0.1 Feature Specification — OpenSpatialDelay (Binaural)](#4-v01-feature-specification)
5. [Signal Flow](#5-signal-flow)
6. [Spatialization Engine](#6-spatialization-engine)
7. [HRTF Implementation](#7-hrtf-implementation)
8. [UI Design Specification](#8-ui-design-specification)
9. [Preset System](#9-preset-system)
10. [ADM-OSC Integration](#10-adm-osc-integration)
11. [Build, CI/CD & Distribution](#11-build-cicd--distribution)
12. [Versioned Roadmap](#12-versioned-roadmap)
13. [Plugin Suite Roadmap](#13-plugin-suite-roadmap)
14. [Appendices](#14-appendices)

---

## 1. Executive Summary

**Spatial Media Library** is an open-source suite of spatial audio VST3/AU plugins that share a common multi-algorithm spatialization engine. The first product, **OpenSpatialDelay**, is a spatial delay effect where each delay tap is positioned in 3D space and rendered to binaural headphones or discrete surround speakers (Quad through 9.1.6 Atmos). The output format is determined by the DAW's track I/O bus, independent from the spatialization algorithm. The plugin targets music producers, live sound engineers, post-production professionals, and installation/research users.

**Key differentiators:**
- User-selectable spatialization algorithm — 7 options (Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP)
- 6 binauralization options: 5 curated HRTF profiles from academically validated, freely licensed databases + 1 CPU-lite option (Woodworth ITD+ILD approximation, not an HRTF)
- Up to 12 manually placeable delay taps with preset trajectory system
- Cumulative pitch shifting across taps
- ADM-OSC integration for interoperability with spatial audio ecosystems (by v1.0)
- Open-source, licensed under GPL-3.0
- macOS (Apple Silicon) + Windows (x64), VST3 + AU

**Target delivery:** Usable v0.1 by end of March 2026.

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
- **Framework:** JUCE 8 (C++17, targeting C++20 features where beneficial)
- **Build system:** CMake 3.22+
- **Spatial audio HRTF parsing:** libmysofa (LGPL 2.1+ — compatible with open-source distribution)
- **OSC transport (v0.3+):** JUCE `juce_osc` module (built-in `OSCSender` / `OSCReceiver` classes over UDP) — no external dependencies required

### 3.2 Module Architecture

The codebase is organized into a shared core library and per-plugin modules:

```
SpatialMediaLab/
|-- Core/                          # Shared spatial engine (used by ALL plugins)
|   |-- SpatialEngine/
|   |   |-- SpatializationAlgorithm.h      # Abstract base class
|   |   |-- VBAPPanner.h / .cpp            # Vector Base Amplitude Panning
|   |   |-- VBIPPanner.h / .cpp            # Vector Base Intensity Panning
|   |   |-- AmbisonicEncoder.h / .cpp      # HOA Ambisonics encoding
|   |   |-- KNNPanner.h / .cpp             # K-Nearest Neighbor panning
|   |   |-- DBAPPanner.h / .cpp            # Distance-Based Amplitude Panning
|   |   |-- SpeakerLayout.h / .cpp         # Speaker configuration definitions
|   |   |-- BinauralRenderer.h / .cpp      # HRTF convolution engine
|   |   |-- HRTFManager.h / .cpp           # SOFA/HRTF loading & management
|   |   `-- SourcePosition.h               # 3D position (azimuth, elevation, distance)
|   |-- DSP/
|   |   |-- PitchShifter.h / .cpp          # Granular or FFT pitch shifting
|   |   |-- SmoothParameter.h / .cpp       # Lock-free parameter smoothing
|   |   `-- InterpolatedDelay.h / .cpp     # Fractional delay line
|   |-- Preset/
|   |   |-- PresetManager.h / .cpp         # Load/save/factory presets
|   |   `-- TrajectoryEngine.h / .cpp      # Spatial trajectory definitions
|   |-- OSC/                                       # (v0.3+)
|   |   |-- ADMOSCSender.h / .cpp          # Send tap positions as ADM-OSC objects
|   |   |-- ADMOSCReceiver.h / .cpp        # Receive positions from external controllers
|   |   `-- ADMOSCConfig.h / .cpp          # IP/port/rate configuration
|   `-- UI/
|       |-- SpatialMapComponent.h / .cpp   # 2D top-down visualizer (shared)
|       |-- LookAndFeel_SML.h / .cpp       # Shared visual theme
|       `-- CommonControls.h / .cpp        # Knobs, sliders, buttons
|
|-- Plugins/
|   |-- OpenSpatialDelay/
|   |   |-- PluginProcessor.h / .cpp       # Delay-specific audio processing
|   |   |-- PluginEditor.h / .cpp          # Delay-specific UI
|   |   |-- DelayLine.h / .cpp             # Multi-tap delay engine
|   |   `-- TapManager.h / .cpp            # Per-tap state management
|   |-- OpenSpatialReverb/                 # (Future)
|   |-- OpenSpatialGranular/               # (Future)
|   |-- OpenSpatialPanner/                 # (Future)
|   `-- OpenSpatialChorus/                 # (Future)
|
|-- Resources/
|   |-- HRTF/                              # Bundled HRTF SOFA files
|   |   |-- MIT_KEMAR.sofa
|   |   |-- SADIE_II_D2_KU100.sofa
|   |   |-- CIPIC_Subject003.sofa
|   |   |-- HUTUBS_PP2.sofa
|   |   `-- Bernschuetz_KU100_Full2Deg.sofa
|   |-- Presets/                           # Factory presets (JSON)
|   `-- Fonts/                             # UI fonts
|
|-- Tests/
|   |-- SpatialEngineTests.cpp
|   |-- DelayLineTests.cpp
|   |-- HRTFTests.cpp
|   `-- analyze_spatial.py                 # Phase alignment validation
|
`-- CMakeLists.txt                         # Top-level build configuration
```

### 3.3 Core Design Principles

1. **Lock-free audio processing:** The `processBlock` callback must never allocate memory, acquire locks, or perform I/O. All parameter changes use `std::atomic` or lock-free FIFOs.
2. **SIMD optimization:** Use JUCE's `FloatVectorOperations` (which maps to ARM NEON on Apple Silicon and SSE/AVX on x64) for all bulk channel operations — gain scaling, buffer copying, accumulation.
3. **Algorithm abstraction:** All spatialization algorithms inherit from `SpatializationAlgorithm` and implement a common interface (`computeGains(SourcePosition, SpeakerLayout) -> float[]`), enabling runtime algorithm switching without architectural changes.
4. **Shared core, independent plugins:** Each plugin in the suite links against the same `SpatialMediaLab_Core` static library. Plugin-specific DSP lives in the plugin module only.

### 3.4 I/O Architecture

**Core design principle:** The output format is **independent from the spatialization algorithm**. The algorithm computes object-position-to-speaker-gains for whatever speaker layout is active. The output format is determined solely by the DAW's track I/O bus configuration, not by user selection.

#### 3.4.1 Input Format

| Format | Channels | Behavior | Version |
|--------|----------|----------|---------|
| Mono | 1 | Direct pass-through to delay engine | v0.1+ |
| Stereo | 2 | L+R summed to mono internally (`(L+R)*0.5`) | v0.2+ |

Independent L/R delay processing is deferred to v1.0.

#### 3.4.2 Output Format

The plugin auto-detects the output format from the host's track channel count. All speaker positions follow ITU-R BS.775 (ear-level) and BS.2051 (height) standards. Convention: 0° = front, positive azimuth = left.

| Output Format | Channels | Speaker Layout | LFE | Version |
|---------------|----------|----------------|-----|---------|
| Binaural (Stereo) | 2 | Virtual speakers → HRTF renderer | No | v0.1+ |
| Quadraphonic | 4 | L(30°), R(-30°), Ls(110°), Rs(-110°) | No | v0.2 |
| 5.1 Surround | 6 | L, R, C, Ls, Rs + LFE (ch3) | Yes | v0.2 |
| 7.1 Surround | 8 | L, R, C, Lss(90°), Rss(-90°), Lsr(135°), Rsr(-135°) + LFE | Yes | v0.2 |
| 7.1.4 (Atmos bed) | 12 | 7.1 ear-level + Tfl, Tfr, Trl, Trr at 45° elevation + LFE | Yes | v0.2 |
| 9.1.6 (Atmos full) | 16 | 9.1 ear-level + Tfl, Tfr, Tsl, Tsr, Trl, Trr at 45° elevation + LFE | Yes | v0.2 |

**Algorithm × Output Format compatibility:**

| Algorithm | Binaural (2ch) | Discrete Surround (4–16ch) |
|-----------|----------------|---------------------------|
| Direct Binaural | Yes | No (auto-disabled on surround tracks) |
| VBAP | Yes (via 16 virtual speakers) | Yes (2D for flat layouts, 3D for height) |
| Ambisonics (HOA) | Yes (SH → binaural decode) | Yes (SH → Tikhonov-regularized speaker decode) |
| VBIP | Yes (via virtual speakers) | Yes (intensity-weighted VBAP) |
| KNN | Yes (via virtual speakers) | Yes (K-nearest neighbor) |

**Rendering paths:**
- **Binaural:** VBAP/VBIP/KNN algorithms route through a 16-speaker virtual layout, rendered to L/R via the selected HRTF profile (ITD+ILD model). Ambisonics encodes to SH and decodes to binaural via SH weights. Direct Binaural bypasses virtual speakers entirely.
- **Discrete Surround:** Algorithms compute gains directly for the physical speaker layout defined by the active output format. LFE is generated from a mono sum of all spatialized object signals, filtered at 120 Hz (2nd-order Butterworth LP) and attenuated by −10 dB. Dry signal is routed to L and R channels only (channels 0–1), not distributed to all speakers.

**Future extension:** Stereo encoding variants (Stereo VBAP, XY, MS, Blumlein) could be added as distinct 2-channel output modes, distinguished from Binaural by a user-facing dropdown that appears only when the DAW bus is stereo. Not currently planned.

#### 3.4.3 Bus Layout Negotiation

The plugin queries the host via JUCE's `BusesLayout` system:
- **Default configuration:** Mono input → Stereo output
- **Supported inputs:** Mono, Stereo
- **Supported outputs:** Stereo, Quadraphonic, 5.1, 7.1, 7.1.4, 9.1.6
- **Internal bus:** Up to 16 discrete output channels. Actual channel count determined by the track the plugin is placed on.
- **Detection flow:** `prepareToPlay()` calls `detectOutputFormat(getTotalNumOutputChannels())` → `activateLayout(format)`, which configures the `SpeakerLayout` struct, computes the Ambisonics decode matrix (Tikhonov-regularized pseudo-inverse), and builds VBAP triplets for height layouts.
- **UI adaptation:** The editor reads `getActiveOutputFormat()` each timer tick. On surround tracks, Direct Binaural is disabled (auto-switches to VBAP) and the HRTF Profile dropdown is hidden.

---

## 4. v0.1 Feature Specification

### 4.1 Delay Engine

| Parameter | Range | Default | Notes |
|-----------|-------|---------|-------|
| Delay Time | 1 ms — 2000 ms | 500 ms | Per tap; also expressible as tempo-synced note value |
| Tempo Sync | On/Off | Off | When On, delay time snaps to note divisions |
| Note Divisions | 1/1, 1/2, 1/4, 1/8, 1/16 (straight, dotted, triplet) | 1/4 | Only active when Tempo Sync is On |
| Feedback | 0% — 100% | 30% | Global; feeds output back through the spatial trajectory |
| Filter (Low-Pass) | 200 Hz — 20 kHz | 20 kHz | Global; applied to feedback path |
| Filter (High-Pass) | 20 Hz — 5 kHz | 20 Hz | Global; applied to feedback path |
| Pitch Shift | -12 st — +12 st | 0 st | Global; cumulative per tap (Tap N = N * shift amount) |
| Dry/Wet | 0% — 100% | 50% | Blend control |
| Input Gain | -inf — +12 dB | 0 dB | Pre-processing level |
| Output Gain | -inf — +12 dB | 0 dB | Post-processing level |

### 4.2 Tap System

| Feature | Specification |
|---------|--------------|
| Maximum taps | 12 |
| Positioning | Manual placement on 2D top-down map |
| Position parameters | Azimuth (-180 to +180 deg), Elevation (-90 to +90 deg), Distance (0.0 to 1.0 normalized) |
| Tap activation | Each tap can be enabled/disabled individually |
| Tap delay time | Each tap has its own delay time (ms or synced note value) |
| Static positions (v0.1) | Taps remain fixed during playback |
| Animated trajectories | Planned for future version (with optional Doppler effect) |

### 4.3 Spatial Rendering (v0.1 Binaural, v0.2+ Multi-Format)

| Feature | Specification |
|---------|--------------|
| Output format | Binaural stereo (v0.1+), Quad/5.1/7.1/7.1.4/9.1.6 surround (v0.2+) — see Section 3.4.2 |
| Spatialization algorithm | User-selectable: 7 options — Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP (binaural routing is a separate render path, not an algorithm choice) |
| HRTF profiles | 5 built-in options (see Section 7) — binaural output only |
| HRTF convolution | Partitioned FFT convolution for low-latency, artifact-free rendering |
| Distance attenuation | Inverse distance law applied automatically (no user control in v0.1) |
| Distance filtering | High-frequency rolloff proportional to distance (air absorption model) |
| LFE generation | 120 Hz LP (2nd-order Butterworth), −10 dB — surround formats with LFE only |

### 4.4 DAW Integration

| Feature | Specification |
|---------|--------------|
| Plugin formats | VST3, AU (macOS only) |
| Sample rates | 44.1 kHz, 48 kHz, 88.2 kHz, 96 kHz (received from host DAW via `prepareToPlay`; no hardcoded sample rates) |
| Buffer sizes | 64 — 4096 samples |
| Parameter automation | All global parameters automatable via host |
| State save/restore | Full state serialization (preset + tap positions + all parameters) |
| Tempo sync | Reads BPM from host transport |

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
          |  |+1*P st|        |+2*P st|        |+N*P st|  (cumulative)
          |  +---+---+        +---+---+        +---+---+
          |      |                 |                 |
          |      |  (each tap's mono signal + 3D position)
          |      |                 |                 |
          |      v                 v                 v
          |  +---+-----------------+-----------------+---+
          |  |     SPATIALIZATION ALGORITHM (per tap)     |
          |  |  User-selectable: VBAP, Ambisonics, or    |
          |  |  Direct Binaural (see Section 6)           |
          |  |                                            |
          |  |  Input:  tap mono + position (az, el, dst) |
          |  |  Output: speaker gains[] for virtual layout|
          |  +---+-----------------+-----------------+---+
          |      |                 |                 |
          |      v                 v                 v
          |  +---+-----------------+-----------------+---+
          |  |       BINAURAL RENDERER (shared)          |
          |  |  Renders virtual speaker signals to L/R   |
          |  |  via HRTF profile (ITD+ILD or convolution)|
          |  |                                           |
          |  |  For Direct Binaural: position → L/R      |
          |  |  For VBAP/Ambi: speakers → binaural → L/R |
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
             | Pitch |    Apply full round-trip shift: N * P
             |N*P st |
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

This delay works like a traditional ping-pong delay, but instead of bouncing between just Left and Right, the signal "travels" through N spatial positions in 3D space. Each tap is a sequential station along one shared delay chain. The signal enters the delay line, appears at Tap 1's spatial position after 1× base delay, at Tap 2's position after 2× base delay, and so on. When it reaches the last tap (Tap N), the feedback path reads that signal, filters and pitch-shifts it by the cumulative amount (N × P), and feeds it back to the delay input — completing the round trip so the whole spatial sequence repeats.

### Two-Stage Spatialization Architecture

The rendering of each tap to the output is a **two-stage process** designed for modularity across the entire Spatial Media Library plugin suite:

**Stage A — Spatialization Algorithm (pluggable, user-selectable):**
Each tap's mono signal and 3D position are fed into the selected algorithm, which computes gain coefficients for the active speaker layout:
- **Direct Binaural:** Bypasses virtual speakers entirely. Source position maps directly to binaural L/R gains. Only available on stereo (binaural) tracks.
- **VBAP:** Triangulates the source position against the speaker layout. Pair-wise (2D) for flat layouts, triplet-wise (3D) for height layouts.
- **Ambisonics (HOA):** Encodes the source into 3rd-order spherical harmonics, then decodes to the speaker layout via Tikhonov-regularized pseudo-inverse matrix.
- **VBIP:** Intensity-weighted VBAP — squares gains for better energy preservation with off-center sources.
- **KNN:** K-nearest-neighbor interpolation against the speaker layout positions.

All algorithms implement the shared `SpatializationAlgorithm` interface (see Section 6.1) and operate on whatever `SpeakerLayout` is active — virtual (for binaural) or physical (for surround). Future algorithms (DBAP) can be added without changing the delay engine.

**Stage B — Output Renderer (output-format-dependent):**
The renderer is determined by the DAW's track I/O bus (see Section 3.4.2):
- **Binaural (2ch):** Virtual speaker contributions are rendered to headphone L/R using the selected HRTF profile (ITD+ILD model). Ambisonics decodes directly to binaural via SH weights.
- **Discrete Surround (4–16ch):** Algorithm gains are routed directly to the physical output channels defined by the active `SpeakerLayout`. LFE is derived from a mono sum of all spatialized signals (120 Hz LP, −10 dB). Dry signal is routed to L/R only.

The algorithm stage is fully reusable across all output formats.

### Feedback Path

**Critical: the feedback path is MONO and pre-spatial.** Spatialization is applied only in the output rendering path. The feedback loop stays in the mono domain, reading from a single point at the end of the tap chain. This ensures clean, stable feedback without spatial rendering artifacts accumulating in the loop.

### Implementation Notes

- **v0.1:** All three algorithm options use the same simplified binaural renderer (`computeBinauralGains()` — Woodworth ITD + broadband ILD). HRTF convolution (partitioned FFT) is a future priority.
- **v0.2:** Adds discrete surround output (Quad through 9.1.6), VBIP and KNN algorithms, 2D/3D VBAP, Ambisonics decode to arbitrary speaker layouts, and LFE generation. All five algorithms are fully implemented for both binaural and surround paths.

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

### 6.2 Algorithms to Implement

| Algorithm | Academic Reference | Priority | Notes |
|-----------|--------------------|----------|-------|
| **VBAP** | Pulkki, "Virtual Sound Source Positioning Using Vector Base Amplitude Panning," JAES 1997 | v0.1 | Industry standard. Triangulation-based. Pair-wise (2D) or triplet-wise (3D). |
| **Ambisonics (HOA)** | Daniel, "Representation de champs acoustiques," PhD Thesis, 2000 | v0.1 | Encode to spherical harmonics, decode to any speaker layout. Up to 7th order (64 channels). |
| **Direct HRTF** | N/A (convolution-based) | v0.1 | For binaural: skip speaker panning, convolve source directly with HRTF at the tap's position. Most accurate for headphone rendering. |
| **KNN** | Based on nearest-neighbor interpolation of HRTF measurements | v0.2 | Use K nearest measured HRTF positions and interpolate. |
| **VBIP** | Intensity-weighted variant of VBAP | v0.2 | Better energy preservation than VBAP for off-center sources. |
| **DBAP** | Lossius, Baltazar, de la Hogue, "DBAP — Distance-Based Amplitude Panning," ICMC 2009 | v0.3 | No sweet spot assumption. Ideal for installations with irregular layouts. |

### 6.3 Binaural Rendering (v0.1)

For v0.1 (binaural output), the rendering path is:

1. For each active tap, look up the nearest HRTF pair (left ear IR, right ear IR) for the tap's azimuth/elevation.
2. Convolve the tap's delayed+pitched audio with the left-ear and right-ear impulse responses.
3. Apply distance attenuation (inverse distance law) and air absorption (6 dB/doubling distance high-shelf filter).
4. Sum all tap contributions into the stereo binaural output.

**HRTF interpolation:** For positions between measured HRTF directions, use spherical linear interpolation (SLERP) between the nearest 3 measured positions (triangulated on the measurement sphere).

**Convolution strategy:** Partitioned overlap-save FFT convolution with partition size matching the host buffer size for zero-latency operation.

---

## 7. HRTF Implementation

### 7.1 Selected HRTF Profiles

Five individual HRTF profiles, each selected for its distinct sonic character and free, redistribution-compatible license:

| # | User-Facing Label | Source Database | Measurement ID | Head Type | License |
|---|------------------|-----------------|----------------|-----------|---------|
| 1 | **Studio Reference** | MIT KEMAR | Large pinna (DB-065) | KEMAR dummy head | MIT-style permissive |
| 2 | **Immersive** | SADIE II (York) | Subject D2 | Neumann KU100 dummy | Apache 2.0 |
| 3 | **Natural** | CIPIC (UC Davis) | Subject 003 | Human | Public domain |
| 4 | **Precise** | HUTUBS (TU Berlin) | Subject PP2 | Human | CC BY 4.0 |
| 5 | **Spatial** | Bernschuetz (TH Koeln) | HRIR_FULL2DEG | Neumann KU100 dummy | CC BY 3.0 |

**Profile character descriptions (for UI tooltips):**
1. **Studio Reference** — The most widely used reference HRTF in spatial audio. Neutral, predictable imaging. Best starting point.
2. **Immersive** — Rich spatial image with strong elevation cues. Adopted by Google for VR audio. Slightly warmer tonality.
3. **Natural** — Measured from a human subject. Most transparent lateral imaging for users with average head geometry.
4. **Precise** — Cross-validated with numerical simulation. Excellent phase accuracy. Analytically wide image.
5. **Spatial** — Ultra-high-resolution measurement (2-degree grid). Smoothest spatial transitions. No interpolation artifacts.

### 7.2 HRTF File Format & Loading

- **File format:** SOFA (Spatially Oriented Format for Acoustics) — the AES69 standard.
- **Parser library:** `libmysofa` (LGPL 2.1+) — lightweight C library for reading SOFA files. Compiles natively on macOS and Windows.
- **Bundle strategy:** All 5 SOFA files are embedded in the plugin binary as binary resources via JUCE's `BinaryData` system. No external file dependencies.
- **Memory footprint:** Estimated 5-15 MB total for all 5 profiles (depending on spatial resolution). Only the active profile is loaded into the convolution engine at runtime.

### 7.3 Future HRTF Features (Post-v0.1)
- Custom SOFA file import (user loads their own HRTF)
- Companion iPhone app for HRTF personalization via ear scanning (LiDAR + photogrammetry)

---

## 8. UI Design Specification

### 8.1 Design Language

Inspired by Sound Particles inDelay (functional layout) and SnappySnap (aesthetic):

- **Background:** Deep dark charcoal/near-black (#0A0A14)
- **Primary accent:** Neon cyan (#00D4FF)
- **Secondary accent:** Electric purple (#8B5CF6)
- **Active elements:** Warm amber/gold (#F59E0B)
- **Text:** Off-white (#E2E8F0) on dark backgrounds
- **Panels:** Subtle glassmorphism with 1px border (#1E293B), slight background blur
- **Corner radius:** 8-12px on panels
- **Typography:** Clean sans-serif (Inter or system font)
- **Tap indicators:** Colored circles on the spatial map, one distinct color per tap

### 8.2 Layout (Approximate 800x500px Window)

```
+------------------------------------------------------------------+
|  OpenSpatialDelay                    [Preset: v] [<] [>] [Save]  |
+------------------------------------------------------------------+
|                                                                    |
|  +--------------------------------------------+  +-------------+ |
|  |                                            |  | ALGORITHM   | |
|  |          2D SPATIAL MAP                    |  | [VBAP    v] | |
|  |          (Top-Down View)                   |  |             | |
|  |                                            |  | HRTF        | |
|  |     Listener (center dot)                  |  | [Studio  v] | |
|  |     Taps (colored circles 1-12)            |  |             | |
|  |     Azimuth ring markings                  |  | DELAY TIME  | |
|  |     Distance rings                         |  | [500 ms   ] | |
|  |                                            |  | [Sync: Off] | |
|  |                                            |  |             | |
|  +--------------------------------------------+  | FEEDBACK    | |
|                                                   | [###  30%]  | |
|  +--------------------------------------------+  |             | |
|  | TAP TIMELINE                               |  | FILTER      | |
|  | [1][2][3][4][5][6][7][8][9][10][11][12]     |  | HP [20 Hz ] | |
|  | Time: |====o==========| 500ms              |  | LP [20 kHz] | |
|  | Az:   |========o======| +45 deg            |  |             | |
|  | El:   |=====o=========| +10 deg            |  | PITCH       | |
|  | Dist: |==o============| 0.3                |  | [+0 st    ] | |
|  +--------------------------------------------+  |             | |
|                                                   | DRY/WET     | |
|  +---+  +---+  +---+                             | [###  50%]  | |
|  |IN |  |OUT|  |D/W|                              |             | |
|  | 0 |  | 0 |  |50%|                              | [IN] [OUT]  | |
|  |dB |  |dB |  |   |                              | 0dB   0dB   | |
|  +---+  +---+  +---+                             +-------------+ |
+------------------------------------------------------------------+
```

### 8.3 UI Components

**Spatial Map (Main Area)**
- 2D top-down circular view
- Listener at center (fixed dot or head icon)
- Concentric rings indicating distance (0.25, 0.5, 0.75, 1.0)
- Azimuth markings at 0/90/180/270 degrees
- Taps shown as numbered, color-coded draggable circles
- Click to add tap, right-click to remove
- Drag to reposition

**Tap Timeline (Below Map)**
- Horizontal strip showing all 12 tap slots
- Selected tap is highlighted; its parameters appear below
- Per-tap sliders: delay time, azimuth, elevation, distance
- Tap enable/disable toggle per slot

**Control Panel (Right Side)**
- Algorithm selector dropdown (VBAP, HOA, Direct HRTF)
- HRTF profile selector dropdown (5 profiles)
- Global delay time (with tempo sync toggle + note division selector)
- Global feedback knob
- Global filter (HP + LP)
- Global pitch shift
- Dry/Wet knob
- Input/Output gain knobs

**Header Bar**
- Plugin name and version
- Preset browser (dropdown + prev/next + save button)

---

## 9. Preset System

### 9.1 Preset File Format

Presets are stored as JSON files:

```json
{
    "name": "Orbital Spiral",
    "version": "1.0",
    "plugin": "OpenSpatialDelay",
    "algorithm": "VBAP",
    "hrtfProfile": 0,
    "globalParams": {
        "feedback": 0.35,
        "filterHP": 80.0,
        "filterLP": 12000.0,
        "pitchShift": 2.0,
        "dryWet": 0.5,
        "inputGain": 0.0,
        "outputGain": 0.0,
        "tempoSync": false,
        "delayTimeMs": 375.0,
        "noteDivision": "1/4"
    },
    "taps": [
        {
            "index": 0,
            "enabled": true,
            "delayTimeMs": 250.0,
            "azimuthDeg": -30.0,
            "elevationDeg": 0.0,
            "distance": 0.4
        },
        {
            "index": 1,
            "enabled": true,
            "delayTimeMs": 500.0,
            "azimuthDeg": 45.0,
            "elevationDeg": 15.0,
            "distance": 0.6
        }
    ]
}
```

### 9.2 Factory Presets (v0.1)

A minimal set shipping with the plugin:

| Preset Name | Description | Taps Used |
|-------------|-------------|-----------|
| **Default** | Single centered tap, 500ms, no pitch shift | 1 |
| **Stereo Ping-Pong** | Two taps at -90 and +90 degrees, alternating | 2 |
| **Circle (Quad)** | 4 taps at 0, 90, 180, 270 degrees equidistant | 4 |
| **Surround 5.1** | 5 taps positioned at L, R, C, Ls, Rs | 5 |
| **Surround 7.1** | 7 taps at standard 7.1 speaker positions | 7 |
| **Atmos 7.1.4** | 11 taps at 7.1.4 speaker positions | 11 |
| **Rising Spiral** | 8 taps spiraling outward with +2st pitch | 8 |
| **Falling Cascade** | 6 taps descending in elevation with -1st pitch | 6 |

---

## 10. ADM-OSC Integration

### 10.1 Overview

**ADM-OSC** is an industry standard for transmitting object-based audio positioning data in real time, developed by L-Acoustics, FLUX:: Immersive, and Radio France, with contributions from BBC, Dolby, d&b audiotechnik, DiGiCo, Lawo, Magix, Merging Technologies, Meyer Sound, Sound Particles, and Steinberg. It implements the ITU-R BS.2076 Audio Definition Model (ADM) over the Open Sound Control (OSC) protocol.

By implementing ADM-OSC, OpenSpatialDelay (and all future Spatial Media Library plugins) can interoperate with the professional spatial audio ecosystem — sending tap positions to external renderers (L-ISA, SPAT Revolution, Dolby Atmos Renderer) and receiving position data from external controllers, consoles, or automation systems.

**ADM-OSC is not required for v0.1.** It is targeted for **v0.3** (send) and **v1.0** (full send + receive), as it becomes critical once surround speaker output is implemented.

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

| OSC Address | Type | Range | Description |
|-------------|------|-------|-------------|
| `/adm/obj/N/gain` | float | 0 to 1 | Linear gain (1.0 = unity) |
| `/adm/obj/N/w` | float | 0 to 1 | Width / extent of object |

#### Coordinate System

- **Polar:** 0° azimuth = front; positive azimuth = left; +90° elevation = directly above
- **Cartesian:** Normalized -1 to +1; x=+1 right, y=+1 front, z=+1 up
- **Conversion:** Per ITU-R BS.2127-0 Section 10.1

### 10.3 Transport Layer

| Parameter | Value | Notes |
|-----------|-------|-------|
| Protocol | UDP | Standard for real-time OSC |
| Default send port | 4001 | ADM-OSC recommended default |
| Default receive port | 4002 | For bidirectional communication |
| Ports | User-configurable | Editable in plugin settings panel |
| IP address | User-configurable | Default: `127.0.0.1` (localhost) for same-machine use |

### 10.4 Behavioral Modes

**Send Mode (v0.3):**
The plugin transmits the spatial position of each active delay tap as an ADM-OSC object. When tap positions change (via user interaction, preset loading, or trajectory animation), the plugin sends updated position messages to the configured destination IP:port. This allows external renderers to mirror the spatial scene.

- Each tap maps to an ADM-OSC object: Tap 1 = `/adm/obj/1/...`, Tap 2 = `/adm/obj/2/...`, etc.
- Position updates are sent at a configurable rate (default: 30 Hz) to avoid UDP flooding
- Gain messages reflect per-tap level (accounting for distance attenuation)

**Receive Mode (v1.0):**
The plugin listens for incoming ADM-OSC messages and updates tap positions accordingly. This enables:
- External spatial controllers (tablets, consoles, motion capture) to drive tap positions
- DAW-to-DAW spatial scene sharing
- Live performance control from mixing consoles (DiGiCo, Lawo, etc.)

**GET Requests (v1.0):**
Per the ADM-OSC spec, sending a message without arguments acts as a GET request. The plugin will respond with the current value. Example: receiving `/adm/obj/4/xyz` (no args) triggers a reply of `/adm/obj/4/xyz -0.5 0.8 0.0`.

### 10.5 Implementation via JUCE

JUCE provides a native `juce_osc` module with `OSCSender` and `OSCReceiver` classes:

```cpp
// Sending tap position via ADM-OSC
juce::OSCSender sender;
sender.connect("127.0.0.1", 4001);

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

When ADM-OSC is enabled, the plugin settings panel will include:

```
+------------------------------------------+
| ADM-OSC Settings                         |
|                                          |
| [x] Enable ADM-OSC                       |
|                                          |
| Mode:  [Send ▼]  (Send / Receive / Both) |
|                                          |
| Send To:                                 |
| IP:   [127.0.0.1    ]  Port: [4001]      |
|                                          |
| Receive On:                              |
| Port: [4002]                             |
|                                          |
| Update Rate: [30 Hz ▼]                   |
|                                          |
| Status: ● Connected (sending)            |
+------------------------------------------+
```

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

  Release (on tag push):
    - Download artifacts from both jobs
    - Create GitHub Release with .zip archives for each platform
```

### 11.3 Distribution

| Phase | Method |
|-------|--------|
| v0.x (current) | GitHub Releases — free, open-source |
| v1.0+ (future) | Potential website with optional paid downloads |
| Copy protection | None (honor system) for v0.x |

### 11.4 Code Signing (Future)

- **macOS:** Apple Developer Program ($99/yr) for notarization. Not required for v0.1 (unsigned binaries can be opened via right-click > Open). Consider when distributing to non-technical users.
- **Windows:** EV code signing certificate for SmartScreen trust. Not required for v0.1.

---

## 12. Versioned Roadmap

### Phase 1: v0.1 — Binaural Spatial Delay (Target: End of March 2026)

**Milestone: A working, installable plugin that a user can load in a DAW.**

| Week | Focus | Deliverables |
|------|-------|-------------|
| 1 (Mar 3-9) | Project scaffolding | CMake setup, JUCE project, module structure, CI pipeline, HRTF loading with libmysofa, basic delay line |
| 2 (Mar 10-16) | Core DSP | Multi-tap delay engine (12 taps), HRTF convolution, binaural renderer, pitch shifter, feedback path with filters |
| 3 (Mar 17-23) | UI + Integration | Spatial map component, tap timeline, control panel, parameter binding, preset save/load |
| 4 (Mar 24-31) | Polish + Release | Factory presets, testing on macOS + Windows, bug fixes, GitHub Release |

### Phase 2: v0.2 — Surround Output + Enhanced Algorithms (Q2 2026) *(Implemented)*

| Feature | Details | Status |
|---------|---------|--------|
| Binaural output | Stereo HRTF-based rendering (ITD+ILD model) | Carried from v0.1 |
| Quadraphonic output | 4-channel speaker output (ITU-R BS.775) | Done |
| 5.1 surround output | 6-channel speaker output + LFE | Done |
| 7.1 surround output | 8-channel speaker output + LFE | Done |
| 7.1.4 output (Atmos bed) | 12-channel with 4 height speakers + LFE | Done |
| 9.1.6 output (Atmos full) | 16-channel with 6 height speakers + LFE | Done |
| Dynamic channel detection | `detectOutputFormat()` maps DAW bus channel count to output format | Done |
| LFE generation | 120 Hz Butterworth LP, −10 dB, from mono sum of spatialized signals | Done |
| Context-sensitive UI | Direct Binaural disabled on surround tracks; HRTF dropdown hidden | Done |
| VBIP algorithm | Intensity-weighted VBAP (squared gains, re-normalized) | Done |
| KNN algorithm | K-nearest-neighbor speaker interpolation | Done |
| 2D/3D VBAP | Pair-wise for flat layouts, triplet-wise for height layouts | Done |
| Ambisonics surround decode | 3rd-order SH → Tikhonov-regularized pseudo-inverse decode matrix | Done |
| Stereo input (summed to mono) | L+R × 0.5 internal mono conversion | Done |

### Phase 3: v0.3 — Direct Binaural Rendering + Architecture Refinement (Q2 2026)

| Feature | Details |
|---------|---------|
| **Direct binaural rendering** | **Per-source HRTF convolution — each tap's 3D position (azimuth + elevation) maps directly to an HRTF lookup via SOFA file. No intermediate virtual speaker layout. Replaces the 16-speaker virtual array binaural path.** |
| **Monitoring format removal** | **Eliminates the monitoring format dropdown entirely. Virtual speaker layouts are no longer needed for binaural output.** |
| **Output-format-aware algorithm UI** | **Binaural (2ch): algorithm locked to "Direct Binaural", greyed out. Surround (>2ch): full algorithm menu (VBAP, VBIP, KNN, Ambisonics).** |
| **Virtual speaker cleanup** | **Removes speaker convolver bank (speakerConvL/R[16]), SH convolver bank (shConvL/R[16]), renderSpeakerBuffers(), renderSHBuffers(), computeSHProjectedHRIRs(). Replaced by 12 per-source convolvers.** |
| **Per-source HRIR update** | **Realtime-safe position-driven HRIR reloading at block boundaries via libmysofa KD-tree lookup + in-place FFT. ~1° threshold, <0.5% CPU typical.** |

### Phase 4: v0.4 — Trajectories + ADM-OSC Send + Enhanced DSP (Q3 2026)

| Feature | Details |
|---------|---------|
| Animated trajectories | Real-time tap movement during playback |
| Trajectory presets | Spiral, bounce, orbit, random drift, figure-8 |
| Tempo-synced trajectories | Movement speed locked to host BPM |
| Doppler effect | Pitch shift proportional to tap velocity |
| DBAP algorithm | Distance-based amplitude panning for installations |
| **ADM-OSC Send** | **Transmit tap positions as ADM-OSC objects via UDP/OSC to external renderers (SPAT Revolution, L-ISA, etc.). Configurable destination IP/port, 30 Hz default update rate. Uses JUCE `juce_osc` module.** |

### Phase 5: v1.0.0 — Production Release (Q1 2026, shipping week of 2026-04-10)

| Feature | Status | Details |
|---------|--------|---------|
| Stereo input support | **Done** | Per-tap L+R/L/R input channel selection with dual delay lines |
| Per-tap pitch shift | **Done** | ±24 semitone range via phase vocoder, additive to global pitch |
| Global modulation (LFO) | **Done** | Wobble modulation with morphable waveform (sine → square) |
| ADM-OSC Receive | **Done** | Incoming ADM-OSC messages control tap positions; OSC control of all parameters via `/osd/` namespace |
| ADM-OSC Send | **Done** | Broadcast tap positions to external renderers (L-ISA, SPAT Revolution, Dolby Atmos Renderer) |
| ADM-OSC Settings (header bar) | **Done** | Enable/disable, IP/port config, send/receive toggles in plugin header — uses ADM-OSC standards for object positions |
| Expanded factory presets | **Done** | 70 factory presets across 9 categories |
| User manual / documentation | **Done** | PDF manual with signal flow diagram, annotated screenshots, glossary, legal notices |
| Custom SOFA import | **Deferred indefinitely** | User loads their own HRTF files |
| AAX format | **Deferred post-v1.0** | Pro Tools compatibility |
| Code signing | **Deferred post-v1.0** | macOS notarization + Windows EV certificate |

---

## 13. Plugin Suite Roadmap

All plugins share the `SpatialMediaLab/Core` engine and UI components.

| Plugin | Description | Target |
|--------|-------------|--------|
| **OpenSpatialDelay** | Spatial delay with per-tap positioning | v0.1: Q1 2026 |
| **OpenSpatialReverb** | Algorithmic reverb with spatial early reflections and diffuse tail positioning | 2027 |
| **OpenSpatialGranular** | Granular synthesis/processing with grains positioned in 3D space (Sound Particles-inspired) | 2027 |
| **OpenSpatialPanner** | Object-based spatial panner with energy distribution (Energy Panner-inspired). Trajectory automation, multi-source management | 2027 |
| **OpenSpatialChorus** | Chorus/flanger with spatially distributed voices | 2027 |

### Shared Core Components Across Suite

| Component | Used By |
|-----------|---------|
| Spatialization algorithms (VBAP, HOA, DBAP, etc.) | All plugins |
| Binaural renderer (HRTF convolution) | All plugins |
| HRTF manager (SOFA loading, profile selection) | All plugins |
| Speaker layout definitions | All plugins |
| 2D spatial map UI component | All plugins |
| Preset manager (JSON load/save) | All plugins |
| Trajectory engine | Delay, Panner, Granular |
| ADM-OSC transport (send/receive) | All plugins |
| Look-and-feel / theme | All plugins |

---

## 14. Appendices

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

### Appendix C: Docker Environment (Legacy Reference)

The existing Docker environment (`Dockerfile`, `docker-compose.yml`, `setup-agent.sh`) was used for initial toolchain validation on ARM64/Linux. It successfully built a 64-channel test plugin but cannot produce macOS or Windows binaries. This environment is retained for reference but is **not part of the production build pipeline**. Native builds and GitHub Actions CI replace it going forward.

### Appendix D: Existing Codebase Disposition

The `Spatial64/` directory contains the proof-of-concept test plugin. Its code will **not be carried forward** directly. However, the following elements validated in that project inform the new architecture:

| Validated Element | Carried Forward As |
|-------------------|--------------------|
| JUCE 8 CMake integration | Same pattern, expanded for multi-target builds |
| 64-channel bus negotiation | Adapted for dynamic channel count |
| `FloatVectorOperations` for SIMD | Core DSP pattern for all buffer operations |
| Lock-free `processBlock` | Mandatory design constraint for all plugins |
| `analyze_spatial.py` validation | Retained in test suite |

### Appendix E: v1.0 Release & GitHub Migration Checklist

This checklist covers the complete v1.0 release process, including freezing v0.9, migrating the repository to the Spatial Media Lab organization, and publishing the first public release.

**Decisions finalized on 2026-03-18:**
- **License:** GPL-3.0
- **Repository:** Fresh repo under `github.com/Spatial-Media-Lab/OpenSpatialDelay` (not transfer)
- **Git history:** Squash to single v1.0 commit for public repo; full dev history stays in archived private repo
- **Downloads:** GitHub Releases page (not website)
- **Visibility:** Public at v1.0 release

#### Phase 1: Freeze v0.9

Follow the Version Freeze SOP from CLAUDE.md:

| Step | Action | Verify |
|------|--------|--------|
| 1 | Copy `Source/` files to `Archive/v0.9/` with version suffix | Files exist in archive |
| 2 | `chmod a-w Archive/v0.9/*` | Files are read-only |
| 3 | Build macOS Release: `cmake --build build --config Release` | AU + VST3 installed |
| 4 | Copy built plugins to `Archive/v0.9/builds/` | Builds stored locally |
| 5 | Update `PLUGIN_CODE` in CMakeLists.txt: `Osd9` → `Os10` | New plugin identity |
| 6 | Update `docs/VERSION_HISTORY.md` and `CLAUDE.md` status section | Docs current |
| 7 | Commit and push to `AndrewRahman/OpenSpatialDelay` | All changes on GitHub |
| 8 | Wait for Windows CI: `gh run list --workflow=build-windows.yml --limit=1` | Build succeeds |
| 9 | Download Windows VST3: `bash scripts/download_windows_build.sh` | Windows build in `build/windows/` |
| 10 | Copy Windows build to `Archive/v0.9/builds/` | Both platforms archived |

#### Phase 2: v1.0.0 Development

v1.0.0 feature status:

| Feature | Status | Notes |
|---------|--------|-------|
| Custom SOFA import | Deferred indefinitely | User loads own HRTF files — not shipping in v1.0.0 |
| ADM-OSC Settings | Done | OSC enable/disable, IP/port in header bar; uses ADM-OSC for object positions |
| AAX format | Deferred post-v1.0 | Pro Tools compatibility |
| Code signing | Deferred post-v1.0 | macOS notarization + Windows EV cert |
| User manual finalization | Done | PDF with glossary, signal flow diagram, annotated screenshots |

#### Phase 3: GitHub Migration

Execute when v1.0 is feature-complete and tested:

| Step | Command | Verify |
|------|---------|--------|
| 1 | `gh auth refresh -h github.com -s admin:org,repo,delete_repo` | Auth succeeds |
| 2 | `gh repo create Spatial-Media-Lab/OpenSpatialDelay --private --description "Spatial delay effect for 3D audio production — VST3 & AU" --homepage "https://spatialmedialab.org"` | Repo created |
| 3 | `git remote rename origin personal` | Old remote preserved |
| 4 | `git remote add origin https://github.com/Spatial-Media-Lab/OpenSpatialDelay.git` | New remote set |
| 5 | `git push -u origin main` | Code pushed |
| 6 | Verify CI triggers: `gh run list --repo Spatial-Media-Lab/OpenSpatialDelay --workflow=build-windows.yml --limit=1` | Windows build runs |
| 7 | Verify download script: `bash scripts/download_windows_build.sh` | Downloads from new repo |
| 8 | Update `CLAUDE.md` GitHub URL → `https://github.com/Spatial-Media-Lab/OpenSpatialDelay` | Reference updated |
| 9 | Update `MEMORY.md` GitHub URL | Reference updated |

#### Phase 4: Public Release

| Step | Command / Action | Verify |
|------|-----------------|--------|
| 1 | Final macOS Release build + verify AU/VST3 install | Plugins work in Reaper |
| 2 | Download final Windows build from CI | VST3 present |
| 3 | Build final user manual PDF | PDF generated |
| 4 | `gh repo edit Spatial-Media-Lab/OpenSpatialDelay --visibility public` | Repo is public |
| 5 | `gh release create v1.0.0 --repo Spatial-Media-Lab/OpenSpatialDelay --title "v1.0.0" --notes "First public release"` | Release created |
| 6 | Attach macOS + Windows builds to release | Both platforms downloadable |
| 7 | Attach user manual PDF to release | Manual downloadable |
| 8 | Verify LICENSE file visible at repo root | GPL-3.0 header |

#### Phase 5: Archive Personal Repo

| Step | Command | Verify |
|------|---------|--------|
| 1 | `gh repo edit AndrewRahman/OpenSpatialDelay --description "ARCHIVED — Moved to github.com/Spatial-Media-Lab/OpenSpatialDelay"` | Description updated |
| 2 | Keep repo (do NOT delete) — serves as URL redirect and history archive | Accessible for reference |

---

*End of Specification Document*
