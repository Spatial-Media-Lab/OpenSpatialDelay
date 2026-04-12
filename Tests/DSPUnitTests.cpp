#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Source/PhaseVocoderPitchShifter.h"
#include "../Source/DopplerVelocity.h"
#include "../Source/FilterBank.h"
#include "TestUtilities.h"
#include <cmath>
#include <vector>

using Catch::Matchers::WithinAbs;

// Estimate dominant frequency by counting zero crossings
static float estimateFrequency (const float* buffer, int numSamples, float sampleRate)
{
    int crossings = 0;
    for (int i = 1; i < numSamples; ++i)
    {
        if ((buffer[i - 1] >= 0.0f && buffer[i] < 0.0f) ||
            (buffer[i - 1] < 0.0f && buffer[i] >= 0.0f))
            crossings++;
    }
    // Each full cycle has 2 zero crossings
    return static_cast<float> (crossings) * sampleRate / (2.0f * static_cast<float> (numSamples));
}

// ============================================================================
// Section 1: PhaseVocoderPitchShifter Tests
// ============================================================================

TEST_CASE ("PV -- latency is exactly 2048 samples", "[pv][latency]")
{
    REQUIRE (PhaseVocoderPitchShifter::getLatency() == 2048);
    REQUIRE (PhaseVocoderPitchShifter::kFFTSize == 2048);
}

TEST_CASE ("PV -- unity pitch preserves sine frequency", "[pv][accuracy]")
{
    PhaseVocoderPitchShifter pv;

    constexpr float freq = 440.0f;
    constexpr float sr = static_cast<float> (kSampleRate);
    constexpr int latencySamples = PhaseVocoderPitchShifter::kFFTSize + PhaseVocoderPitchShifter::kHopSize;
    constexpr int totalSamples = latencySamples + 20000;

    // Process through PV at 0 semitones
    std::vector<float> output;
    output.reserve (totalSamples);
    for (int i = 0; i < totalSamples; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * freq * static_cast<float> (i) / sr);
        output.push_back (pv.process (in, 0.0f));
    }

    // Measure frequency of steady-state output (after latency settling)
    float measuredFreq = estimateFrequency (output.data() + latencySamples,
                                            totalSamples - latencySamples, sr);
    REQUIRE (measuredFreq == Catch::Approx (freq).margin (3.0f));
}

TEST_CASE ("PV -- +12 semitones doubles frequency", "[pv][accuracy]")
{
    PhaseVocoderPitchShifter pv;

    constexpr float freq = 440.0f;
    constexpr float sr = static_cast<float> (kSampleRate);
    constexpr int latencySamples = PhaseVocoderPitchShifter::kFFTSize + PhaseVocoderPitchShifter::kHopSize;
    constexpr int totalSamples = latencySamples + 20000;

    std::vector<float> output;
    output.reserve (totalSamples);
    for (int i = 0; i < totalSamples; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * freq * static_cast<float> (i) / sr);
        output.push_back (pv.process (in, 12.0f));
    }

    float measuredFreq = estimateFrequency (output.data() + latencySamples,
                                            totalSamples - latencySamples, sr);
    float expectedFreq = 880.0f;
    REQUIRE (measuredFreq == Catch::Approx (expectedFreq).margin (10.0f));
}

TEST_CASE ("PV -- -12 semitones halves frequency", "[pv][accuracy]")
{
    PhaseVocoderPitchShifter pv;

    constexpr float freq = 440.0f;
    constexpr float sr = static_cast<float> (kSampleRate);
    constexpr int latencySamples = PhaseVocoderPitchShifter::kFFTSize + PhaseVocoderPitchShifter::kHopSize;
    constexpr int totalSamples = latencySamples + 30000;  // More samples for lower freq measurement

    std::vector<float> output;
    output.reserve (totalSamples);
    for (int i = 0; i < totalSamples; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * freq * static_cast<float> (i) / sr);
        output.push_back (pv.process (in, -12.0f));
    }

    float measuredFreq = estimateFrequency (output.data() + latencySamples,
                                            totalSamples - latencySamples, sr);
    float expectedFreq = 220.0f;
    REQUIRE (measuredFreq == Catch::Approx (expectedFreq).margin (15.0f));
}

TEST_CASE ("PV -- +7 semitones shifts by perfect fifth", "[pv][accuracy]")
{
    PhaseVocoderPitchShifter pv;

    constexpr float freq = 440.0f;
    constexpr float sr = static_cast<float> (kSampleRate);
    constexpr int latencySamples = PhaseVocoderPitchShifter::kFFTSize + PhaseVocoderPitchShifter::kHopSize;
    constexpr int totalSamples = latencySamples + 20000;

    std::vector<float> output;
    output.reserve (totalSamples);
    for (int i = 0; i < totalSamples; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * freq * static_cast<float> (i) / sr);
        output.push_back (pv.process (in, 7.0f));
    }

    float measuredFreq = estimateFrequency (output.data() + latencySamples,
                                            totalSamples - latencySamples, sr);
    float expectedFreq = freq * std::pow (2.0f, 7.0f / 12.0f);  // ~659 Hz
    REQUIRE (measuredFreq == Catch::Approx (expectedFreq).margin (15.0f));
}

TEST_CASE ("PV -- bypass hysteresis: pitch < 0.005 enters bypass", "[pv][bypass]")
{
    PhaseVocoderPitchShifter pv;
    constexpr float sr = static_cast<float> (kSampleRate);
    constexpr int settle = PhaseVocoderPitchShifter::kFFTSize * 3;

    // First, activate PV with 5.0 semitones
    for (int i = 0; i < settle; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (i) / sr);
        pv.process (in, 5.0f);
    }

    // Switch to 0.003 (below enter threshold 0.005) — should enter bypass
    // Process enough samples for crossfade to complete
    constexpr int fadeLen = 2048;
    std::vector<float> pvOut, dryOut;
    for (int i = 0; i < fadeLen; ++i)
    {
        int sample = settle + i;
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (sample) / sr);
        pvOut.push_back (pv.process (in, 0.003f));
    }

    // After crossfade, should be outputting latency-compensated dry
    // Verify the last portion has 440 Hz (not pitched)
    float freq = estimateFrequency (pvOut.data() + fadeLen / 2, fadeLen / 2, sr);
    REQUIRE (freq == Catch::Approx (440.0f).margin (10.0f));
}

TEST_CASE ("PV -- bypass hysteresis: pitch 0.01 stays bypassed", "[pv][bypass]")
{
    PhaseVocoderPitchShifter pv;
    constexpr float sr = static_cast<float> (kSampleRate);
    constexpr int settle = PhaseVocoderPitchShifter::kFFTSize * 3;

    // Start bypassed (default), process at 0.01 (between 0.005 and 0.02)
    for (int i = 0; i < settle; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (i) / sr);
        pv.process (in, 0.01f);
    }

    // Collect output — should still be dry (440 Hz)
    constexpr int measureLen = 4096;
    std::vector<float> output;
    for (int i = 0; i < measureLen; ++i)
    {
        int sample = settle + i;
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (sample) / sr);
        output.push_back (pv.process (in, 0.01f));
    }

    float freq = estimateFrequency (output.data(), measureLen, sr);
    REQUIRE (freq == Catch::Approx (440.0f).margin (5.0f));
}

TEST_CASE ("PV -- bypass hysteresis: pitch > 0.02 exits bypass", "[pv][bypass]")
{
    PhaseVocoderPitchShifter pv;
    constexpr float sr = static_cast<float> (kSampleRate);
    constexpr int settle = PhaseVocoderPitchShifter::kFFTSize * 3;

    // Start bypassed, then activate with 5.0 semitones (well above 0.02)
    for (int i = 0; i < settle; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (i) / sr);
        pv.process (in, 5.0f);
    }

    // Collect steady-state output — should be pitch-shifted (not 440 Hz)
    constexpr int measureLen = 8192;
    std::vector<float> output;
    for (int i = 0; i < measureLen; ++i)
    {
        int sample = settle + i;
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (sample) / sr);
        output.push_back (pv.process (in, 5.0f));
    }

    float freq = estimateFrequency (output.data(), measureLen, sr);
    float expected = 440.0f * std::pow (2.0f, 5.0f / 12.0f);
    REQUIRE (freq == Catch::Approx (expected).margin (15.0f));
}

TEST_CASE ("PV -- reset() clears all state", "[pv][reset]")
{
    PhaseVocoderPitchShifter pv;
    constexpr float sr = static_cast<float> (kSampleRate);

    // Process sine at +5 semitones to fill internal buffers
    for (int i = 0; i < 10000; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (i) / sr);
        pv.process (in, 5.0f);
    }

    // Reset
    pv.reset();

    // Process silence — output should be zero after latency fill
    float maxOutput = 0.0f;
    for (int i = 0; i < PhaseVocoderPitchShifter::kFFTSize * 2; ++i)
    {
        float out = pv.process (0.0f, 0.0f);
        if (i > PhaseVocoderPitchShifter::kFFTSize + PhaseVocoderPitchShifter::kHopSize)
            maxOutput = std::max (maxOutput, std::abs (out));
    }
    REQUIRE (maxOutput < 0.001f);
}

TEST_CASE ("PV -- no glitches in steady-state sine", "[pv][glitch]")
{
    PhaseVocoderPitchShifter pv;
    constexpr float sr = static_cast<float> (kSampleRate);
    constexpr int latencySamples = PhaseVocoderPitchShifter::kFFTSize + PhaseVocoderPitchShifter::kHopSize;
    constexpr int totalSamples = latencySamples + 500 * kBlockSize;

    std::vector<float> output;
    output.reserve (totalSamples);
    for (int i = 0; i < totalSamples; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (i) / sr);
        output.push_back (pv.process (in, 3.0f));
    }

    // Check for glitches in steady-state output only
    auto glitches = detectGlitches (output.data() + latencySamples,
                                    totalSamples - latencySamples, 0.15f);
    REQUIRE (glitches.empty());
}

// ============================================================================
// Section 2: DopplerVelocity Tests
// ============================================================================

TEST_CASE ("Doppler -- stationary object produces 0 semitones", "[doppler][basic]")
{
    DopplerVelocity dv;
    dv.resetAll();  // resets prevAz/prevEl/prevDist to (0, 0, 0.5)

    constexpr float blockDur = static_cast<float> (kBlockSize) / static_cast<float> (kSampleRate);

    // Use position matching resetAll() defaults to avoid initial Doppler
    for (int b = 0; b < 30; ++b)
    {
        dv.update (0, 0.0f, 0.0f, 0.5f, 1.0f, blockDur);
        dv.smooth (0);
    }

    REQUIRE (dv.getSmoothedSemitones (0) == Catch::Approx (0.0f).margin (0.001f));
}

TEST_CASE ("Doppler -- approaching object produces positive pitch shift", "[doppler][basic]")
{
    DopplerVelocity dv;
    dv.resetAll();

    constexpr float blockDur = static_cast<float> (kBlockSize) / static_cast<float> (kSampleRate);

    // Object moves closer (distance decreasing)
    for (int b = 0; b < 30; ++b)
    {
        float dist = 0.9f - static_cast<float> (b) * 0.025f;
        dv.update (0, 0.0f, 0.0f, dist, 1.0f, blockDur);
        dv.smooth (0);
    }

    // Approaching = radial distance decreasing = positive pitch (Doppler up-shift)
    REQUIRE (dv.getSmoothedSemitones (0) > 0.0f);
}

TEST_CASE ("Doppler -- receding object produces negative pitch shift", "[doppler][basic]")
{
    DopplerVelocity dv;
    dv.resetAll();

    constexpr float blockDur = static_cast<float> (kBlockSize) / static_cast<float> (kSampleRate);

    // Object moves away (distance increasing)
    for (int b = 0; b < 30; ++b)
    {
        float dist = 0.1f + static_cast<float> (b) * 0.025f;
        dv.update (0, 0.0f, 0.0f, dist, 1.0f, blockDur);
        dv.smooth (0);
    }

    // Receding = radial distance increasing = negative pitch (Doppler down-shift)
    REQUIRE (dv.getSmoothedSemitones (0) < 0.0f);
}

TEST_CASE ("Doppler -- EMA smoothing eliminates step artifacts", "[doppler][smoothing]")
{
    DopplerVelocity dv;
    dv.resetAll();

    constexpr float blockDur = static_cast<float> (kBlockSize) / static_cast<float> (kSampleRate);

    // Apply a sharp position jump (azimuth 0 -> pi)
    dv.update (0, 0.0f, 0.0f, 0.5f, 1.0f, blockDur);
    dv.smooth (0);
    dv.update (0, kPi, 0.0f, 0.5f, 1.0f, blockDur);
    dv.smooth (0);

    float prevVal = dv.getSmoothedSemitones (0);

    // Track block-to-block deltas over 30 blocks
    float maxDelta = 0.0f;
    for (int b = 0; b < 30; ++b)
    {
        dv.update (0, kPi, 0.0f, 0.5f, 1.0f, blockDur);
        dv.smooth (0);
        float val = dv.getSmoothedSemitones (0);
        float delta = std::abs (val - prevVal);
        maxDelta = std::max (maxDelta, delta);
        prevVal = val;
    }

    // EMA smoothing should prevent huge per-block jumps
    REQUIRE (maxDelta < 1.0f);
}

TEST_CASE ("Doppler -- reset() zeroes all state", "[doppler][reset]")
{
    DopplerVelocity dv;
    dv.resetAll();

    constexpr float blockDur = static_cast<float> (kBlockSize) / static_cast<float> (kSampleRate);

    // Accumulate some Doppler
    for (int b = 0; b < 10; ++b)
    {
        float dist = 0.9f - static_cast<float> (b) * 0.05f;
        dv.update (0, 0.0f, 0.0f, dist, 1.0f, blockDur);
        dv.smooth (0);
    }
    REQUIRE (std::abs (dv.getRawSemitones (0)) > 0.0f);

    // Reset
    dv.reset (0, 0.0f, 0.0f, 0.5f);

    REQUIRE (dv.getRawSemitones (0) == 0.0f);
    REQUIRE (dv.getSmoothedSemitones (0) == 0.0f);
    REQUIRE (dv.getPrevSmoothedSemitones (0) == 0.0f);
    REQUIRE (dv.positionChanged (0) == false);
}

TEST_CASE ("Doppler -- clearDisabled() zeroes raw semitones", "[doppler]")
{
    DopplerVelocity dv;
    dv.resetAll();

    constexpr float blockDur = static_cast<float> (kBlockSize) / static_cast<float> (kSampleRate);

    // Accumulate some Doppler
    for (int b = 0; b < 10; ++b)
    {
        float dist = 0.9f - static_cast<float> (b) * 0.05f;
        dv.update (0, 0.0f, 0.0f, dist, 1.0f, blockDur);
        dv.smooth (0);
    }

    dv.clearDisabled (0);
    REQUIRE (dv.getRawSemitones (0) == 0.0f);
}

TEST_CASE ("Doppler -- zero dopplerAmount produces 0 semitones", "[doppler]")
{
    DopplerVelocity dv;
    dv.resetAll();

    constexpr float blockDur = static_cast<float> (kBlockSize) / static_cast<float> (kSampleRate);

    // Move object with amount = 0
    for (int b = 0; b < 20; ++b)
    {
        float dist = 0.9f - static_cast<float> (b) * 0.03f;
        dv.update (0, 0.0f, 0.0f, dist, 0.0f, blockDur);
        dv.smooth (0);
    }

    REQUIRE (dv.getRawSemitones (0) == 0.0f);
}

TEST_CASE ("Doppler -- semitones clamped to +/-12", "[doppler]")
{
    DopplerVelocity dv;
    dv.resetAll();

    // Extreme position jump in tiny block duration
    constexpr float tinyBlockDur = 0.0001f;
    dv.update (0, 0.0f, 0.0f, 0.01f, 1.0f, tinyBlockDur);
    dv.update (0, 0.0f, 0.0f, 0.99f, 1.0f, tinyBlockDur);

    REQUIRE (std::abs (dv.getRawSemitones (0)) <= 12.0f);
}

// ============================================================================
// Section 3: FilterBank Tests
// ============================================================================

TEST_CASE ("FilterBank -- bypass is transparent", "[filter][bypass]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    // Call updateCoefficients with filterEnabled=false (bypass)
    for (int b = 0; b < 10; ++b)
        fb.updateCoefficients (kSampleRate, 5000.0f, 200.0f, 0.707f, 0.707f, false);

    constexpr int numSamples = 10000;
    float maxDiff = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * 440.0f * static_cast<float> (i)
                                     / static_cast<float> (kSampleRate));
        float out = fb.processTapSample (0, in);
        maxDiff = std::max (maxDiff, std::abs (out - in));
    }

    REQUIRE (maxDiff < 1e-6f);
}

TEST_CASE ("FilterBank -- LP at 1 kHz attenuates 5 kHz", "[filter][lp]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    // Force LP to 1 kHz by running updateCoefficients many times (EMA settling)
    fb.setSmoothedFrequencies (1000.0f, 20.0f, 0.707f, 0.707f);
    fb.invalidateCoefficients();
    for (int b = 0; b < 50; ++b)
        fb.updateCoefficients (kSampleRate, 1000.0f, 20.0f, 0.707f, 0.707f, true);

    constexpr int numSamples = 10000;
    std::vector<float> input (numSamples), output (numSamples);
    fillSine (input.data(), numSamples, 5000.0f, static_cast<float> (kSampleRate), 0.5f, 0);

    for (int i = 0; i < numSamples; ++i)
        output[i] = fb.processTapSample (0, input[i]);

    // Skip first 500 samples for filter settling
    float inputRMS = computeRMS (input.data() + 500, numSamples - 500);
    float outputRMS = computeRMS (output.data() + 500, numSamples - 500);
    float attenuationDB = 20.0f * std::log10 (outputRMS / inputRMS);

    REQUIRE (attenuationDB < -12.0f);
}

TEST_CASE ("FilterBank -- LP at 1 kHz passes 200 Hz", "[filter][lp]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    fb.setSmoothedFrequencies (1000.0f, 20.0f, 0.707f, 0.707f);
    fb.invalidateCoefficients();
    for (int b = 0; b < 50; ++b)
        fb.updateCoefficients (kSampleRate, 1000.0f, 20.0f, 0.707f, 0.707f, true);

    constexpr int numSamples = 10000;
    std::vector<float> input (numSamples), output (numSamples);
    fillSine (input.data(), numSamples, 200.0f, static_cast<float> (kSampleRate), 0.5f, 0);

    for (int i = 0; i < numSamples; ++i)
        output[i] = fb.processTapSample (0, input[i]);

    float inputRMS = computeRMS (input.data() + 500, numSamples - 500);
    float outputRMS = computeRMS (output.data() + 500, numSamples - 500);
    float diffDB = 20.0f * std::log10 (outputRMS / inputRMS);

    REQUIRE (std::abs (diffDB) < 3.0f);
}

TEST_CASE ("FilterBank -- HP at 1 kHz attenuates 200 Hz", "[filter][hp]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    fb.setSmoothedFrequencies (20000.0f, 1000.0f, 0.707f, 0.707f);
    fb.invalidateCoefficients();
    for (int b = 0; b < 50; ++b)
        fb.updateCoefficients (kSampleRate, 20000.0f, 1000.0f, 0.707f, 0.707f, true);

    constexpr int numSamples = 10000;
    std::vector<float> input (numSamples), output (numSamples);
    fillSine (input.data(), numSamples, 200.0f, static_cast<float> (kSampleRate), 0.5f, 0);

    for (int i = 0; i < numSamples; ++i)
        output[i] = fb.processTapSample (0, input[i]);

    float inputRMS = computeRMS (input.data() + 500, numSamples - 500);
    float outputRMS = computeRMS (output.data() + 500, numSamples - 500);
    float attenuationDB = 20.0f * std::log10 (outputRMS / inputRMS);

    REQUIRE (attenuationDB < -12.0f);
}

TEST_CASE ("FilterBank -- HP at 1 kHz passes 5 kHz", "[filter][hp]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    fb.setSmoothedFrequencies (20000.0f, 1000.0f, 0.707f, 0.707f);
    fb.invalidateCoefficients();
    for (int b = 0; b < 50; ++b)
        fb.updateCoefficients (kSampleRate, 20000.0f, 1000.0f, 0.707f, 0.707f, true);

    constexpr int numSamples = 10000;
    std::vector<float> input (numSamples), output (numSamples);
    fillSine (input.data(), numSamples, 5000.0f, static_cast<float> (kSampleRate), 0.5f, 0);

    for (int i = 0; i < numSamples; ++i)
        output[i] = fb.processTapSample (0, input[i]);

    float inputRMS = computeRMS (input.data() + 500, numSamples - 500);
    float outputRMS = computeRMS (output.data() + 500, numSamples - 500);
    float diffDB = 20.0f * std::log10 (outputRMS / inputRMS);

    REQUIRE (std::abs (diffDB) < 3.0f);
}

TEST_CASE ("FilterBank -- air absorption: distance 0 is transparent", "[filter][air]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    // Distance 0 = max cutoff (~20 kHz)
    for (int b = 0; b < 20; ++b)
        fb.updateAirAbsorption (0, kSampleRate, 0.0f, true, true, true);

    constexpr int numSamples = 10000;
    std::vector<float> input (numSamples), output (numSamples);
    fillSine (input.data(), numSamples, 10000.0f, static_cast<float> (kSampleRate), 0.5f, 0);

    for (int i = 0; i < numSamples; ++i)
        output[i] = fb.processAirSample (0, input[i]);

    float inputRMS = computeRMS (input.data() + 500, numSamples - 500);
    float outputRMS = computeRMS (output.data() + 500, numSamples - 500);
    float diffDB = 20.0f * std::log10 (outputRMS / inputRMS);

    REQUIRE (std::abs (diffDB) < 1.5f);
}

TEST_CASE ("FilterBank -- air absorption: distance 1 attenuates highs", "[filter][air]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    // Distance 1 = low cutoff (~500 Hz minimum)
    for (int b = 0; b < 50; ++b)
        fb.updateAirAbsorption (0, kSampleRate, 1.0f, true, true, true);

    constexpr int numSamples = 10000;
    std::vector<float> input (numSamples), output (numSamples);
    fillSine (input.data(), numSamples, 10000.0f, static_cast<float> (kSampleRate), 0.5f, 0);

    for (int i = 0; i < numSamples; ++i)
        output[i] = fb.processAirSample (0, input[i]);

    float inputRMS = computeRMS (input.data() + 500, numSamples - 500);
    float outputRMS = computeRMS (output.data() + 500, numSamples - 500);
    float attenuationDB = 20.0f * std::log10 (outputRMS / inputRMS);

    REQUIRE (attenuationDB < -6.0f);
}

TEST_CASE ("FilterBank -- air absorption inactive is transparent", "[filter][air]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    // airActive = false — should pass through
    fb.updateAirAbsorption (0, kSampleRate, 1.0f, false, true, true);

    constexpr int numSamples = 5000;
    float maxDiff = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        float in = 0.5f * std::sin (2.0f * kPi * 10000.0f * static_cast<float> (i)
                                     / static_cast<float> (kSampleRate));
        float out = fb.processAirSample (0, in);
        maxDiff = std::max (maxDiff, std::abs (out - in));
    }

    REQUIRE (maxDiff < 1e-6f);
}

TEST_CASE ("FilterBank -- resetAll() clears filter state", "[filter][reset]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    fb.setSmoothedFrequencies (1000.0f, 1000.0f, 4.0f, 4.0f);
    fb.invalidateCoefficients();
    for (int b = 0; b < 20; ++b)
        fb.updateCoefficients (kSampleRate, 1000.0f, 1000.0f, 4.0f, 4.0f, true);

    // Process loud resonant signal to build up filter state
    for (int i = 0; i < 5000; ++i)
        fb.processTapSample (0, 0.9f);

    // Reset
    fb.resetAll();

    // Process silence — output should be zero (no ringing)
    float maxOutput = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        float out = fb.processTapSample (0, 0.0f);
        maxOutput = std::max (maxOutput, std::abs (out));
    }

    REQUIRE (maxOutput < 0.001f);
}

TEST_CASE ("FilterBank -- all 12 air filters initialize without crash", "[filter]")
{
    FilterBank fb;
    fb.prepare (kSampleRate, kBlockSize);

    for (int i = 0; i < 12; ++i)
    {
        fb.updateAirAbsorption (i, kSampleRate, 0.5f, true, true, true);
        float out = fb.processAirSample (i, 0.5f);
        // Just verify it doesn't crash and produces finite output
        REQUIRE (std::isfinite (out));
    }
}
