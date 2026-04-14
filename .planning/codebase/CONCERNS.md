# Codebase Concerns

**Analysis Date:** 2026-04-14

## Critical Bugs

### Null Pointer Crash in updateSyncUI Lambda (FIXED in v1.0.36 / Issue #200)

**Status:** Fixed but represents a pattern risk

**Issue:** Constructor-time lambda in `PluginEditor.cpp` captures `this` and directly dereferences `syncDottedButton` and `syncTripletButton` unique_ptrs without null-guard. If editor construction fails partway through (e.g., APVTS not yet created), the lambda fires before unique_ptrs are allocated, causing null deref.

**Files:** 
- `Source/PluginEditor.cpp` (updateSyncUI lambda, ~line 1100–1200 region)
- `Source/PluginEditor.h` (member declarations)

**Trigger:** Rapid editor instantiation or early destruction during setup

**Current mitigation:** Added null checks in v1.0.36 commit cd5f908

**Remaining risk:** This pattern (constructor lambdas with deferred allocation) appears in multiple places. Any new editor feature using similar setup is at risk.

**Safe modification:**
- Always null-guard `unique_ptr` dereferences in constructor lambdas
- Defer lambda assignment until all captured members are fully allocated
- See commit cd5f908 for the fix pattern

---

## Tech Debt

### PluginProcessor.cpp — Monolithic DSP Core (6434 lines)

**Files:** `Source/PluginProcessor.cpp`

**Impact:**
- Single file contains: HRTF profiles, speaker layouts, VBAP triplet computation, spatialization algorithms (7 implementations), phase vocoder integration, binaural rendering, feedback loop, dry/wet mixing, automation, preset save/load, OSC receive/send
- Navigation and testing extremely difficult
- Changes to any DSP path carry cascading risk to all others
- Cognitive load: ~40 functions, ~120 member variables, embedded within class methods

**Why accumulated:**
- Marked "SPATIAL MEDIA LIBRARY" for extraction to modular files (see comment at line 14–24)
- Extraction never completed post-v1.0 release
- Design is modular internally (separate structs/functions) but physically monolithic

**Fix approach:**
- Extract into `Source/SpatialFramework/` directory:
  - `HRTFProfiles.cpp/h` — HRTF data + binaural profiles
  - `SpeakerLayouts.cpp/h` — Output formats + VBAP computation
  - `SpatialisationAlgorithms.cpp/h` — 7 algo implementations
  - `BinauralRenderer.cpp/h` — HRTF convolution + ITD
- Keep delay-specific logic in `PluginProcessor.cpp`
- Update includes and test fixtures
- Estimated effort: 2–3 phases (refactor, test, extract)
- No functional change on user-facing plugin

---

### Phase Vocoder Latency Compensation — PDC Fragility (Issue #63, v1.0.7)

**Files:** 
- `Source/PhaseVocoderPitchShifter.h` (2048-sample latency, lines 26–27, 165, 212–213)
- `Source/PluginProcessor.h` (dry delay line setup, lines 1170–1172)
- `Source/PluginProcessor.cpp` (dry path fill logic, lines 2852–2864, 4620–4648)

**Problem:** PDC depends on exact latency symmetry between wet and dry paths:
- **Wet path:** Phase vocoder reports `setLatencySamples(kFFTSize) = 2048` samples
- **Dry path:** Circular delay line buffers `dryDelayLineL/R` must also be exactly 2048 samples
- **Coupling:** Both are hard-coded. If FFT size changes (e.g., for lower CPU), latency mismatch breaks PDC

**Risk:**
- DAW will report different plugin latency to PDC engine
- Phase alignment breaks at all dry/wet ratios ≠ 100% wet
- Comb filtering artifacts appear (difficult to debug)
- No test coverage for dry/wet mismatch at all ratios

**Mitigation in place:**
- Constants synchronized via `PhaseVocoderPitchShifter::kFFTSize` and explicit comment
- Pre-allocation in `prepareToPlay()` prevents resize surprises

**Safe modification:**
- IF changing FFT size: update BOTH `kFFTSize` in PhaseVocoderPitchShifter.h AND the resize in PluginProcessor.cpp line 2860–2861
- Test dry/wet at 0%, 25%, 50%, 75%, 100% wet with time-domain alignment check
- Add assertion to verify `dryDelayLineL.size() == kFFTSize`

**Fix approach:** Extract PDC logic into separate DSP module with unit tests covering all dry/wet ratios

---

## Fragile Areas

### HRTF Timer-Thread Loading — Test Harness False Positives

**Files:** 
- `Source/PluginProcessor.cpp` (testLoadHRTFProfile method, ~line 2650)
- `Tests/ConvolverGlitchTests.cpp` (createBinauralProcessor, line 334–379)

**Problem:** HRTF profiles load asynchronously via timer thread. In test harness, timer never fires.

**Symptom:** 
- Tests calling Simple mode (profile 0, Woodworth ITD/ILD, no convolution) pass silently
- If test accidentally runs on profile 0 instead of profile 1+, convolution doesn't load
- Binaural rendering produces incorrect output → test passes anyway

**Safe pattern:**
```cpp
// CORRECT: Use testLoadHRTFProfile() for synchronous load
auto proc = createBinauralProcessor(1);  // MIT KEMAR
// Line 376: proc->testLoadHRTFProfile(hrtfProfile);
```

**Risk zones:**
- Any new test adding binaural rendering must call `testLoadHRTFProfile()` after `prepareToPlay()`
- Tests using profile 0 (Simple mode) bypass convolver entirely — no real test coverage
- Profile 0 is rarely used in production; most users pick Immersive/Natural/Precise/etc.

**Safe modification:**
- Always call `testLoadHRTFProfile()` in test setup
- Document this requirement in test utilities
- Consider adding a runtime assertion in `processBlock()` when hrtfProfile > 0 but convolver not ready

---

### Binaural Renderer Buffer Pre-Allocation (Audio-Thread Allocation Trap)

**Files:** 
- `Source/PluginProcessor.cpp` (lines 1157–1161, 1314–1316, 1359–1361)
- `Source/PluginProcessor.h` (member buffers convTmpL/R)

**Problem:** Pre-allocated work buffers used in HRTF convolution. If IR length exceeds pre-allocated size, code hits `jassertfalse` and allocates on audio thread (real-time violation).

**Current safeguard:**
```cpp
if (irLength > preAllocSize) {
    jassertfalse;  // Audio thread allocation — should have been pre-allocated
    tmpL.resize(...);  // UNSAFE
}
```

**Risk:** 
- If loadHRTFProfile() receives unexpectedly long IR, allocation stalls audio
- Pre-allocation size computed once in `prepare()` — assumes IR length never exceeds estimate
- MIT KEMAR (~8000 samples) fits; Bernschuetz KU100 (~19000 samples) is borderline

**Safe modification:**
- Verify pre-alloc size covers maximum IR length across all 6 HRTF profiles at setup
- Add static assertion: `static_assert(MAX_IR_LENGTH == 19748, "Update preAllocSize")`
- Consider dynamic allocation with guard: emit warning if resize needed

---

## Scaling Limits

### Per-Source Delay System — 12-Object Hard Limit (Issue #183 Transport Reset Pop)

**Files:** 
- `Source/PluginProcessor.h` (lines 437, 493–522)

**Current capacity:**
- `MAX_SOURCES = 12` (fixed array)
- Each source: 1 delay line, 2 convolvers (L/R), 2 ITD buffers, feedback path
- Total: ~96 float buffers at 48kHz, 100ms max delay = ~460KB per instance

**Scaling path:**
- Increase `MAX_SOURCES` to 16–24 requires:
  1. Recompile kernel (no binary compat break, backward compat guaranteed by APVTS)
  2. Test convolver count (16 convolvers × 2 channels × 19 HRTF profiles)
  3. Verify buffer layout in `renderDirectBinauralHRTF()` loop
  4. Stress test CPU: 24 objects at 48kHz, 100ms delay, full feedback

**Known issue:** Transport reset with high feedback + pitch shift (#183) causes ~500ms pop/crackle. Root cause is feedback residual not fully damped before reset. Increasing tap count worsens this.

---

### Output Format Registry — 22 Formats (5 Stereo + 8 Surround + 9 Ambisonics)

**Files:** `Source/PluginProcessor.cpp` (lines 45–80)

**Capacity:** 
- VBAP triplet computation for 3D layouts: ~150 triplets cached
- Ambisonics decode matrix (up to 49 channels) precomputed
- Total memory: ~60KB static

**Scaling:** Appears sufficient for Dolby Atmos spec (16 speakers max, encoded as 9.1.6). No known bottleneck.

---

## Known Bugs (Open Issues)

### Issue #183: Audio Drop + Pop on Transport Reset with High Feedback + Pitch Shift

**Status:** OPEN, reproducible

**Symptoms:**
- When feedback > 0.7 and pitch shift > ±6 semitones, transport reset (stop/start) produces ~500ms audio crackle/pop
- Only with Pitch mode enabled; Pure Delay clean
- All DAWs (REAPER, Ableton, Logic tested)

**Files:** 
- `Source/PluginProcessor.cpp` (lines 2775–2850: `transportReset()`)
- `Source/PluginProcessor.cpp` (lines 4600–4700: `processBlock()` feedback path)

**Trigger:** Preset with feedback ≥ 0.75 + pitch shift ≥ ±8 semitones. Stop transport, wait 100ms, start.

**Current mitigation:** None. User workaround: reduce feedback to 0.5–0.6 when using pitch.

**Cause hypothesis:** 
1. Phase vocoder maintains 2048-sample latency
2. High feedback creates feedback residual lasting ~50–100ms
3. `transportReset()` triggers `resetPhaseVocoder()`, but feedback buffer doesn't clear atomically
4. Pitch shift kick-in during residual decay causes phase discontinuity

**Fix approach:**
- Audit feedback buffer clear in `transportReset()`
- Consider 50ms crossfade damping before phase vocoder reset
- Add test case: set up high feedback, enable pitch, transport reset, measure RMS drop

---

### Issue #199: Filter Handle Visibility (FIXED in v1.0.2)

**Status:** Fixed but highlights rendering complexity

**Previous:** Filter EQ handles off-screen when Q extreme (issue #195 + #198)

**Files:** `Source/PluginEditor.cpp` (filter graph rendering, ~4200 lines)

**Risk:** Editor visualization code highly interconnected with parameter layout. Changes to filter algo risk visual glitches.

---

## Security & Validation

### OSC Port Validation (Issue #179)

**Files:** `Source/PluginProcessor.cpp` (OSC send/receive setup)

**Issue:** No validation prevents Send and Receive ports from being set to the same number

**Risk:** OSC receiver binds to port X, sender tries to transmit on port X → undefined behavior (port conflict, possible SEGV on some systems)

**Fix approach:**
- Add runtime check: if `configOscSendPort == configOscReceivePort`, reject or auto-shift one by 1
- Validate in `audioProcessorValueTreeStateParameterChanged()`
- Add unit test

---

### HRTF Data Embedded in Plugin

**Files:** `HRTF/` directory (5 SOFA files, ~60MB total, embedded at build time)

**Risk:** SOFA files contain publicly available research data. License compliance verified in `CLAUDE.md` / `agent_docs/licensing.md`. No secrets embedded.

**Mitigation:** All 6 HRTF datasets properly attributed in manual + binary metadata.

---

## Dependencies at Risk

### libmysofa (SOFA File Reader)

**Package:** libmysofa (fetched via CMake FetchContent from GitHub)

**Risk Level:** LOW (passive dependency)

**Concerns:**
1. Maintained by hoene (Christof Höne, Fraunhofer IDMT)
2. Dependency chain: libmysofa → zlib (system lib on all platforms)
3. No known security advisories (as of 2026-04-14)

**Mitigation:**
- Embedded SOFA files are pre-validated (all 6 profiles load at startup in plugin)
- No dynamic SOFA loading from user input
- SOFA parsing errors handled gracefully → fallback to Simple mode

**Update path:** If libmysofa updates, re-test HRTF loading with all 6 profiles

---

### JUCE 8.0.3

**Version:** Locked to JUCE 8.0.3 (submodule)

**Risk:** JUCE 8.x has known AU/VST3 compat issues with certain DAWs (e.g., Ableton #122, older Logic)

**Mitigated via:**
- Issue #122: Parameter count reduced to 64 (Ableton's auto-populate limit)
- Issue #120: Bundle re-signing after plist patch
- Regular CI testing in REAPER (primary test DAW)

**Scaling concern:** Upgrading JUCE requires full regression test against target DAWs

---

## Test Coverage Gaps

### Dry/Wet Ratio Edge Cases — Latency Compensation Untested

**What's not tested:** 
- PDC correctness at dry/wet = 25%, 50%, 75% (only 0% and 100% implicitly covered)
- Phase alignment between dry and wet paths at all ratios

**Files:** 
- Tests missing: no test case in `DSPUnitTests.cpp` or `IntegrationTests.cpp`
- Related: `ConvolverGlitchTests.cpp` tests convolver in isolation, not dry/wet mix

**Risk:** Silent PDC failure at mixed ratios. Phase-aligned content (e.g., kick drum) exhibits comb filtering.

**Priority:** HIGH — affects all mixed-ratio use cases

**Test plan:** Add `PreReleaseTests.cpp` suite:
```cpp
TEST_CASE("Dry/Wet mix — PDC correct at all ratios") {
    // Process impulse train at 0%, 25%, 50%, 75%, 100% wet
    // Verify delay between dry and wet peaks is always ~2048 samples
}
```

---

### Profile 0 (Simple Mode) Bypass Convolver — No Real Test Coverage

**What's not tested:** 
- Simple mode (Woodworth ITD/ILD) correctness
- All convolver tests use profile 1+ (MIT KEMAR); profile 0 never exercised
- UI mode switching between Simple and Convolver modes

**Files:**
- `Tests/ConvolverGlitchTests.cpp` (always uses profile 1)
- `Source/PluginProcessor.cpp` (renderDirectBinauralHRTF has two codepaths: Simple vs. HRTF)

**Risk:** Simple mode shipped in v1.0 with zero test coverage. If user selects "Simple (Low CPU)", rendering may be incorrect.

**Priority:** MEDIUM — Simple mode is rarely used (most users pick Immersive, Natural, etc.)

**Test plan:** Add test suite:
```cpp
TEST_CASE("Simple profile rendering — ITD/ILD correctness") {
    createBinauralProcessor(0);  // Force profile 0
    // Verify ITD/ILD per source
    // Verify amplitude envelope correct
}
```

---

### Per-Source Trajectory Animation — Visual State Not Validated

**What's not tested:** 
- Trajectory shape transitions (None → Bounce, Bounce → Line, etc.)
- Phase interpolation during shape changes
- Reverse toggle during active trajectory

**Files:**
- `Source/TrajectoryEngine.cpp` (456 lines, untested in test suite)
- `Tests/TrajectoryTests.cpp` (506 lines, only covers mathematical trajectory paths, not animation state)

**Risk:** Editor visualization and automation state mismatch. User sees smooth animation but audio position may glitch.

**Priority:** MEDIUM — trajectory features less commonly used than base delay

---

### Automation Undo Stack — Edge Cases

**What's not tested:**
- Rapid undo/redo with active automation playback
- Undo during multi-source trajectory animation
- Gesture handling at block boundaries (PDC interaction)

**Files:**
- `Source/PluginProcessor.cpp` (captureUndoState, notifyHostStateChanged, ~line 1887+)
- `Tests/` (no undo-specific test)

**History:** Issue #182 (E15b double-undo) consumed 15 versions (v1.0.21–v1.0.36) to resolve. Fragile area.

**Risk:** New automation changes risk regressing issue #182

**Safe modification:**
- Any param change in automation context must wrap with:
  ```cpp
  processorRef.captureUndoState("Action Name");
  // modify param
  processorRef.notifyHostStateChanged();
  ```
- See commit 70a6c49 pattern

---

## Performance Bottlenecks

### HRTF Convolution — CPU Cost at 12 Objects

**Files:** 
- `Source/PluginProcessor.cpp` (renderDirectBinauralHRTF, ~line 3200–3400)
- `Source/PluginProcessor.h` (PartitionedConvolver, lines 347–415)

**Measured:** On Apple M1, 12 objects + MIT KEMAR profile ≈ 8–12% CPU at 48kHz. Scales linearly with object count.

**Bottleneck:** Each object requires:
1. Per-block HRTF lookup (interpolation, not cached)
2. 2 PartitionedConvolver processes (L/R, 1 per source)
3. ITD fractional delay apply
4. Total: ~1200 flops per source per block

**Scaling:** At 96kHz, estimate 14–18% CPU. At 192kHz, possibly CPU limit.

**Optimization path:**
- Cache HRTF lookups if azimuth/elevation unchanged frame-to-frame (0.1° hysteresis already in place)
- Vectorize PartitionedConvolver (currently scalar loop)
- GPU offload (HRTF convolution via Metal on macOS) — future

---

### Filter Bank EQ — 30 Biquad Filters at 12kHz Update Rate

**Files:** `Source/FilterBank.cpp`, `Source/FilterBank.h`

**Measured:** Per-source filter update ≈ 0.1% CPU (negligible)

**Not a bottleneck.** Listed for completeness.

---

## Missing Critical Features

### No Validation of OSC Commands — Injection Risk

**Files:** OSC receive handler in `Source/PluginProcessor.cpp`

**Issue:** OSC messages from network not validated. Malformed messages could crash plugin or cause undefined behavior.

**Risk:** Mitigated in practice (OSC disabled by default; user must explicitly enable and provide IP/port), but no input validation in message handlers.

**Fix approach:** Add message format checks before dereferencing OSC value lists

---

## Version Discipline Concerns

### Version Registry Gaps (Historical)

**Files:** `agent_docs/version_registry.md`

**Issue:** v1.0.4, v1.0.11–v1.0.13, v1.0.15–v1.0.19 skipped or used but not documented (discovered post-release)

**Impact:** Next available = v1.0.1 (O101) for post-release patches. Internal patch versions O102–O136 for development. Clean separation, but gap in registry indicates ad-hoc numbering during development.

**Mitigation:** Version script (`build_version.sh`) enforces semantic versioning. Registry now canonical.

**Going forward:** Always update registry before `build_version.sh` invocation

---

## Documentation Gaps (User-Facing)

### Issue #176–#172: Manual Diagram/Layout Problems

**Status:** OPEN (8 issues filed 2026-04-09)

**Files:** `docs/OSD_Manual_v1.0.pdf` (not in repo, externally stored)

**Impact:** User manual has whitespace issues, unclear diagrams, outdated screenshots. Not a code concern, but impacts user experience.

**Priority:** LOW (addressed in post-v1.0 documentation pass)

---

## Summary Table

| Concern | Severity | Type | Status | Fix Effort |
|---------|----------|------|--------|-----------|
| Null deref in updateSyncUI | CRITICAL | Bug | Fixed v1.0.36 | 1 day (pattern audit) |
| PDC latency coupling | HIGH | Tech Debt | Open | 2 days (unit tests) |
| PluginProcessor.cpp size | HIGH | Tech Debt | Open | 3 phases (extraction) |
| HRTF timer thread gotcha | MEDIUM | Fragile | Mitigated | 1 day (doc + assert) |
| Transport reset pop | HIGH | Bug | Open | 2 days (investigation) |
| Dry/wet test coverage | HIGH | Testing | Missing | 1 day (tests) |
| Simple mode untested | MEDIUM | Testing | Missing | 1 day (tests) |
| Trajectory state validation | MEDIUM | Testing | Missing | 2 days (tests) |
| Undo edge cases | MEDIUM | Testing | Missing | 2 days (tests) |
| libmysofa risk | LOW | Dependency | Stable | Monitor only |
| JUCE 8.0.3 compat | MEDIUM | Dependency | Managed | Monitor only |

---

*Concerns audit: 2026-04-14*
