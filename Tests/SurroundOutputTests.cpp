#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include "../Source/PluginProcessor.h"
#include <cmath>

using Proc = OpenSpatialDelayProcessor;
using OF   = Proc::OutputFormat;
using Catch::Matchers::WithinAbs;

static constexpr float kPi = juce::MathConstants<float>::pi;
static constexpr float kDeg2Rad = kPi / 180.0f;

// ============================================================================
// Section 1: Helper Functions
// ============================================================================

// Build a LayoutContext from an OutputLayoutState (mirrors processBlock line 3550)
static LayoutContext makeLayoutCtx (const Proc::OutputLayoutState& s)
{
    return { s.layout, s.vbapTriplets, s.ambiDecodeMatrix, s.ambiNumSpeakers };
}

// Create a processor ready for testing with a large discrete output bus.
// The 50-channel default is accepted by isBusesLayoutSupported and allows
// any output format to resolve via resolveEffectiveFormat.
static std::unique_ptr<Proc> createTestProcessor (int outputFormat = 0, int algorithm = 4 /*VBAP*/)
{
    auto proc = std::make_unique<Proc>();

    // Set output format and algorithm config params BEFORE prepareToPlay
    proc->configOutputFormat.store (outputFormat, std::memory_order_relaxed);
    proc->configAlgorithm.store (algorithm, std::memory_order_relaxed);

    // Set short delay and 100% wet BEFORE prepareToPlay so smoothed values
    // initialize correctly — prevents 500ms default from starving the delay line
    if (auto* p = proc->apvts.getParameter ("delayTime"))
        p->setValueNotifyingHost (p->convertTo0to1 (1.0f));
    if (auto* p = proc->apvts.getParameter ("dryWet"))
        p->setValueNotifyingHost (p->convertTo0to1 (1.0f));

    proc->prepareToPlay (48000.0, 512);
    return proc;
}

// Create a processor with a constrained output bus matching DAW scenarios.
// Uses setBusesLayout to simulate how DAWs negotiate channel counts.
static std::unique_ptr<Proc> createConstrainedProcessor (int outputFormat, int algorithm,
                                                          juce::AudioChannelSet outputChannelSet)
{
    auto proc = std::make_unique<Proc>();

    proc->configOutputFormat.store (outputFormat, std::memory_order_relaxed);
    proc->configAlgorithm.store (algorithm, std::memory_order_relaxed);

    // Set short delay and 100% wet BEFORE prepareToPlay so smoothed values
    // initialize correctly — prevents 500ms default from starving the delay line
    if (auto* p = proc->apvts.getParameter ("delayTime"))
        p->setValueNotifyingHost (p->convertTo0to1 (1.0f));
    if (auto* p = proc->apvts.getParameter ("dryWet"))
        p->setValueNotifyingHost (p->convertTo0to1 (1.0f));

    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add (juce::AudioChannelSet::stereo());
    layout.outputBuses.add (outputChannelSet);
    proc->setBusesLayout (layout);

    proc->prepareToPlay (48000.0, 512);
    return proc;
}

// Process blocks with a buffer matching the constrained bus channel count.
// Includes warmup period to fill the delay line and let parameter smoothing settle.
static juce::AudioBuffer<float> processBlocksConstrained (Proc& proc, int numBlocks, int blockSize,
                                                           int numOutChannels, float inputLevel = 0.5f)
{
    juce::AudioBuffer<float> result (numOutChannels, blockSize);
    result.clear();
    juce::MidiBuffer midi;

    // Warmup: fill delay line and let smoothing settle (matches createBinauralProcessor pattern)
    for (int w = 0; w < 30; ++w)
    {
        juce::AudioBuffer<float> buffer (numOutChannels, blockSize);
        buffer.clear();
        for (int s = 0; s < blockSize; ++s)
        {
            buffer.setSample (0, s, inputLevel);
            if (numOutChannels > 1)
                buffer.setSample (1, s, inputLevel);
        }
        proc.processBlock (buffer, midi);
    }

    for (int b = 0; b < numBlocks; ++b)
    {
        juce::AudioBuffer<float> buffer (numOutChannels, blockSize);
        buffer.clear();
        for (int s = 0; s < blockSize; ++s)
        {
            buffer.setSample (0, s, inputLevel);
            if (numOutChannels > 1)
                buffer.setSample (1, s, inputLevel);
        }
        proc.processBlock (buffer, midi);
        if (b == numBlocks - 1)
            result = buffer;
    }
    return result;
}

// Set an APVTS parameter by ID and value (in the parameter's native range)
static void setParam (Proc& proc, const juce::String& paramId, float value)
{
    if (auto* p = proc.apvts.getParameter (paramId))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

// Enable a single tap at a given 3D position. Disables all others.
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

// Compute RMS of a single channel in a buffer
static float computeChannelRMS (const juce::AudioBuffer<float>& buffer, int channel)
{
    if (channel >= buffer.getNumChannels()) return 0.0f;
    float sum = 0.0f;
    const float* data = buffer.getReadPointer (channel);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        sum += data[i] * data[i];
    return std::sqrt (sum / static_cast<float> (buffer.getNumSamples()));
}

// Find the channel with the highest RMS, optionally skipping a channel (e.g., LFE)
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

// Sum of squared gains (for constant-power verification)
static float sumOfSquaredGains (const float* gains, int n)
{
    float sum = 0.0f;
    for (int i = 0; i < n; ++i)
        sum += gains[i] * gains[i];
    return sum;
}

// Count non-zero gains (above threshold)
static int countNonZeroGains (const float* gains, int n, float threshold = 0.001f)
{
    int count = 0;
    for (int i = 0; i < n; ++i)
        if (std::abs (gains[i]) > threshold)
            ++count;
    return count;
}

// Find the speaker index with the highest gain
static int findLoudestSpeaker (const float* gains, int n)
{
    int loudest = -1;
    float maxGain = -1.0f;
    for (int i = 0; i < n; ++i)
    {
        if (gains[i] > maxGain) { maxGain = gains[i]; loudest = i; }
    }
    return loudest;
}

// Map output format enum index to the layoutDefs LayoutID used in source.
// Returns -1 for formats that don't use discrete surround layouts (Binaural, Stereo, Ambisonics).
static int formatToLayoutIndex (int formatEnumIndex)
{
    // OutputFormat enum order: Binaural(0), Stereo(1), Quad(2), 5.0(3), 5.1(4), 7.0(5),
    // 7.1(6), Octaphonic(7), 5.1.2(8), 5.1.4(9), 7.1.2(10), 7.1.4(11), 7.1.6(12), 9.1.4(13), 9.1.6(14)
    // LayoutID enum order: Quad(0), S5_0(1), S5_1(2), S7_0(3), S7_1(4), S5_1_2(5), S5_1_4(6),
    // S7_1_2(7), S7_1_4(8), S7_1_6(9), S9_1_4(10), S9_1_6(11), Octaphonic(12), SML13_1(13)
    switch (formatEnumIndex)
    {
        case 2:  return 0;   // Quad
        case 3:  return 1;   // 5.0
        case 4:  return 2;   // 5.1
        case 5:  return 3;   // 7.0
        case 6:  return 4;   // 7.1
        case 7:  return 12;  // Octaphonic
        case 8:  return 5;   // 5.1.2
        case 9:  return 6;   // 5.1.4
        case 10: return 7;   // 7.1.2
        case 11: return 8;   // 7.1.4
        case 12: return 9;   // 7.1.6
        case 13: return 10;  // 9.1.4
        case 14: return 11;  // 9.1.6
        case 15: return 13;  // SML 13.1
        default: return -1;  // Binaural, Stereo, Ambisonics
    }
}

// ============================================================================
// Section 2: Layout Verification Tests [layout]
// ============================================================================

struct LayoutExpectation {
    const char* name;
    int formatIndex;       // index into outputFormatRegistry / OutputFormat enum
    int expectedSpeakers;  // spatial speakers (excluding LFE)
    int expectedTotal;     // total output channels
    int expectedLFE;       // LFE channel index, or -1
    bool hasHeight;
};

static const LayoutExpectation layoutExpectations[] = {
    { "Quad",       2,   4,  4, -1, false },
    { "5.0",        3,   5,  5, -1, false },
    { "5.1",        4,   5,  6,  3, false },
    { "7.0",        5,   7,  7, -1, false },
    { "7.1",        6,   7,  8,  3, false },
    { "Octaphonic",  7,  8,  8, -1, false },
    { "9.1",        8,   9, 10,  3, false },
    { "5.1.2",      9,   7,  8,  3, true  },
    { "5.1.4",      10,  9, 10,  3, true  },
    { "7.1.2",      11,  9, 10,  3, true  },
    { "7.1.4",      12, 11, 12,  3, true  },
    { "7.1.6",      13, 13, 14,  3, true  },
    { "9.1.4",      14, 13, 14,  3, true  },
    { "9.1.6",      15, 15, 16,  3, true  },
    { "SML 13.1",   16, 13, 14, 13, true  },
};

TEST_CASE ("Layout: speaker count and channel count match spec", "[layout]")
{
    for (const auto& exp : layoutExpectations)
    {
        SECTION (exp.name)
        {
            auto proc = createTestProcessor (exp.formatIndex);
            const auto& state = proc->getActiveLayout();

            CHECK (state.layout.numSpeakers   == exp.expectedSpeakers);
            CHECK (state.layout.totalChannels  == exp.expectedTotal);
            CHECK (state.layout.lfeChannelIndex == exp.expectedLFE);
        }
    }
}

TEST_CASE ("Layout: LFE channel index matches expectation for .1 formats", "[layout]")
{
    for (const auto& exp : layoutExpectations)
    {
        if (exp.expectedLFE >= 0)
        {
            SECTION (exp.name)
            {
                auto proc = createTestProcessor (exp.formatIndex);
                const auto& state = proc->getActiveLayout();
                CHECK (state.layout.lfeChannelIndex == exp.expectedLFE);
            }
        }
    }
}

TEST_CASE ("Layout: height speakers have elevation > 0", "[layout]")
{
    for (const auto& exp : layoutExpectations)
    {
        if (exp.hasHeight)
        {
            SECTION (exp.name)
            {
                auto proc = createTestProcessor (exp.formatIndex);
                const auto& layout = proc->getActiveLayout().layout;

                bool foundHeight = false;
                for (int s = 0; s < layout.numSpeakers; ++s)
                {
                    if (layout.speakers[s].elevationRad > 0.01f)
                    {
                        foundHeight = true;
                        break;
                    }
                }
                CHECK (foundHeight);
            }
        }
    }
}

TEST_CASE ("Layout: channel indices don't overlap and skip LFE", "[layout]")
{
    for (const auto& exp : layoutExpectations)
    {
        SECTION (exp.name)
        {
            auto proc = createTestProcessor (exp.formatIndex);
            const auto& layout = proc->getActiveLayout().layout;

            std::set<int> usedChannels;
            for (int s = 0; s < layout.numSpeakers; ++s)
            {
                int ch = layout.speakers[s].channelIndex;
                // Channel index should be valid
                CHECK (ch >= 0);
                CHECK (ch < layout.totalChannels);
                // Should not overlap with LFE
                CHECK (ch != layout.lfeChannelIndex);
                // Should not be a duplicate
                CHECK (usedChannels.find (ch) == usedChannels.end());
                usedChannels.insert (ch);
            }
        }
    }
}

TEST_CASE ("Layout: VBAP triplets are non-empty for 3D layouts", "[layout]")
{
    // Formats with height speakers need 3D VBAP triplets
    for (const auto& exp : layoutExpectations)
    {
        if (exp.hasHeight)
        {
            SECTION (exp.name)
            {
                auto proc = createTestProcessor (exp.formatIndex, 4 /*VBAP*/);
                const auto& state = proc->getActiveLayout();
                CHECK (! state.vbapTriplets.empty());
            }
        }
    }
}

// ============================================================================
// Section 3: Algorithm Unit Tests [algorithm]
// ============================================================================

// Algorithm parameter indices (from createParameterLayout line 305):
// 0=Ambisonics, 1=DBAP, 2=KNN, 3=MDAP, 4=VBAP, 5=VBIP
namespace AlgoIdx {
    constexpr int Ambisonics = 0, DBAP = 1, KNN = 2, MDAP = 3, VBAP = 4, VBIP = 5;
}

// --- VBAP Tests ---

TEST_CASE ("VBAP: source at L speaker (30deg) on 7.1 has L as loudest", "[algorithm][vbap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    SourcePosition src { 30.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    VBAPAlgorithm vbap;
    vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // Speaker 0 is L at 30°, channel 0
    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 0);  // L speaker
}

TEST_CASE ("VBAP: source at R speaker (-30deg) on 7.1 has R as loudest", "[algorithm][vbap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    SourcePosition src { -30.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    VBAPAlgorithm vbap;
    vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 1);  // R speaker
}

TEST_CASE ("VBAP: source at center (0deg) on 5.1 activates C speaker", "[algorithm][vbap]")
{
    auto proc = createTestProcessor (4 /*5.1*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    SourcePosition src { 0.0f, 0.0f, 0.5f };
    float gains[16] = {};
    VBAPAlgorithm vbap;
    vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // 5.1 speaker 2 is C at 0°
    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 2);  // C speaker
}

TEST_CASE ("VBAP: source at rear (135deg) on 7.1 activates Lrs", "[algorithm][vbap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    SourcePosition src { 135.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    VBAPAlgorithm vbap;
    vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // 7.1 speaker 5 is Lrs at 135°
    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 5);  // Lrs speaker
}

TEST_CASE ("VBAP: source at height position on 7.1.4 activates height speaker", "[algorithm][vbap]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    // Tfl is at 45° azimuth, 45° elevation — speaker index 7 in 7.1.4
    SourcePosition src { 45.0f * kDeg2Rad, 45.0f * kDeg2Rad, 0.5f };
    float gains[16] = {};
    VBAPAlgorithm vbap;
    vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    // Height speakers in 7.1.4 are indices 7-10 (Tfl, Tfr, Trl, Trr)
    CHECK (loudest >= 7);
    CHECK (loudest <= 10);
}

TEST_CASE ("VBAP: constant-power normalization", "[algorithm][vbap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    VBAPAlgorithm vbap;

    // Test at positions between speakers (avoids edge cases at exact positions and wrap-around)
    // Note: 2D VBAP pair-finding can produce sos < 1.0 at certain angles, so tolerance is generous
    float testAzimuths[] = { 15.0f, 60.0f, -15.0f, -60.0f };
    for (float azDeg : testAzimuths)
    {
        SECTION ("Azimuth " + std::to_string (static_cast<int> (azDeg)) + " deg")
        {
            SourcePosition src { azDeg * kDeg2Rad, 0.0f, 0.5f };
            float gains[16] = {};
            vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);
            float sos = sumOfSquaredGains (gains, state.layout.numSpeakers);
            CHECK (sos > 0.4f);
            CHECK (sos < 1.2f);
        }
    }
}

TEST_CASE ("VBAP: left-right symmetry", "[algorithm][vbap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    VBAPAlgorithm vbap;

    // Source at +60° vs -60°: gains should be mirrored for symmetric speaker pairs
    SourcePosition srcL { 60.0f * kDeg2Rad, 0.0f, 0.5f };
    SourcePosition srcR { -60.0f * kDeg2Rad, 0.0f, 0.5f };

    float gainsL[16] = {};
    float gainsR[16] = {};
    vbap.computeGains (srcL, ctx, gainsL, state.layout.numSpeakers);
    vbap.computeGains (srcR, ctx, gainsR, state.layout.numSpeakers);

    // 7.1 symmetric pairs: 0/1 (L/R), 3/4 (Ls/Rs), 5/6 (Lrs/Rrs)
    // Center (idx 2) should be same for both
    CHECK_THAT (gainsL[2], WithinAbs (gainsR[2], 0.01f));
    // L↔R pair should swap
    CHECK_THAT (gainsL[0], WithinAbs (gainsR[1], 0.01f));
    CHECK_THAT (gainsL[1], WithinAbs (gainsR[0], 0.01f));
    // Ls↔Rs pair should swap
    CHECK_THAT (gainsL[3], WithinAbs (gainsR[4], 0.01f));
    CHECK_THAT (gainsL[4], WithinAbs (gainsR[3], 0.01f));
}

// --- VBIP Tests ---

TEST_CASE ("VBIP: source at speaker position has that speaker loudest", "[algorithm][vbip]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBIP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    VBIPAlgorithm vbip;

    SourcePosition src { 90.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    vbip.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // 7.1 speaker 3 is Ls at 90°
    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 3);
}

TEST_CASE ("VBIP: tighter focus than VBAP (fewer non-zero gains)", "[algorithm][vbip]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    // Test at a position between speakers
    SourcePosition src { 60.0f * kDeg2Rad, 0.0f, 0.5f };

    float gainsVBAP[16] = {};
    float gainsVBIP[16] = {};

    VBAPAlgorithm vbap;
    VBIPAlgorithm vbip;
    vbap.computeGains (src, ctx, gainsVBAP, state.layout.numSpeakers);
    vbip.computeGains (src, ctx, gainsVBIP, state.layout.numSpeakers);

    int nonZeroVBAP = countNonZeroGains (gainsVBAP, state.layout.numSpeakers);
    int nonZeroVBIP = countNonZeroGains (gainsVBIP, state.layout.numSpeakers);

    // VBIP should have equal or fewer non-zero gains (tighter focus)
    CHECK (nonZeroVBIP <= nonZeroVBAP);
}

TEST_CASE ("VBIP: constant-power normalization", "[algorithm][vbip]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBIP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    VBIPAlgorithm vbip;
    SourcePosition src { 60.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    vbip.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // VBIP squares VBAP gains then renormalizes, so sos should be ~1.0
    // but can vary more than VBAP due to the squaring step
    float sos = sumOfSquaredGains (gains, state.layout.numSpeakers);
    CHECK (sos > 0.3f);
    CHECK (sos < 1.3f);
}

// --- KNN Tests ---

TEST_CASE ("KNN: source at speaker position has that speaker loudest", "[algorithm][knn]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::KNN);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    KNNAlgorithm knn;
    SourcePosition src { -135.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    knn.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // 7.1 speaker 6 is Rrs at -135°
    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 6);
}

TEST_CASE ("KNN: activates exactly k=3 speakers (between speakers)", "[algorithm][knn]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::KNN);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    KNNAlgorithm knn;

    // Test at positions BETWEEN speakers (at exact speaker pos, KNN returns 1 gain)
    float testAzimuths[] = { 15.0f, 60.0f, 110.0f, -15.0f };
    for (float azDeg : testAzimuths)
    {
        SECTION ("Azimuth " + std::to_string (static_cast<int> (azDeg)) + " deg")
        {
            SourcePosition src { azDeg * kDeg2Rad, 0.0f, 0.5f };
            float gains[16] = {};
            knn.computeGains (src, ctx, gains, state.layout.numSpeakers);
            int nonZero = countNonZeroGains (gains, state.layout.numSpeakers);
            CHECK (nonZero == 3);
        }
    }
}

TEST_CASE ("KNN: at exact speaker position returns 1 gain", "[algorithm][knn]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::KNN);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    KNNAlgorithm knn;
    // Source at exact L speaker position (30°)
    SourcePosition src { 30.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    knn.computeGains (src, ctx, gains, state.layout.numSpeakers);

    int nonZero = countNonZeroGains (gains, state.layout.numSpeakers);
    CHECK (nonZero == 1);
    CHECK_THAT (gains[0], WithinAbs (1.0f, 0.01f));  // L speaker gets all gain
}

TEST_CASE ("KNN: constant-power normalization", "[algorithm][knn]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::KNN);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    KNNAlgorithm knn;
    SourcePosition src { 60.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    knn.computeGains (src, ctx, gains, state.layout.numSpeakers);

    float sos = sumOfSquaredGains (gains, state.layout.numSpeakers);
    CHECK_THAT (sos, WithinAbs (1.0f, 0.15f));
}

// --- DBAP Tests ---

TEST_CASE ("DBAP: source at speaker position has that speaker loudest", "[algorithm][dbap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::DBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    DBAPAlgorithm dbap;
    SourcePosition src { 30.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    dbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // 7.1 speaker 0 is L at 30°
    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 0);
}

TEST_CASE ("DBAP: constant-power normalization", "[algorithm][dbap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::DBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    DBAPAlgorithm dbap;
    SourcePosition src { 60.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    dbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    float sos = sumOfSquaredGains (gains, state.layout.numSpeakers);
    CHECK_THAT (sos, WithinAbs (1.0f, 0.15f));
}

// --- MDAP Tests ---

TEST_CASE ("MDAP: source at speaker position has that speaker loudest", "[algorithm][mdap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::MDAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    MDAPAlgorithm mdap;
    SourcePosition src { 90.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    mdap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // 7.1 speaker 3 is Ls at 90°
    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 3);
}

TEST_CASE ("MDAP: wider spread than VBAP (more non-zero gains)", "[algorithm][mdap]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    SourcePosition src { 60.0f * kDeg2Rad, 0.0f, 0.5f };

    float gainsVBAP[16] = {};
    float gainsMDAP[16] = {};

    VBAPAlgorithm vbap;
    MDAPAlgorithm mdap;
    vbap.computeGains (src, ctx, gainsVBAP, state.layout.numSpeakers);
    mdap.computeGains (src, ctx, gainsMDAP, state.layout.numSpeakers);

    int nonZeroVBAP = countNonZeroGains (gainsVBAP, state.layout.numSpeakers);
    int nonZeroMDAP = countNonZeroGains (gainsMDAP, state.layout.numSpeakers);

    // MDAP should activate more or equal speakers due to spread ring
    CHECK (nonZeroMDAP >= nonZeroVBAP);
}

TEST_CASE ("MDAP: constant-power normalization", "[algorithm][mdap]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::MDAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    MDAPAlgorithm mdap;
    SourcePosition src { 60.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    mdap.computeGains (src, ctx, gains, state.layout.numSpeakers);

    float sos = sumOfSquaredGains (gains, state.layout.numSpeakers);
    CHECK_THAT (sos, WithinAbs (1.0f, 0.15f));
}

// --- Ambisonics Decode Tests ---

TEST_CASE ("Ambisonics: source at center (0deg) on 7.1 produces symmetric L/R gains", "[algorithm][ambisonics]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::Ambisonics);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    AmbisonicsAlgorithm ambi;
    SourcePosition src { 0.0f, 0.0f, 0.5f };
    float gains[16] = {};
    ambi.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // L (idx 0) and R (idx 1) should have equal gains for center source
    CHECK_THAT (gains[0], WithinAbs (gains[1], 0.02f));
    // Ls (idx 3) and Rs (idx 4) should have equal gains
    CHECK_THAT (gains[3], WithinAbs (gains[4], 0.02f));
}

TEST_CASE ("Ambisonics: source at speaker position has that speaker loudest", "[algorithm][ambisonics]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::Ambisonics);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    AmbisonicsAlgorithm ambi;
    SourcePosition src { 90.0f * kDeg2Rad, 0.0f, 0.5f };
    float gains[16] = {};
    ambi.computeGains (src, ctx, gains, state.layout.numSpeakers);

    // 7.1 speaker 3 is Ls at 90°
    int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
    CHECK (loudest == 3);
}

// --- Cross-Algorithm: All algorithms on all surround formats at cardinal positions ---

TEST_CASE ("All algorithms: source at L (30deg) has L as loudest on all formats", "[algorithm][cardinal]")
{
    // Formats that have L at 30° as speaker index 0
    int formats[] = { 4 /*5.1*/, 5 /*7.0*/, 6 /*7.1*/, 12 /*7.1.4*/, 15 /*9.1.6*/ };
    int algos[] = { AlgoIdx::VBAP, AlgoIdx::VBIP, AlgoIdx::KNN, AlgoIdx::DBAP, AlgoIdx::MDAP };
    const char* algoNames[] = { "VBAP", "VBIP", "KNN", "DBAP", "MDAP" };

    for (int fi = 0; fi < 5; ++fi)
    {
        for (int ai = 0; ai < 5; ++ai)
        {
            SECTION (std::string (Proc::outputFormatRegistry[static_cast<size_t>(formats[fi])].name)
                     + " + " + algoNames[ai])
            {
                auto proc = createTestProcessor (formats[fi], algos[ai]);
                const auto& state = proc->getActiveLayout();
                auto ctx = makeLayoutCtx (state);

                SourcePosition src { 30.0f * kDeg2Rad, 0.0f, 0.5f };
                float gains[16] = {};

                // Use the actual algorithm instance
                VBAPAlgorithm vbap; VBIPAlgorithm vbip; KNNAlgorithm knn;
                DBAPAlgorithm dbap; MDAPAlgorithm mdap;
                SpatializationAlgorithm* algo = nullptr;
                switch (algos[ai])
                {
                    case AlgoIdx::VBAP: algo = &vbap; break;
                    case AlgoIdx::VBIP: algo = &vbip; break;
                    case AlgoIdx::KNN:  algo = &knn;  break;
                    case AlgoIdx::DBAP: algo = &dbap; break;
                    case AlgoIdx::MDAP: algo = &mdap; break;
                }

                algo->computeGains (src, ctx, gains, state.layout.numSpeakers);

                int loudest = findLoudestSpeaker (gains, state.layout.numSpeakers);
                CHECK (loudest == 0);  // L is always speaker index 0
            }
        }
    }
}

// ============================================================================
// Section 3b: Elevation & Height Isolation Tests [elevation]
// ============================================================================

// Helper: find height speaker indices in a layout (elevation > 0.01 rad)
static std::vector<int> findHeightSpeakerIndices (const SpeakerLayout& layout)
{
    std::vector<int> indices;
    for (int s = 0; s < layout.numSpeakers; ++s)
        if (std::abs (layout.speakers[s].elevationRad) > 0.01f)
            indices.push_back (s);
    return indices;
}

TEST_CASE ("Elevation: VBAP triplets are populated for all height formats", "[elevation]")
{
    for (const auto& exp : layoutExpectations)
    {
        if (! exp.hasHeight) continue;

        SECTION (exp.name)
        {
            auto proc = createTestProcessor (exp.formatIndex, AlgoIdx::VBAP);
            const auto& state = proc->getActiveLayout();
            CHECK (! state.vbapTriplets.empty());
            CHECK (state.vbapTriplets.size() > 5);
        }
    }
}

TEST_CASE ("Elevation: VBAP horizontal source gives zero gain to height speakers on 7.1.4", "[elevation][vbap]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);
    auto heightIndices = findHeightSpeakerIndices (state.layout);

    REQUIRE (! state.vbapTriplets.empty());
    REQUIRE (! heightIndices.empty());

    VBAPAlgorithm vbap;

    for (float azDeg : { 0.0f, 30.0f, 45.0f, 60.0f, 90.0f, 135.0f, 180.0f })
    {
        SECTION ("Azimuth " + std::to_string (static_cast<int> (azDeg)) + " deg")
        {
            SourcePosition src { azDeg * kDeg2Rad, 0.0f, 0.5f };
            float gains[16] = {};
            vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

            for (int hi : heightIndices)
            {
                INFO ("Height speaker index " << hi << " gain = " << gains[hi]);
                CHECK (std::abs (gains[hi]) < 0.001f);
            }
        }
    }
}

TEST_CASE ("Elevation: VBIP horizontal source gives zero gain to height speakers on 7.1.4", "[elevation][vbip]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBIP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);
    auto heightIndices = findHeightSpeakerIndices (state.layout);

    REQUIRE (! state.vbapTriplets.empty());
    VBIPAlgorithm vbip;

    for (float azDeg : { 0.0f, 45.0f, 90.0f, 135.0f })
    {
        SECTION ("Azimuth " + std::to_string (static_cast<int> (azDeg)) + " deg")
        {
            SourcePosition src { azDeg * kDeg2Rad, 0.0f, 0.5f };
            float gains[16] = {};
            vbip.computeGains (src, ctx, gains, state.layout.numSpeakers);

            for (int hi : heightIndices)
            {
                INFO ("Height speaker index " << hi << " gain = " << gains[hi]);
                CHECK (std::abs (gains[hi]) < 0.001f);
            }
        }
    }
}

TEST_CASE ("Elevation: integration — horizontal source silent in height channels on 7.1.4", "[elevation][integration]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);
    enableSingleTap (*proc, 0, 45.0f, 0.0f, 0.5f);

    auto output = processBlocksConstrained (*proc, 8, 512, 12);

    // Height channels in 7.1.4 are ch8-ch11
    float maxHeightRMS = 0.0f;
    for (int ch = 8; ch <= 11; ++ch)
        maxHeightRMS = std::max (maxHeightRMS, computeChannelRMS (output, ch));

    int loudestHoriz = findLoudestChannel (output, 8, 3 /*skip LFE*/);
    float horizRMS = computeChannelRMS (output, loudestHoriz);

    CHECK (maxHeightRMS < 0.01f);
    if (horizRMS > 0.001f)
        CHECK (horizRMS / (maxHeightRMS + 1e-10f) > 100.0f);
}

TEST_CASE ("Elevation: integration — elevated source activates height channels on 7.1.4", "[elevation][integration]")
{
    // Source at (45°, 0°) — height channels should be silent
    auto proc0 = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    setParam (*proc0, "delayTime", 1.0f);
    setParam (*proc0, "dryWet", 1.0f);
    setParam (*proc0, "feedback", 0.0f);
    setParam (*proc0, "outputGain", 0.0f);
    enableSingleTap (*proc0, 0, 45.0f, 0.0f, 0.5f);
    auto out0 = processBlocksConstrained (*proc0, 8, 512, 12);

    // Source at (45°, 45°) — height channels should be active
    auto proc45 = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    setParam (*proc45, "delayTime", 1.0f);
    setParam (*proc45, "dryWet", 1.0f);
    setParam (*proc45, "feedback", 0.0f);
    setParam (*proc45, "outputGain", 0.0f);
    enableSingleTap (*proc45, 0, 45.0f, 45.0f, 0.5f);
    auto out45 = processBlocksConstrained (*proc45, 8, 512, 12);

    float height0_rms = 0.0f;
    for (int ch = 8; ch <= 11; ++ch)
        height0_rms = std::max (height0_rms, computeChannelRMS (out0, ch));

    float height45_rms = 0.0f;
    for (int ch = 8; ch <= 11; ++ch)
        height45_rms = std::max (height45_rms, computeChannelRMS (out45, ch));

    CHECK (height0_rms < 0.01f);
    CHECK (height45_rms > 0.01f);
    CHECK (height45_rms > height0_rms * 10.0f);
}

TEST_CASE ("Elevation: all height formats — horizontal source excludes height speakers", "[elevation]")
{
    for (const auto& exp : layoutExpectations)
    {
        if (! exp.hasHeight) continue;

        SECTION (exp.name)
        {
            auto proc = createTestProcessor (exp.formatIndex, AlgoIdx::VBAP);
            const auto& state = proc->getActiveLayout();
            auto ctx = makeLayoutCtx (state);

            auto heightIdx = findHeightSpeakerIndices (state.layout);
            REQUIRE (! heightIdx.empty());

            for (float azDeg : { 0.0f, 45.0f, 90.0f, 135.0f })
            {
                float azRad = azDeg * kDeg2Rad;
                float gains[16] = {};
                SourcePosition src { azRad, 0.0f, 0.5f };
                VBAPAlgorithm vbap;
                vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

                for (int hi : heightIdx)
                {
                    INFO (exp.name << " az=" << azDeg << " height[" << hi << "]=" << gains[hi]);
                    CHECK (std::abs (gains[hi]) < 0.001f);
                }
            }
        }
    }
}

TEST_CASE ("Elevation: VBAP gain to Tfl increases monotonically 0->45 on 7.1.4", "[elevation]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);

    // Tfl is speaker index 7 (45° az, 45° el) in our 7.1.4 layout
    int tflIdx = 7;

    float prevGain = -1.0f;
    for (int elDeg = 0; elDeg <= 45; elDeg += 5)
    {
        float azRad = 45.0f * kDeg2Rad;
        float elRad = static_cast<float> (elDeg) * kDeg2Rad;
        float gains[16] = {};
        SourcePosition src { azRad, elRad, 0.5f };
        VBAPAlgorithm vbap;
        vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);

        INFO ("Elevation " << elDeg << "° → Tfl gain = " << gains[tflIdx]);
        CHECK (gains[tflIdx] >= prevGain - 0.001f);
        prevGain = gains[tflIdx];
    }

    CHECK (prevGain > 0.3f);
}

TEST_CASE ("Elevation: all algorithms — height isolation for horizontal source on 7.1.4", "[elevation]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    const auto& state = proc->getActiveLayout();
    auto ctx = makeLayoutCtx (state);
    auto heightIdx = findHeightSpeakerIndices (state.layout);

    SourcePosition src { 45.0f * kDeg2Rad, 0.0f, 0.5f };

    SECTION ("VBAP")
    {
        float gains[16] = {};
        VBAPAlgorithm vbap;
        vbap.computeGains (src, ctx, gains, state.layout.numSpeakers);
        for (int hi : heightIdx)
            CHECK (std::abs (gains[hi]) < 0.01f);
    }

    SECTION ("VBIP")
    {
        float gains[16] = {};
        VBIPAlgorithm vbip;
        vbip.computeGains (src, ctx, gains, state.layout.numSpeakers);
        for (int hi : heightIdx)
            CHECK (std::abs (gains[hi]) < 0.01f);
    }

    SECTION ("MDAP")
    {
        float gains[16] = {};
        MDAPAlgorithm mdap;
        mdap.computeGains (src, ctx, gains, state.layout.numSpeakers);
        for (int hi : heightIdx)
        {
            INFO ("MDAP height[" << hi << "]=" << gains[hi]);
            CHECK (std::abs (gains[hi]) < 0.20f);
        }
    }

    SECTION ("KNN")
    {
        float gains[16] = {};
        KNNAlgorithm knn;
        knn.computeGains (src, ctx, gains, state.layout.numSpeakers);
        for (int hi : heightIdx)
        {
            INFO ("KNN height[" << hi << "]=" << gains[hi]);
            CHECK (gains[hi] < 0.20f);
        }
    }

    SECTION ("DBAP")
    {
        float gains[16] = {};
        DBAPAlgorithm dbap;
        dbap.computeGains (src, ctx, gains, state.layout.numSpeakers);
        for (int hi : heightIdx)
        {
            INFO ("DBAP height[" << hi << "]=" << gains[hi]);
            CHECK (gains[hi] < 0.40f);
        }
    }
}

// ============================================================================
// Section 4: Integration Tests [integration]
// ============================================================================

// Helper: process N blocks through the processor with a constant input signal.
// Uses the processor's actual total output channel count (typically 50) for the buffer,
// since processBlock may access channels up to getTotalNumOutputChannels().
// Includes warmup period to fill the delay line and let parameter smoothing settle.
static juce::AudioBuffer<float> processBlocks (Proc& proc, int numBlocks, int blockSize,
                                                int /*numOutChannels_unused*/, float inputLevel = 0.5f)
{
    int totalCh = proc.getTotalNumOutputChannels();
    juce::AudioBuffer<float> result (totalCh, blockSize);
    result.clear();

    juce::MidiBuffer midi;

    // Warmup: fill delay line and let smoothing settle (matches createBinauralProcessor pattern)
    for (int w = 0; w < 30; ++w)
    {
        juce::AudioBuffer<float> buffer (totalCh, blockSize);
        buffer.clear();
        for (int s = 0; s < blockSize; ++s)
        {
            buffer.setSample (0, s, inputLevel);
            buffer.setSample (1, s, inputLevel);
        }
        proc.processBlock (buffer, midi);
    }

    for (int b = 0; b < numBlocks; ++b)
    {
        juce::AudioBuffer<float> buffer (totalCh, blockSize);
        buffer.clear();

        // Write mono input to channels 0 and 1 (stereo input)
        for (int s = 0; s < blockSize; ++s)
        {
            buffer.setSample (0, s, inputLevel);
            buffer.setSample (1, s, inputLevel);
        }

        proc.processBlock (buffer, midi);

        // Keep the last block's output
        if (b == numBlocks - 1)
            result = buffer;
    }

    return result;
}

TEST_CASE ("Integration diagnostic: format activation and signal flow", "[integration][diagnostic]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    auto activeFormat = proc->getActiveOutputFormat();
    REQUIRE (activeFormat == OF::Surround7_1);

    const auto& layout = proc->getActiveLayout().layout;
    REQUIRE (layout.numSpeakers == 7);
    REQUIRE (layout.totalChannels == 8);
    REQUIRE (layout.lfeChannelIndex == 3);

    // Set simple params: min delay, 100% wet, tap 1 at center
    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    auto output = processBlocks (*proc, 8, 512, 8);

    // Check which channels have signal
    float maxRMS = 0.0f;
    int maxCh = -1;
    for (int ch = 0; ch < 8; ++ch)
    {
        float rms = computeChannelRMS (output, ch);
        INFO ("Channel " << ch << " RMS = " << rms);
        if (rms > maxRMS) { maxRMS = rms; maxCh = ch; }
    }
    // Center speaker should be loudest (ch2), not ch0
    CHECK (maxCh == 2);
    CHECK (maxRMS > 0.001f);
}

TEST_CASE ("Integration: 5.1 VBAP tap at center routes to ch2", "[integration]")
{
    auto proc = createTestProcessor (4 /*5.1*/, AlgoIdx::VBAP);

    // Verify format activated correctly
    REQUIRE (proc->getActiveOutputFormat() == OF::Surround5_1);

    // Set minimum delay, 100% wet, no feedback
    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);

    // Single tap at front center (0° azimuth)
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    // Process enough blocks for delay to warm up
    auto output = processBlocks (*proc, 8, 512, 6);

    // Dump channel RMS for debugging
    for (int ch = 0; ch < 6; ++ch)
    {
        float rms = computeChannelRMS (output, ch);
        INFO ("5.1 ch" << ch << " RMS=" << rms);
    }

    // Channel 2 (C) should be the loudest, skip LFE (ch3)
    int loudest = findLoudestChannel (output, 6, 3 /*skip LFE*/);
    CHECK (loudest == 2);
}

TEST_CASE ("Integration: 7.1 VBAP tap at 90deg routes to ch4 (Ls)", "[integration]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    REQUIRE (proc->getActiveOutputFormat() == OF::Surround7_1);

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);

    enableSingleTap (*proc, 0, 90.0f, 0.0f, 0.5f);

    auto output = processBlocks (*proc, 8, 512, 8);

    int loudest = findLoudestChannel (output, 8, 3 /*skip LFE*/);
    CHECK (loudest == 4);  // Ls at 90° → ch4
}

TEST_CASE ("Integration: 7.1.4 VBAP tap at height routes to height channel", "[integration]")
{
    auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
    REQUIRE (proc->getActiveOutputFormat() == OF::Surround7_1_4);

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);

    // Tap at Tfl position: 45° az, 45° el
    enableSingleTap (*proc, 0, 45.0f, 45.0f, 0.5f);

    auto output = processBlocks (*proc, 8, 512, 12);

    int loudest = findLoudestChannel (output, 12, 3 /*skip LFE*/);
    // Height channels in 7.1.4 are ch8 (Tfl), ch9 (Tfr), ch10 (Trl), ch11 (Trr)
    CHECK (loudest >= 8);
    CHECK (loudest <= 11);
}

TEST_CASE ("Integration: 9.1.6 VBAP tap at wide (60deg) routes to ch8 (Lw)", "[integration]")
{
    auto proc = createTestProcessor (15 /*9.1.6*/, AlgoIdx::VBAP);
    REQUIRE (proc->getActiveOutputFormat() == OF::Surround9_1_6);

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);

    enableSingleTap (*proc, 0, 60.0f, 0.0f, 0.5f);

    auto output = processBlocks (*proc, 8, 512, 16);

    int loudest = findLoudestChannel (output, 16, 3 /*skip LFE*/);
    CHECK (loudest == 8);  // Lw at 60° → ch8
}

TEST_CASE ("Integration: Quad VBAP tap at left surround (135deg) routes to ch2", "[integration]")
{
    auto proc = createTestProcessor (2 /*Quad*/, AlgoIdx::VBAP);
    REQUIRE (proc->getActiveOutputFormat() == OF::Quad);

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);

    enableSingleTap (*proc, 0, 135.0f, 0.0f, 0.5f);

    auto output = processBlocks (*proc, 8, 512, 4);

    int loudest = findLoudestChannel (output, 4);
    CHECK (loudest == 2);  // Ls at 135° → ch2
}

// ============================================================================
// Section 5: LFE Tests [lfe]
// ============================================================================

TEST_CASE ("LFE: 5.1 channel 3 receives signal", "[lfe]")
{
    auto proc = createTestProcessor (4 /*5.1*/, AlgoIdx::VBAP);

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    auto output = processBlocks (*proc, 8, 512, 6);

    float lfeRMS = computeChannelRMS (output, 3);
    CHECK (lfeRMS > 0.0001f);  // LFE should have some signal
}

TEST_CASE ("LFE: 7.1 LFE level is approximately -10dB below spatial channels", "[lfe]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);

    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);

    // Tap at center so we get clear signal to C speaker
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    auto output = processBlocks (*proc, 8, 512, 8);

    float lfeRMS = computeChannelRMS (output, 3);
    float centerRMS = computeChannelRMS (output, 2);

    // LFE should be significantly quieter than center (-10dB ≈ 0.316 ratio)
    // Allow generous tolerance since LFE is also low-pass filtered
    if (centerRMS > 0.001f && lfeRMS > 0.0001f)
    {
        float ratio = lfeRMS / centerRMS;
        CHECK (ratio < 0.6f);   // LFE should be at least ~4.4dB below
    }
}

// ============================================================================
// Section 6: Channel Ordering Verification [channel-order]
// ============================================================================

// For each speaker in a layout, place a source there and verify the output
// channel corresponding to that speaker is the loudest.

TEST_CASE ("Channel ordering: Quad — sweep all speakers", "[channel-order]")
{
    // Quad: 0:L@45°, 1:R@-45°, 2:Ls@135°, 3:Rs@-135°
    struct TestPoint { float azDeg; int expectedChannel; const char* name; };
    TestPoint points[] = {
        {  45.0f, 0, "L" },
        { -45.0f, 1, "R" },
        { 135.0f, 2, "Ls" },
        {-135.0f, 3, "Rs" },
    };

    for (const auto& pt : points)
    {
        SECTION (pt.name)
        {
            auto proc = createTestProcessor (2 /*Quad*/, AlgoIdx::VBAP);
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, pt.azDeg, 0.0f, 0.5f);

            auto output = processBlocks (*proc, 8, 512, 4);
            int loudest = findLoudestChannel (output, 4);
            CHECK (loudest == pt.expectedChannel);
        }
    }
}

TEST_CASE ("Channel ordering: 7.1 — sweep all speakers", "[channel-order]")
{
    // 7.1: 0:L@30°, 1:R@-30°, 2:C@0°, 3:LFE, 4:Ls@90°, 5:Rs@-90°, 6:Lrs@135°, 7:Rrs@-135°
    struct TestPoint { float azDeg; int expectedChannel; const char* name; };
    TestPoint points[] = {
        {   30.0f, 0, "L" },
        {  -30.0f, 1, "R" },
        {    0.0f, 2, "C" },
        {   90.0f, 4, "Ls" },
        {  -90.0f, 5, "Rs" },
        {  135.0f, 6, "Lrs" },
        { -135.0f, 7, "Rrs" },
    };

    for (const auto& pt : points)
    {
        SECTION (pt.name)
        {
            auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, pt.azDeg, 0.0f, 0.5f);

            auto output = processBlocks (*proc, 8, 512, 8);
            int loudest = findLoudestChannel (output, 8, 3 /*skip LFE*/);
            CHECK (loudest == pt.expectedChannel);
        }
    }
}

TEST_CASE ("Channel ordering: 7.1.4 — sweep all speakers including height", "[channel-order]")
{
    // 7.1.4: 0:L@30°, 1:R@-30°, 2:C@0°, 3:LFE, 4:Ls@90°, 5:Rs@-90°,
    //        6:Lrs@135°, 7:Rrs@-135°, 8:Tfl@45°↑45°, 9:Tfr@-45°↑45°,
    //        10:Trl@135°↑45°, 11:Trr@-135°↑45°
    struct TestPoint { float azDeg; float elDeg; int expectedChannel; const char* name; };
    TestPoint points[] = {
        {   30.0f,  0.0f,  0, "L" },
        {  -30.0f,  0.0f,  1, "R" },
        {    0.0f,  0.0f,  2, "C" },
        {   90.0f,  0.0f,  4, "Ls" },
        {  -90.0f,  0.0f,  5, "Rs" },
        {  135.0f,  0.0f,  6, "Lrs" },
        { -135.0f,  0.0f,  7, "Rrs" },
        {   45.0f, 45.0f,  8, "Tfl" },
        {  -45.0f, 45.0f,  9, "Tfr" },
        {  135.0f, 45.0f, 10, "Trl" },
        { -135.0f, 45.0f, 11, "Trr" },
    };

    for (const auto& pt : points)
    {
        SECTION (pt.name)
        {
            auto proc = createTestProcessor (12 /*7.1.4*/, AlgoIdx::VBAP);
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, pt.azDeg, pt.elDeg, 0.5f);

            auto output = processBlocks (*proc, 8, 512, 12);
            int loudest = findLoudestChannel (output, 12, 3 /*skip LFE*/);
            CHECK (loudest == pt.expectedChannel);
        }
    }
}

TEST_CASE ("Channel ordering: 9.1.6 — sweep all speakers including wide + height", "[channel-order]")
{
    // 9.1.6: 0:L@30°, 1:R@-30°, 2:C@0°, 3:LFE, 4:Ls@90°, 5:Rs@-90°,
    //        6:Lrs@135°, 7:Rrs@-135°, 8:Lw@60°, 9:Rw@-60°,
    //        10:Tfl@45°↑45°, 11:Tfr@-45°↑45°, 12:Tsl@90°↑45°, 13:Tsr@-90°↑45°,
    //        14:Trl@135°↑45°, 15:Trr@-135°↑45°
    struct TestPoint { float azDeg; float elDeg; int expectedChannel; const char* name; };
    TestPoint points[] = {
        {   30.0f,  0.0f,  0, "L" },
        {  -30.0f,  0.0f,  1, "R" },
        {    0.0f,  0.0f,  2, "C" },
        {   90.0f,  0.0f,  4, "Ls" },
        {  -90.0f,  0.0f,  5, "Rs" },
        {  135.0f,  0.0f,  6, "Lrs" },
        { -135.0f,  0.0f,  7, "Rrs" },
        {   60.0f,  0.0f,  8, "Lw" },
        {  -60.0f,  0.0f,  9, "Rw" },
        {   45.0f, 45.0f, 10, "Tfl" },
        {  -45.0f, 45.0f, 11, "Tfr" },
        {   90.0f, 45.0f, 12, "Tsl" },
        {  -90.0f, 45.0f, 13, "Tsr" },
        {  135.0f, 45.0f, 14, "Trl" },
        { -135.0f, 45.0f, 15, "Trr" },
    };

    for (const auto& pt : points)
    {
        SECTION (pt.name)
        {
            auto proc = createTestProcessor (15 /*9.1.6*/, AlgoIdx::VBAP);
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, pt.azDeg, pt.elDeg, 0.5f);

            auto output = processBlocks (*proc, 8, 512, 16);
            int loudest = findLoudestChannel (output, 16, 3 /*skip LFE*/);
            CHECK (loudest == pt.expectedChannel);
        }
    }
}

// ============================================================================
// Section 7: Pitch Drift Diagnostic [drift]
// ============================================================================

TEST_CASE ("Drift diagnostic: feedback repeats maintain consistent timing", "[drift]")
{
    auto proc = createTestProcessor (6 /*7.1*/, AlgoIdx::VBAP);
    REQUIRE (proc->getActiveOutputFormat() == OF::Surround7_1);

    setParam (*proc, "delayTime", 202.0f);
    setParam (*proc, "feedback", 0.81f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "filterEnabled", 1.0f);
    setParam (*proc, "filterLP", 6000.0f);
    setParam (*proc, "filterHP", 80.0f);
    setParam (*proc, "filterLPQ", 1.2f);
    setParam (*proc, "filterHPQ", 0.707f);
    setParam (*proc, "airAbsorption", 1.0f);

    enableSingleTap (*proc, 0, -88.1f, 0.0f, 0.39f);
    setParam (*proc, "object1_pitchShift", 0.0f);
    setParam (*proc, "object1_dopplerAmount", 0.0f);

    const int blockSize = 512;
    const int totalBlocks = 200;
    int totalCh = proc->getTotalNumOutputChannels();
    juce::MidiBuffer midi;

    // Collect ALL channels to find where energy goes
    std::vector<std::vector<float>> allChannels (8);
    for (auto& ch : allChannels)
        ch.reserve (static_cast<size_t> (totalBlocks * blockSize));

    for (int b = 0; b < totalBlocks; ++b)
    {
        juce::AudioBuffer<float> buffer (totalCh, blockSize);
        buffer.clear();
        if (b == 0)
        {
            buffer.setSample (0, 0, 0.5f);
            buffer.setSample (1, 0, 0.5f);
        }
        proc->processBlock (buffer, midi);
        for (int ch = 0; ch < 8; ++ch)
            for (int s = 0; s < blockSize; ++s)
                allChannels[static_cast<size_t>(ch)].push_back (buffer.getSample (ch, s));
    }

    // Find which channel has the most energy (skip LFE ch3)
    int bestCh = 0;
    float bestRMS = 0.0f;
    for (int ch = 0; ch < 8; ++ch)
    {
        if (ch == 3) continue; // skip LFE
        float sum = 0.0f;
        for (float v : allChannels[static_cast<size_t>(ch)])
            sum += v * v;
        float rms = std::sqrt (sum / static_cast<float> (allChannels[static_cast<size_t>(ch)].size()));
        INFO ("Channel " << ch << " RMS = " << rms);
        if (rms > bestRMS) { bestRMS = rms; bestCh = ch; }
    }
    INFO ("Loudest channel = " << bestCh << " with RMS = " << bestRMS);

    auto& outputSamples = allChannels[static_cast<size_t>(bestCh)];

    float threshold = 0.0001f;
    std::vector<int> peakPositions;
    for (int i = 2; i < (int) outputSamples.size() - 2; ++i)
    {
        float val = std::abs (outputSamples[static_cast<size_t> (i)]);
        if (val > threshold
            && val >= std::abs (outputSamples[static_cast<size_t> (i - 1)])
            && val >= std::abs (outputSamples[static_cast<size_t> (i + 1)])
            && val >= std::abs (outputSamples[static_cast<size_t> (i - 2)])
            && val >= std::abs (outputSamples[static_cast<size_t> (i + 2)]))
        {
            if (peakPositions.empty() || (i - peakPositions.back()) > 4000)
                peakPositions.push_back (i);
        }
    }

    INFO ("Found " << peakPositions.size() << " peaks");
    REQUIRE (peakPositions.size() >= 4);

    std::vector<int> intervals;
    for (size_t p = 1; p < peakPositions.size(); ++p)
    {
        int interval = peakPositions[p] - peakPositions[p - 1];
        intervals.push_back (interval);
    }

    int expectedInterval = static_cast<int> (202.0f * 48.0f);
    int tol = expectedInterval / 50;  // 2% tolerance

    for (size_t i = 0; i < intervals.size(); ++i)
    {
        INFO ("Interval " << i << " = " << intervals[i] << " (expected ~" << expectedInterval << ")");
        CHECK (std::abs (intervals[i] - expectedInterval) < tol);
    }

    if (intervals.size() >= 3)
    {
        int drift = intervals.back() - intervals.front();
        INFO ("Drift first-to-last: " << drift << " samples");
        CHECK (std::abs (drift) < tol);
    }
}

// ============================================================================
// Section 7: Constrained-Bus Regression Tests [bus]
// These tests mirror DAW behavior by using setBusesLayout with standard
// AudioChannelSets instead of the 50-channel discrete default.
// ============================================================================

TEST_CASE ("isBusesLayoutSupported accepts VST3 default layout (9.1.6)", "[bus]")
{
    auto proc = std::make_unique<Proc>();
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses.add (juce::AudioChannelSet::stereo());
    layout.outputBuses.add (juce::AudioChannelSet::create9point1point6());
    CHECK (proc->checkBusesLayoutSupported (layout));
}

TEST_CASE ("isBusesLayoutSupported accepts symmetric layouts for VST3 (issue #111)", "[bus]")
{
    auto proc = std::make_unique<Proc>();

    // Symmetric 7.1 in/out — REAPER proposes this on an 8ch track
    {
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add (juce::AudioChannelSet::create7point1());
        layout.outputBuses.add (juce::AudioChannelSet::create7point1());
        CHECK (proc->checkBusesLayoutSupported (layout));
    }

    // Symmetric 9.1.6 in/out — REAPER proposes this on a 16ch track
    {
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add (juce::AudioChannelSet::create9point1point6());
        layout.outputBuses.add (juce::AudioChannelSet::create9point1point6());
        CHECK (proc->checkBusesLayoutSupported (layout));
    }

    // Asymmetric multichannel in / different out — now accepted (IEM approach, issue #111)
    {
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add (juce::AudioChannelSet::create7point1());
        layout.outputBuses.add (juce::AudioChannelSet::create9point1point6());
        CHECK (proc->checkBusesLayoutSupported (layout));
    }

    // Mono/stereo input still works with any supported output
    {
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add (juce::AudioChannelSet::stereo());
        layout.outputBuses.add (juce::AudioChannelSet::create7point1());
        CHECK (proc->checkBusesLayoutSupported (layout));
    }
}

TEST_CASE ("isBusesLayoutSupported accepts ambisonic() channel sets for VST3 (issue #111)", "[bus]")
{
    auto proc = std::make_unique<Proc>();

    // ambisonic(1-6) must be accepted for VST3 HOA negotiation
    for (int order = 1; order <= 6; ++order)
    {
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add (juce::AudioChannelSet::stereo());
        layout.outputBuses.add (juce::AudioChannelSet::ambisonic (order));
        INFO ("Ambisonics order " << order << " (" << ((order + 1) * (order + 1)) << "ch)");
        CHECK (proc->checkBusesLayoutSupported (layout));
    }

    // Symmetric ambisonic(6) in/out — VST3 default on a 49ch track
    {
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add (juce::AudioChannelSet::ambisonic (6));
        layout.outputBuses.add (juce::AudioChannelSet::ambisonic (6));
        CHECK (proc->checkBusesLayoutSupported (layout));
    }
}

TEST_CASE ("Constrained bus: 7.1 rear speakers receive signal (8ch buffer)", "[bus][regression]")
{
    struct TestPoint { float azDeg; int expectedChannel; const char* name; };
    TestPoint points[] = {
        {  135.0f, 6, "Lrs" },
        { -135.0f, 7, "Rrs" },
        {   90.0f, 4, "Ls" },
        {  -90.0f, 5, "Rs" },
        {   30.0f, 0, "L" },
        {  -30.0f, 1, "R" },
        {    0.0f, 2, "C" },
    };

    for (const auto& pt : points)
    {
        SECTION (pt.name)
        {
            auto proc = createConstrainedProcessor (6 /*7.1*/, AlgoIdx::VBAP,
                                                     juce::AudioChannelSet::create7point1());
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, pt.azDeg, 0.0f, 0.5f);

            auto output = processBlocksConstrained (*proc, 8, 512, 8);
            int loudest = findLoudestChannel (output, 8, 3 /*skip LFE*/);
            INFO ("Expected ch" << pt.expectedChannel << " (" << pt.name << "), got ch" << loudest);
            for (int ch = 0; ch < 8; ++ch)
                INFO ("  ch" << ch << " RMS=" << computeChannelRMS (output, ch));
            CHECK (loudest == pt.expectedChannel);
        }
    }
}

TEST_CASE ("Constrained bus: 7.1.4 rear speakers receive signal (12ch buffer)", "[bus][regression]")
{
    struct TestPoint { float azDeg; float elDeg; int expectedChannel; const char* name; };
    TestPoint points[] = {
        {  135.0f,  0.0f,  6, "Lrs" },
        { -135.0f,  0.0f,  7, "Rrs" },
        {   90.0f,  0.0f,  4, "Ls" },
        {  -90.0f,  0.0f,  5, "Rs" },
        {   45.0f, 45.0f,  8, "Tfl" },
        {  -45.0f, 45.0f,  9, "Tfr" },
        {  135.0f, 45.0f, 10, "Trl" },
        { -135.0f, 45.0f, 11, "Trr" },
    };

    for (const auto& pt : points)
    {
        SECTION (pt.name)
        {
            auto proc = createConstrainedProcessor (12 /*7.1.4*/, AlgoIdx::VBAP,
                                                     juce::AudioChannelSet::create7point1point4());
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, pt.azDeg, pt.elDeg, 0.5f);

            auto output = processBlocksConstrained (*proc, 8, 512, 12);
            int loudest = findLoudestChannel (output, 12, 3 /*skip LFE*/);
            INFO ("Expected ch" << pt.expectedChannel << " (" << pt.name << "), got ch" << loudest);
            for (int ch = 0; ch < 12; ++ch)
                INFO ("  ch" << ch << " RMS=" << computeChannelRMS (output, ch));
            CHECK (loudest == pt.expectedChannel);
        }
    }
}

TEST_CASE ("Constrained bus: all algorithms route to Lrs on 7.1 (8ch)", "[bus][algorithm]")
{
    int algos[] = { AlgoIdx::VBAP, AlgoIdx::VBIP, AlgoIdx::KNN,
                    AlgoIdx::DBAP, AlgoIdx::MDAP };
    const char* names[] = { "VBAP", "VBIP", "KNN", "DBAP", "MDAP" };

    for (int a = 0; a < 5; ++a)
    {
        SECTION (names[a])
        {
            auto proc = createConstrainedProcessor (6 /*7.1*/, algos[a],
                                                     juce::AudioChannelSet::create7point1());
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, 135.0f, 0.0f, 0.5f);

            auto output = processBlocksConstrained (*proc, 8, 512, 8);
            float lrsRMS = computeChannelRMS (output, 6);
            float lsRMS = computeChannelRMS (output, 4);
            INFO (names[a] << ": Lrs(ch6) RMS=" << lrsRMS << " Ls(ch4) RMS=" << lsRMS);
            CHECK (lrsRMS > 0.001f);   // Lrs must have signal
            CHECK (lrsRMS > lsRMS);    // Lrs should be louder than Ls for source at 135°
        }
    }
}

// ============================================================================
// Section 8: SML 13.1 Tests [sml]
// ============================================================================

TEST_CASE ("Channel ordering: SML 13.1 — sweep all speakers including zenith", "[channel-order][sml]")
{
    // SML 13.1: 0:FC@0°, 1:FR@-45°, 2:R@-90°, 3:RR@-135°, 4:RC@180°, 5:RL@135°,
    //           6:L@90°, 7:FL@45°, 8:UFR@-45°↑45°, 9:URR@-135°↑45°,
    //           10:URL@135°↑45°, 11:UFL@45°↑45°, 12:T@0°↑90°, 13:LFE
    struct TestPoint { float azDeg; float elDeg; int expectedChannel; const char* name; };
    TestPoint points[] = {
        {    0.0f,  0.0f,  0, "FC" },
        {  -45.0f,  0.0f,  1, "FR" },
        {  -90.0f,  0.0f,  2, "R" },
        { -135.0f,  0.0f,  3, "RR" },
        {  180.0f,  0.0f,  4, "RC" },
        {  135.0f,  0.0f,  5, "RL" },
        {   90.0f,  0.0f,  6, "L" },
        {   45.0f,  0.0f,  7, "FL" },
        {  -45.0f, 45.0f,  8, "UFR" },
        { -135.0f, 45.0f,  9, "URR" },
        {  135.0f, 45.0f, 10, "URL" },
        {   45.0f, 45.0f, 11, "UFL" },
        {    0.0f, 90.0f, 12, "Zenith" },
    };

    for (const auto& pt : points)
    {
        SECTION (pt.name)
        {
            auto proc = createTestProcessor (16 /*SML 13.1*/, AlgoIdx::VBAP);
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, pt.azDeg, pt.elDeg, 0.5f);

            auto output = processBlocks (*proc, 8, 512, 14);
            int loudest = findLoudestChannel (output, 14, 13 /*skip LFE*/);
            INFO ("Expected ch" << pt.expectedChannel << " (" << pt.name << "), got ch" << loudest);
            for (int ch = 0; ch < 14; ++ch)
                INFO ("  ch" << ch << " RMS=" << computeChannelRMS (output, ch));
            CHECK (loudest == pt.expectedChannel);
        }
    }
}

TEST_CASE ("SML 13.1: LFE receives low-pass filtered signal", "[sml][lfe]")
{
    auto proc = createTestProcessor (16 /*SML 13.1*/, AlgoIdx::VBAP);
    setParam (*proc, "delayTime", 1.0f);
    setParam (*proc, "dryWet", 1.0f);
    setParam (*proc, "feedback", 0.0f);
    setParam (*proc, "outputGain", 0.0f);
    enableSingleTap (*proc, 0, 0.0f, 0.0f, 0.5f);

    auto output = processBlocks (*proc, 8, 512, 14);
    float lfeRMS = computeChannelRMS (output, 13);  // LFE at ch13
    INFO ("LFE (ch13) RMS=" << lfeRMS);
    CHECK (lfeRMS > 0.0001f);  // LFE should have signal from LP-filtered mono sum
}

TEST_CASE ("SML 13.1: all algorithms produce signal", "[sml][algorithm]")
{
    int algos[] = { AlgoIdx::VBAP, AlgoIdx::VBIP, AlgoIdx::KNN,
                    AlgoIdx::DBAP, AlgoIdx::MDAP };
    const char* names[] = { "VBAP", "VBIP", "KNN", "DBAP", "MDAP" };

    for (int a = 0; a < 5; ++a)
    {
        SECTION (names[a])
        {
            auto proc = createTestProcessor (16 /*SML 13.1*/, algos[a]);
            setParam (*proc, "delayTime", 1.0f);
            setParam (*proc, "dryWet", 1.0f);
            setParam (*proc, "feedback", 0.0f);
            setParam (*proc, "outputGain", 0.0f);
            enableSingleTap (*proc, 0, 45.0f, 0.0f, 0.5f);  // Front Left

            auto output = processBlocks (*proc, 8, 512, 14);
            // Channel 7 (FL at 45°) should have signal
            float flRMS = computeChannelRMS (output, 7);
            INFO (names[a] << ": FL(ch7) RMS=" << flRMS);
            CHECK (flRMS > 0.001f);
        }
    }
}
