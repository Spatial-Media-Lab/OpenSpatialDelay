#include <catch2/catch_test_macros.hpp>
#include "../Source/PluginProcessor.h"

// Issue #68: Verify parameter count, naming, automatability, and ordering.
// These tests catch regressions from parameter layout changes.
// Issue E20: admOscEnabled removed from APVTS → 143 params (was 144), 79 non-automatable (was 80).

TEST_CASE ("Parameter count is exactly 143", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();
    CHECK (params.size() == 143);
}

TEST_CASE ("All parameters have non-empty names", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    for (auto* p : proc->getParameters())
    {
        REQUIRE (p != nullptr);
        CHECK_FALSE (p->getName (256).isEmpty());
    }
}

// Issue #189: Parameter automatability is DAW-conditional via PluginHostType.
// In Ableton: 64 automatable, 79 non-automatable (fits auto-populate limit, issue #122).
// In all other DAWs: all 143 automatable (REAPER, Logic, Pro Tools, Nuendo).
// Test harness is not Ableton, so the default processor has all 143 automatable.

TEST_CASE ("Non-Ableton: all 143 automatable", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    int automatable = 0, nonAutomatable = 0;
    for (auto* p : proc->getParameters())
    {
        if (p->isAutomatable())
            ++automatable;
        else
            ++nonAutomatable;
    }
    CHECK (automatable == 143);
    CHECK (nonAutomatable == 0);
}

TEST_CASE ("Ableton mode: 64 automatable + 79 non-automatable", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor> (true);
    int automatable = 0, nonAutomatable = 0;
    for (auto* p : proc->getParameters())
    {
        if (p->isAutomatable())
            ++automatable;
        else
            ++nonAutomatable;
    }
    CHECK (automatable == 64);
    CHECK (nonAutomatable == 79);
}

TEST_CASE ("Global params appear before per-tap params", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();

    // First 23 params are globals; index 23 should be "Tap 01 On"
    // (was 24 before admOscEnabled removed from APVTS in issue E20)
    REQUIRE (params.size() >= 24);
    CHECK (params[0]->getName (256) == "Delay Time");
    CHECK (params[1]->getName (256) == "Delay Sync");
    CHECK (params[4]->getName (256) == "Feedback");
    CHECK (params[10]->getName (256) == "Dry/Wet");
    CHECK (params[22]->getName (256) == "Global Tap Speed");

    // Per-tap: index 23 = Tap 01 On, 33 = Tap 02 On, etc.
    CHECK (params[23]->getName (256) == "Tap 01 On");
    CHECK (params[33]->getName (256) == "Tap 02 On");
    CHECK (params[43]->getName (256) == "Tap 03 On");
}

TEST_CASE ("Per-tap params are grouped by tap", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();

    // Tap 01: indices 23-32 (10 params)
    CHECK (params[23]->getName (256) == "Tap 01 On");
    CHECK (params[24]->getName (256) == "Tap 01 Azimuth");
    CHECK (params[25]->getName (256) == "Tap 01 Elevation");
    CHECK (params[26]->getName (256) == "Tap 01 Distance");
    CHECK (params[27]->getName (256) == "Tap 01 Doppler Amount");
    CHECK (params[28]->getName (256) == "Tap 01 Pitch Shift");
    CHECK (params[29]->getName (256) == "Tap 01 Trajectory Shape");
    CHECK (params[30]->getName (256) == "Tap 01 Trajectory Speed");
    CHECK (params[31]->getName (256) == "Tap 01 Trajectory Direction");
    CHECK (params[32]->getName (256) == "Tap 01 Input Channel");

    // Tap 12: indices 133-142 (last 10 params)
    CHECK (params[133]->getName (256) == "Tap 12 On");
    CHECK (params[142]->getName (256) == "Tap 12 Input Channel");
}
