---
name: spatial-audio-dsp
description: >
  Expert in spatial audio DSP for JUCE C++ audio plugins (VST3/AU).
  Covers 8 spatialization algorithms (Constant Power, VBAP, VBIP, KNN, Ambisonics HOA, DBAP, MDAP, Direct Binaural),
  HRTF convolution via SOFA/libmysofa, SH-domain rendering (ACN/SN3D),
  virtual speaker deployment, binaural rendering pipelines,
  ADM-OSC protocol integration, coordinate system conventions (ADM/ITU-R),
  real-time lock-free DSP constraints, and CMake build systems.
  Activate on 'spatial audio', 'HRTF', 'binaural', 'ambisonics', 'VBAP', 'VBIP', 'KNN',
  'DBAP', 'MDAP', 'spatialization', 'SOFA', 'spherical harmonics', 'virtual speaker',
  'convolution', 'ITD', 'ILD', 'ADM-OSC', 'object-based audio', 'surround panning',
  'Dolby Atmos', 'speaker layout', 'panning algorithm'.
  NOT for music composition, mixing/mastering, voice synthesis (use voice-audio-engineer),
  or game engine middleware integration (use sound-engineer).
allowed-tools: Read,Write,Edit,Bash,Glob,Grep,WebFetch,WebSearch
metadata:
  category: Development & Engineering
  pairs-with:
    - skill: sound-engineer
      reason: Game audio middleware + spatial audio integration
  tags:
    - spatial-audio
    - dsp
    - hrtf
    - ambisonics
    - juce
    - audio-plugin
    - c++
    - cmake
    - sofa
    - binaural
---

# Spatial Audio DSP — Expert Skill for JUCE C++ Audio Plugin Development

This skill provides deep expertise in spatial audio algorithm implementation, HRTF-based binaural rendering, and real-time DSP for professional audio plugins targeting DAW environments (VST3/AU).

---

## 1. Spatialization Algorithms

Eight algorithms implemented, each with distinct mathematical foundations and use cases.

### 1.1 Constant Power — Cosine-Distance All-Speaker Panning (Default)

All-speaker weighting using cosine of angular distance with hemisphere cutoff:
1. Convert source and all speakers to unit Cartesian vectors
2. For each speaker: `rawGain = max(0, dot(source, speaker))`
3. Speakers beyond 90° from source get zero gain (hemisphere cutoff)
4. Constant-power normalization: `scale = 1 / sqrt(Σ rawGain²)`

**Effect:** Activates all speakers within 90° of the source with natural cosine rolloff. Produces wider, smoother spatial images than VBAP's 2-3 speaker selection. Computationally lightest of all algorithms (no sorting, no triangulation, no Euclidean distance).

**Best for:** Most surround work. Default algorithm for all surround output formats. Natural-sounding panning with smooth transitions between speakers.

**Industry precedent:** Steinberg Nuendo/Cubase "Constant Power" surround panner.

### 1.3 VBAP — Vector Base Amplitude Panning (Pulkki 1997, JAES)

**2D (flat layouts: Quad, 5.1, 7.1):**
Find the speaker pair spanning the source azimuth. Gains via sine law:
```
g1 = sin(az_spk2 - az_src) / sin(az_spk2 - az_spk1)
g2 = sin(az_src - az_spk1) / sin(az_spk2 - az_spk1)
```
Constant-power normalization: `scale = 1 / sqrt(g1² + g2²)`.

**3D (height layouts: 7.1.4, 9.1.6, virtual speakers):**
Pre-compute Delaunay triangulation of speakers into triplets. For each triplet {i,j,k}, store 3x3 inverse of direction matrix `[spk_i spk_j spk_k]`. At runtime:
1. Convert source to Cartesian: `p = [cos(el)sin(az), cos(el)cos(az), sin(el)]`
2. For each triplet: `gains = inv_matrix × p`
3. Select triplet where all 3 gains are positive (source inside triangle)
4. If multiple valid: pick highest gain sum
5. Constant-power normalize: `scale = 1 / sqrt(g0² + g1² + g2²)`

Fallback: If no valid triplet (degenerate geometry or below-horizon), use nearest speaker by dot product.

Determinant threshold: skip triplets with `|det| < 0.01` (near-collinear speakers).

**Best for:** Standard surround layouts with symmetric speaker coverage. Sharpest localization of all algorithms.

**Known limitation:** Zenith centering problem above ~60° elevation when highest physical speakers are at +45°. Solution (not yet implemented): imaginary zenith speaker with `1/sqrt(M)` downmix to top ring (Pulkki's documented approach).

### 1.3 VBIP — Vector Base Intensity Panning

Intensity-weighted variant of VBAP. Steps:
1. Compute standard VBAP gains (2D or 3D)
2. Square all gains: `g_i = g_i²`
3. Re-normalize to constant power: `scale = 1 / sqrt(Σ g_i²)`

**Effect:** Tighter spatial focus by de-emphasizing distant speakers. Intensity proportional to squared amplitude. Perceptually sharper than VBAP — useful for discrete spatial events (delay taps).

### 1.4 KNN — K-Nearest Neighbor Panning (K=3)

1. Convert source and all speakers to unit Cartesian vectors
2. Compute angular distance for each speaker: `dist = acos(dot(source, speaker))`
3. Partial sort to find K=3 nearest speakers
4. Early return if source within ~0.001 rad (~0.06°) of any speaker → gain=1.0
5. Inverse-distance-squared weighting: `w_i = 1 / (dist_i² + ε)`, ε=1e-6
6. Normalize weights: `g_i = w_i / Σ w_i`
7. Constant-power re-normalization

**Best for:** Irregular/non-standard speaker layouts where VBAP triangulation is ill-conditioned. Graceful zenith handling (no degenerate triplets). Produces more diffuse images than VBAP (activates 3 speakers always).

### 1.5 Ambisonics — 3rd-Order HOA (ACN/SN3D)

**Encoding:** For source at (az, el), compute 16 SH coefficients:
```
shCoeffs[acn] = evalSH(acn, az, el) * maxrE[order(acn)]
```
Max-rE weights (Zotter & Frank 2012): `[1.0, cos(π/8), cos(2π/8), cos(3π/8)]`

**Decoding to speakers:** Tikhonov-regularized pseudo-inverse of encoding matrix:
```
E[spk][ch] = evalSH(ch, spk_az, spk_el)   // encoding matrix
D = E^T (E E^T + ε I)^{-1}                  // decode matrix, ε=0.01
speaker_gains = D × shCoeffs                 // per-speaker gains
```
Gauss-Jordan elimination with partial pivoting for matrix inversion.

**Minimum speakers:** (N+1)² = 16 for 3rd order. Fewer speakers → truncated decode (lower effective order).

**Best for:** Regular (ideally spherical) speaker arrays, head-tracked VR playback. Rotation-invariant encoding — source count independent of decode complexity.

**Reference:** Daniel, J. (2000). "Representation de champs acoustiques," PhD thesis.

### 1.6 DBAP — Distance-Based Amplitude Panning (Lossius et al., ICMC 2009)

1. Convert source and speakers to 3D Cartesian
2. Compute Euclidean distance from source to each speaker
3. Weight = 1 / distance² (inverse-square law, 6 dB per doubling)
4. Normalize to constant power

**Best for:** Arbitrary non-standard layouts with no sweet spot assumption. Concert installations, art installations, experimental speaker deployments. Works with any speaker placement — no triangulation, no regularity assumption.

### 1.7 MDAP — Multiple-Direction Amplitude Panning (Pulkki 2000)

VBAP with source spread for wider spatial images:
1. For desired direction D, place 8 auxiliary sources on a ring around D on the unit sphere
2. Compute VBAP gains for each auxiliary source independently
3. Sum all gain vectors and normalize to constant power

**Effect:** Activates more speakers → wider, more stable spatial image than point-source VBAP. Mitigates (but doesn't eliminate) zenith centering problem through spread ring.

**Spread radius:** Currently fixed. Future: user-controllable parameter.

### 1.8 Direct Binaural — Woodworth ITD+ILD (Internal Only)

Simplified binaural model for "Simple (Low CPU)" monitoring:

**ITD (Interaural Time Difference):**
```
lateral = sin(az) * cos(el)
t = (r/c) * (|lateral| + asin(|lateral|))
```
where r = head radius (0.0875–0.0920m per profile), c = 343 m/s.

**ILD (Interaural Level Difference):**
```
ildDb = ildScale * 8 * |lateral|
farEarGain = dBtoLinear(-ildDb)
```
Broadband — no spectral coloring from pinnae. No elevation cues.

**Distance attenuation:** `distGain = 1 / max(0.1, distance * 4 + 0.25)`

Not in user-facing dropdown. Used only when HRTF Profile = Simple (index 0).

---

## 2. Binauralization Pipelines

Three rendering paths, dispatched by output format and HRTF profile selection:

### Path A: Direct Per-Source HRTF (Binaural output, HRTF profiles 1-5)

```
For each enabled source (tap):
  1. Read delay line at tap position → mono sample
  2. Accumulate into per-source buffer (block-sized)
  3. If source position changed >1°: update HRIR via libmysofa KD-tree lookup
  4. Convolve source buffer with per-source PartitionedConvolver (L and R independently)
  5. Sum all convolver outputs → wet L/R
```

12 independent per-source PartitionedConvolvers. HRIR loaded at source's exact 3D direction — no virtual speaker intermediary. This is the highest-quality binaural path.

### Path B: Simple Woodworth (Binaural output, profile 0)

```
For each sample:
  For each enabled source:
    1. Read delay tap → mono
    2. Compute Woodworth ITD+ILD → L/R gains + delay offset
    3. Apply to mono signal → accumulate into L/R
```

Per-sample computation, no convolution. Lowest CPU. No spectral coloring → poor elevation cues.

### Path C: Discrete Surround (>2ch output)

```
For each sample:
  For each enabled source:
    1. Read delay tap → mono
    2. algorithm.computeGains(source, layout) → speaker gains
    3. Multiply mono × gain → accumulate into physical channel buffers
  LFE: mono wet sum → 120Hz LP (2nd-order Butterworth) → -10dB → LFE channel
```

No HRTF. User selects algorithm (Constant Power/VBAP/VBIP/KNN/Ambisonics/DBAP/MDAP).

### Algorithm-to-Pipeline Selection Matrix

| Output Format | HRTF Profile | Pipeline | Algorithm Used |
|--------------|-------------|----------|----------------|
| Binaural (2ch) | 1-5 (SOFA) | A: Per-source HRTF | N/A (direct) |
| Binaural (2ch) | 0 (Simple) | B: Woodworth | Direct Binaural |
| Stereo (2ch) | — | Stereo variant modes | 5 mic simulations |
| Surround (>2ch) | — | C: Discrete | User-selected |
| Ambisonics (>2ch) | — | C: Discrete | Ambisonics Encode |

---

## 3. HRTF Convolution Architecture

### 3.1 Partitioned FFT Convolution (Overlap-Save)

```
FFT size = smallest power-of-2 >= blockSize + irLen - 1
Preparation: IR → zero-pad to FFT size → forward FFT → store frequency domain
Per block:
  1. Accumulate input into ring buffer (blockSize samples)
  2. Forward FFT of input buffer
  3. Complex multiply: output_freq = input_freq × ir_freq
  4. Inverse FFT → time domain
  5. Extract valid samples (overlap-save) → add to output
```

Uses `juce::dsp::FFT` for transforms. Work buffers: `inputAccum`, `fftWorkBuf`, `overlapBuf`, `irFreqDomain`.

### 3.2 SOFA Database Loading (libmysofa v1.3.2)

- `mysofa_open_data()` for in-memory SOFA parsing (binary resource embedded)
- Automatic resampling to session sample rate
- `mysofa_getfilter_float()` for nearest-neighbor HRIR lookup via internal KD-tree
- `mysofa_loudness()` normalizes frontal HRIR to sumOfSquares=2.0 (off-axis relative energy preserved)
- Cartesian coordinate system internally (`mysofa_tocartesian()` called automatically)

### 3.3 Cross-Profile Normalization

Different SOFA datasets have different off-axis energy levels. Self-calibrating normalization:

```
targetRMS = 1 / sqrt(irLen)                    // frontal level after mysofa_loudness
avgRMS = sqrt(totalEnergy / (numSpeakers * 2 * irLen))  // speaker-averaged RMS
normGain = targetRMS / avgRMS                   // <1 for hot datasets, ~1 for neutral
```

Applied to both speaker-domain and SH-domain convolvers. Prevents SADIE D2 and HUTUBS PP2 (higher off-axis energy) from over-amplifying.

### 3.4 SH-Projected HRIRs (Ambisonics Path)

For Ambisonics SH-domain HRTF convolution, project all SOFA measurement HRIRs onto SH basis:

```
SH_IR[c][n] = (2l+1)/M * Σ_m { Y_c^SN3D(dir_m) * HRIR_m[n] }
```

- `(2l+1)/M` is the correct weight for SN3D normalization (NOT `4π/M` which is N3D)
- Iterates over all M measurement positions in the SOFA file
- Produces 16 SH-channel HRIRs (one per HOA channel) for L and R ears independently
- Assumes approximately uniform measurement distribution (small bias for non-uniform grids)

### 3.5 HRTF Profiles

| Index | Name | Source | License | Character |
|-------|------|--------|---------|-----------|
| 0 | Simple (Low CPU) | Woodworth model | N/A | No spectral coloring |
| 1 | Studio Reference | MIT KEMAR Large Pinna | MIT-style | Neutral, clinical |
| 2 | Immersive | SADIE II D2 KU100 | Apache 2.0 | Warm, enveloping |
| 3 | Natural | CIPIC Subject003 | Public Domain | Organic, natural |
| 4 | Precise | HUTUBS PP2 | CC BY 4.0 | Detailed, analytical |
| 5 | Spatial | Bernschuetz KU100 2° | CC BY 3.0 | Wide, spacious |

---

## 4. Virtual Speaker Deployment

16 virtual speakers used for binaural rendering when algorithms compute speaker gains (VBAP/VBIP/KNN in binaural mode):

```
Ear level ring (9 speakers, 0° elevation):
  C(0°)  L(30°)  R(-30°)  Lw(60°)  Rw(-60°)  Ls(90°)  Rs(-90°)  Lrs(135°)  Rrs(-135°)

Top ring (6 speakers, +45° elevation):
  Tfl(45°,45°)  Tfr(-45°,45°)  Tsl(90°,45°)  Tsr(-90°,45°)  Trl(135°,45°)  Trr(-135°,45°)

Zenith (1 speaker, +90° elevation):
  T(0°,90°)
```

**Rationale:** 9.1.6 Atmos layout plus zenith. 16 total = minimum for well-conditioned 3rd-order HOA decode matrix ((3+1)² = 16). Zenith included specifically for Ambisonics mathematical completeness.

**VBAP triplets:** 30 pre-computed triangulations covering upper hemisphere. Each triplet stores speaker indices {i,j,k} and pre-computed 3x3 inverse matrix. Determinant threshold 0.01 for degenerate geometry.

**Below-horizon handling:** No virtual speakers below 0° elevation. Below-horizon sources projected to nearest ear-level speaker via dot product fallback. Perceptually acceptable — below-horizon localization is inherently poor (Blauert 1997, "cone of confusion").

**Physical surround layouts:**
- Quad: 4 speakers at ±30°, ±110° (all ear level)
- 5.1: L(30°) R(-30°) C(0°) Ls(110°) Rs(-110°) + LFE
- 7.1: L(30°) R(-30°) C(0°) Lss(90°) Rss(-90°) Lsr(135°) Rsr(-135°) + LFE
- 7.1.4: 7.1 + Tfl(45°,45°) Tfr(-45°,45°) Trl(135°,45°) Trr(-135°,45°)
- 9.1.6: 7.1 + Lw(60°) Rw(-60°) + Tfl Tfr Tsl Tsr Trl Trr (all +45° elevation)
- Octaphonic: 8 speakers at 45° intervals, no LFE

All per ITU-R BS.775 / BS.2051 standard positions.

---

## 5. Coordinate Systems

### 5.1 ADM Convention (Used Throughout Plugin)

```
Azimuth:   0° = front,  +90° = left,  -90° = right,  ±180° = back
Elevation: 0° = ear level,  +90° = zenith,  -90° = nadir
Distance:  0 = origin (head center),  1 = maximum
```

Positive azimuth = LEFT. This matches ADM (ITU-R BS.2076), IEM StereoEncoder, SPAT Revolution, and most spatial audio tools.

### 5.2 Cartesian Convention (ITU-R BS.2127-0)

Used by ADM-OSC for object position messages:
```
x: +1 = right,  -1 = left
y: +1 = front,  -1 = back
z: +1 = up,     -1 = down
```

### 5.3 Conversion Formulas

**Cartesian → Polar (ITU-R BS.2127-0 Section 10.1):**
```cpp
azDeg  = atan2(-x, y) * (180.0f / pi);       // CRITICAL: -x, not x
elDeg  = atan2(z, sqrt(x*x + y*y)) * (180.0f / pi);
dist   = clamp(0, 1, sqrt(x*x + y*y + z*z));
```

The `atan2(-x, y)` negation is essential: standard Cartesian has positive-x = right, but ADM has positive azimuth = left. Without the negation, left/right would be swapped.

**Polar → Cartesian (for algorithm internals):**
```cpp
px = cos(elRad) * sin(azRad);   // lateral (positive = left)
py = cos(elRad) * cos(azRad);   // frontal (positive = front)
pz = sin(elRad);                // vertical (positive = up)
```

**SOFA coordinate handling:**
libmysofa converts SOFA SourcePosition to Cartesian internally via `mysofa_tocartesian()`. To convert back to spherical for SH evaluation:
```cpp
azRad = atan2(y, x);            // libmysofa Cartesian: x=front, y=left
elRad = asin(clamp(-1, 1, z / r));
```

### 5.4 UI Spatial Map

- 0° azimuth = top of map (front of listener)
- Clockwise rotation on map = clockwise in real space
- `ReverseSlider` on azimuth knob ensures: clockwise knob turn = clockwise movement on map
- Matches IEM StereoEncoder convention

---

## 6. ADM-OSC Protocol

### 6.1 Overview

Audio Definition Model via Open Sound Control — industry standard for real-time object-based audio positioning. Developed by L-Acoustics, FLUX:: Immersive, Radio France with contributions from BBC, Dolby, d&b audiotechnik, DiGiCo, Lawo, Magix, Merging Technologies, Meyer Sound, Sound Particles, Steinberg.

Enables interoperability with: SPAT Revolution, L-ISA Controller, Dolby Atmos Renderer, SpaceMap Go, QLAB 5, Nuendo, dearVR.

**Standards:** ITU-R BS.2076-2 (Audio Definition Model), ITU-R BS.2127-0 (coordinate conversion), OSC 1.0 (Wright 2002, CNMAT).

### 6.2 Message Namespace

Pattern: `/adm/obj/N/property` where N = 1-based object number (1-12 for 12 delay taps).

**Polar position messages:**

| OSC Address | Type | Unit | Range |
|-------------|------|------|-------|
| `/adm/obj/N/azim` | float | degrees | -180 to +180 |
| `/adm/obj/N/elev` | float | degrees | -90 to +90 |
| `/adm/obj/N/dist` | float | normalized | 0 to 1 |
| `/adm/obj/N/aed` | float[3] | az,el,dist | see above |

**Cartesian position messages:**

| OSC Address | Type | Unit | Range |
|-------------|------|------|-------|
| `/adm/obj/N/x` | float | normalized | -1 to +1 |
| `/adm/obj/N/y` | float | normalized | -1 to +1 |
| `/adm/obj/N/z` | float | normalized | -1 to +1 |
| `/adm/obj/N/xyz` | float[3] | x,y,z | -1 to +1 |

**Future (spec-defined, not yet implemented):**

| OSC Address | Type | Range | Purpose |
|-------------|------|-------|---------|
| `/adm/obj/N/gain` | float | 0 to 1 | Linear gain (1.0 = unity) |
| `/adm/obj/N/w` | float | 0 to 1 | Width / extent of object |

### 6.3 Transport & Connection

- Protocol: UDP
- Default receive port: 4002 (user-configurable, persisted in state XML)
- Default send port: 4001 (future: ADM-OSC Send)
- Host: 127.0.0.1 (localhost)
- JUCE implementation: `juce::OSCReceiver` with `MessageLoopCallback` listener
- Thread safety: receives on message thread, updates APVTS via `setValueNotifyingHost()` (thread-safe)

### 6.4 Override Mechanism

When OSC messages arrive for an object:
1. Set `oscOverrideActive[objIdx] = true` with timestamp
2. Trajectory animation pauses for that object
3. 60Hz timer checks: if last OSC message > 500ms ago, release override
4. Trajectory animation resumes automatically

Allows smooth handoff: external OSC control takes priority; when controller stops sending, internal trajectory resumes after 500ms timeout.

### 6.5 Partial Cartesian Updates

Individual `/x`, `/y`, `/z` messages update cached Cartesian values (`oscCartesianX/Y/Z[objIdx]`) and immediately recompute polar coordinates via the ITU-R BS.2127-0 conversion. Controllers can send single-axis updates without requiring a complete `/xyz` bundle.

### 6.6 Testing Tool

`scripts/adm_osc_test.py` — Python CLI using `python-osc` library. 7 test modes:
- `--manual` — Single AED message
- `--orbit` — Continuous azimuth orbit
- `--sweep` — Elevation sweep -90° to +90°
- `--multi` — Coordinated spiral of all 12 objects
- `--xyz` — Cartesian position update
- `--aed` — Combined AED message
- `--axes` — Individual /x, /y, /z updates

Configurable: `--port` (default 4002), `--host` (default 127.0.0.1), `--rate` (default 60Hz).

---

## 7. Spherical Harmonics Reference

### 7.1 ACN Ordering (Ambisonic Channel Number)

Channel index = n² + n + m, where n = order, m = degree (-n ≤ m ≤ n).

```
Order 0 (1 ch):  [0] W    — omnidirectional
Order 1 (3 ch):  [1] Y    [2] Z    [3] X     — dipoles
Order 2 (5 ch):  [4] V    [5] T    [6] R    [7] S    [8] U
Order 3 (7 ch):  [9] Q   [10] O   [11] M   [12] K   [13] L   [14] N   [15] P
```

Total: (N+1)² channels. 3rd order → 16 channels.

### 7.2 SN3D Normalization

Semi-Normalized 3D. Orthogonality relation:
```
∫ |Y_c^SN3D(θ,φ)|² dΩ = 4π / (2l + 1)
```

Analysis weight for SH projection of discrete measurements:
```
weight = (2l + 1) / M
```
where l = order of channel c, M = number of measurement positions.

**Critical:** This is `(2l+1)/M`, NOT `4π/M` (which would be correct for N3D normalization). Using the wrong weight introduces order-dependent level errors.

### 7.3 Max-rE Weighting (Zotter & Frank 2012)

Reduces SH side-lobes for improved perceptual localization:

| Order | Weight | Value |
|-------|--------|-------|
| 0 | 1.0 | 1.000 |
| 1 | cos(π/8) | 0.924 |
| 2 | cos(2π/8) | 0.707 |
| 3 | cos(3π/8) | 0.383 |

Applied multiplicatively to SH coefficients during encoding.

### 7.4 evalSH() Basis Functions

Real spherical harmonics evaluated per ACN channel. Standard formulas:
- ACN 0: `Y₀⁰ = 1` (omnidirectional)
- ACN 1: `Y₁⁻¹ = sin(az)cos(el)` (Y, left-right)
- ACN 2: `Y₁⁰ = sin(el)` (Z, up-down)
- ACN 3: `Y₁¹ = cos(az)cos(el)` (X, front-back)
- Higher orders: standard associated Legendre polynomials with SN3D normalization

---

## 8. Real-Time DSP Constraints

### 8.1 Lock-Free Audio Callback

`processBlock()` must NEVER:
- Allocate memory (malloc, new, vector resize)
- Acquire locks (mutex, spinlock, critical section)
- Log or print (DBG, std::cout, juce::Logger)
- Perform string operations (juce::String construction, APVTS getParameter by name)
- Call system APIs that may block (file I/O, network)

### 8.2 Optimization Patterns

- **Power-of-2 delay buffer:** Size rounded up to next power of 2. Index via `& mask` instead of `% size` — eliminates integer division in hot path.
- **Cached parameter pointers:** `std::atomic<float>*` arrays initialized at construction. Eliminates per-sample APVTS string lookups (72+ lookups eliminated).
- **Position-change gating:** Track previous object positions. Skip Doppler calculation, air absorption filter updates, and HRIR reloading when position is static. Saves ~0.5ms per static object.
- **LinearSmoothedValue:** 100ms ramp for parameter changes. Prevents clicks on abrupt changes.

### 8.3 Numerical Safety

- **NaN guard:** `if (!std::isfinite(feedbackSample)) { feedbackSample = 0; filters.reset(); }`
- **Soft clipper:** `f(x) = threshold + (x - threshold) / (1 + (x - threshold)²)`, threshold=0.8. Applied on delay write and feedback output. Prevents runaway in self-oscillation.
- **Cubic Hermite interpolation:** Catmull-Rom on delay line reads. 4-point interpolation for high-frequency preservation. Buffer margin of 2 samples maintained.
- **Delay bounds:** `objDelaySamples = jlimit(1.0f, bufferSize - 2, ...)` prevents out-of-bounds reads.

### 8.4 Double-Buffer Pattern for Layout Changes

Speaker layout changes (output format switch) use atomic double-buffer:
1. Prepare new layout in inactive buffer (message thread)
2. `activeLayoutIndex.store(newIndex, memory_order_release)`
3. Audio thread reads with `memory_order_acquire`
4. No lock needed — release/acquire barrier ensures visibility

---

## 9. Build System

### 9.1 Configuration

- Framework: JUCE 8.0.3 (git submodule at `JUCE/`)
- Language: C++17
- Build: CMake 3.22+
- Dependencies: libmysofa v1.3.2 (FetchContent), zlib (system macOS, vcpkg Windows)
- HRTF data: 5 SOFA files in `HRTF/` embedded as binary resources (Git LFS tracked)

### 9.2 macOS Build

```bash
export PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:$PATH"
cd Project/OpenSpatialDelay
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Post-build auto-install:
- AU → `~/Library/Audio/Plug-Ins/Components/`
- VST3 → `~/Library/Audio/Plug-Ins/VST3/`

### 9.3 Windows Build

GitHub Actions CI on push to `main`. Uses Visual Studio 17 2022 x64, vcpkg for zlib.
Download: `bash scripts/download_windows_build.sh` → `build/windows/`

---

## 10. Anti-Patterns

| Anti-Pattern | Correct Approach |
|-------------|-----------------|
| Allocating memory in processBlock | Pre-allocate all buffers in prepareToPlay |
| String lookups in audio callback | Cache `std::atomic<float>*` at construction |
| Loading HRTF on audio thread | Background thread + lock-free pointer swap |
| Missing constant-power normalization | Always `scale = 1/sqrt(Σ g²)` after gain computation |
| Using `4π/M` weight for SN3D SH projection | Correct: `(2l+1)/M` |
| Nearest-neighbor HRTF without interpolation | Produces zipper artifacts on moving sources |
| Squaring VBIP gains without renormalization | Must renormalize after squaring |
| ILD-only binaural (no ITD) | Produces lateralization, not true spatialization |
| `atan2(x, y)` for ADM Cartesian→Polar | Must use `atan2(-x, y)` for left-positive convention |
| OSC handling on audio thread | Use message thread callback for APVTS safety |
| Modifying filter coefficients per-sample | Compute at block rate, interpolate per-sample |

---

## 11. Performance Specifications

| Operation | Typical Cost | Implementation |
|-----------|-------------|----------------|
| HRTF convolution (per source) | ~2ms | Partitioned FFT overlap-save |
| Ambisonics SH encode (per source) | ~0.1ms | Direct eval, 16 channels |
| VBAP 3D triplet search (per source) | ~0.01ms | 30 triplets, matrix multiply |
| KNN search K=3 (per source) | ~0.005ms | Partial sort + inverse-distance |
| DBAP gain computation (per source) | ~0.003ms | Euclidean distance + normalize |
| processBlock total (12 sources) | <5ms @ 512 samples | Lock-free, pre-allocated |
| HRIR update (position change) | ~0.5ms | KD-tree lookup + IR copy |
| Soft clipper (per sample) | ~2ns | Branchless polynomial |

Audio allocation budget: 5-10% of frame time. At 44.1kHz with 512 sample blocks: ~11.6ms frame budget → processBlock must complete in <5ms.
