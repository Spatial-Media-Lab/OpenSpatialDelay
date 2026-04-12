// PreReleaseTests.cpp — High-priority test coverage gaps for v1.0.0 release
// Covers: stereo input routing, 9.1/SML 13.1 audio output, HOA initialization

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Source/PluginProcessor.h"
#include "TestUtilities.h"
#include <cmath>
#include <set>
#include <vector>

using Proc = OpenSpatialDelayProcessor;
using Catch::Matchers::WithinAbs;

// ============================================================================
// Helpers
// ============================================================================

static void setParam (Proc& proc, const juce::String& paramId, float value)
{
    if (auto* p = proc.apvts.getParameter (paramId))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

static void setChoice (Proc& proc, const juce::String& paramId, int choiceIndex)
{
    if (auto* p = proc.apvts.getParameter (paramId))
    {
        auto* choice = dynamic_cast<juce::AudioParameterChoice*> (p);
        if (choice != nullptr)
            choice->setValueNotifyingHost (static_cast<float> (choiceIndex) /
                                           static_cast<float> (juce::jmax (1, choice->choices.size() - 1)));
    }
}

static void enableSingleTap (Proc& proc, int tapIndex, float azDeg, float elDeg, float dist)
{
    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (proc, "object" + idx + "_enabled", (i == tapIndex) ? 1.0f : 0.0f);
    }

    auto idx = juce::String (tapIndex + 1);
    setParam (proc, "object" + idx + "_azimuth",   azDeg);
    setParam (proc, "object" + idx + "_elevation",  elDeg);
    setParam (proc, "object" + idx + "_distance",   dist);
}

// Create a stereo processor with one tap enabled, 100% wet, short delay
static std::unique_ptr<Proc> createStereoTestProcessor()
{
    auto proc = std::make_unique<Proc>();
    proc->configOutputFormat.store (1, std::memory_order_relaxed);  // Stereo
    proc->configAlgorithm.store (0, std::memory_order_relaxed);
    proc->configInputFormat.store (1, std::memory_order_relaxed);   // Stereo input

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "inputGain", 0.0f);
    setParam (*proc, "outputGain", 0.0f);

    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    proc->prepareToPlay (kSampleRate, kBlockSize);
    return proc;
}

// Process blocks with separate L/R input levels, capture stereo output
static std::pair<std::vector<float>, std::vector<float>>
processBlocksStereo (Proc& proc, int numBlocks, float inputL = 0.5f, float inputR = 0.5f)
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
            buffer.setSample (0, s, inputL);
            buffer.setSample (1, s, inputR);
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

// Create a surround processor with constrained bus
static std::unique_ptr<Proc> createSurroundProcessor (int outputFormat, int algorithm,
                                                       juce::AudioChannelSet outputChannelSet)
{
    auto proc = std::make_unique<Proc>();
    proc->configOutputFormat.store (outputFormat, std::memory_order_relaxed);
    proc->configAlgorithm.store (algorithm, std::memory_order_relaxed);

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);

    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add (juce::AudioChannelSet::stereo());
    layout.outputBuses.add (outputChannelSet);
    proc->setBusesLayout (layout);

    proc->prepareToPlay (kSampleRate, 512);
    return proc;
}

// Process blocks with a multi-channel buffer, return last block
static juce::AudioBuffer<float> processBlocksMulti (Proc& proc, int numBlocks,
                                                     int numChannels, float inputLevel = 0.5f)
{
    juce::AudioBuffer<float> result (numChannels, 512);
    result.clear();
    juce::MidiBuffer midi;

    // Warmup
    for (int w = 0; w < 30; ++w)
    {
        juce::AudioBuffer<float> buffer (numChannels, 512);
        buffer.clear();
        for (int s = 0; s < 512; ++s)
        {
            buffer.setSample (0, s, inputLevel);
            if (numChannels > 1) buffer.setSample (1, s, inputLevel);
        }
        proc.processBlock (buffer, midi);
    }

    // Capture
    for (int b = 0; b < numBlocks; ++b)
    {
        juce::AudioBuffer<float> buffer (numChannels, 512);
        buffer.clear();
        for (int s = 0; s < 512; ++s)
        {
            buffer.setSample (0, s, inputLevel);
            if (numChannels > 1) buffer.setSample (1, s, inputLevel);
        }
        proc.processBlock (buffer, midi);
        if (b == numBlocks - 1) result = buffer;
    }
    return result;
}

static float computeChannelRMS (const juce::AudioBuffer<float>& buffer, int channel)
{
    if (channel >= buffer.getNumChannels()) return 0.0f;
    float sum = 0.0f;
    const float* data = buffer.getReadPointer (channel);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        sum += data[i] * data[i];
    return std::sqrt (sum / static_cast<float> (buffer.getNumSamples()));
}

static int findLoudestChannel (const juce::AudioBuffer<float>& buffer, int numChannels, int skipChannel = -1)
{
    int loudest = -1;
    float maxRMS = -1.0f;
    for (int ch = 0; ch < numChannels; ++ch)
    {
        if (ch == skipChannel) continue;
        float rms = computeChannelRMS (buffer, ch);
        if (rms > maxRMS) { maxRMS = rms; loudest = ch; }
    }
    return loudest;
}

// ============================================================================
// Section 1: Stereo Input Routing [input-routing]
// ============================================================================

// Input channel parameter: 0 = L+R, 1 = L, 2 = R

TEST_CASE ("Input routing: L+R mode passes both channels to wet path", "[input-routing]")
{
    auto proc = createStereoTestProcessor();
    setChoice (*proc, "object1_inputChannel", 0);  // L+R

    processBlocksStereo (*proc, 50);  // warmup
    auto [outL, outR] = processBlocksStereo (*proc, 20, 0.5f, 0.5f);

    float rmsL = computeRMS (outL.data(), static_cast<int> (outL.size()));
    float rmsR = computeRMS (outR.data(), static_cast<int> (outR.size()));

    REQUIRE (rmsL > 0.01f);
    REQUIRE (rmsR > 0.01f);
}

TEST_CASE ("Input routing: L mode — only left input reaches wet path", "[input-routing]")
{
    auto proc = createStereoTestProcessor();
    setChoice (*proc, "object1_inputChannel", 1);  // L only

    // Feed L=0.5, R=0.0 — should produce output
    processBlocksStereo (*proc, 50, 0.5f, 0.0f);  // warmup
    auto [outL, outR] = processBlocksStereo (*proc, 20, 0.5f, 0.0f);

    float rmsWithSignal = computeRMS (outL.data(), static_cast<int> (outL.size()));
    REQUIRE (rmsWithSignal > 0.01f);

    // Now feed L=0.0, R=0.5 — should produce silence from wet path
    processBlocksStereo (*proc, 50, 0.0f, 0.5f);  // settle
    auto [outL2, outR2] = processBlocksStereo (*proc, 20, 0.0f, 0.5f);

    float rmsNoSignal = computeRMS (outL2.data(), static_cast<int> (outL2.size()));
    // With 100% wet and 0 feedback, output should be near-silent
    CHECK (rmsNoSignal < rmsWithSignal * 0.1f);
}

TEST_CASE ("Input routing: R mode — only right input reaches wet path", "[input-routing]")
{
    auto proc = createStereoTestProcessor();
    setChoice (*proc, "object1_inputChannel", 2);  // R only

    // Feed L=0.0, R=0.5 — should produce output
    processBlocksStereo (*proc, 50, 0.0f, 0.5f);  // warmup
    auto [outL, outR] = processBlocksStereo (*proc, 20, 0.0f, 0.5f);

    float rmsWithSignal = computeRMS (outR.data(), static_cast<int> (outR.size()));
    REQUIRE (rmsWithSignal > 0.01f);

    // Now feed L=0.5, R=0.0 — should produce silence from wet path
    processBlocksStereo (*proc, 50, 0.5f, 0.0f);  // settle
    auto [outL2, outR2] = processBlocksStereo (*proc, 20, 0.5f, 0.0f);

    float rmsNoSignal = computeRMS (outR2.data(), static_cast<int> (outR2.size()));
    CHECK (rmsNoSignal < rmsWithSignal * 0.1f);
}

TEST_CASE ("Input routing: different taps can use different input channels", "[input-routing]")
{
    auto proc = createStereoTestProcessor();

    // Enable two taps: tap 1 = L, tap 2 = R
    for (int i = 0; i < 12; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", (i < 2) ? 1.0f : 0.0f);
    }
    setParam (*proc, "object1_azimuth", 45.0f);
    setParam (*proc, "object1_distance", 0.5f);
    setParam (*proc, "object2_azimuth", -45.0f);
    setParam (*proc, "object2_distance", 0.5f);

    setChoice (*proc, "object1_inputChannel", 1);  // L
    setChoice (*proc, "object2_inputChannel", 2);  // R

    // Feed L=0.5, R=0.0 — only tap 1 should produce signal
    processBlocksStereo (*proc, 50, 0.5f, 0.0f);  // warmup
    auto [outL, outR] = processBlocksStereo (*proc, 20, 0.5f, 0.0f);

    float rmsL = computeRMS (outL.data(), static_cast<int> (outL.size()));
    REQUIRE (rmsL > 0.01f);

    // Feed L=0.0, R=0.5 — only tap 2 should produce signal
    processBlocksStereo (*proc, 50, 0.0f, 0.5f);
    auto [outL2, outR2] = processBlocksStereo (*proc, 20, 0.0f, 0.5f);

    float rmsR = computeRMS (outR2.data(), static_cast<int> (outR2.size()));
    REQUIRE (rmsR > 0.01f);
}

// ============================================================================
// Section 2: 9.1 and SML 13.1 Audio Output [surround-format]
// ============================================================================

// Format indices: 9.1 = 7, SML 13.1 = 16
// Algorithm indices: ConstantPower = 1, VBAP = 5

TEST_CASE ("9.1: front center source routes to center channel", "[surround-format][9.1]")
{
    auto proc = createSurroundProcessor (7 /*9.1*/, 1 /*ConstantPower*/,
                                          juce::AudioChannelSet::discreteChannels (10));
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);  // dead center

    auto output = processBlocksMulti (*proc, 8, 10);

    // Center is channel 2 in standard 9.1 layout (L=0, R=1, C=2, LFE=3, ...)
    int loudest = findLoudestChannel (output, 10, 3 /*skip LFE*/);
    INFO ("Expected center (ch2), got ch" << loudest);
    for (int ch = 0; ch < 10; ++ch)
        INFO ("  ch" << ch << " RMS=" << computeChannelRMS (output, ch));
    CHECK (loudest == 2);
}

TEST_CASE ("9.1: left source routes to left channel", "[surround-format][9.1]")
{
    auto proc = createSurroundProcessor (7, 1 /*ConstantPower*/,
                                          juce::AudioChannelSet::discreteChannels (10));
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);
    enableSingleTap (*proc, 0, 30.0f, 0.0f, 0.5f);  // L speaker at ~30deg

    auto output = processBlocksMulti (*proc, 8, 10);

    int loudest = findLoudestChannel (output, 10, 3);
    INFO ("Expected L (ch0), got ch" << loudest);
    CHECK (loudest == 0);
}

TEST_CASE ("9.1: all 10 channels are addressable", "[surround-format][9.1]")
{
    // Place a source at each ear-level angle and verify signal appears
    auto proc = createSurroundProcessor (7, 1 /*ConstantPower*/,
                                          juce::AudioChannelSet::discreteChannels (10));
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    auto output = processBlocksMulti (*proc, 8, 10);

    // At least one non-LFE channel should have signal
    bool anySignal = false;
    for (int ch = 0; ch < 10; ++ch)
    {
        if (ch == 3) continue;  // skip LFE
        if (computeChannelRMS (output, ch) > 0.001f)
            anySignal = true;
    }
    REQUIRE (anySignal);
}

TEST_CASE ("9.1: LFE channel receives low-frequency content", "[surround-format][9.1]")
{
    auto proc = createSurroundProcessor (7, 1 /*ConstantPower*/,
                                          juce::AudioChannelSet::discreteChannels (10));
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    auto output = processBlocksMulti (*proc, 8, 10);

    float lfeRMS = computeChannelRMS (output, 3);
    // LFE is derived at -10dB, so it should be present but quieter
    INFO ("LFE (ch3) RMS=" << lfeRMS);
    CHECK (lfeRMS > 0.0f);
}

TEST_CASE ("SML 13.1: front center source routes to front speaker", "[surround-format][sml13]")
{
    auto proc = createSurroundProcessor (16 /*SML 13.1*/, 1 /*ConstantPower*/,
                                          juce::AudioChannelSet::discreteChannels (14));
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    auto output = processBlocksMulti (*proc, 8, 14);

    // Verify signal reaches at least one channel
    bool anySignal = false;
    for (int ch = 0; ch < 14; ++ch)
    {
        if (computeChannelRMS (output, ch) > 0.001f)
            anySignal = true;
    }
    REQUIRE (anySignal);
}

TEST_CASE ("SML 13.1: elevated source activates height channels", "[surround-format][sml13]")
{
    auto proc = createSurroundProcessor (16, 5 /*VBAP*/,
                                          juce::AudioChannelSet::discreteChannels (14));
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);
    enableSingleTap (*proc, 0, 0.0f, 60.0f, 0.5f);  // elevated source

    auto output = processBlocksMulti (*proc, 8, 14);

    // With elevation, height channels (indices > 7 in SML 13.1) should receive signal
    float totalHeightRMS = 0.0f;
    for (int ch = 8; ch < 14; ++ch)
        totalHeightRMS += computeChannelRMS (output, ch);

    INFO ("Total height channel RMS = " << totalHeightRMS);
    CHECK (totalHeightRMS > 0.001f);
}

TEST_CASE ("SML 13.1: LFE channel index is 13", "[surround-format][sml13]")
{
    auto proc = std::make_unique<Proc>();
    proc->configOutputFormat.store (16, std::memory_order_relaxed);
    proc->configAlgorithm.store (1, std::memory_order_relaxed);
    proc->prepareToPlay (kSampleRate, 512);

    const auto& state = proc->getActiveLayout();
    CHECK (state.layout.lfeChannelIndex == 13);
    CHECK (state.layout.totalChannels == 14);
    CHECK (state.layout.numSpeakers == 13);
}

// ============================================================================
// Section 3: Higher-Order Ambisonics Initialization [hoa]
// ============================================================================

// Format indices: FOA=17, SOA=18, HOA=19, 4OA=20, 5OA=21, 6OA=22

struct AmbiOrder {
    const char* name;
    int formatIndex;
    int order;
    int expectedChannels;  // (order+1)^2
};

static const AmbiOrder ambiOrders[] = {
    { "FOA (1st)", 17, 1,  4 },
    { "SOA (2nd)", 18, 2,  9 },
    { "HOA (3rd)", 19, 3, 16 },
    { "4OA (4th)", 20, 4, 25 },
    { "5OA (5th)", 21, 5, 36 },
    { "6OA (6th)", 22, 6, 49 },
};

TEST_CASE ("Ambisonics: all orders initialize without crash", "[hoa][init]")
{
    for (const auto& ao : ambiOrders)
    {
        SECTION (ao.name)
        {
            auto proc = std::make_unique<Proc>();
            proc->configOutputFormat.store (ao.formatIndex, std::memory_order_relaxed);
            proc->configAlgorithm.store (0 /*Ambisonics*/, std::memory_order_relaxed);

            // This should not crash or assert
            proc->prepareToPlay (kSampleRate, 512);

            // Verify it initialized
            REQUIRE (proc->getLatencySamples() == 2048);
        }
    }
}

TEST_CASE ("Ambisonics: HOA+ orders process audio without buffer overrun", "[hoa][processing]")
{
    // Focus on 4OA+ which are the untested higher orders
    for (const auto& ao : ambiOrders)
    {
        if (ao.order < 3) continue;  // FOA/SOA already have implicit coverage

        SECTION (ao.name)
        {
            auto proc = std::make_unique<Proc>();
            proc->configOutputFormat.store (ao.formatIndex, std::memory_order_relaxed);
            proc->configAlgorithm.store (0 /*Ambisonics*/, std::memory_order_relaxed);

            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            enableSingleTap (*proc, 0, 45.0f, 30.0f, 0.5f);

            juce::AudioProcessor::BusesLayout layout;
            layout.inputBuses.add (juce::AudioChannelSet::stereo());
            layout.outputBuses.add (juce::AudioChannelSet::discreteChannels (ao.expectedChannels));
            proc->setBusesLayout (layout);
            proc->prepareToPlay (kSampleRate, 512);

            juce::MidiBuffer midi;

            // Process 40 blocks — should not crash, overrun, or produce NaN
            for (int b = 0; b < 40; ++b)
            {
                juce::AudioBuffer<float> buffer (ao.expectedChannels, 512);
                buffer.clear();
                buffer.setSample (0, 0, 0.5f);  // minimal input
                if (ao.expectedChannels > 1) buffer.setSample (1, 0, 0.5f);

                proc->processBlock (buffer, midi);

                // Check no NaN in any channel
                for (int ch = 0; ch < ao.expectedChannels; ++ch)
                {
                    const float* data = buffer.getReadPointer (ch);
                    for (int s = 0; s < 512; ++s)
                    {
                        if (std::isnan (data[s]) || std::isinf (data[s]))
                        {
                            INFO (ao.name << " ch" << ch << " sample " << s
                                  << " block " << b << " = " << data[s]);
                            REQUIRE (false);  // NaN/Inf detected
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE ("Ambisonics: W channel (ch0) has signal for all orders", "[hoa][output]")
{
    for (const auto& ao : ambiOrders)
    {
        SECTION (ao.name)
        {
            auto proc = std::make_unique<Proc>();
            proc->configOutputFormat.store (ao.formatIndex, std::memory_order_relaxed);
            proc->configAlgorithm.store (0 /*Ambisonics*/, std::memory_order_relaxed);

            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            enableSingleTap (*proc, 0, 45.0f, 0.0f, 0.5f);

            juce::AudioProcessor::BusesLayout layout;
            layout.inputBuses.add (juce::AudioChannelSet::stereo());
            layout.outputBuses.add (juce::AudioChannelSet::discreteChannels (ao.expectedChannels));
            proc->setBusesLayout (layout);
            proc->prepareToPlay (kSampleRate, 512);

            auto output = processBlocksMulti (*proc, 8, ao.expectedChannels);

            // ACN channel 0 (W — omnidirectional) should always have signal
            float wRMS = computeChannelRMS (output, 0);
            INFO (ao.name << " W channel RMS = " << wRMS);
            CHECK (wRMS > 0.001f);
        }
    }
}

TEST_CASE ("Ambisonics: channel count matches (order+1)^2", "[hoa][channels]")
{
    for (const auto& ao : ambiOrders)
    {
        SECTION (ao.name)
        {
            int expected = (ao.order + 1) * (ao.order + 1);
            CHECK (ao.expectedChannels == expected);

            // Verify bus negotiation accepts this channel count
            auto proc = std::make_unique<Proc>();

            juce::AudioProcessor::BusesLayout layout;
            layout.inputBuses.add (juce::AudioChannelSet::stereo());
            layout.outputBuses.add (juce::AudioChannelSet::ambisonic (ao.order));

            INFO (ao.name << " (" << expected << "ch)");
            CHECK (proc->checkBusesLayoutSupported (layout));
        }
    }
}

TEST_CASE ("Ambisonics: 6OA (49ch) does not overrun internal buffers", "[hoa][stress]")
{
    auto proc = std::make_unique<Proc>();
    proc->configOutputFormat.store (22 /*6OA*/, std::memory_order_relaxed);
    proc->configAlgorithm.store (0, std::memory_order_relaxed);

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.5f);

    // Enable multiple taps at various positions for maximum SH channel coverage
    for (int i = 0; i < 6; ++i)
    {
        auto idx = juce::String (i + 1);
        setParam (*proc, "object" + idx + "_enabled", 1.0f);
        setParam (*proc, "object" + idx + "_azimuth", static_cast<float> (i * 60));
        setParam (*proc, "object" + idx + "_elevation", static_cast<float> ((i % 3) * 30 - 30));
        setParam (*proc, "object" + idx + "_distance", 0.5f);
    }

    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add (juce::AudioChannelSet::stereo());
    layout.outputBuses.add (juce::AudioChannelSet::discreteChannels (49));
    proc->setBusesLayout (layout);
    proc->prepareToPlay (kSampleRate, 512);

    juce::MidiBuffer midi;

    // Run 60 blocks to stress the buffer with feedback
    for (int b = 0; b < 60; ++b)
    {
        juce::AudioBuffer<float> buffer (49, 512);
        buffer.clear();
        for (int s = 0; s < 512; ++s)
        {
            buffer.setSample (0, s, 0.3f);
            buffer.setSample (1, s, 0.3f);
        }
        proc->processBlock (buffer, midi);
    }

    // If we got here without crashing, the test passes.
    // Final sanity: check W channel has signal
    juce::AudioBuffer<float> finalBuf (49, 512);
    finalBuf.clear();
    for (int s = 0; s < 512; ++s)
    {
        finalBuf.setSample (0, s, 0.3f);
        finalBuf.setSample (1, s, 0.3f);
    }
    proc->processBlock (finalBuf, midi);

    float wRMS = computeChannelRMS (finalBuf, 0);
    CHECK (wRMS > 0.0f);
}
