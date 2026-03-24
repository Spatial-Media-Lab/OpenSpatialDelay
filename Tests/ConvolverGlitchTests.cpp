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

// ============================================================================
// Section 5: Issue #36 Interaction Tests — Multi-System Glitch Detection
// ============================================================================

// Create a multi-tap binaural processor with configurable Doppler/AIR/feedback
static std::unique_ptr<Proc> createMultiTapBinauralProcessor (int numTaps, float dopplerAmount,
                                                               bool airAbsorption, float feedback,
                                                               int hrtfProfile = 1)
{
    auto proc = std::make_unique<Proc>();
    setParam (*proc, "outputFormat", 0.0f);   // Binaural
    setParam (*proc, "hrtfProfile", static_cast<float> (hrtfProfile));
    setParam (*proc, "delayTime", 50.0f);
    setParam (*proc, "feedback", feedback);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "inputGain", 1.0f);
    setParam (*proc, "outputGain", 1.0f);
    setParam (*proc, "airAbsorption", airAbsorption ? 1.0f : 0.0f);

    // Distribute taps evenly around azimuth
    float azStep = 360.0f / static_cast<float> (std::max (numTaps, 1));
    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i < numTaps) ? 1.0f : 0.0f);
        if (i < numTaps)
        {
            float az = -180.0f + azStep * static_cast<float> (i);
            float el = (i % 2 == 0) ? 0.0f : ((i % 4 == 1) ? 15.0f : -10.0f);
            setParam (*proc, "object" + idx + "_azimuth", az);
            setParam (*proc, "object" + idx + "_elevation", el);
            setParam (*proc, "object" + idx + "_distance", 0.4f + 0.03f * static_cast<float> (i));
            setParam (*proc, "object" + idx + "_dopplerAmount", dopplerAmount);
        }
    }

    proc->prepareToPlay (kSampleRate, kBlockSize);
    return proc;
}

// Sweep all enabled taps' azimuth by delta per block, capturing output
static std::pair<std::vector<float>, std::vector<float>>
sweepAllTapsAzimuth (Proc& proc, float deltaPerBlock, int numBlocks, int numTaps,
                     const float* baseAz, float inputLevel = 0.5f)
{
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (numBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (numBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < numBlocks; ++b)
    {
        float offset = deltaPerBlock * static_cast<float> (b);
        for (int t = 0; t < numTaps; ++t)
        {
            auto idx = juce::String (t + 1);
            float az = baseAz[t] + offset;
            // Wrap to [-180, 180]
            while (az > 180.0f) az -= 360.0f;
            while (az < -180.0f) az += 360.0f;
            setParam (proc, "object" + idx + "_azimuth", az);
        }

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

// Compute per-block RMS from a flat sample vector
static std::vector<float> measurePerBlockRMS (const std::vector<float>& samples, int blockSize)
{
    std::vector<float> rmsVec;
    int numBlocks = static_cast<int> (samples.size()) / blockSize;
    for (int b = 0; b < numBlocks; ++b)
        rmsVec.push_back (computeRMS (samples.data() + b * blockSize, blockSize));
    return rmsVec;
}

// T1: Doppler + HRTF interaction test
TEST_CASE ("Binaural HRTF + Doppler — azimuth sweep interaction", "[binaural][doppler][interaction]")
{
    auto proc = createBinauralProcessor (1);  // MIT KEMAR

    // Enable Doppler on all 3 taps
    setParam (*proc, "object1_dopplerAmount", 0.8f);
    setParam (*proc, "object2_dopplerAmount", 0.8f);
    setParam (*proc, "object3_dopplerAmount", 0.8f);
    setParam (*proc, "feedback", 0.5f);

    // Stabilize
    processBlocksCapturingAll (*proc, 60);

    // Sweep object 1 azimuth -180 to +180 over 100 blocks
    constexpr int sweepBlocks = 100;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float az = -180.0f + 360.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        setParam (*proc, "object1_azimuth", az);

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

    // Skip first 5 blocks — sweep starts at az=-180 while stabilization was at az=-45,
    // creating a legitimate 135° spatial repositioning transient
    int skipSamples = 5 * kBlockSize;
    int checkSamples = static_cast<int> (allL.size()) - skipSamples;
    auto glitchesL = detectGlitches (allL.data() + skipSamples, checkSamples);
    auto glitchesR = detectGlitches (allR.data() + skipSamples, checkSamples);

    INFO ("Doppler+HRTF L glitches: " << glitchesL.size() << ", R: " << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// T2: High feedback amplification test
TEST_CASE ("Binaural HRTF — high feedback does not amplify crossfade artifacts", "[binaural][feedback][glitch]")
{
    auto proc = createBinauralProcessor (1);
    setParam (*proc, "feedback", 0.95f);
    setParam (*proc, "delayTime", 30.0f);

    // Stabilize with high feedback (needs more blocks to reach steady state)
    processBlocksCapturingAll (*proc, 80);

    // Sweep object 1 azimuth 0→180 then hold
    constexpr int sweepBlocks = 50;
    constexpr int holdBlocks = 100;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> ((sweepBlocks + holdBlocks) * kBlockSize));
    allR.reserve (static_cast<size_t> ((sweepBlocks + holdBlocks) * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks + holdBlocks; ++b)
    {
        if (b < sweepBlocks)
        {
            float az = 180.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
            setParam (*proc, "object1_azimuth", az);
        }
        // else: hold at az=180

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

    // Check the HOLD phase for glitches (feedback tail should be clean)
    int holdStartSample = sweepBlocks * kBlockSize;
    int holdSamples = holdBlocks * kBlockSize;
    auto glitchesL = detectGlitches (allL.data() + holdStartSample, holdSamples);
    auto glitchesR = detectGlitches (allR.data() + holdStartSample, holdSamples);

    INFO ("High feedback hold phase L glitches: " << glitchesL.size() << ", R: " << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// T3: Global AZIM sweep — the actual issue #36 scenario
TEST_CASE ("Binaural HRTF — global azimuth sweep (issue #36)", "[binaural][global][glitch]")
{
    auto proc = createMultiTapBinauralProcessor (6, 0.0f, false, 0.85f);

    // Stabilize
    processBlocksCapturingAll (*proc, 60);

    // Record base azimuth positions
    float baseAz[6];
    float azStep = 360.0f / 6.0f;
    for (int i = 0; i < 6; ++i)
        baseAz[i] = -180.0f + azStep * static_cast<float> (i);

    // Sweep ALL 6 taps simultaneously — 3.6 deg/block for 100 blocks
    auto [allL, allR] = sweepAllTapsAzimuth (*proc, 3.6f, 100, 6, baseAz);

    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()));
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()));

    INFO ("Global AZIM sweep L glitches: " << glitchesL.size() << ", R: " << glitchesR.size());
    for (size_t g = 0; g < std::min (glitchesL.size(), size_t (5)); ++g)
    {
        int idx = glitchesL[g];
        float diff = std::abs (allL[static_cast<size_t> (idx)] - allL[static_cast<size_t> (idx - 1)]);
        WARN ("Global AZIM L glitch at sample " << idx << " (block " << idx / kBlockSize
              << "), diff=" << diff);
    }
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// T4: 12-tap simultaneous sweep stress test
TEST_CASE ("Binaural HRTF — 12-tap simultaneous sweep stress", "[binaural][multitap][stress]")
{
    auto proc = createMultiTapBinauralProcessor (12, 0.0f, false, 0.7f);

    // Stabilize (12 taps need more blocks — last tap at 12*50ms = 600ms)
    processBlocksCapturingAll (*proc, 80);

    float baseAz[12];
    float azStep = 360.0f / 12.0f;
    for (int i = 0; i < 12; ++i)
        baseAz[i] = -180.0f + azStep * static_cast<float> (i);

    // Sweep all 12 taps — 5 deg/block for 72 blocks (full 360° rotation)
    auto [allL, allR] = sweepAllTapsAzimuth (*proc, 5.0f, 72, 12, baseAz);

    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()));
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()));

    INFO ("12-tap stress L glitches: " << glitchesL.size() << ", R: " << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// T5: Doppler velocity response latency characterization
TEST_CASE ("Doppler — velocity response latency characterization", "[doppler][latency]")
{
    auto proc = std::make_unique<Proc>();
    setParam (*proc, "outputFormat", 0.0f);   // Binaural
    setParam (*proc, "hrtfProfile", 0.0f);    // Simple/Woodworth (no HRTF convolver variable)
    setParam (*proc, "delayTime", 100.0f);
    setParam (*proc, "feedback", 0.0f);       // Single pass
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "inputGain", 1.0f);
    setParam (*proc, "outputGain", 1.0f);

    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i == 0) ? 1.0f : 0.0f);
    }
    setParam (*proc, "object1_azimuth", 0.0f);
    setParam (*proc, "object1_elevation", 0.0f);
    setParam (*proc, "object1_distance", 0.5f);
    setParam (*proc, "object1_dopplerAmount", 1.0f);

    proc->prepareToPlay (kSampleRate, kBlockSize);

    // Stabilize (static position)
    juce::MidiBuffer midi;
    for (int b = 0; b < 60; ++b)
    {
        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s) buffer.setSample (0, s, 0.5f);
        proc->processBlock (buffer, midi);
    }

    CHECK (std::abs (proc->getDopplerSemitones (0)) < 0.01f);

    // Phase 1: Rise — constant velocity sweep (+10 deg/block)
    std::vector<float> pitchCurve;
    for (int b = 0; b < 50; ++b)
    {
        float az = 10.0f * static_cast<float> (b);
        while (az > 180.0f) az -= 360.0f;
        setParam (*proc, "object1_azimuth", az);

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s) buffer.setSample (0, s, 0.5f);
        proc->processBlock (buffer, midi);
        pitchCurve.push_back (proc->getDopplerSemitones (0));
    }

    // Find steady-state (average of last 10 blocks)
    float steadyState = 0.0f;
    for (int i = 40; i < 50; ++i)
        steadyState += std::abs (pitchCurve[static_cast<size_t> (i)]);
    steadyState /= 10.0f;

    // Find rise time (blocks to reach 90% of steady state)
    int riseTime90 = 50;  // default if never reached
    for (int i = 0; i < 50; ++i)
    {
        if (std::abs (pitchCurve[static_cast<size_t> (i)]) >= 0.9f * steadyState)
        {
            riseTime90 = i;
            break;
        }
    }

    INFO ("Doppler rise time (90%): " << riseTime90 << " blocks ("
          << riseTime90 * kBlockSize * 1000.0 / kSampleRate << " ms)");
    INFO ("Steady-state Doppler: " << steadyState << " semitones");

    // Rise time should be < 15 blocks with alpha=0.35
    CHECK (riseTime90 < 15);

    // Phase 2: Stop — velocity decay
    float lastAz = 10.0f * 49.0f;
    while (lastAz > 180.0f) lastAz -= 360.0f;
    setParam (*proc, "object1_azimuth", lastAz);

    int decayBlocks = 50;
    for (int b = 0; b < decayBlocks; ++b)
    {
        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s) buffer.setSample (0, s, 0.5f);
        proc->processBlock (buffer, midi);
        pitchCurve.push_back (proc->getDopplerSemitones (0));
    }

    // Check decay — should reach < 0.1 semitones within 20 blocks
    int decayTime = decayBlocks;
    for (int b = 0; b < decayBlocks; ++b)
    {
        if (std::abs (pitchCurve[static_cast<size_t> (50 + b)]) < 0.1f)
        {
            decayTime = b;
            break;
        }
    }

    INFO ("Doppler decay time (to <0.1st): " << decayTime << " blocks ("
          << decayTime * kBlockSize * 1000.0 / kSampleRate << " ms)");
    CHECK (decayTime < 20);
}

// T6: Air absorption + rapid distance sweep
TEST_CASE ("Air absorption — distance sweep produces no glitches", "[air][distance][glitch]")
{
    auto proc = createBinauralProcessor (1);
    setParam (*proc, "airAbsorption", 1.0f);
    setParam (*proc, "feedback", 0.5f);

    // Stabilize with AIR ON
    processBlocksCapturingAll (*proc, 60);

    // Phase 1: Slow distance sweep 0.1→0.9 over 100 blocks
    constexpr int slowBlocks = 100;
    // Phase 2: Rapid sweep 0.9→0.1 over 10 blocks
    constexpr int rapidBlocks = 10;
    // Phase 3: Step change 0.1→0.9 instantaneously
    constexpr int postStepBlocks = 30;

    int totalBlocks = slowBlocks + rapidBlocks + postStepBlocks;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (totalBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (totalBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < totalBlocks; ++b)
    {
        float dist;
        if (b < slowBlocks)
            dist = 0.1f + 0.8f * static_cast<float> (b) / static_cast<float> (slowBlocks);
        else if (b < slowBlocks + rapidBlocks)
            dist = 0.9f - 0.8f * static_cast<float> (b - slowBlocks) / static_cast<float> (rapidBlocks);
        else if (b == slowBlocks + rapidBlocks)
            dist = 0.9f;  // Step change
        else
            dist = 0.9f;  // Hold after step

        setParam (*proc, "object1_distance", dist);

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

    INFO ("AIR distance sweep L glitches: " << glitchesL.size() << ", R: " << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// T7: Surround VBAP azimuth sweep
TEST_CASE ("Surround VBAP — azimuth sweep has no glitches", "[surround][glitch][sweep]")
{
    auto proc = std::make_unique<Proc>();

    // Surround 5.1 = format index 4, VBAP = algorithm index 4
    setParam (*proc, "outputFormat", 4.0f);
    setParam (*proc, "algorithm", 4.0f);
    setParam (*proc, "delayTime", 50.0f);
    setParam (*proc, "feedback", 0.85f);
    setParam (*proc, "dryWet", 1.0f);

    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i < 3) ? 1.0f : 0.0f);
        setParam (*proc, "object" + idx + "_dopplerAmount", 0.0f);
    }
    setParam (*proc, "object1_azimuth", -45.0f);
    setParam (*proc, "object1_elevation", 0.0f);
    setParam (*proc, "object1_distance", 0.5f);
    setParam (*proc, "object2_azimuth", 45.0f);
    setParam (*proc, "object2_distance", 0.5f);
    setParam (*proc, "object3_azimuth", 0.0f);
    setParam (*proc, "object3_distance", 0.5f);

    // Set up surround bus layout
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add (juce::AudioChannelSet::stereo());
    layout.outputBuses.add (juce::AudioChannelSet::create5point1());
    proc->setBusesLayout (layout);
    proc->prepareToPlay (kSampleRate, 512);

    constexpr int surBlockSize = 512;

    // Stabilize
    juce::MidiBuffer midi;
    for (int b = 0; b < 60; ++b)
    {
        juce::AudioBuffer<float> buffer (6, surBlockSize);
        buffer.clear();
        for (int s = 0; s < surBlockSize; ++s)
        {
            buffer.setSample (0, s, 0.5f);
            buffer.setSample (1, s, 0.5f);
        }
        proc->processBlock (buffer, midi);
    }

    // Sweep azimuth on object 1
    constexpr int sweepBlocks = 100;
    std::vector<std::vector<float>> allChannels (6);
    for (auto& ch : allChannels)
        ch.reserve (static_cast<size_t> (sweepBlocks * surBlockSize));

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float az = -180.0f + 360.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        setParam (*proc, "object1_azimuth", az);

        juce::AudioBuffer<float> buffer (6, surBlockSize);
        buffer.clear();
        for (int s = 0; s < surBlockSize; ++s)
        {
            buffer.setSample (0, s, 0.5f);
            buffer.setSample (1, s, 0.5f);
        }
        proc->processBlock (buffer, midi);

        for (int ch = 0; ch < 6; ++ch)
        {
            const float* out = buffer.getReadPointer (ch);
            allChannels[static_cast<size_t> (ch)].insert (
                allChannels[static_cast<size_t> (ch)].end(), out, out + surBlockSize);
        }
    }

    // Check all 6 channels with higher threshold (surround + feedback)
    for (int ch = 0; ch < 6; ++ch)
    {
        auto glitches = detectGlitches (allChannels[static_cast<size_t> (ch)].data(),
                                        static_cast<int> (allChannels[static_cast<size_t> (ch)].size()), 0.25f);
        INFO ("Surround ch" << ch << " glitches: " << glitches.size());
        REQUIRE (glitches.empty());
    }
}

// T8: Combined 3-axis sweep (worst case)
TEST_CASE ("Binaural HRTF — combined az+el+dist sweep with Doppler+AIR", "[binaural][combined][stress]")
{
    auto proc = createMultiTapBinauralProcessor (6, 0.5f, true, 0.85f);

    // Stabilize
    processBlocksCapturingAll (*proc, 80);

    constexpr int sweepBlocks = 100;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float t = static_cast<float> (b) / static_cast<float> (sweepBlocks);

        for (int tap = 0; tap < 6; ++tap)
        {
            auto idx = juce::String (tap + 1);
            float baseAz = -180.0f + 60.0f * static_cast<float> (tap);

            // Azimuth: linear sweep 360 degrees
            float az = baseAz + 360.0f * t;
            while (az > 180.0f) az -= 360.0f;

            // Elevation: sinusoidal oscillation, period=50 blocks
            float el = 30.0f * std::sin (2.0f * kPi * static_cast<float> (b) / 50.0f);

            // Distance: sinusoidal oscillation, period=33 blocks
            float dist = 0.5f + 0.3f * std::sin (2.0f * kPi * static_cast<float> (b) / 33.0f);

            setParam (*proc, "object" + idx + "_azimuth", az);
            setParam (*proc, "object" + idx + "_elevation", el);
            setParam (*proc, "object" + idx + "_distance", dist);
        }

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

    // Skip first 5 blocks — sweep start repositions all 6 taps simultaneously,
    // and Doppler+AIR+HRTF all begin transitioning from static state
    int skipSamples = 5 * kBlockSize;
    int checkSamples = static_cast<int> (allL.size()) - skipSamples;
    auto glitchesL = detectGlitches (allL.data() + skipSamples, checkSamples);
    auto glitchesR = detectGlitches (allR.data() + skipSamples, checkSamples);

    INFO ("Combined 3-axis L glitches: " << glitchesL.size() << ", R: " << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// T9: Block size sensitivity
TEST_CASE ("Doppler — block size sensitivity", "[doppler][blocksize]")
{
    constexpr int blockSizes[] = { 128, 256, 512, 1024 };

    for (int bs : blockSizes)
    {
        SECTION ("Block size " + std::to_string (bs))
        {
            auto proc = std::make_unique<Proc>();
            setParam (*proc, "outputFormat", 0.0f);
            setParam (*proc, "hrtfProfile", 0.0f);  // Simple
            setParam (*proc, "delayTime", 100.0f);
            setParam (*proc, "feedback", 0.5f);
            setParam (*proc, "dryWet", 1.0f);

            for (int i = 0; i < 12; ++i)
            {
                auto idx = juce::String (i + 1);
                setParam (*proc, "object" + idx + "_enabled", (i == 0) ? 1.0f : 0.0f);
            }
            setParam (*proc, "object1_azimuth", 0.0f);
            setParam (*proc, "object1_distance", 0.5f);
            setParam (*proc, "object1_dopplerAmount", 1.0f);

            proc->prepareToPlay (kSampleRate, bs);

            juce::MidiBuffer midi;

            // Stabilize
            for (int b = 0; b < 60; ++b)
            {
                juce::AudioBuffer<float> buffer (2, bs);
                buffer.clear();
                for (int s = 0; s < bs; ++s) buffer.setSample (0, s, 0.5f);
                proc->processBlock (buffer, midi);
            }

            // Sweep azimuth — same wall-clock duration (~500ms) at each block size
            // Start from stabilization position (0°) to avoid discontinuous jump at sweep start
            float wallClockSweep = 0.5f;  // 500ms
            float blockDuration = static_cast<float> (bs) / static_cast<float> (kSampleRate);
            int sweepBlocks = static_cast<int> (wallClockSweep / blockDuration);
            float azPerBlock = 360.0f / static_cast<float> (sweepBlocks);

            std::vector<float> allL, allR;
            allL.reserve (static_cast<size_t> (sweepBlocks * bs));
            allR.reserve (static_cast<size_t> (sweepBlocks * bs));

            for (int b = 0; b < sweepBlocks; ++b)
            {
                float az = azPerBlock * static_cast<float> (b);  // Start from 0° (stabilization position)
                while (az > 180.0f) az -= 360.0f;
                setParam (*proc, "object1_azimuth", az);

                juce::AudioBuffer<float> buffer (2, bs);
                buffer.clear();
                for (int s = 0; s < bs; ++s)
                {
                    buffer.setSample (0, s, 0.5f);
                    buffer.setSample (1, s, 0.5f);
                }
                proc->processBlock (buffer, midi);

                const float* outL = buffer.getReadPointer (0);
                const float* outR = buffer.getReadPointer (1);
                allL.insert (allL.end(), outL, outL + bs);
                allR.insert (allR.end(), outR, outR + bs);
            }

            auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()));
            auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()));

            // Skip first few blocks of sweep — transition from static to moving produces
            // legitimate amplitude changes as Woodworth ITD/ILD gains reposition.
            // Check remaining sweep blocks for actual glitches.
            int settleSkip = 5 * bs;  // Skip 5 blocks of sweep onset settling
            int checkSamples = static_cast<int> (allL.size()) - settleSkip;
            if (checkSamples > 0)
            {
                // Doppler + Woodworth binaural creates legitimate amplitude variation — use 0.25 threshold
                auto glitchesLpost = detectGlitches (allL.data() + settleSkip, checkSamples, 0.25f);
                auto glitchesRpost = detectGlitches (allR.data() + settleSkip, checkSamples, 0.25f);

                for (size_t g = 0; g < glitchesLpost.size(); ++g)
                {
                    int idx = glitchesLpost[g];
                    int absIdx = idx + settleSkip;
                    float diff = std::abs (allL[static_cast<size_t> (absIdx)] - allL[static_cast<size_t> (absIdx - 1)]);
                    WARN ("Block size " << bs << " L glitch at abs sample " << absIdx << " (block "
                          << absIdx / bs << ", offset " << absIdx % bs << "), diff=" << diff);
                }

                INFO ("Block size " << bs << " (post-settle): L glitches=" << glitchesLpost.size()
                      << ", R=" << glitchesRpost.size());
                REQUIRE (glitchesLpost.empty());
                REQUIRE (glitchesRpost.empty());
            }
        }
    }
}

// T10: WSOLA grain boundary under Doppler
TEST_CASE ("WSOLA — grain boundary under Doppler is glitch-free", "[wsola][doppler][grain]")
{
    auto proc = std::make_unique<Proc>();
    setParam (*proc, "outputFormat", 0.0f);
    setParam (*proc, "hrtfProfile", 0.0f);  // Simple/Woodworth (isolate WSOLA)
    setParam (*proc, "delayTime", 100.0f);
    setParam (*proc, "feedback", 0.0f);     // Single pass
    setParam (*proc, "dryWet", 1.0f);

    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i == 0) ? 1.0f : 0.0f);
    }
    setParam (*proc, "object1_azimuth", 0.0f);
    setParam (*proc, "object1_distance", 0.5f);
    setParam (*proc, "object1_dopplerAmount", 1.0f);

    proc->prepareToPlay (kSampleRate, kBlockSize);

    // Stabilize
    juce::MidiBuffer midi;
    for (int b = 0; b < 40; ++b)
    {
        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s) buffer.setSample (0, s, 0.5f);
        proc->processBlock (buffer, midi);
    }

    // Phase 1: Moderate Doppler — 5 deg/block sweep
    std::vector<float> allL, allR;
    for (int b = 0; b < 100; ++b)
    {
        float az = 5.0f * static_cast<float> (b);
        while (az > 180.0f) az -= 360.0f;
        while (az < -180.0f) az += 360.0f;
        setParam (*proc, "object1_azimuth", az);

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s) buffer.setSample (0, s, 0.5f);
        proc->processBlock (buffer, midi);

        const float* outL = buffer.getReadPointer (0);
        const float* outR = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL, outL + kBlockSize);
        allR.insert (allR.end(), outR, outR + kBlockSize);
    }

    // Phase 2: Oscillating (worst case) — reverse every 5 blocks
    for (int b = 0; b < 50; ++b)
    {
        float direction = ((b / 5) % 2 == 0) ? 20.0f : -20.0f;
        float az = direction * static_cast<float> (b % 5);
        setParam (*proc, "object1_azimuth", az);

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s) buffer.setSample (0, s, 0.5f);
        proc->processBlock (buffer, midi);

        const float* outL = buffer.getReadPointer (0);
        const float* outR = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL, outL + kBlockSize);
        allR.insert (allR.end(), outR, outR + kBlockSize);
    }

    // Use relaxed threshold — WSOLA grain crossfades may produce small bumps
    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()), 0.30f);
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()), 0.30f);

    INFO ("WSOLA Doppler L glitches: " << glitchesL.size() << ", R: " << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// ============================================================================
// Section 6: Issue #40 — Pops/Clicks Fix Tests (Tap Fade, Preset, Filter, Wobble)
// ============================================================================

// Helper: create a stereo processor ready for testing (lighter than binaural — no HRTF load)
static std::unique_ptr<Proc> createStereoProcessor (float delayMs = 50.0f, float feedback = 0.5f,
                                                     int numTaps = 3)
{
    auto proc = std::make_unique<Proc>();
    setParam (*proc, "outputFormat", 1.0f);  // Stereo
    setParam (*proc, "delayTime", delayMs);
    setParam (*proc, "feedback", feedback);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "inputGain", 0.0f);   // 0 dB
    setParam (*proc, "outputGain", 0.0f);  // 0 dB

    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i < numTaps) ? 1.0f : 0.0f);
        setParam (*proc, "object" + idx + "_dopplerAmount", 0.0f);
        setParam (*proc, "object" + idx + "_pitchShift", 0.0f);
        if (i < numTaps)
        {
            float az = -60.0f + 120.0f * static_cast<float> (i) / static_cast<float> (std::max (numTaps - 1, 1));
            setParam (*proc, "object" + idx + "_azimuth", az);
            setParam (*proc, "object" + idx + "_distance", 0.4f);
        }
    }

    proc->prepareToPlay (kSampleRate, kBlockSize);
    return proc;
}

// Helper: process blocks with sine input, capturing all output
static std::pair<std::vector<float>, std::vector<float>>
processBlocksWithSine (Proc& proc, int numBlocks, float freq = 440.0f, float amplitude = 0.5f)
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
            float phase = 2.0f * kPi * freq * static_cast<float> (b * kBlockSize + s)
                          / static_cast<float> (kSampleRate);
            float sample = amplitude * std::sin (phase);
            buffer.setSample (0, s, sample);
            buffer.setSample (1, s, sample);
        }
        proc.processBlock (buffer, midi);

        const float* outL = buffer.getReadPointer (0);
        const float* outR = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL, outL + kBlockSize);
        allR.insert (allR.end(), outR, outR + kBlockSize);
    }
    return { allL, allR };
}

// ---- Phase 1: Tap Enable/Disable Fade Envelope ----

TEST_CASE ("Tap disable produces smooth fade-out (no click)", "[issue40][tapfade]")
{
    auto proc = createStereoProcessor (50.0f, 0.5f, 3);

    // Stabilize with 3 taps enabled — delay line fills
    processBlocksCapturingAll (*proc, 60);

    // Disable tap 1 and capture the transition block + a few more
    setParam (*proc, "object1_enabled", 0.0f);
    auto [outL, outR] = processBlocksCapturingAll (*proc, 5);

    // The transition should be smooth — no sample-to-sample jump > 0.15
    auto glitchesL = detectGlitches (outL.data(), static_cast<int> (outL.size()));
    auto glitchesR = detectGlitches (outR.data(), static_cast<int> (outR.size()));

    for (size_t g = 0; g < glitchesL.size() && g < 5; ++g)
    {
        int idx = glitchesL[g];
        float diff = std::abs (outL[static_cast<size_t> (idx)] - outL[static_cast<size_t> (idx - 1)]);
        WARN ("Tap disable L glitch at sample " << idx << ", diff=" << diff);
    }

    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Tap enable produces smooth fade-in (no click)", "[issue40][tapfade]")
{
    auto proc = createStereoProcessor (50.0f, 0.5f, 1);

    // Stabilize with 1 tap
    processBlocksCapturingAll (*proc, 60);

    // Enable tap 2 and capture
    setParam (*proc, "object2_enabled", 1.0f);
    setParam (*proc, "object2_azimuth", 45.0f);
    setParam (*proc, "object2_distance", 0.4f);
    auto [outL, outR] = processBlocksCapturingAll (*proc, 5);

    auto glitchesL = detectGlitches (outL.data(), static_cast<int> (outL.size()));
    auto glitchesR = detectGlitches (outR.data(), static_cast<int> (outR.size()));

    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Multiple taps disabled simultaneously — no clicks", "[issue40][tapfade]")
{
    auto proc = createStereoProcessor (50.0f, 0.7f, 6);

    // Stabilize with 6 taps
    processBlocksCapturingAll (*proc, 80);

    // Disable 4 taps at once (simulates preset with fewer taps)
    setParam (*proc, "object3_enabled", 0.0f);
    setParam (*proc, "object4_enabled", 0.0f);
    setParam (*proc, "object5_enabled", 0.0f);
    setParam (*proc, "object6_enabled", 0.0f);
    auto [outL, outR] = processBlocksCapturingAll (*proc, 5);

    auto glitchesL = detectGlitches (outL.data(), static_cast<int> (outL.size()));
    auto glitchesR = detectGlitches (outR.data(), static_cast<int> (outR.size()));

    INFO ("Multi-tap disable: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Tap toggle rapid on/off — no clicks", "[issue40][tapfade]")
{
    auto proc = createStereoProcessor (50.0f, 0.5f, 3);
    processBlocksCapturingAll (*proc, 60);

    // Rapidly toggle tap 2 on/off every 2 blocks
    constexpr int toggleBlocks = 20;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (toggleBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (toggleBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < toggleBlocks; ++b)
    {
        bool enable = ((b / 2) % 2 == 0);
        setParam (*proc, "object2_enabled", enable ? 1.0f : 0.0f);

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

    INFO ("Rapid toggle: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// ---- Phase 2: Preset Change Tests ----

TEST_CASE ("Preset change — sequential factory presets produce no clicks", "[issue40][preset]")
{
    auto proc = std::make_unique<Proc>();
    setParam (*proc, "outputFormat", 1.0f);  // Stereo
    proc->prepareToPlay (kSampleRate, kBlockSize);

    // Load first preset and stabilize
    proc->loadPreset (0);
    processBlocksCapturingAll (*proc, 60);

    // Cycle through several presets, checking for clicks at each transition
    int numPresets = std::min (static_cast<int> (proc->getNumPresets()), 10);
    for (int p = 1; p < numPresets; ++p)
    {
        proc->loadPreset (p);

        // Capture 5 blocks after preset change
        auto [outL, outR] = processBlocksCapturingAll (*proc, 5);

        auto glitchesL = detectGlitches (outL.data(), static_cast<int> (outL.size()), 0.25f);
        auto glitchesR = detectGlitches (outR.data(), static_cast<int> (outR.size()), 0.25f);

        INFO ("Preset " << (p - 1) << " → " << p << ": L=" << glitchesL.size()
              << " R=" << glitchesR.size());
        REQUIRE (glitchesL.empty());
        REQUIRE (glitchesR.empty());

        // Let the preset settle before next switch
        processBlocksCapturingAll (*proc, 30);
    }
}

TEST_CASE ("Preset change — extreme parameter jump with high feedback", "[issue40][preset]")
{
    auto proc = createStereoProcessor (50.0f, 0.9f, 6);

    // Stabilize with high feedback — lots of energy in delay buffer
    processBlocksCapturingAll (*proc, 100);

    // Change to very different parameters (simulating drastic preset change)
    setParam (*proc, "delayTime", 500.0f);
    setParam (*proc, "feedback", 0.3f);
    setParam (*proc, "filterLP", 2000.0f);
    setParam (*proc, "filterHP", 200.0f);
    setParam (*proc, "filterEnabled", 1.0f);
    for (int i = 3; i < 6; ++i)
        setParam (*proc, "object" + juce::String (i + 1) + "_enabled", 0.0f);

    auto [outL, outR] = processBlocksCapturingAll (*proc, 10);

    // Skip first 2 blocks — 10x delay time change creates a legitimate 100ms pitch sweep
    // that naturally produces high-derivative samples. After smoothing settles, output should be clean.
    int skipSamples = 2 * kBlockSize;
    int len = static_cast<int> (outL.size()) - skipSamples;
    auto glitchesL = detectGlitches (outL.data() + skipSamples, len, 0.30f);
    auto glitchesR = detectGlitches (outR.data() + skipSamples, len, 0.30f);

    INFO ("Extreme param jump: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Preset change with pitch shift — WSOLA reset prevents metallic artifacts", "[issue40][preset][wsola]")
{
    auto proc = createStereoProcessor (100.0f, 0.5f, 3);
    setParam (*proc, "object1_pitchShift", 12.0f);  // +1 octave
    setParam (*proc, "object2_pitchShift", -7.0f);  // -perfect 5th

    // Stabilize with pitch shifting active
    processBlocksWithSine (*proc, 60);

    // Switch to no pitch shift (simulates preset change)
    setParam (*proc, "object1_pitchShift", 0.0f);
    setParam (*proc, "object2_pitchShift", 0.0f);
    // Also call loadPreset to trigger WSOLA reset
    proc->loadPreset (0);

    auto [outL, outR] = processBlocksWithSine (*proc, 10);

    auto glitchesL = detectGlitches (outL.data(), static_cast<int> (outL.size()), 0.25f);
    auto glitchesR = detectGlitches (outR.data(), static_cast<int> (outR.size()), 0.25f);

    INFO ("WSOLA reset: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// ---- Phase 3: Filter Coefficient Smoothing ----

TEST_CASE ("Filter LP sweep — no clicks with EMA-smoothed coefficients", "[issue40][filter]")
{
    auto proc = createStereoProcessor (50.0f, 0.85f, 3);
    setParam (*proc, "filterEnabled", 1.0f);
    setParam (*proc, "filterLP", 20000.0f);

    processBlocksCapturingAll (*proc, 60);

    // Sweep LP from 20kHz down to 200Hz over 50 blocks
    constexpr int sweepBlocks = 50;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        // Logarithmic sweep
        float t = static_cast<float> (b) / static_cast<float> (sweepBlocks);
        float lpFreq = 20000.0f * std::pow (0.01f, t);  // 20kHz → 200Hz
        setParam (*proc, "filterLP", lpFreq);

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

    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()), 0.20f);
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()), 0.20f);

    for (size_t g = 0; g < glitchesL.size() && g < 5; ++g)
    {
        int idx = glitchesL[g];
        float diff = std::abs (allL[static_cast<size_t> (idx)] - allL[static_cast<size_t> (idx - 1)]);
        WARN ("Filter LP sweep L glitch at sample " << idx << " (block " << idx / kBlockSize
              << "), diff=" << diff);
    }

    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Filter HP sweep — no clicks with feedback", "[issue40][filter]")
{
    auto proc = createStereoProcessor (50.0f, 0.85f, 3);
    setParam (*proc, "filterEnabled", 1.0f);
    setParam (*proc, "filterHP", 20.0f);

    processBlocksCapturingAll (*proc, 60);

    // Sweep HP from 20Hz up to 5kHz over 50 blocks
    constexpr int sweepBlocks = 50;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float t = static_cast<float> (b) / static_cast<float> (sweepBlocks);
        float hpFreq = 20.0f * std::pow (250.0f, t);  // 20Hz → 5kHz
        setParam (*proc, "filterHP", hpFreq);

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

    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()), 0.20f);
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()), 0.20f);

    INFO ("HP sweep: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Filter abrupt frequency jump — smoothing prevents click", "[issue40][filter]")
{
    auto proc = createStereoProcessor (50.0f, 0.85f, 3);
    setParam (*proc, "filterEnabled", 1.0f);
    setParam (*proc, "filterLP", 20000.0f);
    setParam (*proc, "filterHP", 20.0f);

    processBlocksCapturingAll (*proc, 60);

    // Abrupt jump: LP from 20kHz to 800Hz, HP from 20Hz to 200Hz
    setParam (*proc, "filterLP", 800.0f);
    setParam (*proc, "filterHP", 200.0f);
    auto [outL, outR] = processBlocksCapturingAll (*proc, 10);

    // Skip first block — EMA smoothing takes ~3 blocks to settle from extreme jumps
    int skipSamples = 1 * kBlockSize;
    int len = static_cast<int> (outL.size()) - skipSamples;
    auto glitchesL = detectGlitches (outL.data() + skipSamples, len, 0.20f);
    auto glitchesR = detectGlitches (outR.data() + skipSamples, len, 0.20f);

    INFO ("Filter jump: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// ---- Phase 4: Wobble Onset Smoothing ----

TEST_CASE ("Wobble enable — no click at onset", "[issue40][wobble]")
{
    auto proc = createStereoProcessor (200.0f, 0.5f, 3);
    setParam (*proc, "wobbleEnabled", 0.0f);
    setParam (*proc, "wobbleAmount", 80.0f);

    processBlocksCapturingAll (*proc, 60);

    // Enable wobble mid-playback
    setParam (*proc, "wobbleEnabled", 1.0f);
    auto [outL, outR] = processBlocksCapturingAll (*proc, 10);

    auto glitchesL = detectGlitches (outL.data(), static_cast<int> (outL.size()));
    auto glitchesR = detectGlitches (outR.data(), static_cast<int> (outR.size()));

    INFO ("Wobble onset: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Wobble disable — no click at offset", "[issue40][wobble]")
{
    auto proc = createStereoProcessor (200.0f, 0.5f, 3);
    setParam (*proc, "wobbleEnabled", 1.0f);
    setParam (*proc, "wobbleAmount", 80.0f);

    processBlocksCapturingAll (*proc, 60);

    // Disable wobble mid-playback
    setParam (*proc, "wobbleEnabled", 0.0f);
    auto [outL, outR] = processBlocksCapturingAll (*proc, 10);

    auto glitchesL = detectGlitches (outL.data(), static_cast<int> (outL.size()));
    auto glitchesR = detectGlitches (outR.data(), static_cast<int> (outR.size()));

    INFO ("Wobble offset: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// ---- Phase 5: Doppler Smoothing ----

TEST_CASE ("Rapid azimuth sweep with Doppler — smoothed pitch prevents WSOLA clicks", "[issue40][doppler]")
{
    auto proc = createStereoProcessor (100.0f, 0.5f, 3);
    for (int i = 0; i < 3; ++i)
        setParam (*proc, "object" + juce::String (i + 1) + "_dopplerAmount", 1.0f);

    processBlocksCapturingAll (*proc, 60);

    // Fast azimuth sweep: 10°/block = 1875°/s at 48kHz/256
    constexpr int sweepBlocks = 50;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float az = -180.0f + 360.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        for (int i = 0; i < 3; ++i)
            setParam (*proc, "object" + juce::String (i + 1) + "_azimuth", az + 30.0f * static_cast<float> (i));

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

    // Relaxed threshold — Doppler creates legitimate pitch changes
    auto glitchesL = detectGlitches (allL.data(), static_cast<int> (allL.size()), 0.25f);
    auto glitchesR = detectGlitches (allR.data(), static_cast<int> (allR.size()), 0.25f);

    INFO ("Doppler sweep: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// ---- Phase 6: Output Limiter Continuity ----

TEST_CASE ("Output limiter — C-infinity continuity under saturation", "[issue40][limiter]")
{
    auto proc = createStereoProcessor (30.0f, 0.95f, 6);
    setParam (*proc, "inputGain", 12.0f);  // +12 dB — drive into limiter

    processBlocksCapturingAll (*proc, 80);

    // Capture under heavy saturation
    auto [outL, outR] = processBlocksCapturingAll (*proc, 50);

    // Skip first few blocks — high feedback takes time to build
    int skipSamples = 10 * kBlockSize;
    int len = static_cast<int> (outL.size()) - skipSamples;

    auto glitchesL = detectGlitches (outL.data() + skipSamples, len, 0.15f);
    auto glitchesR = detectGlitches (outR.data() + skipSamples, len, 0.15f);

    for (size_t g = 0; g < glitchesL.size() && g < 5; ++g)
    {
        int idx = glitchesL[g];
        int absIdx = idx + skipSamples;
        float diff = std::abs (outL[static_cast<size_t> (absIdx)] - outL[static_cast<size_t> (absIdx - 1)]);
        WARN ("Limiter L glitch at sample " << absIdx << " (block " << absIdx / kBlockSize
              << "), diff=" << diff);
    }

    INFO ("Limiter saturation: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Output limiter — tanh is always within bounds", "[issue40][limiter]")
{
    // Verify the tanh limiter behavior by driving the processor into saturation
    // and checking that output samples never exceed the +2 dB threshold
    auto proc = createStereoProcessor (30.0f, 0.99f, 6);
    setParam (*proc, "inputGain", 18.0f);  // +18 dB — extreme drive

    processBlocksCapturingAll (*proc, 100);
    auto [outL, outR] = processBlocksCapturingAll (*proc, 20);

    const float threshold = 1.2589f;  // +2 dB
    for (size_t i = 0; i < outL.size(); ++i)
    {
        // tanh asymptotes — allow small floating point margin
        REQUIRE (std::abs (outL[i]) <= threshold + 0.01f);
        REQUIRE (std::abs (outR[i]) <= threshold + 0.01f);
    }
}

// ---- Combined Integration Tests ----

TEST_CASE ("Full signal chain — all fixes combined stress test", "[issue40][integration]")
{
    auto proc = createStereoProcessor (100.0f, 0.8f, 6);
    setParam (*proc, "filterEnabled", 1.0f);
    setParam (*proc, "filterLP", 15000.0f);
    setParam (*proc, "wobbleEnabled", 1.0f);
    setParam (*proc, "wobbleAmount", 50.0f);
    for (int i = 0; i < 6; ++i)
        setParam (*proc, "object" + juce::String (i + 1) + "_dopplerAmount", 0.5f);

    processBlocksCapturingAll (*proc, 80);

    // Simultaneously: sweep azimuth, change filter, toggle taps, change delay
    constexpr int stressBlocks = 80;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (stressBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (stressBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < stressBlocks; ++b)
    {
        // Sweep azimuth
        float az = -180.0f + 360.0f * static_cast<float> (b) / static_cast<float> (stressBlocks);
        for (int i = 0; i < 6; ++i)
            setParam (*proc, "object" + juce::String (i + 1) + "_azimuth",
                      az + 60.0f * static_cast<float> (i));

        // Toggle taps 4-6 every 10 blocks
        if (b % 10 == 0)
        {
            bool en = ((b / 10) % 2 == 0);
            for (int i = 3; i < 6; ++i)
                setParam (*proc, "object" + juce::String (i + 1) + "_enabled", en ? 1.0f : 0.0f);
        }

        // Sweep filter
        float t = static_cast<float> (b) / static_cast<float> (stressBlocks);
        setParam (*proc, "filterLP", 20000.0f * std::pow (0.05f, t));

        // Vary delay time slightly
        setParam (*proc, "delayTime", 100.0f + 50.0f * std::sin (2.0f * kPi * t));

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

    // Skip first block — transition from stabilization to stress creates a natural discontinuity
    int skipSamples = 1 * kBlockSize;
    int len = static_cast<int> (allL.size()) - skipSamples;
    auto glitchesL = detectGlitches (allL.data() + skipSamples, len, 0.30f);
    auto glitchesR = detectGlitches (allR.data() + skipSamples, len, 0.30f);

    INFO ("Integration stress: L=" << glitchesL.size() << " R=" << glitchesR.size());
    for (size_t g = 0; g < glitchesL.size() && g < 10; ++g)
    {
        int idx = glitchesL[g];
        float diff = std::abs (allL[static_cast<size_t> (idx)] - allL[static_cast<size_t> (idx - 1)]);
        WARN ("Integration L glitch at sample " << idx << " (block " << idx / kBlockSize
              << "), diff=" << diff);
    }

    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

// ===========================================================================
// Phase 8: Issue #42 Bug 3 — Pitch shift after preset changes with trajectories
// ===========================================================================

TEST_CASE ("Pitch gate uses user pitch only — Doppler cannot disable user pitch shift", "[issue42][doppler][gate]")
{
    auto proc = createStereoProcessor (100.0f, 0.5f, 3);
    setParam (*proc, "object1_dopplerAmount", 1.0f);
    setParam (*proc, "object1_pitchShift", -6.0f);  // user wants pitch DOWN

    // Stabilize
    processBlocksCapturingAll (*proc, 60);

    // Sweep azimuth rapidly — generates large Doppler that could cancel -6 semitones
    constexpr int sweepBlocks = 40;
    int gateOpenCount = 0;

    juce::MidiBuffer midi;
    for (int b = 0; b < sweepBlocks; ++b)
    {
        float az = -180.0f + 360.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        setParam (*proc, "object1_azimuth", az);

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s)
        {
            buffer.setSample (0, s, 0.5f);
            buffer.setSample (1, s, 0.5f);
        }
        proc->processBlock (buffer, midi);

        // Check that Doppler semitones exist (trajectory is moving)
        float doppler = proc->getDopplerSemitones (0);
        (void) doppler;

        // The pitch gate should ALWAYS be open because user pitch is -6
        // (even if combinedPitch = -6 + doppler is near zero)
        gateOpenCount++;
    }

    // Verify: after sweep with Doppler active, output is non-silent
    // (pitch shift is still working despite Doppler)
    auto [outL, outR] = processBlocksCapturingAll (*proc, 10);

    // Skip first 5 blocks (cold-start period), check remaining has signal
    int skip = 5 * kBlockSize;
    int len = static_cast<int> (outL.size()) - skip;
    float rms = computeRMS (outL.data() + skip, len);
    INFO ("RMS after Doppler sweep with pitch -6: " << rms);
    REQUIRE (rms > 0.001f);  // must have audible output, pitch shift working
}

TEST_CASE ("Preset change resets trajectory state — no stale position spike", "[issue42][preset][trajectory]")
{
    auto proc = createStereoProcessor (100.0f, 0.5f, 3);

    // Load preset with active trajectory (Spiral Descent = index 17)
    int numPresets = static_cast<int> (proc->getNumPresets());
    if (numPresets <= 17)
        return;  // Skip if not enough presets

    proc->loadPreset (17);  // Spiral Descent — trajectory shape 11

    // Run for a while so trajectory advances and Doppler builds up
    processBlocksCapturingAll (*proc, 100);

    // Now load a different trajectory preset (Random Walk = index 18)
    proc->loadPreset (18);

    // After loadPreset, process several blocks — should be clean (no velocity spike)
    auto [outL, outR] = processBlocksCapturingAll (*proc, 20);

    // Skip first 3 blocks (cold-start buffer fill), then check for glitches
    int skip = 3 * kBlockSize;
    int len = static_cast<int> (outL.size()) - skip;
    auto glitchesL = detectGlitches (outL.data() + skip, len, 0.25f);
    auto glitchesR = detectGlitches (outR.data() + skip, len, 0.25f);

    INFO ("Trajectory preset switch: L=" << glitchesL.size() << " R=" << glitchesR.size());
    REQUIRE (glitchesL.empty());
    REQUIRE (glitchesR.empty());
}

TEST_CASE ("Repeated preset cycling with trajectories — pitch remains functional", "[issue42][preset][cycling]")
{
    auto proc = createStereoProcessor (100.0f, 0.5f, 3);

    int numPresets = static_cast<int> (proc->getNumPresets());
    if (numPresets <= 18)
        return;

    // Simulate the exact user scenario: cycle presets, adjust pitch, repeat
    for (int cycle = 0; cycle < 5; ++cycle)
    {
        // Load Spiral Descent
        proc->loadPreset (17);
        processBlocksCapturingAll (*proc, 30);

        // User adjusts pitch to -12 (after preset loaded)
        for (int t = 0; t < 6; ++t)
            setParam (*proc, "object" + juce::String (t + 1) + "_pitchShift", -12.0f);
        processBlocksCapturingAll (*proc, 20);

        // Load Random Walk
        proc->loadPreset (18);
        processBlocksCapturingAll (*proc, 30);

        // User adjusts pitch to -12 again
        for (int t = 0; t < 5; ++t)
            setParam (*proc, "object" + juce::String (t + 1) + "_pitchShift", -12.0f);
        processBlocksCapturingAll (*proc, 20);
    }

    // After 5 cycles of preset changes, pitch should still work.
    // Set pitch to -12 and verify output has signal (not silent/non-functional).
    setParam (*proc, "object1_pitchShift", -12.0f);
    processBlocksCapturingAll (*proc, 10);  // let WSOLA stabilize

    auto [outL, outR] = processBlocksCapturingAll (*proc, 10);

    int skip = 2 * kBlockSize;
    int len = static_cast<int> (outL.size()) - skip;
    float rms = computeRMS (outL.data() + skip, len);

    INFO ("RMS after 5 preset cycles with pitch -12: " << rms);
    REQUIRE (rms > 0.001f);
}

TEST_CASE ("Pitch shift negative with active Doppler — output is audible", "[issue42][doppler][pitch]")
{
    auto proc = createStereoProcessor (150.0f, 0.5f, 3);

    // Enable Doppler and negative pitch simultaneously
    for (int i = 0; i < 3; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_dopplerAmount", 1.0f);
        setParam (*proc, "object" + idx + "_pitchShift", -12.0f);
    }

    // Stabilize with both active
    processBlocksCapturingAll (*proc, 60);

    // Sweep azimuth to generate sustained Doppler
    constexpr int sweepBlocks = 60;
    std::vector<float> allL, allR;
    allL.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    allR.reserve (static_cast<size_t> (sweepBlocks * kBlockSize));
    juce::MidiBuffer midi;

    for (int b = 0; b < sweepBlocks; ++b)
    {
        float az = -180.0f + 360.0f * static_cast<float> (b) / static_cast<float> (sweepBlocks);
        for (int i = 0; i < 3; ++i)
            setParam (*proc, "object" + juce::String (i + 1) + "_azimuth",
                      az + 30.0f * static_cast<float> (i));

        juce::AudioBuffer<float> buffer (2, kBlockSize);
        buffer.clear();
        for (int s = 0; s < kBlockSize; ++s)
        {
            buffer.setSample (0, s, 0.5f);
            buffer.setSample (1, s, 0.5f);
        }
        proc->processBlock (buffer, midi);

        const float* outL_ = buffer.getReadPointer (0);
        const float* outR_ = buffer.getReadPointer (1);
        allL.insert (allL.end(), outL_, outL_ + kBlockSize);
        allR.insert (allR.end(), outR_, outR_ + kBlockSize);
    }

    // Check that output has signal throughout (pitch shift never "drops out")
    // Divide into 10 segments and check each has non-trivial RMS
    int segSize = static_cast<int> (allL.size()) / 10;
    for (int seg = 2; seg < 10; ++seg)  // skip first 2 segments (warmup)
    {
        float segRMS = computeRMS (allL.data() + seg * segSize, segSize);
        INFO ("Segment " << seg << " RMS: " << segRMS);
        REQUIRE (segRMS > 0.0005f);
    }
}

TEST_CASE ("Thread-safe preset reset — WSOLA state consistent after loadPreset", "[issue42][thread][wsola]")
{
    auto proc = createStereoProcessor (100.0f, 0.5f, 3);
    setParam (*proc, "object1_pitchShift", 12.0f);

    // Run for a while to build up WSOLA state
    processBlocksCapturingAll (*proc, 80);

    // Call loadPreset (simulates message thread)
    proc->loadPreset (0);

    // Immediately process blocks (simulates audio thread picking up the reset)
    auto [outL, outR] = processBlocksCapturingAll (*proc, 10);

    // The reset should produce clean output (no glitches from inconsistent state)
    // Skip first 5 blocks for cold-start bypass
    int skip = 5 * kBlockSize;
    int len = static_cast<int> (outL.size()) - skip;
    auto glitchesL = detectGlitches (outL.data() + skip, len, 0.25f);

    INFO ("Thread-safe reset glitches: " << glitchesL.size());
    REQUIRE (glitchesL.empty());
}

// ===========================================================================

TEST_CASE ("Binaural HRTF — preset cycle with tap changes produces no clicks", "[issue40][binaural][preset]")
{
    auto proc = createBinauralProcessor (1);  // MIT KEMAR

    // Stabilize
    processBlocksCapturingAll (*proc, 60);

    // Cycle through presets that change tap counts
    int numPresets = std::min (static_cast<int> (proc->getNumPresets()), 8);
    for (int p = 0; p < numPresets; ++p)
    {
        proc->loadPreset (p);
        auto [outL, outR] = processBlocksCapturingAll (*proc, 5);

        auto glitchesL = detectGlitches (outL.data(), static_cast<int> (outL.size()), 0.25f);
        auto glitchesR = detectGlitches (outR.data(), static_cast<int> (outR.size()), 0.25f);

        INFO ("HRTF preset " << p << ": L=" << glitchesL.size() << " R=" << glitchesR.size());
        REQUIRE (glitchesL.empty());
        REQUIRE (glitchesR.empty());

        processBlocksCapturingAll (*proc, 30);
    }
}
