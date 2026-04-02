# CLAUDE.md — OpenSpatialDelay

## Build & Test Rules

### Versioning: Major.Minor.Bugfix (issue #55)

OpenSpatialDelay uses **semantic versioning vX.Y.Z**:
- **X (Major):** Breaking changes, new architecture, incompatible preset format
- **Y (Minor):** New features, new output formats, new algorithms
- **Z (Bugfix):** Bug fixes, threshold tweaks, documentation updates

**Current release: v1.0.0** (commit 3ef9c79, 2026-04-02)

All prior bugfix builds (v1.0.1–v1.0.2 for issues #76, #77) have been collapsed into v1.0.0. The next bugfix build will be v1.0.1.

### MANDATORY: Versioned builds for every fix attempt

Every code change that needs manual listening/testing MUST be built as a uniquely-named versioned plugin. **Never reuse a version number. Never skip this step.**

```bash
# 1. Commit your changes
git add ... && git commit -m "..."

# 2. Build as a versioned plugin (increment Z each time)
bash scripts/build_version.sh <commit-hash> v1.0.Z O10Z
```

**Why this is critical:** The default `cmake --build` installs to `OpenSpatialDelay v1.0.component`. The user tests with individually-named versioned plugins (e.g., `OpenSpatialDelay v1.0.1.component`) loaded side-by-side in REAPER for A/B comparison. If you don't use `build_version.sh`, the user will never hear your changes.

**Version number registry (do not reuse) — reset 2026-04-02, collapsed after issue #77:**
- v1.0.0 / O100 — baseline (commit 3ef9c79, includes issues #76 + #77)
- Next available: **v1.0.1 / O101**

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

### PartitionedConvolver
Uses spectral envelope EMA smoothing: magnitude and phase smoothed separately per frequency bin. This prevents comb filtering from phase-misaligned time-domain blending.

### Phase Vocoder Pitch Shifter
Replaces WSOLA-Lite. Uses STFT (2048-point FFT, 4x overlap) with:
- Laroche-Dolson phase locking for tonal content
- Röbel-style spectral flux transient detection with adaptive median threshold
- Phase reset on transient frames preserves attack sharpness
- Latency: 2048 samples (reported to DAW via setLatencySamples)
- Range: ±12 semitones (combined with Doppler)

### Dry Path Latency Compensation (v1.0.7) + Stereo Dry (v1.0.1)
The phase vocoder adds 2048 samples of latency to the wet path. The dry signal must be delayed by the same amount so the DAW's plugin delay compensation (PDC) is correct at all dry/wet settings. Implemented as stereo circular `dryDelayLineL/R` buffers that pre-fill `dryCompBufferL/R` from raw DAW input at the start of each processBlock.

The dry/wet mix happens in a single post-render stage in processBlock — render paths output raw wet signal only. This ensures the dry signal truly bypasses the entire plugin (Input selector only affects the wet path). Equal-power crossfade (cos/sin) replaces linear (1-dw/dw) for constant perceived loudness at all mix settings.

### ITD delay line
For MIT KEMAR SOFA file, ITD values are always 0 (embedded in HRIR waveform). The ITD delay line is effectively a pass-through for this dataset.
