#include <catch2/catch_test_macros.hpp>
#include "../Source/PluginProcessor.h"

// Issue #68: Verify parameter count, naming, automatability, and ordering.
// These tests catch regressions from parameter layout changes.

TEST_CASE ("Parameter count is exactly 144", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();
    CHECK (params.size() == 144);
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

// Issue #122: 80 params marked non-automatable to stay within Ableton's 64-param
// auto-populate threshold. Automatable: 16 global + 4 per-tap × 12 = 64.
TEST_CASE ("64 automatable + 80 non-automatable", "[params]")
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
    CHECK (automatable == 64);
    CHECK (nonAutomatable == 80);
}

TEST_CASE ("Global params appear before per-tap params", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();

    // First 24 params are globals; index 24 should be "Tap 01 On"
    REQUIRE (params.size() >= 25);
    CHECK (params[0]->getName (256) == "Delay Time");
    CHECK (params[1]->getName (256) == "Delay Sync");
    CHECK (params[4]->getName (256) == "Feedback");
    CHECK (params[10]->getName (256) == "Dry/Wet");
    CHECK (params[23]->getName (256) == "Global Tap Speed");

    // Per-tap: index 24 = Tap 01 On, 34 = Tap 02 On, etc.
    CHECK (params[24]->getName (256) == "Tap 01 On");
    CHECK (params[34]->getName (256) == "Tap 02 On");
    CHECK (params[44]->getName (256) == "Tap 03 On");
}

TEST_CASE ("Per-tap params are grouped by tap", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();

    // Tap 01: indices 24-33 (10 params)
    CHECK (params[24]->getName (256) == "Tap 01 On");
    CHECK (params[25]->getName (256) == "Tap 01 Azimuth");
    CHECK (params[26]->getName (256) == "Tap 01 Elevation");
    CHECK (params[27]->getName (256) == "Tap 01 Distance");
    CHECK (params[28]->getName (256) == "Tap 01 Doppler Amount");
    CHECK (params[29]->getName (256) == "Tap 01 Pitch Shift");
    CHECK (params[30]->getName (256) == "Tap 01 Trajectory Shape");
    CHECK (params[31]->getName (256) == "Tap 01 Trajectory Speed");
    CHECK (params[32]->getName (256) == "Tap 01 Trajectory Direction");
    CHECK (params[33]->getName (256) == "Tap 01 Input Channel");

    // Tap 12: indices 134-143 (last 10 params)
    CHECK (params[134]->getName (256) == "Tap 12 On");
    CHECK (params[143]->getName (256) == "Tap 12 Input Channel");
}
