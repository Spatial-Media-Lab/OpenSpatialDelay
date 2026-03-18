#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Source/PluginProcessor.h"

using CT = OpenSpatialDelayProcessor;
using TR = CT::TrajectoryResult;
using Catch::Matchers::WithinAbs;

// Shape indices (alphabetical, matching computeTrajectory switch)
namespace TrajShape {
    constexpr int None = 0, Bounce = 1, Cross = 2, Figure8 = 3, Heart = 4,
                  Helix = 5, Infinity = 6, Line = 7, Orbit = 8, Random = 9,
                  Spiral = 10, Square = 11, Triangle = 12;
}

// ============================================================================
// Phase 1: Characterization Tests — capture current behavior before changes
// ============================================================================

TEST_CASE("None shape returns base position unchanged", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::None, 0.5f, 45.0f, 20.0f, 0.7f);
    CHECK_THAT(r.azDeg, WithinAbs(45.0f, 0.01f));
    CHECK_THAT(r.elDeg, WithinAbs(20.0f, 0.01f));
    CHECK_THAT(r.dist,  WithinAbs(0.7f, 0.001f));
    CHECK_FALSE(r.controlsAz);
    CHECK_FALSE(r.controlsEl);
    CHECK_FALSE(r.controlsDist);
}

TEST_CASE("Orbit at phase 0 starts at base azimuth", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Orbit, 0.0f, 90.0f, 10.0f, 0.5f);
    CHECK_THAT(r.azDeg, WithinAbs(90.0f, 0.01f));
    CHECK_THAT(r.elDeg, WithinAbs(10.0f, 0.01f));
    CHECK_THAT(r.dist,  WithinAbs(0.5f, 0.001f));
    CHECK(r.controlsAz);
    CHECK_FALSE(r.controlsEl);
    CHECK_FALSE(r.controlsDist);
}

TEST_CASE("Orbit at phase 0.5 is 180 degrees from base", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Orbit, 0.5f, 0.0f, 0.0f, 0.5f);
    // 0 + 360 * 0.5 = 180, wraps to 180 or -180
    CHECK(std::abs(std::abs(r.azDeg) - 180.0f) < 0.01f);
}

TEST_CASE("Orbit at phase 0.25 is +90 degrees from base", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Orbit, 0.25f, 0.0f, 0.0f, 0.5f);
    CHECK_THAT(r.azDeg, WithinAbs(90.0f, 0.01f));
}

TEST_CASE("Bounce controlsAz and controlsEl flags", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Bounce, 0.5f, 0.0f, 0.0f, 0.5f);
    CHECK(r.controlsAz);
    CHECK(r.controlsEl);
    CHECK_FALSE(r.controlsDist);
}

TEST_CASE("Bounce at phase 0.5 is at peak displacement", "[trajectory][characterization]")
{
    // tri at 0.5 = 1.0, so az = base + 90*(2*1-1) = base + 90
    auto r = CT::computeTrajectory(TrajShape::Bounce, 0.5f, 0.0f, 0.0f, 0.5f);
    CHECK_THAT(r.azDeg, WithinAbs(90.0f, 0.01f));
    CHECK_THAT(r.elDeg, WithinAbs(30.0f, 0.01f));
}

TEST_CASE("Cross controls azimuth and elevation", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Cross, 0.1f, 0.0f, 0.0f, 0.5f);
    CHECK(r.controlsAz);
    CHECK(r.controlsEl);
    CHECK_FALSE(r.controlsDist);
}

TEST_CASE("Helix at phase 0 starts at baseAz, elevation -90", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Helix, 0.0f, 45.0f, 0.0f, 0.5f);
    CHECK_THAT(r.azDeg, WithinAbs(45.0f, 0.01f));
    CHECK_THAT(r.elDeg, WithinAbs(-90.0f, 0.5f));
    CHECK(r.controlsAz);
    CHECK(r.controlsEl);
    CHECK_FALSE(r.controlsDist);
}

TEST_CASE("Helix at phase 1.0 reaches baseAz+360 (wraps), elevation +90", "[trajectory][characterization]")
{
    // phase 1.0 → easedPhase = 0.5*(1-cos(pi)) = 1.0 → elDeg = -90 + 180 = 90
    auto r = CT::computeTrajectory(TrajShape::Helix, 0.999f, 0.0f, 0.0f, 0.5f);
    CHECK(r.elDeg > 85.0f);  // near +90
}

TEST_CASE("Infinity controls azimuth and distance", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Infinity, 0.25f, 0.0f, 0.0f, 0.5f);
    CHECK(r.controlsAz);
    CHECK_FALSE(r.controlsEl);
    CHECK(r.controlsDist);
}

TEST_CASE("Infinity is origin-relative (lemniscate)", "[trajectory][characterization]")
{
    // Lemniscate at phase 0: x = a*cos(0)/(1+0) = a, y = 0 → dist ≈ sqrt(baseDist² + a²)
    // Just verify it varies with baseDist
    auto r1 = CT::computeTrajectory(TrajShape::Infinity, 0.0f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Infinity, 0.0f, 0.0f, 0.0f, 0.7f);
    CHECK(r2.dist > r1.dist);
}

TEST_CASE("Random controls all three axes", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Random, 0.3f, 0.0f, 0.0f, 0.5f);
    CHECK(r.controlsAz);
    CHECK(r.controlsEl);
    CHECK(r.controlsDist);
}

TEST_CASE("Random distance is already origin-relative", "[trajectory][characterization]")
{
    // At phase 0, all sine terms contribute offsets from baseDist
    auto r1 = CT::computeTrajectory(TrajShape::Random, 0.0f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Random, 0.0f, 0.0f, 0.0f, 0.7f);
    // Distance should differ by approximately 0.4 (the baseDist difference)
    CHECK_THAT(r2.dist - r1.dist, WithinAbs(0.4f, 0.15f));
}

TEST_CASE("Line controls azimuth and distance", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Line, 0.0f, 0.0f, 0.0f, 0.5f);
    CHECK(r.controlsAz);
    CHECK_FALSE(r.controlsEl);
    CHECK(r.controlsDist);
}

// --- Azimuth wrapping ---
TEST_CASE("Azimuth wraps to -180..+180 range", "[trajectory][characterization]")
{
    // Orbit at phase 0.75 with baseAz=90: 90 + 270 = 360 → wraps to 0
    auto r = CT::computeTrajectory(TrajShape::Orbit, 0.75f, 90.0f, 0.0f, 0.5f);
    CHECK(r.azDeg >= -180.0f);
    CHECK(r.azDeg <= 180.0f);
}

// --- Distance clamping ---
TEST_CASE("Distance clamped to 0..1", "[trajectory][characterization]")
{
    // Infinity with large baseDist could exceed 1.0 before clamping
    auto r = CT::computeTrajectory(TrajShape::Infinity, 0.125f, 0.0f, 0.0f, 0.95f);
    CHECK(r.dist >= 0.0f);
    CHECK(r.dist <= 1.0f);
}

// --- Elevation clamping ---
TEST_CASE("Elevation clamped to -90..+90", "[trajectory][characterization]")
{
    // Bounce with extreme baseEl
    auto r = CT::computeTrajectory(TrajShape::Bounce, 0.5f, 0.0f, 80.0f, 0.5f);
    CHECK(r.elDeg >= -90.0f);
    CHECK(r.elDeg <= 90.0f);
}

// --- Reverse mode ---
TEST_CASE("Reverse flips phase", "[trajectory][characterization]")
{
    auto fwd = CT::computeTrajectory(TrajShape::Orbit, 0.25f, 0.0f, 0.0f, 0.5f);
    auto rev = CT::computeTrajectory(TrajShape::Orbit, 0.75f, 0.0f, 0.0f, 0.5f, true);
    // reverse phase 0.75 → 1-0.75 = 0.25 → same as forward 0.25
    CHECK_THAT(fwd.azDeg, WithinAbs(rev.azDeg, 0.01f));
}

// --- controlsAz/El/Dist flags for all shapes ---
TEST_CASE("Control flags are correct for all shapes", "[trajectory][characterization]")
{
    struct Expected { int shape; bool az, el, dist; };
    std::vector<Expected> cases = {
        { TrajShape::None,     false, false, false },
        { TrajShape::Bounce,   true,  true,  false },
        { TrajShape::Cross,    true,  true,  false },
        { TrajShape::Figure8,  true,  true,  true  },
        { TrajShape::Heart,    true,  false, true  },
        { TrajShape::Helix,    true,  true,  false },
        { TrajShape::Infinity, true,  false, true  },
        { TrajShape::Line,     true,  false, true  },
        { TrajShape::Orbit,    true,  false, false },
        { TrajShape::Random,   true,  true,  true  },
        { TrajShape::Spiral,   true,  false, true  },
        { TrajShape::Square,   true,  true,  true  },
        { TrajShape::Triangle, true,  true,  true  },
    };

    for (auto& c : cases)
    {
        auto r = CT::computeTrajectory(c.shape, 0.3f, 30.0f, 15.0f, 0.5f);
        INFO("Shape " << c.shape);
        CHECK(r.controlsAz   == c.az);
        CHECK(r.controlsEl   == c.el);
        CHECK(r.controlsDist == c.dist);
    }
}

// ============================================================================
// Phase 2: RED Tests — Origin-Relative Distance
// These tests WILL FAIL until the distance formulas are fixed.
// Each test verifies that changing baseDist shifts the trajectory's distance.
// ============================================================================

TEST_CASE("Spiral distance oscillates around baseDist", "[trajectory][origin-relative]")
{
    // Spiral at phase 0: cos(0) = 1 → should be near baseDist + amplitude
    // At two different baseDist values, the distance should differ proportionally
    auto r1 = CT::computeTrajectory(TrajShape::Spiral, 0.0f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Spiral, 0.0f, 0.0f, 0.0f, 0.7f);
    float diff = r2.dist - r1.dist;
    // If origin-relative, diff should be ~0.4 (the baseDist difference)
    CHECK_THAT(diff, WithinAbs(0.4f, 0.15f));
}

TEST_CASE("Spiral at baseDist=0.2 phase=0.5 should be 0.2 + half of max radius", "[trajectory][origin-relative]")
{
    // Archimedean spiral: dist = baseDist + 0.75 * phase
    auto r = CT::computeTrajectory(TrajShape::Spiral, 0.5f, 0.0f, 0.0f, 0.2f);
    CHECK_THAT(r.dist, WithinAbs(0.2f + 0.75f * 0.5f, 0.05f));  // ~0.575
}

TEST_CASE("Heart distance oscillates around baseDist", "[trajectory][origin-relative]")
{
    auto r1 = CT::computeTrajectory(TrajShape::Heart, 0.0f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Heart, 0.0f, 0.0f, 0.0f, 0.7f);
    float diff = r2.dist - r1.dist;
    CHECK_THAT(diff, WithinAbs(0.4f, 0.15f));
}

TEST_CASE("Figure-8 distance scales with baseDist", "[trajectory][origin-relative]")
{
    // Figure-8 uses Cartesian → polar. At phase 0.125 (front lobe peak),
    // the shape should be offset from origin by baseDist
    auto r1 = CT::computeTrajectory(TrajShape::Figure8, 0.125f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Figure8, 0.125f, 0.0f, 0.0f, 0.7f);
    // Both should be displaced from their base distance; r2 should be further out
    CHECK(r2.dist > r1.dist);
    float diff = r2.dist - r1.dist;
    CHECK(diff > 0.15f);  // meaningful distance difference
}

TEST_CASE("Figure-8 azimuth rotates with baseAz", "[trajectory][origin-relative]")
{
    auto r1 = CT::computeTrajectory(TrajShape::Figure8, 0.25f, 0.0f, 0.0f, 0.5f);
    auto r2 = CT::computeTrajectory(TrajShape::Figure8, 0.25f, 90.0f, 0.0f, 0.5f);
    float azDiff = r2.azDeg - r1.azDeg;
    // Wrap difference
    while (azDiff > 180.0f)  azDiff -= 360.0f;
    while (azDiff < -180.0f) azDiff += 360.0f;
    CHECK_THAT(azDiff, WithinAbs(90.0f, 5.0f));
}

TEST_CASE("Square distance scales with baseDist", "[trajectory][origin-relative]")
{
    auto r1 = CT::computeTrajectory(TrajShape::Square, 0.125f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Square, 0.125f, 0.0f, 0.0f, 0.7f);
    float diff = r2.dist - r1.dist;
    CHECK_THAT(diff, WithinAbs(0.4f, 0.25f));
}

TEST_CASE("Square azimuth rotates with baseAz", "[trajectory][origin-relative]")
{
    auto r1 = CT::computeTrajectory(TrajShape::Square, 0.0f, 0.0f, 0.0f, 0.5f);
    auto r2 = CT::computeTrajectory(TrajShape::Square, 0.0f, 90.0f, 0.0f, 0.5f);
    float azDiff = r2.azDeg - r1.azDeg;
    while (azDiff > 180.0f)  azDiff -= 360.0f;
    while (azDiff < -180.0f) azDiff += 360.0f;
    CHECK_THAT(azDiff, WithinAbs(90.0f, 5.0f));
}

TEST_CASE("Triangle distance scales with baseDist", "[trajectory][origin-relative]")
{
    auto r1 = CT::computeTrajectory(TrajShape::Triangle, 0.0f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Triangle, 0.0f, 0.0f, 0.0f, 0.7f);
    float diff = r2.dist - r1.dist;
    CHECK_THAT(diff, WithinAbs(0.4f, 0.25f));
}

TEST_CASE("Triangle azimuth rotates with baseAz", "[trajectory][origin-relative]")
{
    auto r1 = CT::computeTrajectory(TrajShape::Triangle, 0.0f, 0.0f, 0.0f, 0.5f);
    auto r2 = CT::computeTrajectory(TrajShape::Triangle, 0.0f, 90.0f, 0.0f, 0.5f);
    float azDiff = r2.azDeg - r1.azDeg;
    while (azDiff > 180.0f)  azDiff -= 360.0f;
    while (azDiff < -180.0f) azDiff += 360.0f;
    CHECK_THAT(azDiff, WithinAbs(90.0f, 5.0f));
}

// ============================================================================
// GitHub Issue Tests — trajectory shape corrections
// ============================================================================

TEST_CASE("Issue #5: Figure-8 back lobe rotates opposite to front lobe", "[trajectory][issues]")
{
    // Sample sequential azimuth values in each lobe to verify rotation direction
    // Front lobe: phase 0.0 → 0.25 should show increasing azimuth (CW on map)
    auto f1 = CT::computeTrajectory(TrajShape::Figure8, 0.05f, 0.0f, 0.0f, 0.5f);
    auto f2 = CT::computeTrajectory(TrajShape::Figure8, 0.20f, 0.0f, 0.0f, 0.5f);
    float frontDelta = f2.azDeg - f1.azDeg;

    // Back lobe: phase 0.55 → 0.70 should show azimuth going opposite direction
    auto b1 = CT::computeTrajectory(TrajShape::Figure8, 0.55f, 0.0f, 0.0f, 0.5f);
    auto b2 = CT::computeTrajectory(TrajShape::Figure8, 0.70f, 0.0f, 0.0f, 0.5f);
    float backDelta = b2.azDeg - b1.azDeg;

    // Rotation directions should be opposite (product of deltas should be negative)
    CHECK(frontDelta * backDelta < 0.0f);
}

TEST_CASE("Issue #6: Heart trajectory has two upper lobes", "[trajectory][issues]")
{
    // A heart shape should have two local maxima in the front direction
    // Sample at many points and count peaks in Y (positive = front)
    int peaks = 0;
    float prevDist = 0.0f;
    bool rising = true;
    for (int s = 1; s <= 100; ++s)
    {
        float phase = (float)s / 100.0f;
        auto r = CT::computeTrajectory(TrajShape::Heart, phase, 0.0f, 0.0f, 0.0f);
        // Convert to Cartesian Y (front direction) using azimuth and distance
        float azRad = r.azDeg * 3.14159265f / 180.0f;
        float y = r.dist * std::cos(azRad);
        if (s > 1)
        {
            if (rising && y < prevDist) { peaks++; rising = false; }
            if (!rising && y > prevDist) { rising = true; }
        }
        prevDist = y;
    }
    CHECK(peaks >= 2);  // two distinct bumps at the top
}

TEST_CASE("Issue #7: Infinity trajectory crosses through origin", "[trajectory][issues]")
{
    // A proper lemniscate crosses through the origin at the center
    // At phase 0.25 or 0.75, distance should be near the origin point
    auto mid1 = CT::computeTrajectory(TrajShape::Infinity, 0.25f, 0.0f, 0.0f, 0.0f);
    auto mid2 = CT::computeTrajectory(TrajShape::Infinity, 0.75f, 0.0f, 0.0f, 0.0f);
    // At crossing points, distance should be very small (near origin)
    CHECK(mid1.dist < 0.15f);
    CHECK(mid2.dist < 0.15f);
}

TEST_CASE("Issue #8: Spiral distance monotonically increases", "[trajectory][issues]")
{
    // Archimedean spiral: distance should increase from near 0 to near max over one cycle
    float prevDist = -1.0f;
    bool monotonic = true;
    for (int s = 0; s <= 20; ++s)
    {
        float phase = (float)s / 20.0f;
        auto r = CT::computeTrajectory(TrajShape::Spiral, phase, 0.0f, 0.0f, 0.0f);
        if (r.dist < prevDist - 0.01f) monotonic = false;
        prevDist = r.dist;
    }
    CHECK(monotonic);
    // Should reach near the edge
    auto end = CT::computeTrajectory(TrajShape::Spiral, 0.99f, 0.0f, 0.0f, 0.0f);
    CHECK(end.dist > 0.5f);
}

TEST_CASE("Issue #10: Line trajectory reaches amplitude 0.75", "[trajectory][issues]")
{
    // Line should sweep ±0.75 from origin, reaching distance 0.75 at extremes
    float maxDist = 0.0f;
    for (int s = 0; s <= 100; ++s)
    {
        float phase = (float)s / 100.0f;
        auto r = CT::computeTrajectory(TrajShape::Line, phase, 0.0f, 0.0f, 0.0f);
        if (r.dist > maxDist) maxDist = r.dist;
    }
    CHECK(maxDist > 0.65f);  // should reach ~0.75
}
