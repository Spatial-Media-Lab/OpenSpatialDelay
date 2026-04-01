#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Source/PluginProcessor.h"

using Proc = OpenSpatialDelayProcessor;
using Catch::Matchers::WithinAbs;

// ============================================================================
// Helper: create a processor ready for OSC testing
// ============================================================================
static std::unique_ptr<Proc> createOscProcessor()
{
    auto proc = std::make_unique<Proc>();
    proc->prepareToPlay (48000.0, 512);
    return proc;
}

// Helper: build and deliver an OSC message to the processor
static void sendOSC (Proc& proc, const juce::String& address, std::initializer_list<float> values)
{
    juce::OSCMessage msg (address);
    for (float v : values)
        msg.addFloat32 (v);
    proc.testProcessOSCMessage (msg);
}

// Helper: read a parameter's denormalized value
static float readParam (Proc& proc, const juce::String& paramID)
{
    if (auto* p = proc.apvts.getRawParameterValue (paramID))
        return p->load();
    return -9999.0f;
}

// ============================================================================
// Section 1: ADM-OSC Position Receive (existing /adm/obj/N/ namespace)
// ============================================================================

TEST_CASE ("OSC: /adm/obj/N/azim sets azimuth", "[osc][position]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/adm/obj/1/azim", { 90.0f });
    REQUIRE_THAT (readParam (*proc, "object1_azimuth"), WithinAbs (90.0f, 0.5f));
}

TEST_CASE ("OSC: /adm/obj/N/elev sets elevation", "[osc][position]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/adm/obj/2/elev", { 45.0f });
    REQUIRE_THAT (readParam (*proc, "object2_elevation"), WithinAbs (45.0f, 0.5f));
}

TEST_CASE ("OSC: /adm/obj/N/dist sets distance", "[osc][position]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/adm/obj/3/dist", { 0.75f });
    REQUIRE_THAT (readParam (*proc, "object3_distance"), WithinAbs (0.75f, 0.01f));
}

TEST_CASE ("OSC: /adm/obj/N/aed sets all three position params", "[osc][position]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/adm/obj/4/aed", { -45.0f, 30.0f, 0.6f });
    REQUIRE_THAT (readParam (*proc, "object4_azimuth"),   WithinAbs (-45.0f, 0.5f));
    REQUIRE_THAT (readParam (*proc, "object4_elevation"), WithinAbs (30.0f, 0.5f));
    REQUIRE_THAT (readParam (*proc, "object4_distance"),  WithinAbs (0.6f, 0.01f));
}

// ============================================================================
// Section 2: /osd/obj/N/ Position Aliases (receive-only)
// ============================================================================

TEST_CASE ("OSC: /osd/obj/N/azim alias sets azimuth", "[osc][alias]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/1/azim", { -120.0f });
    REQUIRE_THAT (readParam (*proc, "object1_azimuth"), WithinAbs (-120.0f, 0.5f));
}

TEST_CASE ("OSC: /osd/obj/N/elev alias sets elevation", "[osc][alias]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/5/elev", { -30.0f });
    REQUIRE_THAT (readParam (*proc, "object5_elevation"), WithinAbs (-30.0f, 0.5f));
}

TEST_CASE ("OSC: /osd/obj/N/dist alias sets distance", "[osc][alias]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/6/dist", { 0.2f });
    REQUIRE_THAT (readParam (*proc, "object6_distance"), WithinAbs (0.2f, 0.01f));
}

TEST_CASE ("OSC: /osd/obj/N/aed alias sets all three", "[osc][alias]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/7/aed", { 135.0f, -15.0f, 0.9f });
    REQUIRE_THAT (readParam (*proc, "object7_azimuth"),   WithinAbs (135.0f, 0.5f));
    REQUIRE_THAT (readParam (*proc, "object7_elevation"), WithinAbs (-15.0f, 0.5f));
    REQUIRE_THAT (readParam (*proc, "object7_distance"),  WithinAbs (0.9f, 0.01f));
}

// ============================================================================
// Section 3: /osd/obj/N/ Per-Object Non-Position Params
// ============================================================================

TEST_CASE ("OSC: /osd/obj/N/enabled sets tap enabled", "[osc][per-object]")
{
    auto proc = createOscProcessor();
    // Disable first, then enable via OSC
    if (auto* p = proc->apvts.getParameter ("object1_enabled"))
        p->setValueNotifyingHost (0.0f);
    sendOSC (*proc, "/osd/obj/1/enabled", { 1.0f });
    REQUIRE (readParam (*proc, "object1_enabled") >= 0.5f);
}

TEST_CASE ("OSC: /osd/obj/N/doppler sets doppler amount", "[osc][per-object]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/2/doppler", { 0.75f });
    REQUIRE_THAT (readParam (*proc, "object2_dopplerAmount"), WithinAbs (0.75f, 0.01f));
}

TEST_CASE ("OSC: /osd/obj/N/pitch sets per-tap pitch", "[osc][per-object]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/3/pitch", { -12.0f });
    REQUIRE_THAT (readParam (*proc, "object3_pitchShift"), WithinAbs (-12.0f, 0.5f));
}

TEST_CASE ("OSC: /osd/obj/N/trajectory sets shape", "[osc][per-object]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/4/trajectory", { 9.0f });  // Orbit
    REQUIRE_THAT (readParam (*proc, "object4_trajectoryShape"), WithinAbs (9.0f, 0.5f));
}

TEST_CASE ("OSC: /osd/obj/N/speed sets trajectory speed", "[osc][per-object]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/5/speed", { 2.5f });
    REQUIRE_THAT (readParam (*proc, "object5_trajectorySpeed"), WithinAbs (2.5f, 0.05f));
}

TEST_CASE ("OSC: /osd/obj/N/direction sets trajectory direction", "[osc][per-object]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/6/direction", { 1.0f });  // Reverse
    REQUIRE_THAT (readParam (*proc, "object6_trajectoryDirection"), WithinAbs (1.0f, 0.5f));
}

TEST_CASE ("OSC: /osd/obj/N/input sets input channel", "[osc][per-object]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/obj/7/input", { 2.0f });  // R only
    REQUIRE_THAT (readParam (*proc, "object7_inputChannel"), WithinAbs (2.0f, 0.5f));
}

// ============================================================================
// Section 4: /osd/global/ Global Params
// ============================================================================

TEST_CASE ("OSC: /osd/global/delaytime sets delay time", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/delaytime", { 250.0f });
    REQUIRE_THAT (readParam (*proc, "delayTime"), WithinAbs (250.0f, 1.0f));
}

TEST_CASE ("OSC: /osd/global/feedback sets feedback", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/feedback", { 0.8f });
    REQUIRE_THAT (readParam (*proc, "feedback"), WithinAbs (0.8f, 0.01f));
}

TEST_CASE ("OSC: /osd/global/drywet sets dry/wet mix", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/drywet", { 0.3f });
    REQUIRE_THAT (readParam (*proc, "dryWet"), WithinAbs (0.3f, 0.01f));
}

TEST_CASE ("OSC: /osd/global/filterlp sets LP frequency", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/filterlp", { 5000.0f });
    REQUIRE_THAT (readParam (*proc, "filterLP"), WithinAbs (5000.0f, 50.0f));
}

TEST_CASE ("OSC: /osd/global/filterhp sets HP frequency", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/filterhp", { 200.0f });
    REQUIRE_THAT (readParam (*proc, "filterHP"), WithinAbs (200.0f, 5.0f));
}

TEST_CASE ("OSC: /osd/global/filterlpq sets LP Q", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/filterlpq", { 2.0f });
    REQUIRE_THAT (readParam (*proc, "filterLPQ"), WithinAbs (2.0f, 0.05f));
}

TEST_CASE ("OSC: /osd/global/filterhpq sets HP Q", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/filterhpq", { 3.0f });
    REQUIRE_THAT (readParam (*proc, "filterHPQ"), WithinAbs (3.0f, 0.05f));
}

TEST_CASE ("OSC: /osd/global/filterenabled sets filter toggle", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/filterenabled", { 1.0f });
    REQUIRE (readParam (*proc, "filterEnabled") >= 0.5f);
}

TEST_CASE ("OSC: /osd/global/inputgain sets input gain", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/inputgain", { 6.0f });
    REQUIRE_THAT (readParam (*proc, "inputGain"), WithinAbs (6.0f, 0.5f));
}

TEST_CASE ("OSC: /osd/global/outputgain sets output gain", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/outputgain", { -3.0f });
    REQUIRE_THAT (readParam (*proc, "outputGain"), WithinAbs (-3.0f, 0.5f));
}

TEST_CASE ("OSC: /osd/global/algorithm sets algorithm", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/algorithm", { 2.0f });  // KNN
    REQUIRE (proc->configAlgorithm.load (std::memory_order_relaxed) == 2);
}

TEST_CASE ("OSC: /osd/global/hrtfprofile sets HRTF profile", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/hrtfprofile", { 3.0f });  // CIPIC
    REQUIRE (proc->configHrtfProfile.load (std::memory_order_relaxed) == 3);
}

TEST_CASE ("OSC: /osd/global/air sets air absorption toggle", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/air", { 1.0f });
    REQUIRE (readParam (*proc, "airAbsorption") >= 0.5f);
}

TEST_CASE ("OSC: /osd/global/wobble sets wobble enable", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/wobble", { 1.0f });
    REQUIRE (readParam (*proc, "wobbleEnabled") >= 0.5f);
}

TEST_CASE ("OSC: /osd/global/wobbleamount sets wobble depth", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/wobbleamount", { 50.0f });
    REQUIRE_THAT (readParam (*proc, "wobbleAmount"), WithinAbs (50.0f, 1.0f));
}

TEST_CASE ("OSC: /osd/global/wobblemorph sets wobble waveform", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/wobblemorph", { 75.0f });
    REQUIRE_THAT (readParam (*proc, "wobbleMorph"), WithinAbs (75.0f, 1.0f));
}

TEST_CASE ("OSC: /osd/global/temposync sets tempo sync", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/temposync", { 1.0f });
    REQUIRE (readParam (*proc, "tempoSync") >= 0.5f);
}

TEST_CASE ("OSC: /osd/global/syncmode sets sync mode", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/syncmode", { 2.0f });  // Triplet
    REQUIRE_THAT (readParam (*proc, "syncMode"), WithinAbs (2.0f, 0.5f));
}

TEST_CASE ("OSC: /osd/global/outputformat sets output format", "[osc][global]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/outputformat", { 5.0f });  // Binaural
    REQUIRE (proc->configOutputFormat.load (std::memory_order_relaxed) == 5);
}

// ============================================================================
// Section 5: Edge Cases & Boundary Conditions
// ============================================================================

TEST_CASE ("OSC: Out-of-range object number is ignored", "[osc][edge]")
{
    auto proc = createOscProcessor();
    float prevAz = readParam (*proc, "object1_azimuth");
    sendOSC (*proc, "/adm/obj/0/azim", { 90.0f });   // 0 is below valid range (1-12)
    sendOSC (*proc, "/adm/obj/13/azim", { 90.0f });   // 13 is above valid range
    REQUIRE_THAT (readParam (*proc, "object1_azimuth"), WithinAbs (prevAz, 0.1f));
}

TEST_CASE ("OSC: Azimuth clamps at +-180", "[osc][edge]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/adm/obj/1/azim", { 999.0f });
    REQUIRE_THAT (readParam (*proc, "object1_azimuth"), WithinAbs (180.0f, 0.5f));
    sendOSC (*proc, "/adm/obj/1/azim", { -999.0f });
    REQUIRE_THAT (readParam (*proc, "object1_azimuth"), WithinAbs (-180.0f, 0.5f));
}

TEST_CASE ("OSC: Elevation clamps at +-90", "[osc][edge]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/adm/obj/1/elev", { 200.0f });
    REQUIRE_THAT (readParam (*proc, "object1_elevation"), WithinAbs (90.0f, 0.5f));
}

TEST_CASE ("OSC: Distance clamps at 0-1", "[osc][edge]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/adm/obj/1/dist", { 5.0f });
    REQUIRE_THAT (readParam (*proc, "object1_distance"), WithinAbs (1.0f, 0.01f));
    sendOSC (*proc, "/adm/obj/1/dist", { -1.0f });
    REQUIRE_THAT (readParam (*proc, "object1_distance"), WithinAbs (0.0f, 0.01f));
}

TEST_CASE ("OSC: Unknown /osd/ property is silently ignored", "[osc][edge]")
{
    auto proc = createOscProcessor();
    // Should not crash or change any param
    sendOSC (*proc, "/osd/obj/1/nonexistent", { 42.0f });
    sendOSC (*proc, "/osd/global/nonexistent", { 42.0f });
    SUCCEED ("No crash from unknown OSC property");
}

TEST_CASE ("OSC: All 12 objects addressable", "[osc][edge]")
{
    auto proc = createOscProcessor();
    for (int i = 1; i <= 12; ++i)
    {
        float targetAz = static_cast<float> (i * 15);
        sendOSC (*proc, "/osd/obj/" + juce::String (i) + "/azim", { targetAz });
        REQUIRE_THAT (readParam (*proc, "object" + juce::String (i) + "_azimuth"),
                      WithinAbs (targetAz, 0.5f));
    }
}

// ============================================================================
// Section 6: Global tap offset knobs (UI-only, stored in atomics)
// ============================================================================

TEST_CASE ("OSC: /osd/global/tapazimuth sets global AZ offset", "[osc][global-tap]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/tapazimuth", { 45.0f });
    REQUIRE_THAT (proc->globalTapOffset[0].load(), WithinAbs (45.0f, 0.01f));
    REQUIRE (proc->globalTapOffsetChanged.load());
}

TEST_CASE ("OSC: /osd/global/tapelevation sets global EL offset", "[osc][global-tap]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/tapelevation", { -30.0f });
    REQUIRE_THAT (proc->globalTapOffset[1].load(), WithinAbs (-30.0f, 0.01f));
}

TEST_CASE ("OSC: /osd/global/tapdistance sets global DIST offset", "[osc][global-tap]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/tapdistance", { 0.5f });
    REQUIRE_THAT (proc->globalTapOffset[2].load(), WithinAbs (0.5f, 0.01f));
}

TEST_CASE ("OSC: /osd/global/tappitch sets global PITCH offset", "[osc][global-tap]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/tappitch", { -12.0f });
    REQUIRE_THAT (proc->globalTapOffset[3].load(), WithinAbs (-12.0f, 0.01f));
}

TEST_CASE ("OSC: /osd/global/tapdoppler sets global DOPPLER offset", "[osc][global-tap]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/tapdoppler", { -30.0f });
    REQUIRE_THAT (proc->globalTapOffset[4].load(), WithinAbs (-30.0f, 0.01f));
}

TEST_CASE ("OSC: /osd/global/tapspeed sets global SPEED offset", "[osc][global-tap]")
{
    auto proc = createOscProcessor();
    sendOSC (*proc, "/osd/global/tapspeed", { 2.5f });
    REQUIRE_THAT (proc->globalTapOffset[5].load(), WithinAbs (2.5f, 0.01f));
}

TEST_CASE ("OSC: Global tap offsets clamp to valid range", "[osc][global-tap][edge]")
{
    auto proc = createOscProcessor();

    sendOSC (*proc, "/osd/global/tapazimuth", { 999.0f });
    REQUIRE_THAT (proc->globalTapOffset[0].load(), WithinAbs (180.0f, 0.01f));

    sendOSC (*proc, "/osd/global/tapelevation", { -200.0f });
    REQUIRE_THAT (proc->globalTapOffset[1].load(), WithinAbs (-90.0f, 0.01f));

    sendOSC (*proc, "/osd/global/tapdistance", { 5.0f });
    REQUIRE_THAT (proc->globalTapOffset[2].load(), WithinAbs (1.0f, 0.01f));

    sendOSC (*proc, "/osd/global/tappitch", { -48.0f });
    REQUIRE_THAT (proc->globalTapOffset[3].load(), WithinAbs (-24.0f, 0.01f));

    sendOSC (*proc, "/osd/global/tapspeed", { 10.0f });
    REQUIRE_THAT (proc->globalTapOffset[5].load(), WithinAbs (5.0f, 0.01f));
}
