# Testing Patterns

**Analysis Date:** 2026-04-14

## Test Framework

**Runner:**
- Catch2 v3.7.1 (fetched via FetchContent in `CMakeLists.txt`)
- Config: `CMakeLists.txt` lines 181–279 (OpenSpatialDelayTests target)
- Main macro: `Catch2::Catch2WithMain` (built-in main, no boilerplate needed)

**Assertion Library:**
- Catch2 matchers: `Catch::Matchers::WithinAbs()` for floating-point tolerance
- Standard macros: `REQUIRE()` (abort on fail), `CHECK()` (continue on fail)
- Approx comparisons: `Catch::Approx()` with explicit `.margin()` or `.epsilon()`

**Run Commands:**
```bash
# Build and run all tests
cmake --build build --target OpenSpatialDelayTests -j$(sysctl -n hw.ncpu)
./build/OpenSpatialDelayTests

# Run specific test suite (Catch2 tag filter)
./build/OpenSpatialDelayTests "[dsp]"
./build/OpenSpatialDelayTests "[trajectory][characterization]"

# Run with verbose output
./build/OpenSpatialDelayTests -v
```

## Test File Organization

**Location:**
- Test files live in `Tests/` directory (co-located strategy, not `src/tests/`)
- Test files paired with source files: `DSPUnitTests.cpp` tests DSP modules, `TrajectoryTests.cpp` tests trajectory
- Shared utilities: `Tests/TestUtilities.h` (helper functions, constants)

**Naming:**
- Pattern: `<Subject>Tests.cpp` or `<Subject>UnitTests.cpp`
- Examples: `DSPUnitTests.cpp`, `IntegrationTests.cpp`, `TrajectoryTests.cpp`, `ParameterCountTests.cpp`, `ConvolverGlitchTests.cpp`, `PreReleaseTests.cpp`, `OscTests.cpp`, `SurroundOutputTests.cpp`

**Structure:**
```
Tests/
├── DSPUnitTests.cpp          # Phase vocoder, Doppler, filter tests
├── TrajectoryTests.cpp       # Trajectory shape, animation tests
├── IntegrationTests.cpp      # Full processor tests (dry/wet, parameter interaction)
├── ParameterCountTests.cpp   # Parameter layout and automatable verification
├── ConvolverGlitchTests.cpp  # HRTF convolver glitch detection
├── OscTests.cpp              # OSC receive/send tests
├── SurroundOutputTests.cpp   # Multi-channel output format tests
├── PreReleaseTests.cpp       # Before-release validation suite
└── TestUtilities.h           # Shared helpers and constants
```

## Test Structure

**Suite Organization:**

```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Source/PluginProcessor.h"
#include "TestUtilities.h"
#include <cmath>
#include <vector>

using Catch::Matchers::WithinAbs;

// ============================================================================
// Section 1: Characterization Tests — capture current behavior
// ============================================================================

TEST_CASE("None shape returns base position unchanged", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::None, 0.5f, 45.0f, 20.0f, 0.7f);
    CHECK_THAT(r.azDeg, WithinAbs(45.0f, 0.01f));
    CHECK_THAT(r.elDeg, WithinAbs(20.0f, 0.01f));
    CHECK_THAT(r.dist,  WithinAbs(0.7f, 0.001f));
    CHECK_FALSE(r.controlsAz);
}

// ============================================================================
// Section 2: Regression Tests — catch breaking changes
// ============================================================================

TEST_CASE("Parameter count is exactly 143", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();
    CHECK(params.size() == 143);
}
```

**Patterns:**
- Section headers: `// ============================================================================` with comment
- Test tags for filtering: `"[category][subcategory]"` — multiple tags per test
- Common tags: `[dsp]`, `[trajectory]`, `[params]`, `[drywet]`, `[convolver]`, `[glitch]`, `[issue<number>]`, `[characterization]`, `[bypass]`, `[accuracy]`, `[latency]`
- Setup/teardown: inline constructors (tests are short-lived, no TearDown needed)

## Mocking

**Framework:**
- No dedicated mock library (Catch2 alone, no Mockito or Google Mock)
- Manual test doubles created inline

**Patterns:**

From `IntegrationTests.cpp`:
```cpp
// Create a minimal processor with specific config
static std::unique_ptr<Proc> createStereoProcessor()
{
    auto proc = std::make_unique<Proc>();
    
    // Direct atomic store for lock-free config
    proc->configOutputFormat.store(1, std::memory_order_relaxed);  // 1 = Stereo
    proc->configAlgorithm.store(0, std::memory_order_relaxed);
    
    setParam(*proc, "delayTime", 50.0f);
    setParam(*proc, "feedback", 0.3f);
    setParam(*proc, "inputGain", 0.0f);
    setParam(*proc, "outputGain", 0.0f);
    
    // Enable 1 tap
    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String(i + 1);
        setParam(*proc, "object" + idx + "_enabled", (i < 1) ? 1.0f : 0.0f);
    }
    
    proc->prepareToPlay(kSampleRate, kBlockSize);
    return proc;
}
```

**What to Mock:**
- Processor configuration: use atomic `.store()` to set config without going through parameter listeners
- Test double generators: `createLowpassIR()`, `createStereoProcessor()`, `createBinauralProcessor()`
- Fake signals: `fillSine()`, custom input buffers

**What NOT to Mock:**
- JUCE components (`AudioBuffer`, `AudioProcessor`, `juce::dsp::FFT`) — test with real implementations
- Real signal processing: convolvers, filters, pitch shifters all tested with actual audio I/O
- HRTF database: synchronous loading during tests (not async like production, see Gotchas)

## Fixtures and Factories

**Test Data:**

From `TestUtilities.h`:
```cpp
static constexpr float kPi = juce::MathConstants<float>::pi;
static constexpr double kSampleRate = 48000.0;
static constexpr int kBlockSize = 256;

// Compute RMS of a buffer
static inline float computeRMS(const float* buffer, int numSamples)
{
    float sumSq = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sumSq += buffer[i] * buffer[i];
    return std::sqrt(sumSq / static_cast<float>(numSamples));
}

// Generate a sine wave into a buffer
static inline void fillSine(float* buffer, int numSamples, float freq, float sampleRate,
                           float amplitude, int startSample)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float phase = 2.0f * kPi * freq * static_cast<float>(startSample + i) / sampleRate;
        buffer[i] = amplitude * std::sin(phase);
    }
}

// Detect clicks/pops in an audio buffer using first-derivative threshold
static inline std::vector<int> detectGlitches(const float* buffer, int numSamples, 
                                              float threshold = 0.15f)
{
    std::vector<int> glitchIndices;
    for (int i = 1; i < numSamples; ++i)
    {
        float diff = std::abs(buffer[i] - buffer[i - 1]);
        if (diff > threshold)
            glitchIndices.push_back(i);
    }
    return glitchIndices;
}
```

**Location:**
- Shared helpers: `Tests/TestUtilities.h` (included by all test files)
- Inline factories in test files: `createStereoProcessor()`, `createLowpassIR()`, `createBinauralProcessor()` (specific to test file)
- Parameter setters: `static void setParam(Proc& proc, const juce::String& paramId, float value)` pattern

## Coverage

**Requirements:**
- Not enforced by CMake — no coverage target configured
- Target: 162+ tests across 8 test files (as of v1.0.0)

**View Coverage:**
```bash
# LLVM coverage (macOS)
cmake --build build --target OpenSpatialDelayTests
xcrun llvm-cov report ./build/OpenSpatialDelayTests

# No dedicated coverage tool; coverage assessed by visual inspection
```

## Test Types

**Unit Tests:**
- Scope: Single class/function in isolation
- Examples: `ParameterCountTests.cpp` (APVTS structure), `DSPUnitTests.cpp` (phase vocoder pitch shifting, Doppler)
- Approach: Create minimal test doubles, verify mathematical properties (frequency preservation, latency)
- File: `Tests/DSPUnitTests.cpp` (lines 31–210), `Tests/ParameterCountTests.cpp` (lines 8–101)

**Integration Tests:**
- Scope: Multiple components working together (processor + HRTF + parameter system)
- Examples: Dry/wet mixing, latency compensation, parameter automation
- Approach: Full processor initialized, fed with sine/silence, output captured and analyzed
- File: `Tests/IntegrationTests.cpp` (lines 78–162+)

**Characterization Tests:**
- Scope: Capture current behavior before refactoring
- Examples: Trajectory shape positions, orbit phase wrapping, trajectory flags
- Approach: Call pure functions with known inputs, assert expected outputs
- File: `Tests/TrajectoryTests.cpp` (lines 20–150)

**E2E Tests (Pre-Release):**
- File: `Tests/PreReleaseTests.cpp`
- Scope: Full plugin with real DAW simulation (if applicable)
- Coverage: Not detailed in visible test suite; likely covers full feature workflows

## Common Patterns

**Async Testing:**
- `PhaseVocoderPitchShifter` must be processed for `kFFTSize + kHopSize` samples before steady-state (latency settling)
- Example from `DSPUnitTests.cpp` (lines 37–59):
```cpp
TEST_CASE("PV -- unity pitch preserves sine frequency", "[pv][accuracy]")
{
    PhaseVocoderPitchShifter pv;
    constexpr float freq = 440.0f;
    constexpr float sr = static_cast<float>(kSampleRate);
    constexpr int latencySamples = PhaseVocoderPitchShifter::kFFTSize + PhaseVocoderPitchShifter::kHopSize;
    constexpr int totalSamples = latencySamples + 20000;
    
    std::vector<float> output;
    output.reserve(totalSamples);
    for (int i = 0; i < totalSamples; ++i)
    {
        float in = 0.5f * std::sin(2.0f * kPi * freq * static_cast<float>(i) / sr);
        output.push_back(pv.process(in, 0.0f));  // Process one sample per call
    }
    
    // Measure frequency of steady-state output (after latency settling)
    float measuredFreq = estimateFrequency(output.data() + latencySamples,
                                          totalSamples - latencySamples, sr);
    REQUIRE(measuredFreq == Catch::Approx(freq).margin(3.0f));
}
```

- Tolerance margins: Dynamic based on signal type
  - Sine frequency: `.margin(3.0f)` Hz for 440 Hz = ~0.7% tolerance
  - Low frequency (220 Hz): `.margin(15.0f)` Hz = ~7% tolerance (longer measurement window needed)
  - Position values: `.WithinAbs(45.0f, 0.01f)` = ±0.01° absolute tolerance

**Error Testing:**
- NaN/Inf handling: `resetFeedback()` prevents filter state corruption
- Not visible in current test suite; likely covered in `PreReleaseTests.cpp`
- Pattern inferred from code: check `isnan()`, `isinf()` during debug builds

**Capture-and-Analyze Pattern:**
```cpp
// From IntegrationTests.cpp (lines 20–47)
static std::pair<std::vector<float>, std::vector<float>>
processBlocksCapturingAll(Proc& proc, int numBlocks, float inputLevelL = 0.5f, float inputLevelR = 0.5f)
{
    std::vector<float> allL, allR;
    allL.reserve(static_cast<size_t>(numBlocks * kBlockSize));
    allR.reserve(static_cast<size_t>(numBlocks * kBlockSize));
    
    juce::MidiBuffer midi;
    for (int b = 0; b < numBlocks; ++b)
    {
        juce::AudioBuffer<float> buffer(2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s)
        {
            buffer.setSample(0, s, inputLevelL);
            buffer.setSample(1, s, inputLevelR);
        }
        proc.processBlock(buffer, midi);
        
        for (int s = 0; s < kBlockSize; ++s)
        {
            allL.push_back(buffer.getSample(0, s));
            allR.push_back(buffer.getSample(1, s));
        }
    }
    return {allL, allR};
}
```

## Critical Gotchas

**HRTF Synchronous Loading (Mandatory):**
- Tests must call `testLoadHRTFProfile()` or use processors created with synchronous HRTF init
- Without this, timer thread doesn't fire in test harness
- Binaural convolution tests silently pass in Simple mode (false positives)
- Inferred from CLAUDE.md: "HRTF tests need synchronous loading"

**Dry Path Latency (2048 samples):**
- Phase vocoder latency must match dry delay line compensation
- If FFT size or overlap changes, both paths must be updated together
- Test in `IntegrationTests.cpp` (lines 163–200): `TEST_CASE("DryWet -- dry path latency compensation is 2048 samples")`

**Sample-Exact Frequency Measurement:**
- Zero-crossing frequency estimation in `estimateFrequency()` (from `DSPUnitTests.cpp` lines 13–25)
- Requires settling time (multiple cycles) for accurate measurement
- Longer measurement windows for lower frequencies (7% tolerance at 220 Hz vs. 0.7% at 440 Hz)

## Test Execution

**From Source Tree:**
```bash
cd /Users/andrewrahman/conductor/repos/openspatialdelay

# Configure (once)
cmake -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev

# Build tests
cmake --build build --target OpenSpatialDelayTests -j$(sysctl -n hw.ncpu)

# Run all tests
./build/OpenSpatialDelayTests

# Run specific suite
./build/OpenSpatialDelayTests "[trajectory]"
./build/OpenSpatialDelayTests "[dsp]"
```

**Key Test Files by Responsibility:**
- `Tests/ParameterCountTests.cpp` — Parameter layout (143 total, 64 automatable in Ableton)
- `Tests/DSPUnitTests.cpp` — Phase vocoder, bypass hysteresis, frequency accuracy
- `Tests/TrajectoryTests.cpp` — Trajectory animation, shape positioning, phase wrapping
- `Tests/IntegrationTests.cpp` — Processor initialization, dry/wet mixing, latency compensation
- `Tests/ConvolverGlitchTests.cpp` — HRTF convolver glitch detection, FFT cache, IR crossfading
- `Tests/OscTests.cpp` — OSC receive/send functionality
- `Tests/SurroundOutputTests.cpp` — Multi-channel output format validation

---

*Testing analysis: 2026-04-14*
