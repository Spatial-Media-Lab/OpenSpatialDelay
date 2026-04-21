# OpenSpatialDelay

## Why
Spatial delay plugin — each echo lives in 3D space. VST3 + AU, macOS + Windows. Licensed under GPL-3.0. See LICENSE for full text.

## What
- `Source/` — plugin processor, editor, DSP (PluginProcessor.cpp is ~175KB)
- `Tests/` — Catch2 test suite (162+ tests)
- `HRTF/` — 5 SOFA files for binaural rendering (the plugin exposes 6 binauralization options: these 5 profiles + a CPU-lite Woodworth ITD/ILD fallback that uses no SOFA file)
- `scripts/` — build_version.sh, install helpers
- `docs/` — user manual, legal notices, assets
- `fonts/` — DM Sans, JetBrains Mono, Roboto (SIL OFL / Apache 2.0)
- `JUCE/` — JUCE 8.0.3 framework (submodule)

## How

Build (configure once):
```
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev
```

Build and run tests:
```
cmake --build build --target OpenSpatialDelayTests -j$(sysctl -n hw.ncpu)
./build/OpenSpatialDelayTests
```

IMPORTANT: Build versioned plugins for every fix attempt — never use plain `cmake --build`:
```
bash scripts/build_version.sh <commit-hash> v1.0.Z O10Z
```
The user A/B-tests named plugins side-by-side in REAPER. Without `build_version.sh`, your changes are never heard. See `agent_docs/version_registry.md` for the current registry and next available version number.

## Conventions
- Semantic versioning vX.Y.Z. Never reuse a version number.
- VST3 + AU only. No other formats.
- No `sudo`. No `killall AudioComponentRegistrar` (destroys AU cache — real incident).
- Stay on the same version number while iterating a single fix; increment only for new builds sent to user.

## Gotchas
- **HRTF tests need synchronous loading.** Use `testLoadHRTFProfile()` or `createBinauralProcessor()`. Without this, the timer thread never fires in the test harness and binaural tests silently pass in Simple mode (false positives).
- **Dry path has 2048-sample latency compensation.** The phase vocoder's latency must match the dry delay line (`dryDelayLineL/R`). If you change FFT size or overlap, update both paths or PDC breaks at all dry/wet ratios.
- **ITD delay line is a pass-through for MIT KEMAR.** ITD values are 0 in this SOFA file (embedded in HRIR waveform). Don't assume ITD is active.

## Progressive disclosure
For task-specific context, read the relevant file first:
- `agent_docs/version_registry.md` — version number registry, next available number, build naming rules
- `agent_docs/architecture.md` — signal flow, HRTF rendering, PartitionedConvolver, Phase Vocoder, dry path compensation
- `agent_docs/licensing.md` — JUCE tiers, third-party license table, attribution obligations
