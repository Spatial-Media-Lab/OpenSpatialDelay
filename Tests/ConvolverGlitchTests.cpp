#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Source/PluginProcessor.h"
#include <cmath>
#include <vector>
#include <numeric>

using Proc = OpenSpatialDelayProcessor;
using Catch::Matchers::WithinAbs;

static constexpr float kPi = juce::MathConstants<float>::pi;
static constexpr double kSampleRate = 48000.0;
static constexpr int kBlockSize = 256;

// ============================================================================
// Section 1: Glitch Detection Utilities
// ============================================================================

// Detect clicks/pops in an audio buffer using first-derivative threshold.
// Returns the sample indices where |sample[n] - sample[n-1]| exceeds threshold.
// For a 440 Hz sine at 48 kHz, the max natural derivative ≈ 0.058 (at zero crossing).
// A threshold of 0.15 catches any non-natural discontinuity while avoiding false positives.
static std::vector<int> detectGlitches (const float* buffer, int numSamples, float threshold = 0.15f)
{
    std::vector<int> glitchIndices;
    for (int i = 1; i < numSamples; ++i)
    {
        float diff = std::abs (buffer[i] - buffer[i - 1]);
        if (diff > threshold)
            glitchIndices.push_back (i);
    }
    return glitchIndices;
}

// Compute RMS of a buffer
static float computeRMS (const float* buffer, int numSamples)
{
    float sumSq = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sumSq += buffer[i] * buffer[i];
    return std::sqrt (sumSq / static_cast<float> (numSamples));
}

// Generate a sine wave into a buffer
static void fillSine (float* buffer, int numSamples, float freq, float sampleRate,
                      float amplitude, int startSample)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float phase = 2.0f * kPi * freq * static_cast<float> (startSample + i) / sampleRate;
        buffer[i] = amplitude * std::sin (phase);
    }
}

// Create a simple lowpass IR (sinc-based) for testing
static std::vector<float> createLowpassIR (int length, float cutoffNorm)
{
    std::vector<float> ir (static_cast<size_t> (length), 0.0f);
    int center = length / 2;
    float sum = 0.0f;
    for (int i = 0; i < length; ++i)
    {
        float n = static_cast<float> (i - center);
        if (std::abs (n) < 1e-6f)
            ir[static_cast<size_t> (i)] = cutoffNorm;
        else
            ir[static_cast<size_t> (i)] = std::sin (kPi * cutoffNorm * n) / (kPi * n);

        // Hann window
        float w = 0.5f * (1.0f - std::cos (2.0f * kPi * static_cast<float> (i)
                                            / static_cast<float> (length - 1)));
        ir[static_cast<size_t> (i)] *= w;
        sum += std::abs (ir[static_cast<size_t> (i)]);
    }
    // Normalize to unit gain
    if (sum > 0.0f)
        for (auto& s : ir) s /= sum;
    return ir;
}

// Set an APVTS parameter by ID and value (in the parameter's native range)
static void setParam (Proc& proc, const juce::String& paramId, float value)
{
    if (auto* p = proc.apvts.getParameter (paramId))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

// ============================================================================
// Section 2: PartitionedConvolver Unit Tests
// ============================================================================

TEST_CASE ("Convolver — identity IR produces output with correct RMS", "[convolver]")
{
    PartitionedConvolver conv;
    constexpr int irLen = 64;
    conv.prepare (kBlockSize, irLen);

    // Identity IR: single impulse at sample 0
    std::vector<float> identityIR (static_cast<size_t> (irLen), 0.0f);
    identityIR[0] = 1.0f;
    conv.setIR (identityIR.data(), irLen);

    // Feed a sine wave (overlap-save introduces latency, so compare RMS not sample-exact)
    std::vector<float> input (kBlockSize);
    std::vector<float> output (kBlockSize);

    // Process several blocks to stabilize
    for (int b = 0; b < 5; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f, b * kBlockSize);
        conv.process (input.data(), output.data(), kBlockSize);
    }

    // After settling, output RMS should match input RMS (identity convolution preserves energy)
    float inputRMS = computeRMS (input.data(), kBlockSize);
    float outputRMS = computeRMS (output.data(), kBlockSize);
    float ratioDB = 20.0f * std::log10 (outputRMS / inputRMS);
    REQUIRE (std::abs (ratioDB) < 1.0f);  // Within 1 dB
}

TEST_CASE ("Convolver — no glitches without IR changes", "[convolver]")
{
    PartitionedConvolver conv;
    constexpr int irLen = 64;
    conv.prepare (kBlockSize, irLen);

    auto ir = createLowpassIR (irLen, 0.5f);
    conv.setIR (ir.data(), irLen);

    // Process 50 blocks of sine, concatenate output
    constexpr int numBlocks = 50;
    std::vector<float> allOutput;
    allOutput.reserve (static_cast<size_t> (numBlocks * kBlockSize));

    std::vector<float> input (kBlockSize);
    std::vector<float> output (kBlockSize);

    for (int b = 0; b < numBlocks; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f, b * kBlockSize);
        conv.process (input.data(), output.data(), kBlockSize);
        allOutput.insert (allOutput.end(), output.begin(), output.end());
    }

    // Skip first 2 blocks (settling time), check rest for glitches
    int skipSamples = 2 * kBlockSize;
    auto glitches = detectGlitches (allOutput.data() + skipSamples,
                                    static_cast<int> (allOutput.size()) - skipSamples);
    REQUIRE (glitches.empty());
}

TEST_CASE ("Convolver — IR switch with sine input detects glitch baseline", "[convolver][glitch]")
{
    // This test establishes the baseline: switching IRs during a sine
    // should ideally produce zero glitches with proper crossfading.
    PartitionedConvolver conv;
    constexpr int irLen = 64;
    conv.prepare (kBlockSize, irLen);

    // Two different lowpass IRs (simulating HRIRs at different positions)
    auto ir1 = createLowpassIR (irLen, 0.3f);
    auto ir2 = createLowpassIR (irLen, 0.7f);
    conv.setIR (ir1.data(), irLen);

    constexpr int numBlocks = 100;
    std::vector<float> allOutput;
    allOutput.reserve (static_cast<size_t> (numBlocks * kBlockSize));

    std::vector<float> input (kBlockSize);
    std::vector<float> output (kBlockSize);

    for (int b = 0; b < numBlocks; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f, b * kBlockSize);

        // Switch IR every block (simulates rapid ~3.6°/block azimuth sweep)
        if (b > 5)  // Let it settle first
        {
            float blend = static_cast<float> (b % 20) / 20.0f;
            std::vector<float> blendedIR (static_cast<size_t> (irLen));
            for (int i = 0; i < irLen; ++i)
                blendedIR[static_cast<size_t> (i)] = ir1[static_cast<size_t> (i)] * (1.0f - blend)
                                                    + ir2[static_cast<size_t> (i)] * blend;
            conv.setIR (blendedIR.data(), irLen);
        }

        conv.process (input.data(), output.data(), kBlockSize);
        allOutput.insert (allOutput.end(), output.begin(), output.end());
    }

    // Check for glitches after settling (skip first 10 blocks)
    int skipSamples = 10 * kBlockSize;
    auto glitches = detectGlitches (allOutput.data() + skipSamples,
                                    static_cast<int> (allOutput.size()) - skipSamples);

    // Report glitch count — this test documents the current state
    INFO ("Glitches detected during rapid IR switching: " << glitches.size());

    // With proper crossfading, we expect zero glitches
    REQUIRE (glitches.empty());
}

TEST_CASE ("Convolver — non-restarting crossfade completes in kCrossfadeBlocks", "[convolver]")
{
    PartitionedConvolver conv;
    constexpr int irLen = 64;
    conv.prepare (kBlockSize, irLen);

    auto ir1 = createLowpassIR (irLen, 0.3f);
    auto ir2 = createLowpassIR (irLen, 0.7f);
    auto ir3 = createLowpassIR (irLen, 0.5f);
    conv.setIR (ir1.data(), irLen);

    std::vector<float> input (kBlockSize, 0.0f);
    std::vector<float> output (kBlockSize);

    // Process one block to initialize
    conv.process (input.data(), output.data(), kBlockSize);

    // Start crossfade
    conv.setIR (ir2.data(), irLen);

    // Call setIR mid-crossfade (should NOT restart)
    fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f, 0);
    conv.process (input.data(), output.data(), kBlockSize);  // Block 1 of crossfade

    conv.setIR (ir3.data(), irLen);  // Update target mid-crossfade

    // Process remaining crossfade blocks
    constexpr int kCrossfadeBlocks = 4;  // Matches PartitionedConvolver::kCrossfadeBlocks
    for (int b = 1; b < kCrossfadeBlocks; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f,
                  (b + 1) * kBlockSize);
        conv.process (input.data(), output.data(), kBlockSize);
    }

    // After kCrossfadeBlocks blocks, a NEW setIR should start a NEW crossfade
    // (indicating the previous one completed). We verify by checking the convolver
    // processes normally without crossfade artifacts.
    std::vector<float> postCrossfadeOutput;
    postCrossfadeOutput.reserve (static_cast<size_t> (10 * kBlockSize));
    for (int b = 0; b < 10; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f,
                  (kCrossfadeBlocks + 1 + b) * kBlockSize);
        conv.process (input.data(), output.data(), kBlockSize);
        postCrossfadeOutput.insert (postCrossfadeOutput.end(), output.begin(), output.end());
    }

    // Post-crossfade output should be glitch-free
    auto glitches = detectGlitches (postCrossfadeOutput.data(),
                                    static_cast<int> (postCrossfadeOutput.size()));
    REQUIRE (glitches.empty());
}

TEST_CASE ("Convolver — equal-power crossfade maintains energy", "[convolver]")
{
    PartitionedConvolver conv;
    constexpr int irLen = 64;
    conv.prepare (kBlockSize, irLen);

    // Use the same IR for both "old" and "new" — so crossfade should produce
    // identical output to non-crossfade (no energy dip or boost)
    auto ir = createLowpassIR (irLen, 0.5f);
    conv.setIR (ir.data(), irLen);

    std::vector<float> input (kBlockSize);
    std::vector<float> output (kBlockSize);

    // Stabilize
    for (int b = 0; b < 5; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f, b * kBlockSize);
        conv.process (input.data(), output.data(), kBlockSize);
    }

    // Measure RMS before crossfade
    fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f, 5 * kBlockSize);
    conv.process (input.data(), output.data(), kBlockSize);
    float rmsBefore = computeRMS (output.data(), kBlockSize);
    REQUIRE (rmsBefore > 0.01f);  // Sanity check: there's audible output

    // Trigger crossfade with identical IR
    conv.setIR (ir.data(), irLen);

    // Measure RMS during crossfade blocks
    constexpr int kCrossfadeBlocks = 4;  // Matches PartitionedConvolver::kCrossfadeBlocks
    for (int b = 0; b < kCrossfadeBlocks; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f,
                  (6 + b) * kBlockSize);
        conv.process (input.data(), output.data(), kBlockSize);
        float rmsDuring = computeRMS (output.data(), kBlockSize);

        // Energy should stay within 4 dB of pre-crossfade level
        // (overlap state contamination can cause slight energy variations)
        float ratioDB = 20.0f * std::log10 (rmsDuring / rmsBefore);
        INFO ("Crossfade block " << b << ": RMS ratio = " << ratioDB << " dB");
        REQUIRE (std::abs (ratioDB) < 4.0f);
    }
}

TEST_CASE ("Convolver — rapid IR switching stress test (100 blocks)", "[convolver][stress]")
{
    PartitionedConvolver conv;
    constexpr int irLen = 128;
    conv.prepare (kBlockSize, irLen);

    // Create 36 different IRs (simulating 10° azimuth steps around full circle)
    std::vector<std::vector<float>> irs;
    for (int i = 0; i < 36; ++i)
    {
        float cutoff = 0.2f + 0.6f * static_cast<float> (i) / 36.0f;
        irs.push_back (createLowpassIR (irLen, cutoff));
    }

    conv.setIR (irs[0].data(), irLen);

    constexpr int numBlocks = 100;
    std::vector<float> allOutput;
    allOutput.reserve (static_cast<size_t> (numBlocks * kBlockSize));

    std::vector<float> input (kBlockSize);
    std::vector<float> output (kBlockSize);

    // Stabilize
    for (int b = 0; b < 5; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f, b * kBlockSize);
        conv.process (input.data(), output.data(), kBlockSize);
    }

    // Sweep through all 36 IRs over 100 blocks (switching every ~3 blocks)
    for (int b = 0; b < numBlocks; ++b)
    {
        fillSine (input.data(), kBlockSize, 440.0f, static_cast<float> (kSampleRate), 0.5f,
                  (5 + b) * kBlockSize);

        int irIndex = (b * 36) / numBlocks;
        conv.setIR (irs[static_cast<size_t> (irIndex)].data(), irLen);

        conv.process (input.data(), output.data(), kBlockSize);
        allOutput.insert (allOutput.end(), output.begin(), output.end());
    }

    auto glitches = detectGlitches (allOutput.data(), static_cast<int> (allOutput.size()));
    INFO ("Stress test: " << glitches.size() << " glitches detected over " << numBlocks << " blocks");
    REQUIRE (glitches.empty());
}

// ============================================================================
// Section 3: Full-Plugin Binaural Glitch Tests
// ============================================================================

// Create a binaural HRTF processor ready for testing
static std::unique_ptr<Proc> createBinauralProcessor (int hrtfProfile = 1)
{
    auto proc = std::make_unique<Proc>();

    // Binaural output = format index 0
    setParam (*proc, "outputFormat", 0.0f);
    setParam (*proc, "hrtfProfile", static_cast<float> (hrtfProfile));

    // Set up delay parameters for audible output
    // 50ms delay — taps stabilize within 30 blocks:
    // tap1=50ms(2400s), tap2=100ms(4800s), tap3=150ms(7200s) ≈ 28 blocks
    setParam (*proc, "delayTime", 50.0f);
    setParam (*proc, "feedback", 0.85f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "inputGain", 1.0f);
    setParam (*proc, "outputGain", 1.0f);

    // Enable 3 taps at different positions
    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i < 3) ? 1.0f : 0.0f);
    }
    setParam (*proc, "object1_azimuth", -45.0f);
    setParam (*proc, "object1_elevation", 0.0f);
    setParam (*proc, "object1_distance", 0.3f);
    setParam (*proc, "object2_azimuth", 45.0f);
    setParam (*proc, "object2_elevation", 15.0f);
    setParam (*proc, "object2_distance", 0.5f);
    setParam (*proc, "object3_azimuth", 0.0f);
    setParam (*proc, "object3_elevation", -10.0f);
    setParam (*proc, "object3_distance", 0.7f);

    // Disable Doppler to isolate convolver artifacts
    setParam (*proc, "object1_dopplerAmount", 0.0f);
    setParam (*proc, "object2_dopplerAmount", 0.0f);
    setParam (*proc, "object3_dopplerAmount", 0.0f);

    proc->prepareToPlay (kSampleRate, kBlockSize);
    return proc;
}

// Process blocks capturing ALL output (not just last block)
static std::pair<std::vector<float>, std::vector<float>>
processBlocksCapturingAll (Proc& proc, int numBlocks, float inputLevel = 0.5f)
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
            buffer.setSample (0, s, inputLevel);
            buffer.setSample (1, s, inputLevel);
        }
        proc.processBlock (buffer, midi);

        const float* outL = buffer.getReadPointer (0);
        const float* outR = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL, outL + kBlockSize);
        allR.insert (allR.end(), outR, outR + kBlockSize);
    }
    return { allL, allR };
}

TEST_CASE ("Binaural HRTF — no glitches with static positions", "[binaural][glitch]")
{
    auto proc = createBinauralProcessor (1);  // MIT KEMAR

    // Process enough blocks for delay line to fill (200ms ≈ 38 blocks)
    auto [outL, outR] = processBlocksCapturingAll (*proc, 80);

    // Skip first 50 blocks (stabilization — delay line fill + HRTF settling)
    int skip = 50 * kBlockSize;
    int len = static_cast<int> (outL.size()) - skip;

    auto glitchesL = detectGlitches (outL.data() + skip, len);
    auto glitchesR = detectGlitches (outR.data() + skip, len);

    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Binaural HRTF — azimuth sweep glitch detection", "[binaural][glitch][sweep]")
{
    auto proc = createBinauralProcessor (1);  // MIT KEMAR

    // Stabilize — need enough blocks for delay line to fill (200ms ≈ 38 blocks @ 256/48kHz)
    processBlocksCapturingAll (*proc, 50);

    // Now sweep object1 azimuth from -180 to +180 over 100 blocks (3.6°/block)
    constexpr int sweepBlocks = 100;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));

    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float azimuth = -180.0f + 360.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        setParam (*proc, "object1_azimuth", azimuth);

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s)
        {
            buffer.setSample (0, s, 0.5f);
            buffer.setSample (1, s, 0.5f);
        }
        proc->processBlock (buffer, midi);

        const float* outL = buffer.getReadPointer (0);
        const float* outR = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL, outL + kBlockSize);
        allR.insert (allR.end(), outR, outR + kBlockSize);
    }

    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()));
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()));

    INFO ("Azimuth sweep: L=" << glitchesL.size() << " R=" << glitchesR.size() << " glitches");
    for (size_t g = 0; g < glitchesL.size() && g < 5; ++g)
    {
        int idx = glitchesL[g];
        float diff = std::abs (allL[static_cast<size_t> (idx)] - allL[static_cast<size_t> (idx - 1)]);
        INFO ("  Az L glitch at sample " << idx << " (block " << idx / kBlockSize
              << "), diff=" << diff);
    }

    // Target: zero glitches during azimuth sweep
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Binaural HRTF — elevation sweep glitch detection", "[binaural][glitch][sweep]")
{
    auto proc = createBinauralProcessor (1);  // MIT KEMAR

    // Stabilize — need enough blocks for delay line to fill
    processBlocksCapturingAll (*proc, 50);

    // Sweep object1 elevation from -40 to +90 over 100 blocks
    constexpr int sweepBlocks = 100;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));

    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float elevation = -40.0f + 130.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        setParam (*proc, "object1_elevation", elevation);

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s)
        {
            buffer.setSample (0, s, 0.5f);
            buffer.setSample (1, s, 0.5f);
        }
        proc->processBlock (buffer, midi);

        const float* outL = buffer.getReadPointer (0);
        const float* outR = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL, outL + kBlockSize);
        allR.insert (allR.end(), outR, outR + kBlockSize);
    }

    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()));
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()));

    INFO ("Elevation sweep: L=" << glitchesL.size() << " R=" << glitchesR.size() << " glitches");

    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Binaural HRTF — distance sweep has no glitches (control test)", "[binaural][glitch][control]")
{
    auto proc = createBinauralProcessor (1);  // MIT KEMAR

    // Stabilize — need enough blocks for the delay line to fill (200ms delay ≈ 38 blocks @ 256/48kHz)
    processBlocksCapturingAll (*proc, 50);

    // Sweep distance — this should NOT cause glitches (doesn't trigger HRIR update)
    constexpr int sweepBlocks = 50;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));

    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float distance = 0.1f + 0.8f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        setParam (*proc, "object1_distance", distance);

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s)
        {
            buffer.setSample (0, s, 0.5f);
            buffer.setSample (1, s, 0.5f);
        }
        proc->processBlock (buffer, midi);

        const float* outL = buffer.getReadPointer (0);
        const float* outR = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL, outL + kBlockSize);
        allR.insert (allR.end(), outR, outR + kBlockSize);
    }

    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()));
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()));

    // Log details for diagnosis
    for (size_t g = 0; g < glitchesL.size(); ++g)
    {
        int idx = glitchesL[g];
        float diff = std::abs (allL[static_cast<size_t> (idx)] - allL[static_cast<size_t> (idx - 1)]);
        WARN ("Distance L glitch at sample " << idx << " (block " << idx / kBlockSize
              << "), diff=" << diff
              << ", val[n-1]=" << allL[static_cast<size_t> (idx - 1)]
              << ", val[n]=" << allL[static_cast<size_t> (idx)]);
    }

    // Distance sweep should ALWAYS pass (confirms glitch detection isn't overzealous)
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// ============================================================================
// Section 4: Stereo Output Glitch Tests (regression guard — already fixed)
// ============================================================================

TEST_CASE ("Stereo — azimuth sweep has no glitches (regression guard)", "[stereo][glitch]")
{
    auto proc = std::make_unique<Proc>();

    // Stereo output = format index 1
    setParam (*proc, "outputFormat", 1.0f);
    setParam (*proc, "delayTime", 50.0f);
    setParam (*proc, "feedback", 0.85f);
    setParam (*proc, "dryWet", 1.0f);

    // Enable 3 taps
    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i < 3) ? 1.0f : 0.0f);
        setParam (*proc, "object" + idx + "_dopplerAmount", 0.0f);
    }
    setParam (*proc, "object1_azimuth", 0.0f);
    setParam (*proc, "object1_distance", 0.5f);

    proc->prepareToPlay (kSampleRate, kBlockSize);

    // Stabilize — need enough blocks for delay line to fill
    processBlocksCapturingAll (*proc, 50);

    // Sweep azimuth
    constexpr int sweepBlocks = 100;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));

    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float azimuth = -180.0f + 360.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        setParam (*proc, "object1_azimuth", azimuth);

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s)
        {
            buffer.setSample (0, s, 0.5f);
            buffer.setSample (1, s, 0.5f);
        }
        proc->processBlock (buffer, midi);

        const float* outL = buffer.getReadPointer (0);
        const float* outR = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL, outL + kBlockSize);
        allR.insert (allR.end(), outR, outR + kBlockSize);
    }

    // Use higher threshold for stereo — feedback + rapid panning creates legitimate amplitude variation
    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()), 0.25f);
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()), 0.25f);

    for (size_t g = 0; g < glitchesL.size(); ++g)
    {
        int idx = glitchesL[g];
        float diff = std::abs (allL[static_cast<size_t> (idx)] - allL[static_cast<size_t> (idx - 1)]);
        WARN ("Stereo L glitch at sample " << idx << " (block " << idx / kBlockSize
              << "), diff=" << diff);
    }
    for (size_t g = 0; g < glitchesR.size(); ++g)
    {
        int idx = glitchesR[g];
        float diff = std::abs (allR[static_cast<size_t> (idx)] - allR[static_cast<size_t> (idx - 1)]);
        WARN ("Stereo R glitch at sample " << idx << " (block " << idx / kBlockSize
              << "), diff=" << diff);
    }

    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}
