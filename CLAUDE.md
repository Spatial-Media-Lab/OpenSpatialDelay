# CLAUDE.md — OpenSpatialDelay

## Build & Test Rules

### Versioning: Major.Minor.Bugfix (issue #55)

OpenSpatialDelay uses **semantic versioning vX.Y.Z**:
- **X (Major):** Breaking changes, new architecture, incompatible preset format
- **Y (Minor):** New features, new output formats, new algorithms
- **Z (Bugfix):** Bug fixes, threshold tweaks, documentation updates

**Current release: v1.0.0** (pre-release, shipping end of week 2026-04-10)

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
- v1.0.1 / O101 — fix dry signal attenuation at 0% wet (issue #97, commit a0f39c5)
- v1.0.2 / O102 — call updateHostDisplay() on config changes (issue #94, commit dbf19ce)
- v1.0.3 / O103 — fix direction toggle for Bounce, Line, Random (issue #100, commit 9dd6266)
- v1.0.5 / O105 — reset phase vocoder on preset change chirp (issue #99, commit a9df52a)
- v1.0.6 / O106 — transport fade-in to prevent scrub/seek click (issue #103, commit 7901a0a)
- v1.0.8 / O108 — fix Ableton crash + automation reset (issue #122, commit 0d3cc53)
- v1.0.9 / O109 — re-sign bundles after plist patching for Ableton VST3 visibility (issue #120)
- v1.0.10 / O110 — reduce automatable params to 64 for Ableton auto-populate (issue #122, commit 4adfd1d)
- v1.0.7 / O107 — 4-layer tape wobble emulation (issue #92, commit 893acb2)
- v1.0.14 / O114 — shared LookAndFeel + visibility throttle for multi-instance crash (issue #131)
- v1.0.17 / O117 — shared FFT cache for multi-instance vDSP stability (issue #131, commit e8ee6ec)
- v1.0.20 / O120 — default Input to Stereo on stereo tracks, disable on mono (issue #155, commit 376b5fe)
- Next available: **v1.0.21 / O121**

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

## Licensing & Monetization

### Dual-license model
OpenSpatialDelay is dual-licensed: **GPL-3.0** (free/open-source) and **Commercial License** (proprietary/closed-source, via Spatial Media Lab). Both paths must be compatible with every distributed third-party component.

### JUCE 8 commercial tiers (as of April 2026)

| Tier | Revenue Cap | Cost | Closed-Source OK |
|------|-------------|------|------------------|
| Starter | $20,000/year | Free | Yes |
| Indie | $300,000/year | $40/mo or $800 perpetual | Yes |
| Pro | No limit | $175/mo or $3,500 perpetual | Yes |

**Starter tier allows free commercial distribution** with no splash screen and no attribution required, as long as total entity revenue stays under $20K/year. Upgrade to Indie at $20K+, Pro at $300K+.

Source: [JUCE 8 EULA](https://juce.com/legal/juce-8-licence/) and [juce.com/get-juce](https://juce.com/get-juce/)

### Third-party license compatibility (verified 2026-04-05)

All distributed third-party components are compatible with both GPL-3.0 and commercial licensing:

| Component | License | Commercial Use | Obligation |
|-----------|---------|----------------|------------|
| JUCE 8 | GPL-3.0 / Commercial | Yes (Starter tier free < $20K) | Upgrade tier at revenue thresholds |
| libmysofa v1.3.2 | BSD-3-Clause | Yes | Include license notice |
| zlib | zlib License | Yes | Include license notice |
| MIT KEMAR (Studio Reference) | MIT | Yes | Include copyright + permission notice |
| SADIE II D2 KU100 (Immersive) | Apache 2.0 | Yes | Include license notice |
| CIPIC Subject 003 (Natural) | Public Domain | Yes | None |
| HUTUBS PP2 (Precise) | CC BY 4.0 | Yes | Attribution required |
| Bernschuetz KU100 (Spatial) | CC BY 3.0 | Yes | Attribution required |
| DM Sans | SIL OFL 1.1 | Yes | Cannot sell font standalone |
| JetBrains Mono | SIL OFL 1.1 | Yes | Cannot sell font standalone |
| Roboto | Apache 2.0 | Yes | Include license notice |

All attribution obligations are fulfilled by the legal notices document at `docs/OpenSpatialDelay_Legal_Notices.docx` (generated by `docs/generate_legal_notices.js`) and the Third-Party Notices chapter in the user manual.
