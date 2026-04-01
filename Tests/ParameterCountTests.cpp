#include <catch2/catch_test_macros.hpp>
#include "../Source/PluginProcessor.h"

// Issue #68: Verify parameter count, naming, automatability, and ordering.
// These tests catch regressions from parameter layout changes.

TEST_CASE ("Parameter count is exactly 148", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();
    CHECK (params.size() == 148);
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

TEST_CASE ("144 automatable + 4 non-automatable", "[params]")
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
    CHECK (automatable == 144);
    CHECK (nonAutomatable == 4);
}

TEST_CASE ("Global params appear before per-tap params", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();

    // First 28 params are globals; index 28 should be "Tap 01 On"
    REQUIRE (params.size() >= 29);
    CHECK (params[0]->getName (256) == "Delay Time");
    CHECK (params[1]->getName (256) == "Delay Sync");
    CHECK (params[4]->getName (256) == "Feedback");
    CHECK (params[10]->getName (256) == "Dry/Wet");
    CHECK (params[27]->getName (256) == "Global Tap Speed");

    // Per-tap: index 28 = Tap 01 On, 38 = Tap 02 On, etc.
    CHECK (params[28]->getName (256) == "Tap 01 On");
    CHECK (params[38]->getName (256) == "Tap 02 On");
    CHECK (params[48]->getName (256) == "Tap 03 On");
}

TEST_CASE ("Per-tap params are grouped by tap", "[params]")
{
    auto proc = std::make_unique<OpenSpatialDelayProcessor>();
    auto& params = proc->getParameters();

    // Tap 01: indices 28-37 (10 params)
    CHECK (params[28]->getName (256) == "Tap 01 On");
    CHECK (params[29]->getName (256) == "Tap 01 Azimuth");
    CHECK (params[30]->getName (256) == "Tap 01 Elevation");
    CHECK (params[31]->getName (256) == "Tap 01 Distance");
    CHECK (params[32]->getName (256) == "Tap 01 Doppler Amount");
    CHECK (params[33]->getName (256) == "Tap 01 Pitch Shift");
    CHECK (params[34]->getName (256) == "Tap 01 Trajectory Shape");
    CHECK (params[35]->getName (256) == "Tap 01 Trajectory Speed");
    CHECK (params[36]->getName (256) == "Tap 01 Trajectory Direction");
    CHECK (params[37]->getName (256) == "Tap 01 Input Channel");

    // Tap 12: indices 138-147 (last 10 params)
    CHECK (params[138]->getName (256) == "Tap 12 On");
    CHECK (params[147]->getName (256) == "Tap 12 Input Channel");
}
