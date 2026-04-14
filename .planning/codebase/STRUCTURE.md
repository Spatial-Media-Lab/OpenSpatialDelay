# Codebase Structure

**Analysis Date:** 2026-04-14

## Directory Layout

```
openspatialdelay/
├── Source/                 # Plugin DSP and UI implementation
├── Tests/                  # Catch2 unit and integration tests (162+ tests)
├── HRTF/                   # SOFA files — 5 HRTF profiles (MIT KEMAR, CIPIC, etc.)
├── fonts/                  # Embedded fonts (DM Sans, JetBrains Mono, Roboto)
├── scripts/                # Build helpers (build_version.sh, install_plugins.sh)
├── tools/                  # Standalone utilities (screenshot_tool.cpp)
├── docs/                   # User manual, release notes, design docs
├── agent_docs/             # Architecture reference docs for Claude
├── JUCE/                   # JUCE 8.0.3 framework (git submodule)
├── Archive/                # v0.1–v0.9 source archives (frozen)
├── CMakeLists.txt          # CMake build configuration
└── build/                  # Build output directory (generated)
```

## Directory Purposes

**Source/**
- Purpose: Core plugin implementation — processor, editor, DSP modules, preset data
- Contains: C++17 implementation files (.cpp, .h)
- Key files:
  - `PluginProcessor.cpp` (297 KB): Main DSP + signal processing logic
  - `PluginProcessor.h` (63 KB): Processor class, spatial algorithms, HRTF infrastructure
  - `PluginEditor.cpp` (197 KB): UI components, spatial map, parameter binding
  - `PluginEditor.h` (31 KB): Editor class + SpatialMapComponent declaration
  - `PresetData.cpp` (61 KB): Hardcoded factory presets
  - `TrajectoryEngine.cpp/h`: Per-object animation state machine
  - `DopplerVelocity.cpp/h`: Per-block velocity tracking for pitch shift
  - `FilterBank.cpp/h`: IIR filter state management
  - `PhaseVocoderPitchShifter.h`: STFT pitch shifter with transient detection
  - `SharedFFTCache.h`: Process-global FFT cache (single instance per plugin)

**Tests/**
- Purpose: Catch2 unit and integration test suite (162+ tests)
- Contains: Test .cpp files
- Key files:
  - `TrajectoryTests.cpp`: Trajectory shape computation (circle, figure-8, spiral, lissajous)
  - `DSPUnitTests.cpp`: Individual DSP component tests (filters, Doppler, pitch shift)
  - `ConvolverGlitchTests.cpp`: Real-time convolution glitch detection (largest test file)
  - `SurroundOutputTests.cpp`: Speaker gain computation for all layouts
  - `OscTests.cpp`: OSC protocol parsing and dispatch
  - `IntegrationTests.cpp`: Full signal path tests (preset load, processBlock)
  - `PreReleaseTests.cpp`: Final validation before release (parameter counts, version strings)
  - `ParameterCountTests.cpp`: Audit parameter enumeration order
  - `TestUtilities.h`: Helper functions (RMS, sine generation, glitch detection)

**HRTF/**
- Purpose: SOFA files embedded as binary resources at compile time
- Contains: 5 SOFA files (binary data, read-only)
- Files:
  - `mit_kemar_large_pinna.sofa`: MIT KEMAR (studio reference)
  - `sadie_d2_ku100.sofa`: SADIE II (immersive)
  - `cipic_subject_003.sofa`: CIPIC subject 003 (natural)
  - `hutubs_pp2.sofa`: HUTUBS PP2 (precise)
  - `bernschuetz_ku100.sofa`: Bernschuetz KU100 (spatial)
- Embedded by: CMakeLists.txt via `juce_add_binary_data(HRTFData ...)`

**fonts/**
- Purpose: TrueType fonts embedded as binary resources
- Contains: TTF files
- Files:
  - DM Sans (Regular, Medium, SemiBold, Bold)
  - JetBrains Mono (Regular, Medium, Bold)
  - Roboto (Medium)
- Licenses: SIL OFL (DM Sans, Roboto), Apache 2.0 (JetBrains Mono)
- Embedded by: CMakeLists.txt via `juce_add_binary_data(FontData ...)`

**scripts/**
- Purpose: Build and installation helpers
- Key files:
  - `build_version.sh`: Build versioned plugin with commit hash (must use for testable builds)
  - `install_plugins.sh`: Copy compiled AU/VST3 to macOS system locations
  - `download_windows_build.sh`: Fetch latest Windows VST3 from GitHub Actions
  - `build_version.sh` usage: `bash scripts/build_version.sh <commit-hash> <version> <order>`
    - Example: `bash scripts/build_version.sh 768c248 v1.0.0 O100`
    - CRITICAL: Must use versioned builds for user A/B testing; never use plain `cmake --build`

**tools/**
- Purpose: Standalone utilities (not part of plugin binary)
- Files:
  - `screenshot_tool.cpp`: Off-screen GUI capture for documentation (CMake target `screenshot_tool`)

**docs/**
- Purpose: User documentation, release notes, design philosophy
- Key files:
  - `OpenSpatialDelay_Manual_v1.0.pdf`: User guide (784 KB)
  - `RELEASE_NOTES_v1.0.0.md`: Changelog (v0.5–v1.0.0)
  - `VERSION_HISTORY.md`: Full version lineage
  - `RELEASE_PLAN_v1.0.0.md`: Phase breakdown for v1.0.0 release
  - `design-philosophy.md`: Conceptual rationale
  - `harness/`: Test harness documentation and reference images

**agent_docs/**
- Purpose: Architecture and integration documentation for Claude agent context
- Key files:
  - `architecture.md`: 5-stage signal flow, HRTF rendering, phase vocoder latency, dry path compensation
  - `licensing.md`: JUCE tiers, third-party license table, attribution obligations
  - `version_registry.md`: Version number registry, next available O-number, build naming rules

**JUCE/**
- Purpose: JUCE 8.0.3 framework (git submodule, read-only)
- Contains: Full JUCE source tree
- Used by: CMakeLists.txt via `add_subdirectory(JUCE)`

## Key File Locations

**Entry Points:**

- `Source/PluginProcessor.cpp` line ~4293: `processBlock()` — main audio processing loop
- `Source/PluginProcessor.h` line ~611: `OpenSpatialDelayProcessor` class declaration
- `Source/PluginEditor.cpp` line ~1: `PluginEditor` class — GUI instantiation

**Configuration:**

- `CMakeLists.txt` lines 1–360: Build system (plugin formats, dependencies, targets)
- `.env` files (not read): Environment variables for build/deploy (see CLAUDE.md for location notes)
- `Source/PluginProcessor.h` line ~20: HRTF profile names array

**Core Logic:**

- `Source/PluginProcessor.cpp` lines 4293–5050: `processBlock()` + `renderDirectBinauralHRTF()`
- `Source/TrajectoryEngine.cpp` line ~1: Trajectory shape animation (6 shapes + random)
- `Source/DopplerVelocity.cpp` line ~1: Per-object velocity tracking and pitch computation
- `Source/FilterBank.cpp` line ~1: IIR filter coefficient management
- `Source/PhaseVocoderPitchShifter.h` lines 23–200: STFT pitch shifter + transient detection

**Testing:**

- `Tests/ConvolverGlitchTests.cpp`: Largest test suite (144 KB, validates glitch-free convolution)
- `Tests/TrajectoryTests.cpp`: Trajectory shape + wrapping
- `Tests/TestUtilities.h`: Helper functions (computeRMS, fillSine, detectGlitches)

**Spatial Algorithms & HRTF:**

- `Source/PluginProcessor.h` lines 156–273: 7 spatialization algorithms (SpatializationAlgorithm + implementations)
- `Source/PluginProcessor.h` lines 285–343: HRTFDatabase class
- `Source/PluginProcessor.h` lines 349–428: PartitionedConvolver (dual-slot crossfade)
- `Source/PluginProcessor.h` lines 434–537: BinauralRenderer (HRTF convolution orchestration)

## Naming Conventions

**Files:**

- `.cpp` + `.h` pairs for class/module implementation
- `Plugin*.cpp/h`: Processor and Editor main classes
- `CamelCase.cpp/h`: Utility classes (TrajectoryEngine, DopplerVelocity, FilterBank)
- `*Data.cpp/h`: Preset/binary data (PresetData, HRTFData, FontData — generated or hardcoded)

**Directories:**

- PascalCase for main project directories (Source, Tests, HRTF, Archive)
- snake_case for build/utility directories (scripts, tools, build)
- lowercase for assets (fonts, docs, agent_docs)

**Classes & Types:**

- PascalCase: `OpenSpatialDelayProcessor`, `PluginEditor`, `SpatializationAlgorithm`, `BinauralRenderer`
- Struct names with `Struct` suffix or plain PascalCase: `BinauralProfile`, `VirtualSpeaker`, `ObjectState`
- Enum class: PascalCase (e.g., `OutputFormat::Binaural`)

**Constants & Statics:**

- `kPrefixedConstant` for class-level constants (e.g., `kMaxObjects`, `kFFTSize`)
- `MAX_UPPERCASE` for global plugin constants (e.g., `MAX_DELAY_SECONDS`)
- `NUM_PREFIX` for count constants (e.g., `NUM_HRTF_PROFILES`, `NUM_OUTPUT_FORMATS`)

**Parameters:**

- Global params: `delayTime`, `tempoSync`, `noteDivision`, `syncMode`, `feedback`, `filterEnabled`, `filterHP`, `filterHPQ`, `filterLP`, `filterLPQ`, `dryWet`, `outputGain`, `outputFormat`, `algorithmMode`, `hrtfProfile`, `inputFormat`, `oscEnabled`, `oscPort`
- Per-object params: `{object_azimuth, object_elevation, object_distance, object_enabled, object_wobbleAmount, object_trajectoryShape, object_trajectorySpeed, object_dopplerAmount, object_pitchShift, object_outputGain, object_pitchShiftEnable, object_wobbleEnable}` (indices 0–11)

## Where to Add New Code

**New Feature (DSP):**

- Primary code: `Source/PluginProcessor.cpp` (or new `Source/NewModule.cpp/h` if self-contained)
- Tests: `Tests/NewFeatureTests.cpp` (use TestUtilities.h helpers)
- Integration point: Call from `processBlock()` or per-sample render loop

**New Spatialization Algorithm:**

- Declaration: `Source/PluginProcessor.h` (inherit from `SpatializationAlgorithm`)
- Implementation: `Source/PluginProcessor.cpp` (implement `computeGains()` + optional `computeBinauralGains()`)
- Registration: Update algorithm dropdown in `createParameterLayout()` and algorithm instance dispatch in `processBlock()`
- Tests: Add test cases to `Tests/SurroundOutputTests.cpp`

**New Component (UI):**

- Implementation: `Source/PluginEditor.cpp/h` (inherit from juce::Component)
- Parameters: Expose getters/setters for editor access
- Integration: Add to editor layout in `PluginEditor::resized()`

**New Utility Module:**

- Header: `Source/NewUtility.h` (if header-only like PhaseVocoderPitchShifter)
- Or pair: `Source/NewUtility.cpp/h` (if stateful like TrajectoryEngine)
- Include in: CMakeLists.txt target_sources if .cpp file
- Tests: Create `Tests/NewUtilityTests.cpp`

**Unit Tests:**

- Location: `Tests/` directory
- Framework: Catch2 (BDD-style `TEST_CASE` and `SECTION`)
- Utilities: Use `TestUtilities.h` helpers (computeRMS, fillSine, detectGlitches)
- Async testing: Use `testLoadHRTFProfile()` or `createBinauralProcessor()` for timer-dependent code
- Pattern: Prepare → process → assert (see ConvolverGlitchTests.cpp)

## Special Directories

**build/**
- Purpose: CMake build output
- Generated: Yes (created by `cmake -B build`)
- Committed: No (.gitignore)
- Contents: Intermediate .o files, plugin binaries, test executable
- Cleaning: `rm -rf build/` then reconfigure

**Archive/**
- Purpose: Frozen source archives for v0.1–v0.9 (historical reference)
- Generated: No
- Committed: Yes (read-only)
- Use: Never modify; previous versions stored for reference only

**JUCE/**
- Purpose: Framework submodule (read-only)
- Generated: No
- Committed: Git submodule pointer
- Updating: `git submodule update --init --recursive` (rare; JUCE 8.0.3 pinned)

**agent_docs/**
- Purpose: Claude agent context (not part of binary)
- Generated: No
- Committed: Yes
- Update: By hand when architecture changes significantly

---

*Structure analysis: 2026-04-14*
