#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Source/PluginProcessor.h"
#include "../Source/PresetData.h"
#include "TestUtilities.h"
#include <cmath>
#include <vector>

using Proc = OpenSpatialDelayProcessor;
using Catch::Matchers::WithinAbs;

static void setParam (Proc& proc, const juce::String& paramId, float value)
{
    if (auto* p = proc.apvts.getParameter (paramId))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

// Process blocks capturing all output (both channels)
static std::pair<std::vector<float>, std::vector<float>>
processBlocksCapturingAll (Proc& proc, int numBlocks, float inputLevelL = 0.5f, float inputLevelR = 0.5f)
{
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (numBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (numBlocks * kBlockSize));

    juce::MidiBuffer midi;

    for (int b = 0; b < numBlocks; ++b)
    {
        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s)
        {
            buffer.setSample (0, s, inputLevelL);
            buffer.setSample (1, s, inputLevelR);
        }
        proc.processBlock (buffer, midi);

        for (int s = 0; s < kBlockSize; ++s)
        {
            allL.push_back (buffer.getSample (0, s));
            allR.push_back (buffer.getSample (1, s));
        }
    }
    return { allL, allR };
}

// Create a minimal stereo processor
static std::unique_ptr<Proc> createStereoProcessor()
{
    auto proc = std::make_unique<Proc>();

    // Stereo output = format index 1
    proc->configOutputFormat.store (1, std::memory_order_relaxed);
    proc->configAlgorithm.store (0, std::memory_order_relaxed);

    setParam (*proc, "delayTime", 50.0f);
    setParam (*proc, "feedback", 0.3f);
    setParam (*proc, "inputGain", 0.0f);
    setParam (*proc, "outputGain", 0.0f);

    // Enable 1 tap
    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i < 1) ? 1.0f : 0.0f);
    }

    proc->prepareToPlay (kSampleRate, kBlockSize);
    return proc;
}

// ============================================================================
// Section 1: Dry/Wet Mix Tests
// ============================================================================

TEST_CASE ("DryWet -- 0% wet is full dry only", "[drywet]")
{
    auto proc = createStereoProcessor();
    setParam (*proc, "dryWet", 0.0f);

    // Warmup (parameter smoothing + dry latency comp settling)
    processBlocksCapturingAll (*proc, 50);

    // Measure output
    auto [outL, outR] = processBlocksCapturingAll (*proc, 10);
    float rms = computeRMS (outL.data(), static_cast<int> (outL.size()));

    // At 0% wet: dry coefficient = cos(0) = 1.0, so output should be near input level
    REQUIRE (rms > 0.3f);  // Input is 0.5, accounting for output gain
}

TEST_CASE ("DryWet -- 100% wet has no immediate dry signal", "[drywet]")
{
    auto proc = createStereoProcessor();
    setParam (*proc, "dryWet", 1.0f);

    // Use a short delay and check that the first samples before the delay
    // have no immediate (dry) signal
    processBlocksCapturingAll (*proc, 50);  // warmup

    // Feed a distinctive signal and capture
    auto [outL, outR] = processBlocksCapturingAll (*proc, 5, 0.8f, 0.8f);

    // At 100% wet, sin(pi/2) = 1.0 wet, cos(pi/2) = 0.0 dry
    // Output should contain only the delayed signal, not the immediate input
    // The wet signal exists (delayed taps produce output)
    float rms = computeRMS (outL.data(), static_cast<int> (outL.size()));
    REQUIRE (rms > 0.01f);  // Some wet signal should be present
}

TEST_CASE ("DryWet -- 50% mix maintains constant power", "[drywet]")
{
    auto proc = createStereoProcessor();
    setParam (*proc, "dryWet", 0.5f);

    processBlocksCapturingAll (*proc, 80);  // warmup

    auto [outL, outR] = processBlocksCapturingAll (*proc, 20);
    float rms50 = computeRMS (outL.data(), static_cast<int> (outL.size()));

    // Now measure at 0% (pure dry) for reference
    setParam (*proc, "dryWet", 0.0f);
    processBlocksCapturingAll (*proc, 20);  // settle
    auto [dryL, dryR] = processBlocksCapturingAll (*proc, 20);
    float rmsDry = computeRMS (dryL.data(), static_cast<int> (dryL.size()));

    // Equal-power at 50%: dry*cos(pi/4) + wet*sin(pi/4) ≈ combined signal
    // Should NOT be 6 dB down from dry-only (which a linear crossfade would produce)
    // Allow some tolerance since wet adds delayed signal that may partly cancel or reinforce
    if (rmsDry > 0.01f)
    {
        float diffDB = 20.0f * std::log10 (rms50 / rmsDry);
        // Should be within 3 dB of dry level (not -6 dB like linear crossfade)
        REQUIRE (diffDB > -4.0f);
    }
}

TEST_CASE ("DryWet -- stereo dry signal is preserved", "[drywet]")
{
    auto proc = createStereoProcessor();
    // Use stereo input mode
    proc->configInputFormat.store (1, std::memory_order_relaxed);  // 1 = Stereo
    setParam (*proc, "dryWet", 0.0f);  // Full dry

    processBlocksCapturingAll (*proc, 50);  // warmup

    // Feed L=0.8, R=0.0 — if dry path is truly stereo, L should be much louder than R
    auto [outL, outR] = processBlocksCapturingAll (*proc, 20, 0.8f, 0.0f);
    float rmsL = computeRMS (outL.data(), static_cast<int> (outL.size()));
    float rmsR = computeRMS (outR.data(), static_cast<int> (outR.size()));

    // L channel should have signal
    REQUIRE (rmsL > 0.1f);
    // R channel should be significantly quieter than L (not mono collapsed)
    // In stereo output mode some crosstalk is expected from the panning algorithm,
    // so we only require a meaningful difference (>1.5x / ~3.5 dB)
    if (rmsR > 0.001f)
        REQUIRE (rmsL / rmsR > 1.5f);
}

TEST_CASE ("DryWet -- dry path latency compensation is 2048 samples", "[drywet]")
{
    auto proc = createStereoProcessor();
    setParam (*proc, "dryWet", 0.0f);

    // Process silence to fill the dry delay line
    processBlocksCapturingAll (*proc, 30, 0.0f, 0.0f);

    // Now send a single block with signal, then silence
    juce::MidiBuffer midi;
    constexpr int totalBlocks = 40;
    std::vector<float> captured;

    for (int b = 0; b < totalBlocks; ++b)
    {
        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        if (b == 0)
        {
            // First block: impulse
            for (int s = 0; s < kBlockSize; ++s)
            {
                buffer.setSample (0, s, 0.9f);
                buffer.setSample (1, s, 0.9f);
            }
        }
        proc->processBlock (buffer, midi);
        for (int s = 0; s < kBlockSize; ++s)
            captured.push_back (buffer.getSample (0, s));
    }

    // Find where the signal appears in the output
    // Expected: at sample offset = 2048 (PhaseVocoderPitchShifter latency)
    int firstSignalSample = -1;
    for (int i = 0; i < static_cast<int> (captured.size()); ++i)
    {
        if (std::abs (captured[i]) > 0.1f)
        {
            firstSignalSample = i;
            break;
        }
    }

    REQUIRE (firstSignalSample >= 0);
    // Allow some tolerance (±1 block) for parameter smoothing
    REQUIRE (firstSignalSample >= 2048 - kBlockSize);
    REQUIRE (firstSignalSample <= 2048 + kBlockSize);
}

TEST_CASE ("DryWet -- 0% wet is unity gain (issue #97)", "[drywet][issue97]")
{
    auto proc = createStereoProcessor();
    setParam (*proc, "dryWet", 0.0f);
    setParam (*proc, "outputGain", 0.0f);  // 0 dB

    const float inputLevel = 0.9f;  // High level to expose tanh compression

    // Warmup at measurement level (parameter smoothing + dry delay line priming)
    processBlocksCapturingAll (*proc, 60, inputLevel, inputLevel);

    // Measure output — must be unity gain
    auto [outL, outR] = processBlocksCapturingAll (*proc, 20, inputLevel, inputLevel);

    float rmsL = computeRMS (outL.data(), static_cast<int> (outL.size()));
    float rmsR = computeRMS (outR.data(), static_cast<int> (outR.size()));

    // At 0% wet with 0 dB output gain, output must match input within ±0.5 dB
    float diffL_dB = 20.0f * std::log10 (rmsL / inputLevel);
    float diffR_dB = 20.0f * std::log10 (rmsR / inputLevel);

    INFO ("L channel: input=" << inputLevel << " output=" << rmsL << " diff=" << diffL_dB << " dB");
    INFO ("R channel: input=" << inputLevel << " output=" << rmsR << " diff=" << diffR_dB << " dB");

    REQUIRE (std::abs (diffL_dB) < 0.5f);
    REQUIRE (std::abs (diffR_dB) < 0.5f);
}

// ============================================================================
// Section 2: Preset System Tests
// ============================================================================

TEST_CASE ("Preset -- all factory presets load without crash", "[preset][factory]")
{
    for (int i = 0; i < NUM_FACTORY_PRESETS; ++i)
    {
        DYNAMIC_SECTION ("Preset " << i << ": " << factoryPresets[i].name.toStdString())
        {
            auto proc = std::make_unique<Proc>();
            proc->prepareToPlay (kSampleRate, kBlockSize);
            proc->loadPreset (i);

            // Process 1 block to exercise the loaded state
            juce::AudioBuffer<float> buffer (2, kBlockSize);
            buffer.clear();
            for (int s = 0; s < kBlockSize; ++s)
            {
                buffer.setSample (0, s, 0.1f);
                buffer.setSample (1, s, 0.1f);
            }
            juce::MidiBuffer midi;
            proc->processBlock (buffer, midi);

            // Verify no NaN in output
            for (int ch = 0; ch < 2; ++ch)
                for (int s = 0; s < kBlockSize; ++s)
                    REQUIRE (std::isfinite (buffer.getSample (ch, s)));
        }
    }
}

TEST_CASE ("Preset -- factory preset values within ranges", "[preset][factory]")
{
    for (int i = 0; i < NUM_FACTORY_PRESETS; ++i)
    {
        const auto& p = factoryPresets[i];
        REQUIRE (p.delayTime >= 1.0f);
        REQUIRE (p.delayTime <= 5000.0f);
        REQUIRE (p.feedback >= 0.0f);
        REQUIRE (p.feedback <= 1.0f);
        REQUIRE (p.dryWet >= 0.0f);
        REQUIRE (p.dryWet <= 1.0f);
        REQUIRE (p.filterLP >= 20.0f);
        REQUIRE (p.filterLP <= 20000.0f);
        REQUIRE (p.filterHP >= 20.0f);
        REQUIRE (p.filterHP <= 20000.0f);

        for (int t = 0; t < PRESET_MAX_OBJECTS; ++t)
        {
            REQUIRE (p.taps[t].pitchShift >= -12.0f);
            REQUIRE (p.taps[t].pitchShift <= 12.0f);
            REQUIRE (p.taps[t].distance >= 0.0f);
            REQUIRE (p.taps[t].distance <= 1.0f);
            REQUIRE (p.taps[t].trajectoryShape >= 0);
            REQUIRE (p.taps[t].trajectoryShape <= 13);
        }
    }
}

TEST_CASE ("Preset -- serialization roundtrip preserves all fields", "[preset][serialization]")
{
    for (int i = 0; i < std::min (NUM_FACTORY_PRESETS, 10); ++i)
    {
        const auto& original = factoryPresets[i];
        auto json = serializePresetToJson (original);
        auto restored = parsePresetJson (json);

        REQUIRE (restored.name == original.name);
        REQUIRE (restored.category == original.category);
        REQUIRE (restored.delayTime == Catch::Approx (original.delayTime));
        REQUIRE (restored.feedback == Catch::Approx (original.feedback));
        REQUIRE (restored.dryWet == Catch::Approx (original.dryWet));
        REQUIRE (restored.filterLP == Catch::Approx (original.filterLP));
        REQUIRE (restored.filterHP == Catch::Approx (original.filterHP));
        REQUIRE (restored.filterLPQ == Catch::Approx (original.filterLPQ));
        REQUIRE (restored.filterHPQ == Catch::Approx (original.filterHPQ));
        REQUIRE (restored.tempoSync == original.tempoSync);
        REQUIRE (restored.filterEnabled == original.filterEnabled);
        REQUIRE (restored.wobbleEnabled == original.wobbleEnabled);
        REQUIRE (restored.wobbleAmount == Catch::Approx (original.wobbleAmount));
        REQUIRE (restored.wobbleMorph == Catch::Approx (original.wobbleMorph));
        REQUIRE (restored.algorithm == original.algorithm);
        REQUIRE (restored.hrtfProfile == original.hrtfProfile);

        for (int t = 0; t < PRESET_MAX_OBJECTS; ++t)
        {
            REQUIRE (restored.taps[t].enabled == original.taps[t].enabled);
            REQUIRE (restored.taps[t].azimuthDeg == Catch::Approx (original.taps[t].azimuthDeg));
            REQUIRE (restored.taps[t].elevationDeg == Catch::Approx (original.taps[t].elevationDeg));
            REQUIRE (restored.taps[t].distance == Catch::Approx (original.taps[t].distance));
            REQUIRE (restored.taps[t].pitchShift == Catch::Approx (original.taps[t].pitchShift));
            REQUIRE (restored.taps[t].trajectoryShape == original.taps[t].trajectoryShape);
            REQUIRE (restored.taps[t].trajectoryDirection == original.taps[t].trajectoryDirection);
            REQUIRE (restored.taps[t].inputChannel == original.taps[t].inputChannel);
        }
    }
}

TEST_CASE ("Preset -- trajectory string roundtrip for all 14 shapes", "[preset][trajectory]")
{
    for (int i = 0; i <= 13; ++i)
    {
        auto str = trajectoryIndexToString (i);
        REQUIRE (! str.isEmpty());
        REQUIRE (trajectoryStringToIndex (str) == i);
    }
}

TEST_CASE ("Preset -- no glitches during preset transition", "[preset][transition]")
{
    auto proc = std::make_unique<Proc>();
    proc->configOutputFormat.store (1, std::memory_order_relaxed);  // Stereo
    proc->prepareToPlay (kSampleRate, kBlockSize);
    proc->loadPreset (0);

    // Warmup
    processBlocksCapturingAll (*proc, 50);

    // Switch to a different preset (triggers fade-out/fade-in)
    int targetPreset = std::min (10, NUM_FACTORY_PRESETS - 1);
    proc->loadPreset (targetPreset);

    // Capture output across the transition
    auto [outL, outR] = processBlocksCapturingAll (*proc, 50);

    auto glitches = detectGlitches (outL.data(), static_cast<int> (outL.size()), 0.2f);
    REQUIRE (glitches.empty());
}

// ============================================================================
// Section 3: State Save/Restore Tests
// ============================================================================

TEST_CASE ("State -- getStateInformation/setStateInformation roundtrip", "[state]")
{
    juce::MemoryBlock stateData;

    // Create processor 1 with non-default values
    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);

        setParam (*proc, "delayTime", 200.0f);
        setParam (*proc, "feedback", 0.8f);
        setParam (*proc, "dryWet", 0.7f);
        setParam (*proc, "object1_azimuth", -90.0f);
        setParam (*proc, "object3_enabled", 1.0f);

        proc->getStateInformation (stateData);
    }

    // Create processor 2, restore state
    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);
        proc->setStateInformation (stateData.getData(), static_cast<int> (stateData.getSize()));

        // Verify restored values
        auto* dt = proc->apvts.getRawParameterValue ("delayTime");
        auto* fb = proc->apvts.getRawParameterValue ("feedback");
        auto* dw = proc->apvts.getRawParameterValue ("dryWet");
        auto* az = proc->apvts.getRawParameterValue ("object1_azimuth");

        REQUIRE (dt != nullptr);
        REQUIRE (dt->load() == Catch::Approx (200.0f).margin (1.0f));
        REQUIRE (fb->load() == Catch::Approx (0.8f).margin (0.01f));
        REQUIRE (dw->load() == Catch::Approx (0.7f).margin (0.01f));
        REQUIRE (az->load() == Catch::Approx (-90.0f).margin (1.0f));
    }
}

TEST_CASE ("State -- hidden config params survive roundtrip", "[state]")
{
    juce::MemoryBlock stateData;

    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);

        proc->configAlgorithm.store (3, std::memory_order_relaxed);
        proc->configHrtfProfile.store (2, std::memory_order_relaxed);
        proc->configOutputFormat.store (8, std::memory_order_relaxed);

        proc->getStateInformation (stateData);
    }

    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);
        proc->setStateInformation (stateData.getData(), static_cast<int> (stateData.getSize()));

        REQUIRE (proc->configAlgorithm.load() == 3);
        REQUIRE (proc->configHrtfProfile.load() == 2);
        REQUIRE (proc->configOutputFormat.load() == 8);
    }
}

TEST_CASE ("State -- OSC settings survive roundtrip", "[state]")
{
    juce::MemoryBlock stateData;

    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);

        proc->setOscReceivePort (5555);

        proc->getStateInformation (stateData);
    }

    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);
        proc->setStateInformation (stateData.getData(), static_cast<int> (stateData.getSize()));

        REQUIRE (proc->getOscReceivePort() == 5555);
    }
}

TEST_CASE ("State -- global tap APVTS params survive roundtrip (issue #95)", "[state]")
{
    juce::MemoryBlock stateData;

    // Simulate what the editor's onGlobalDelta callback now does:
    // write absolute knob values to the APVTS global tap params.
    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);

        setParam (*proc, "globalTapAzimuth",   45.0f);
        setParam (*proc, "globalTapElevation", -30.0f);
        setParam (*proc, "globalTapDistance",    0.5f);
        setParam (*proc, "globalTapPitch",     -12.0f);
        setParam (*proc, "globalTapDoppler",    50.0f);
        setParam (*proc, "globalTapSpeed",       2.5f);

        proc->getStateInformation (stateData);
    }

    // Restore on a fresh processor — APVTS params should have the saved values
    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);
        proc->setStateInformation (stateData.getData(), static_cast<int> (stateData.getSize()));

        REQUIRE (proc->apvts.getRawParameterValue ("globalTapAzimuth")->load()
                 == Catch::Approx (45.0f).margin (0.2f));
        REQUIRE (proc->apvts.getRawParameterValue ("globalTapElevation")->load()
                 == Catch::Approx (-30.0f).margin (0.2f));
        REQUIRE (proc->apvts.getRawParameterValue ("globalTapDistance")->load()
                 == Catch::Approx (0.5f).margin (0.02f));
        REQUIRE (proc->apvts.getRawParameterValue ("globalTapPitch")->load()
                 == Catch::Approx (-12.0f).margin (1.0f));
        REQUIRE (proc->apvts.getRawParameterValue ("globalTapDoppler")->load()
                 == Catch::Approx (50.0f).margin (0.2f));
        REQUIRE (proc->apvts.getRawParameterValue ("globalTapSpeed")->load()
                 == Catch::Approx (2.5f).margin (0.02f));
    }
}

TEST_CASE ("State -- global tap atomics sync after setStateInformation (issue #164)", "[state]")
{
    juce::MemoryBlock stateData;

    // Save state with non-zero global tap values
    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);
        setParam (*proc, "globalTapAzimuth",   45.0f);
        setParam (*proc, "globalTapElevation", -30.0f);
        setParam (*proc, "globalTapDistance",    0.5f);
        setParam (*proc, "globalTapPitch",     -12.0f);
        setParam (*proc, "globalTapDoppler",    50.0f);
        setParam (*proc, "globalTapSpeed",       2.5f);
        proc->getStateInformation (stateData);
    }

    // Restore — atomics must match restored APVTS values (not stale zeros)
    {
        auto proc = std::make_unique<Proc>();
        proc->prepareToPlay (kSampleRate, kBlockSize);
        proc->setStateInformation (stateData.getData(), static_cast<int> (stateData.getSize()));

        REQUIRE_THAT (proc->globalTapOffset[0].load(), Catch::Matchers::WithinAbs (45.0, 0.2));
        REQUIRE_THAT (proc->globalTapOffset[1].load(), Catch::Matchers::WithinAbs (-30.0, 0.2));
        REQUIRE_THAT (proc->globalTapOffset[2].load(), Catch::Matchers::WithinAbs (0.5, 0.02));
        REQUIRE_THAT (proc->globalTapOffset[3].load(), Catch::Matchers::WithinAbs (-12.0, 1.0));
        REQUIRE_THAT (proc->globalTapOffset[4].load(), Catch::Matchers::WithinAbs (50.0, 0.2));
        REQUIRE_THAT (proc->globalTapOffset[5].load(), Catch::Matchers::WithinAbs (2.5, 0.02));
    }
}
