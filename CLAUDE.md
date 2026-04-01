# CLAUDE.md — OpenSpatialDelay

## Build & Test Rules

### MANDATORY: Versioned builds for every fix attempt

Every code change that needs manual listening/testing MUST be built as a uniquely-named versioned plugin. **Never reuse a version number. Never skip this step.**

```bash
# 1. Commit your changes
git add ... && git commit -m "..."

# 2. Build as a versioned plugin (increment X each time)
bash scripts/build_version.sh <commit-hash> v1.0.X O10X
```

**Why this is critical:** The default `cmake --build` installs to `OpenSpatialDelay v1.0.component`. The user tests with individually-named versioned plugins (e.g., `OpenSpatialDelay v1.0.4.component`) loaded side-by-side in REAPER for A/B comparison. If you don't use `build_version.sh`, the user will never hear your changes.

**Version number registry (do not reuse) — reset 2026-03-25, see issue #55:**
- v1.0.0 / O100 — baseline (commit 023670a)
- v1.0.1 / O101 — dual-convolver output crossfade + feedback smoothing (commit 99d1525)
- v1.0.2 / O102 — smootherstep crossfade + 6-block duration, 195/195 pass (commit 44185d4)
- v1.0.3 / O103 — WSOLA float precision fix + 5 bug fixes + doubled buffers (commit 19ddb28)
- v1.0.4 / O104 — spectral envelope EMA smoothing
- v1.0.5 / O105 — feedback EMA smoothing for HRTF crossfade pops
- v1.0.6 / O106 — Phase vocoder pitch shifter replacing WSOLA-Lite (issue #60)
- v1.0.7 / O107 — dry path latency compensation + unique CFBundleIdentifier per build (issues #63, #62)
- v1.0.8 / O108 — transport-aware PV reset for intermittent buzzing (issue #65)
- v1.0.9 / O109 — AU parameter ordering via unique version hints + blank row fix (issue #68)
- v1.0.10 / O110 — hide 4 config params from DAW automation list (issue #68)
- Next available: **v1.0.11 / O111**

### Running tests

```bash
# Configure (first time or after clean)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev

# Build and run tests
cmake --build build --target OpenSpatialDelayTests -j$(sysctl -n hw.ncpu)
./build/OpenSpatialDelayTests

# HRTF tests require synchronous profile loading (testLoadHRTFProfile)
# The timer thread doesn't fire in the test harness
```

### Test infrastructure notes

- All HRTF binaural tests use `testLoadHRTFProfile()` for synchronous HRTF loading
- Without this, the timer-based loading never fires and tests run in Simple mode (false positives)
- The `createBinauralProcessor()` helper handles this automatically

## Architecture Notes

### HRTF rendering signal flow
1. `renderDirectBinauralHRTF()` — 3-pass architecture
2. Pass 1: per-sample delay engine → per-source mono accumulation
3. Pass 2: per-block HRTF convolution via `BinauralRenderer::renderSourceBuffers()`
4. Pass 3: per-sample dry/wet mix + output gain

### PartitionedConvolver (v1.0.4)
Uses spectral envelope EMA smoothing: magnitude and phase smoothed separately per frequency bin. This prevents comb filtering from phase-misaligned time-domain blending.

### Phase Vocoder Pitch Shifter (v1.0.6)
Replaces WSOLA-Lite. Uses STFT (2048-point FFT, 4x overlap) with:
- Laroche-Dolson phase locking for tonal content
- Röbel-style spectral flux transient detection with adaptive median threshold
- Phase reset on transient frames preserves attack sharpness
- Latency: 2048 samples (reported to DAW via setLatencySamples)
- Range: ±12 semitones (combined with Doppler)

### Dry Path Latency Compensation (v1.0.7)
The phase vocoder adds 2048 samples of latency to the wet path. The dry signal must be delayed by the same amount so the DAW's plugin delay compensation (PDC) is correct at all dry/wet settings. Implemented as a circular `dryDelayLine` buffer that pre-fills `dryCompBuffer` at the start of each processBlock, before dispatch to render methods. All 5 render paths read from `dryCompBuffer` instead of `monoInputBuffer` for dry mixing.

### ITD delay line
For MIT KEMAR SOFA file, ITD values are always 0 (embedded in HRIR waveform). The ITD delay line is effectively a pass-through for this dataset.
