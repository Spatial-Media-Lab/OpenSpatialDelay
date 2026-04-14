# Coding Conventions

**Analysis Date:** 2026-04-14

## Naming Patterns

**Files:**
- Source files: `PluginProcessor.cpp`, `PluginEditor.cpp`, `PhaseVocoderPitchShifter.h` — PascalCase for both headers and implementation
- Class-based modules: `DopplerVelocity.h/cpp`, `TrajectoryEngine.h/cpp`, `FilterBank.h/cpp` — single responsibility extracted from core
- Test files: `DSPUnitTests.cpp`, `IntegrationTests.cpp`, `ConvolverGlitchTests.cpp` — descriptive names with subject and purpose

**Functions:**
- Public methods: camelCase — `computeGains()`, `prepareToPlay()`, `processBlock()`, `setIR()`, `getSmoothedSemitones()`
- Static utilities: camelCase — `computeTrajectory()`, `convertToMinPhase()`, `detectOnset()`, `makeLayoutFromDef()`
- Private/internal helpers: camelCase with underscore prefix optional — `computeVBAPGains3D()`, `evalSH()`, `resetSlot()`
- Getters: camelCase with `get` prefix or bare accessor — `getIRLength()`, `isLoaded()`, `isSimpleMode()`, `canUndo()`
- Setters: `set` prefix — `setIR()`, `setProfile()`, `setITDEnabled()`, `setMinPhaseEnabled()`
- Pure functions: CamelCase or camelCase — `wrapAzimuth()`, `degToRad()`

**Variables:**
- Member variables: camelCase with trailing underscore — `fftSize_`, `savedState_`, `smoothedVelocity_`, `finalAz_`, `active_`
- Local variables: camelCase — `buffer`, `pitch`, `phase`, `azRad`, `sourceIndex`
- Constants: `k` prefix + PascalCase — `kMaxObjects`, `kBlockSize`, `kSampleRate`, `kCrossfadeBlocks`, `kFFTSize`, `kPi`
- Static/global constants: UPPER_SNAKE_CASE for true constants; `k` prefix for class-scoped — `NUM_HRTF_PROFILES`, `MAX_DELAY_SECONDS`, `NUM_VIRTUAL_SPEAKERS`
- Config/state: lowercase with underscores — `configOutputFormat`, `configAlgorithm`, `configHrtfProfile`, `configInputFormat`
- Atomic variables: trailing underscore — `activeLayoutIndex`, `posChanged_` (booleans)

**Types:**
- Classes: PascalCase — `OpenSpatialDelayProcessor`, `PartitionedConvolver`, `BinauralRenderer`, `SpatializationAlgorithm`
- Enums: PascalCase for type, UPPER_CASE for values — `enum class OutputFormat { Binaural, Stereo, Quad, ... }`
- Structs: PascalCase — `BinauralProfile`, `VirtualSpeaker`, `ObjectState`, `SourcePosition`, `BinauralGains`
- Namespace: lowercase — `TrajShape` (pseudo-namespace via `namespace { constexpr int None = 0, ... }` in tests)

## Code Style

**Formatting:**
- No `.clang-format` or `.prettierrc` files detected — formatting follows JUCE conventions by convention
- Spacing: 4-space indentation inferred from source files
- Brace style: Allman (opening brace on same line) — `void prepare (int maxBlockSize, int irLength) {`
- Line length: appears unconstrained; implementation files are large (297KB for `PluginProcessor.cpp`)

**Linting:**
- No dedicated linter config; JUCE framework + CMake compiler flags enforce warnings
- Compiler flags: `-Wno-nan-infinity-disabled` (macOS), `/Zc:preprocessor` (MSVC)
- Fast-math enabled: `-ffast-math` (macOS), `/fp:fast` (MSVC) — explicit NaN guards required

**C++ Standard:**
- C++17 enforced via `set(CMAKE_CXX_STANDARD 17)` in `CMakeLists.txt`
- Uses: `std::unique_ptr`, `std::vector`, `std::array`, `std::atomic`, structured bindings

## Import Organization

**Order:**
1. Local headers: `#include "PluginProcessor.h"`, `#include "HRTFData.h"`
2. JUCE framework: `#include <JuceHeader.h>`, `#include <juce_core/juce_core.h>`, `#include <juce_dsp/juce_dsp.h>`
3. External C libraries (wrapped in extern "C"): `#include "mysofa.h"` via `extern "C" { ... }`
4. Standard library: `#include <cmath>`, `#include <vector>`, `#include <array>`, `#include <algorithm>`, `#include <atomic>`, `#include <numeric>`
5. Test framework: `#include <catch2/catch_test_macros.hpp>`, `#include <catch2/catch_approx.hpp>`, `#include <catch2/matchers/catch_matchers_floating_point.hpp>`

**Path Aliases:**
- None detected; relative includes used (`#include "../Source/PluginProcessor.h"` from `Tests/`)
- Include directories specified in CMakeLists.txt: libmysofa headers, JUCE-generated `JuceHeader.h`

**Forward Declarations:**
- Used for opaque types: `struct MYSOFA_EASY;` in `PluginProcessor.h` (avoids including libmysofa in header)

## Error Handling

**Patterns:**
- Return `nullptr` for failed lookups: `if (auto* p = proc.apvts.getParameter(paramId)) { ... }`
- State validation via boolean flags: `bool loaded`, `bool isPrepared()`, `bool isLoaded()`
- Defensive checks before dereference: `if (src >= 0 && src < MAX_SOURCES)` in `getCurrentITDL()`
- Silent degradation on config mismatch: no exceptions; fallback to defaults (e.g., Simple profile as HRTF default)
- NaN recovery: `resetFeedback()` clears IIR filter state (issue #54, #133)
- Lock-free reads with memory ordering: `std::memory_order_relaxed` for config, `std::memory_order_acquire` for layout swaps

**No exceptions in audio thread:** Uses atomic state, time-delayed updates (PluginUndoManager), and deferred operations

## Logging

**Framework:** JUCE `juce::Logger` or `std::cout` in debug builds (not observed in production code)

**Patterns:**
- No logging calls in `processBlock()` (audio thread)
- Parameter changes logged at host level (APVTS listeners)
- Test diagnostic output via `CHECK`, `REQUIRE`, and custom assertions (Catch2)

**Comments:**
- When to comment: algorithm intent, magic numbers, issue references — `// Issue #131: Shared FFT cache`
- Block separators: `// ============================================================================` with title
- Section markers: `// #############################################################################` for major divisions (SPATIAL MEDIA LIBRARY vs PLUGIN-SPECIFIC)

**JSDoc/TSDoc:**
- Doxygen-style doc comments: `/** ... */` for public methods
- Example from `PluginProcessor.h`: `/** Compute speaker gains for a source position in the given layout. */`
- Parameter/return documentation: `/** Get ITD-free interpolated HRIR pair for a direction. The returned HRIRs have ITD removed... */`
- Used consistently on public interfaces; implementation details less documented

## Function Design

**Size:**
- Large functions acceptable: `processBlock()` in `PluginProcessor.cpp` is ~900 lines (signal flow is complex)
- Helper functions extracted for testability: `DopplerVelocity`, `TrajectoryEngine`, `FilterBank` are separate classes
- Single-responsibility: each extracted class handles one concern (Doppler, trajectory animation, filtering)

**Parameters:**
- By const reference for read-only: `const SourcePosition& source`, `const LayoutContext& ctx`
- By pointer for out-parameters: `float* outputGains`, `float* outL`, `float* outR`, `float* irL`
- By value for scalars: `float azimuthRad`, `int numSamples`, `bool enabled`
- Atomic references where needed: `std::atomic<int>& configAlgorithm`

**Return Values:**
- Structures for multi-value returns: `struct TrajectoryResult { float azDeg, elDeg, dist; bool controlsAz, controlsEl, controlsDist; }`
- Or `std::pair<>`: `return { allL, allR };` in integration tests
- Void for in-place operations: `void setIR(const float* ir, int length);`
- Boolean for validity: `bool isLoaded() const { return loaded; }`

## Module Design

**Exports:**
- Header files define the public interface; implementation in `.cpp`
- Static methods for pure functions: `static TrajectoryResult computeTrajectory(...)`
- Const methods for queries: `int getIRLength() const`
- Private/protected for internal state — clearly separated in class definition

**Barrel Files:**
- `#include <JuceHeader.h>` includes all JUCE modules (JUCE's aggregate header pattern)
- `HRTFData.h` and `FontData.h` are generated binary data (juce_add_binary_data)

**Header Organization:**
- Forward declarations at top (after includes)
- Type definitions (enums, structs) before classes
- Class declarations with public interface, then private members
- Comments separating major sections with `//==============================================================================`

**Reusable Abstractions:**
- `SpatializationAlgorithm` base class + 7 concrete implementations (VBAP, Ambisonics, KNN, DBAP, VBIP, DirectBinaural)
- `BinauralRenderer` — encapsulates HRTF convolution for reuse across output formats
- `HRTFDatabase` — SOFA file loading and HRIR interpolation
- Marked with comments: `// SPATIAL MEDIA LIBRARY — Reusable across all SML plugins`

**Extraction Pattern:**
- Core processor delegates to extracted modules: `DopplerVelocity`, `TrajectoryEngine`, `FilterBank`, `PartitionedConvolver`
- Each has its own test file for independent verification
- Reduces `PluginProcessor.cpp` size conceptually (would be 400KB+ without extraction)

---

*Convention analysis: 2026-04-14*
