#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Source/PluginProcessor.h"

using CT = OpenSpatialDelayProcessor;
using TR = CT::TrajectoryResult;
using Catch::Matchers::WithinAbs;

// Shape indices (alphabetical, matching computeTrajectory switch)
namespace TrajShape {
    constexpr int None = 0, Bounce = 1, Circle = 2, Cross = 3, Figure8 = 4,
                  Heart = 5, Helix = 6, Infinity = 7, Line = 8, Orbit = 9,
                  Random = 10, Spiral = 11, Square = 12, Triangle = 13;
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
    // tri at 0.5 = 1.0, so az = base - 90*(2*1-1) = base - 90
    auto r = CT::computeTrajectory(TrajShape::Bounce, 0.5f, 0.0f, 0.0f, 0.5f);
    CHECK_THAT(r.azDeg, WithinAbs(-90.0f, 0.01f));
    CHECK_THAT(r.elDeg, WithinAbs(-30.0f, 0.01f));
}

TEST_CASE("Cross controls azimuth and elevation", "[trajectory][characterization]")
{
    auto r = CT::computeTrajectory(TrajShape::Cross, 0.1f, 0.0f, 0.0f, 0.5f);
    CHECK(r.controlsAz);
    CHECK(r.controlsEl);
    CHECK_FALSE(r.controlsDist);
}

TEST_CASE("Helix at phase 0 starts at baseAz, elevation +90", "[trajectory][characterization]")
{
    // #14: Helix now starts at top (elevation +90) at phase 0
    auto r = CT::computeTrajectory(TrajShape::Helix, 0.0f, 45.0f, 0.0f, 0.5f);
    CHECK_THAT(r.azDeg, WithinAbs(45.0f, 0.01f));
    CHECK_THAT(r.elDeg, WithinAbs(90.0f, 0.5f));
    CHECK(r.controlsAz);
    CHECK(r.controlsEl);
    CHECK_FALSE(r.controlsDist);
}

TEST_CASE("Helix at phase 1.0 reaches elevation -90", "[trajectory][characterization]")
{
    // #14: Helix now ends at bottom (elevation -90) at phase 1.0
    auto r = CT::computeTrajectory(TrajShape::Helix, 0.999f, 0.0f, 0.0f, 0.5f);
    CHECK(r.elDeg < -85.0f);  // near -90
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
    // With a = distScale, the lemniscate peak always reaches ~1.0 regardless of baseDist
    // Verify it stays within valid range and reaches near 1.0
    auto r1 = CT::computeTrajectory(TrajShape::Infinity, 0.0f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Infinity, 0.0f, 0.0f, 0.0f, 0.7f);
    CHECK(r1.dist > 0.5f);
    CHECK(r2.dist > 0.5f);
    CHECK(r1.dist <= 1.0f);
    CHECK(r2.dist <= 1.0f);
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

TEST_CASE("Spiral at phase 0 has larger radius than phase near 1 (outward spiral)", "[trajectory][origin-relative]")
{
    // Spiral uses rLocal = maxRadius*(1-phase), so phase=0 has max local radius
    // and phase~1 converges to origin. Verify outward-to-inward progression.
    auto rStart = CT::computeTrajectory(TrajShape::Spiral, 0.0f, 0.0f, 0.0f, 0.0f);
    auto rEnd   = CT::computeTrajectory(TrajShape::Spiral, 0.95f, 0.0f, 0.0f, 0.0f);
    CHECK(rStart.dist > rEnd.dist);   // phase 0 is further from center than phase ~1
    CHECK(rStart.dist > 0.5f);        // at baseDist=0, outer edge is well past center
}

TEST_CASE("Spiral is centered on origin (Cartesian approach)", "[trajectory][origin-relative]")
{
    // #15: At baseDist=0, spiral starts at outer edge (phase=0) and ends at center (phase=1)
    auto rStart = CT::computeTrajectory(TrajShape::Spiral, 0.0f, 0.0f, 0.0f, 0.0f);
    auto rEnd   = CT::computeTrajectory(TrajShape::Spiral, 0.99f, 0.0f, 0.0f, 0.0f);
    CHECK(rStart.dist > 0.5f);  // starts at outer edge
    CHECK_THAT(rEnd.dist, WithinAbs(0.0f, 0.02f));  // ends at center
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
    // With full-range amplitude (loopR=0.5*distScale), the far lobe peak
    // reaches ~1.0 at any baseDist. Verify both stay in valid range.
    auto r1 = CT::computeTrajectory(TrajShape::Figure8, 0.125f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Figure8, 0.125f, 0.0f, 0.0f, 0.7f);
    CHECK(r1.dist > 0.3f);
    CHECK(r2.dist > 0.3f);
    CHECK(r1.dist <= 1.01f);
    CHECK(r2.dist <= 1.01f);
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
    // With full-range amplitude, corners reach ~1.0 at any baseDist
    auto r1 = CT::computeTrajectory(TrajShape::Square, 0.125f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Square, 0.125f, 0.0f, 0.0f, 0.7f);
    CHECK(r1.dist > 0.3f);
    CHECK(r2.dist > 0.3f);
    CHECK(r1.dist <= 1.5f);  // corners can slightly exceed 1.0 due to geometry
    CHECK(r2.dist <= 1.5f);
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
    // With circumR = distScale, vertices reach baseDist + circumR ≈ 1.0
    auto r1 = CT::computeTrajectory(TrajShape::Triangle, 0.0f, 0.0f, 0.0f, 0.3f);
    auto r2 = CT::computeTrajectory(TrajShape::Triangle, 0.0f, 0.0f, 0.0f, 0.7f);
    CHECK(r1.dist > 0.5f);
    CHECK(r2.dist > 0.5f);
    CHECK_THAT(r1.dist, WithinAbs(1.0f, 0.1f));
    CHECK_THAT(r2.dist, WithinAbs(1.0f, 0.1f));
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

TEST_CASE("Issue #8: Spiral extends outward from origin", "[trajectory][issues]")
{
    // #15: Spiral now starts at outer edge (phase=0) and spirals inward to center (phase=1)
    // At baseDist=0, phase=0 should be at max radius, phase=1 near center
    auto start = CT::computeTrajectory(TrajShape::Spiral, 0.01f, 0.0f, 0.0f, 0.0f);
    auto end   = CT::computeTrajectory(TrajShape::Spiral, 0.99f, 0.0f, 0.0f, 0.0f);
    CHECK(start.dist > 0.5f);  // starts near outer edge
    CHECK_THAT(end.dist, WithinAbs(0.0f, 0.02f));  // ends near center
    // Verify the spiral at half phase has intermediate extent
    auto mid = CT::computeTrajectory(TrajShape::Spiral, 0.5f, 0.0f, 0.0f, 0.0f);
    CHECK(mid.dist > 0.1f);
}

TEST_CASE("Issue #10: Line trajectory reaches full amplitude at baseDist=0", "[trajectory][issues]")
{
    // Line should sweep ±1.0 from origin at baseDist=0, reaching distance 1.0
    float maxDist = 0.0f;
    for (int s = 0; s <= 100; ++s)
    {
        float phase = (float)s / 100.0f;
        auto r = CT::computeTrajectory(TrajShape::Line, phase, 0.0f, 0.0f, 0.0f);
        if (r.dist > maxDist) maxDist = r.dist;
    }
    CHECK(maxDist > 0.9f);  // should reach ~1.0
}

// ============================================================================
// Issue #3: Distance Scaling — amplitude scales inversely with baseDist
// ============================================================================

TEST_CASE("Issue #3: Line at baseDist=0 has full amplitude", "[trajectory][distance-scaling]")
{
    // distScale = 1.0, amplitude = 1.0 → max dist ≈ 1.0
    auto r = CT::computeTrajectory(TrajShape::Line, 0.0f, 0.0f, 0.0f, 0.0f);
    CHECK(r.dist > 0.9f);
}

TEST_CASE("Issue #3: Line at baseDist=1.0 collapses to origin", "[trajectory][distance-scaling]")
{
    // distScale = 0.0, amplitude = 0 → dist ≈ baseDist = 1.0
    auto r = CT::computeTrajectory(TrajShape::Line, 0.0f, 0.0f, 0.0f, 1.0f);
    CHECK_THAT(r.dist, WithinAbs(1.0f, 0.05f));
}

TEST_CASE("Issue #3: Line at baseDist=0.5 has half amplitude", "[trajectory][distance-scaling]")
{
    // distScale = 0.5, amplitude = 0.5. Line moves along rotated axis while
    // baseDist offsets perpendicular — max dist = sqrt(0.5² + 0.5²) ≈ 0.707
    float maxDist = 0.0f;
    for (int s = 0; s <= 100; ++s)
    {
        auto r = CT::computeTrajectory(TrajShape::Line, (float)s / 100.0f, 0.0f, 0.0f, 0.5f);
        if (r.dist > maxDist) maxDist = r.dist;
    }
    CHECK(maxDist > 0.6f);
    CHECK(maxDist <= 1.01f);
}

TEST_CASE("Issue #3: Spiral maxRadius scales inversely", "[trajectory][distance-scaling]")
{
    // #15: Spiral starts at outer edge (phase=0) and spirals inward.
    // At baseDist=0, phase=0 starts at max radius
    auto r0 = CT::computeTrajectory(TrajShape::Spiral, 0.01f, 0.0f, 0.0f, 0.0f);
    CHECK(r0.dist > 0.5f);

    // At baseDist=1.0, maxRadius = 0 → spiral collapses, dist ≈ 1.0
    auto r1 = CT::computeTrajectory(TrajShape::Spiral, 0.01f, 0.0f, 0.0f, 1.0f);
    CHECK_THAT(r1.dist, WithinAbs(1.0f, 0.05f));
}

TEST_CASE("Issue #3: Square collapses at baseDist=1.0", "[trajectory][distance-scaling]")
{
    for (float phase : { 0.0f, 0.25f, 0.5f, 0.75f })
    {
        auto r = CT::computeTrajectory(TrajShape::Square, phase, 0.0f, 0.0f, 1.0f);
        CHECK_THAT(r.dist, WithinAbs(1.0f, 0.05f));
    }
}

TEST_CASE("Issue #3: Triangle collapses at baseDist=1.0", "[trajectory][distance-scaling]")
{
    for (float phase : { 0.0f, 0.33f, 0.67f })
    {
        auto r = CT::computeTrajectory(TrajShape::Triangle, phase, 0.0f, 0.0f, 1.0f);
        CHECK_THAT(r.dist, WithinAbs(1.0f, 0.05f));
    }
}

// ============================================================================
// Issue #100: Direction toggle must affect Bounce, Line, and Random
// ============================================================================

TEST_CASE("Issue #100: Bounce reverse flips the trajectory diagonally", "[trajectory][direction]")
{
    // Reverse mirrors the azimuth offset so the az–el relationship flips.
    // At the same phase, azimuth should be negated relative to base while
    // elevation stays the same.
    float baseAz = 0.0f;
    auto fwd = CT::computeTrajectory(TrajShape::Bounce, 0.1f, baseAz, 0.0f, 0.5f, false);
    auto rev = CT::computeTrajectory(TrajShape::Bounce, 0.1f, baseAz, 0.0f, 0.5f, true);
    // Azimuth offsets should be opposite signs
    float fwdAzOff = fwd.azDeg - baseAz;
    float revAzOff = rev.azDeg - baseAz;
    CHECK_THAT(revAzOff, WithinAbs(-fwdAzOff, 0.01f));
    // Elevation should be identical (same triangle value)
    CHECK_THAT(rev.elDeg, WithinAbs(fwd.elDeg, 0.01f));
}

TEST_CASE("Issue #100: Line reverse produces different position at same phase", "[trajectory][direction]")
{
    // Line uses cos which is even, so phase→1-phase was a no-op.
    // Fix adds a half-period offset when reversed.
    auto fwd = CT::computeTrajectory(TrajShape::Line, 0.1f, 0.0f, 0.0f, 0.5f, false);
    auto rev = CT::computeTrajectory(TrajShape::Line, 0.1f, 0.0f, 0.0f, 0.5f, true);
    CHECK(std::abs(fwd.azDeg - rev.azDeg) > 1.0f);
}

TEST_CASE("Issue #100: Line reverse is half-period offset", "[trajectory][direction]")
{
    // Reverse at phase p should equal forward at phase p+0.5 (mod 1)
    auto rev  = CT::computeTrajectory(TrajShape::Line, 0.2f, 45.0f, 0.0f, 0.3f, true);
    auto fwd5 = CT::computeTrajectory(TrajShape::Line, 0.7f, 45.0f, 0.0f, 0.3f, false);
    CHECK_THAT(rev.azDeg, WithinAbs(fwd5.azDeg, 0.5f));
    CHECK_THAT(rev.dist,  WithinAbs(fwd5.dist, 0.01f));
}

TEST_CASE("Issue #100: Random fallback reverse produces different output", "[trajectory][direction]")
{
    // computeTrajectory's Random fallback uses sin (not symmetric),
    // so the global phase=1-phase already works here.
    auto fwd = CT::computeTrajectory(TrajShape::Random, 0.3f, 0.0f, 0.0f, 0.5f, false);
    auto rev = CT::computeTrajectory(TrajShape::Random, 0.3f, 0.0f, 0.0f, 0.5f, true);
    CHECK(std::abs(fwd.azDeg - rev.azDeg) > 1.0f);
}

TEST_CASE("Issue #3: Shapes with no distance modulation are unaffected", "[trajectory][distance-scaling]")
{
    // Bounce, Cross, Helix, Orbit — distance stays at baseDist regardless
    for (int shape : { TrajShape::Bounce, TrajShape::Cross, TrajShape::Helix, TrajShape::Orbit })
    {
        auto r = CT::computeTrajectory(shape, 0.5f, 45.0f, 20.0f, 0.7f);
        INFO("Shape " << shape);
        CHECK_THAT(r.dist, WithinAbs(0.7f, 0.01f));
    }
}
