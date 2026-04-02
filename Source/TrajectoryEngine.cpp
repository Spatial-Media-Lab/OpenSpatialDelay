#include "TrajectoryEngine.h"

//==============================================================================
void TrajectoryEngine::tick (int t, const ObjectInput& input, float dt)
{
    if (input.shape == 0)
    {
        active_[t].store (false, std::memory_order_relaxed);
        prevShape_[t] = 0;
        return;
    }

    if (input.oscOverride)
        return;

    // Detect shape change: reset phase
    if (prevShape_[t] == 0 || prevShape_[t] != input.shape)
    {
        phase_[t] = 0.0f;
        noise_[t].initialized = false;
        randomTime_[t] = 0.0f;
        baseAzimuth_[t]   = input.originAz;
        baseElevation_[t] = input.originEl;
        baseDistance_[t]   = input.originDist;
    }
    prevShape_[t] = input.shape;

    // Advance phase — Spiral (11), Random (10), Square (12) run at half speed
    float effectiveSpeed = (input.shape == 10 || input.shape == 11 || input.shape == 12)
                           ? input.speed * 0.5f : input.speed;
    phase_[t] += effectiveSpeed * dt;
    if (phase_[t] >= 1.0f)
        phase_[t] -= std::floor (phase_[t]);

    if (input.shape == 10)  // Random — multi-sine noise
    {
        auto& rn = noise_[t];
        if (! rn.initialized)
        {
            auto randSign = [&]() { return rng_.nextBool() ? 1.0f : -1.0f; };
            float azAmps[]   = { 50.0f, 27.0f, 19.0f, 11.0f };
            float elAmps[]   = { 30.0f, 16.0f, 12.0f, 8.0f };
            float distAmps[] = { 0.50f, 0.30f, 0.20f };
            for (int k = 0; k < 4; ++k)
            {
                rn.freqAz[k]  = 0.15f + rng_.nextFloat() * 1.75f;
                rn.phaseAz[k] = rng_.nextFloat() * juce::MathConstants<float>::twoPi;
                rn.ampAz[k]   = azAmps[k] * randSign();
                rn.freqEl[k]  = 0.15f + rng_.nextFloat() * 1.75f;
                rn.phaseEl[k] = rng_.nextFloat() * juce::MathConstants<float>::twoPi;
                rn.ampEl[k]   = elAmps[k] * randSign();
            }
            for (int k = 0; k < 3; ++k)
            {
                rn.freqDist[k]  = 0.15f + rng_.nextFloat() * 1.25f;
                rn.phaseDist[k] = rng_.nextFloat() * juce::MathConstants<float>::twoPi;
                rn.ampDist[k]   = distAmps[k] * randSign();
            }
            rn.initialized = true;
        }

        randomTime_[t] += (input.reverse ? -1.0f : 1.0f) * effectiveSpeed * dt;
        float p = randomTime_[t] * juce::MathConstants<float>::twoPi;
        float az = 0.0f, el = 0.0f, dist = 0.0f;
        for (int k = 0; k < 4; ++k)
        {
            az += rn.ampAz[k] * std::sin (p * rn.freqAz[k] + rn.phaseAz[k]);
            el += rn.ampEl[k] * std::sin (p * rn.freqEl[k] + rn.phaseEl[k]);
        }
        for (int k = 0; k < 3; ++k)
            dist += rn.ampDist[k] * std::sin (p * rn.freqDist[k] + rn.phaseDist[k]);

        finalAz_[t] = input.originAz + az;
        finalEl_[t] = juce::jlimit (-90.0f, 90.0f, input.originEl + el);
        float distScaleR = 1.0f - input.originDist;
        finalDist_[t] = juce::jlimit (0.0f, 1.0f, input.originDist + dist * distScaleR);

        while (finalAz_[t] > 180.0f)  finalAz_[t] -= 360.0f;
        while (finalAz_[t] < -180.0f) finalAz_[t] += 360.0f;
    }
    else
    {
        auto result = computeTrajectory (input.shape, phase_[t],
                                          input.originAz, input.originEl, input.originDist,
                                          input.reverse);
        finalAz_[t]   = result.azDeg;
        finalEl_[t]   = result.elDeg;
        finalDist_[t] = result.dist;
    }

    active_[t].store (true, std::memory_order_relaxed);

    baseAzimuth_[t]   = input.originAz;
    baseElevation_[t] = input.originEl;
    baseDistance_[t]   = input.originDist;
}

//==============================================================================
TrajectoryEngine::TrajectoryState TrajectoryEngine::getState (int objectIndex) const
{
    TrajectoryState ts;
    if (objectIndex < 0 || objectIndex >= kMaxObjects)
        return ts;

    ts.originAzDeg = baseAzimuth_[objectIndex];
    ts.originElDeg = baseElevation_[objectIndex];
    ts.originDist  = baseDistance_[objectIndex];
    ts.shape       = prevShape_[objectIndex];
    ts.phase       = phase_[objectIndex];
    ts.reverse     = false;  // caller must check param separately
    ts.randomTime  = randomTime_[objectIndex];
    return ts;
}

//==============================================================================
TrajectoryEngine::RandomPosition
TrajectoryEngine::evaluateRandomNoise (int objectIndex, float time) const
{
    RandomPosition rp { 0.0f, 0.0f, 0.0f };
    if (objectIndex < 0 || objectIndex >= kMaxObjects || ! noise_[objectIndex].initialized)
        return rp;

    auto& rn = noise_[objectIndex];
    float p = time * juce::MathConstants<float>::twoPi;

    for (int k = 0; k < 4; ++k)
    {
        rp.azDeg += rn.ampAz[k] * std::sin (p * rn.freqAz[k] + rn.phaseAz[k]);
        rp.elDeg += rn.ampEl[k] * std::sin (p * rn.freqEl[k] + rn.phaseEl[k]);
    }
    for (int k = 0; k < 3; ++k)
        rp.dist += rn.ampDist[k] * std::sin (p * rn.freqDist[k] + rn.phaseDist[k]);

    return rp;
}

//==============================================================================
void TrajectoryEngine::reset (int i, float azDeg, float elDeg, float dist)
{
    active_[i].store (false, std::memory_order_relaxed);
    finalAz_[i]   = azDeg;
    finalEl_[i]   = elDeg;
    finalDist_[i] = dist;
    phase_[i]     = 0.0f;
    prevShape_[i] = 0;
    randomTime_[i] = 0.0f;
    noise_[i].initialized = false;
    baseAzimuth_[i]   = azDeg;
    baseElevation_[i] = elDeg;
    baseDistance_[i]   = dist;
}

void TrajectoryEngine::resetAll()
{
    for (int i = 0; i < kMaxObjects; ++i)
        reset (i, 0.0f, 0.0f, 0.5f);
}

//==============================================================================
// Trajectory shape computation — pure function, no side effects
//==============================================================================
TrajectoryEngine::TrajectoryResult
TrajectoryEngine::computeTrajectory (int shape, float phase,
                                      float baseAz, float baseEl, float baseDist,
                                      bool reverse)
{
    if (reverse)
        phase = 1.0f - phase;

    TrajectoryResult r;
    r.controlsAz = false;
    r.controlsEl = false;
    r.controlsDist = false;

    const float distScale = 1.0f - baseDist;

    switch (shape)
    {
        case 1: // Bounce
        {
            float tri = 1.0f - std::abs (2.0f * phase - 1.0f);
            // Flip the trajectory diagonally: negate azimuth offset so
            // the az–elevation relationship mirrors (issue #100)
            float azSign = reverse ? -1.0f : 1.0f;
            r.azDeg = baseAz - azSign * 90.0f * (2.0f * tri - 1.0f);
            r.elDeg = baseEl - 30.0f * (2.0f * tri - 1.0f);
            r.dist  = baseDist;
            r.controlsAz = r.controlsEl = true;
            break;
        }

        case 2: // Circle
        {
            const float circleR = 1.0f * distScale;
            float p = phase * juce::MathConstants<float>::twoPi;
            float localX = circleR * std::sin (p);
            float localY = circleR * std::cos (p);
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 3: // Cross
        {
            float p = phase * 4.0f;
            int segment = juce::jlimit (0, 3, static_cast<int> (p));
            float t = p - static_cast<float> (segment);
            float tri = 1.0f - std::abs (2.0f * t - 1.0f);
            if (segment == 0)      { r.azDeg = baseAz;                       r.elDeg = baseEl + 60.0f * tri; }
            else if (segment == 1) { r.azDeg = baseAz + 90.0f * tri;         r.elDeg = baseEl; }
            else if (segment == 2) { r.azDeg = baseAz;                       r.elDeg = baseEl - 60.0f * tri; }
            else                   { r.azDeg = baseAz - 90.0f * tri;         r.elDeg = baseEl; }
            r.dist = baseDist;
            r.controlsAz = r.controlsEl = true;
            break;
        }

        case 4: // Figure-8
        {
            const float loopR = 0.5f * distScale;
            constexpr float halfPi = juce::MathConstants<float>::halfPi;
            constexpr float twoPi  = juce::MathConstants<float>::twoPi;
            float fig8Phase = 1.0f - phase;
            float localX, localY;
            if (fig8Phase < 0.5f)
            {
                float t = fig8Phase * 2.0f;
                float theta = -halfPi + twoPi * t;
                localX = loopR * std::cos (theta);
                localY = loopR + loopR * std::sin (theta);
            }
            else
            {
                float t = (fig8Phase - 0.5f) * 2.0f;
                float theta = halfPi - twoPi * t;
                localX = loopR * std::cos (theta);
                localY = -loopR + loopR * std::sin (theta);
            }
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float rotX = localX * cosA + localY * sinA;
            float rotY = -localX * sinA + localY * cosA;
            float mapX = baseCx + rotX;
            float mapY = baseCy + rotY;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsEl = r.controlsDist = true;
            break;
        }

        case 5: // Heart
        {
            float p = phase * juce::MathConstants<float>::twoPi;
            float sinP = std::sin (p);
            float hx = 16.0f * sinP * sinP * sinP;
            float hy = 13.0f * std::cos (p) - 5.0f * std::cos (2.0f * p)
                      - 2.0f * std::cos (3.0f * p) - std::cos (4.0f * p);
            const float scale = (1.0f / 17.0f) * distScale;
            float localX = hx * scale;
            float localY = hy * scale;
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 6: // Helix
        {
            r.azDeg = baseAz + 360.0f * phase;
            float easedPhase = 0.5f * (1.0f - std::cos (phase * juce::MathConstants<float>::pi));
            r.elDeg = 90.0f - 180.0f * easedPhase;
            r.dist  = baseDist;
            r.controlsAz = r.controlsEl = true;
            break;
        }

        case 7: // Infinity (Lemniscate)
        {
            float p = phase * juce::MathConstants<float>::twoPi;
            const float a = 1.0f * distScale;
            float sinP = std::sin (p);
            float cosP = std::cos (p);
            float denom = 1.0f + sinP * sinP;
            float localX = a * cosP / denom;
            float localY = a * sinP * cosP / denom;
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 8: // Line
        {
            // phase=1-phase is a no-op for cos (even function);
            // offset by half-period to actually reverse direction
            if (reverse)
                phase = std::fmod (phase + 0.5f, 1.0f);
            const float amplitude = 1.0f * distScale;
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float p = phase * juce::MathConstants<float>::twoPi;
            float localX = amplitude * std::cos (p);
            float localY = 0.0f;
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 9: // Orbit
            r.azDeg = baseAz + 360.0f * phase;
            r.elDeg = baseEl;
            r.dist  = baseDist;
            r.controlsAz = true;
            break;

        case 10: // Random (fallback — actual random noise handled in tick())
        {
            float p = phase * juce::MathConstants<float>::twoPi;
            r.azDeg = baseAz + 50.0f * std::sin (p * 0.5f)
                             + 35.0f * std::sin (p * 1.3592f + 0.7f)
                             + 20.0f * std::sin (p * 2.3346f + 2.1f)
                             + 12.0f * std::sin (p * 3.6946f + 4.3f);
            r.elDeg = baseEl + 30.0f * std::sin (p * 0.7071f + 1.1f)
                             + 20.0f * std::sin (p * 1.5708f + 3.5f)
                             + 12.0f * std::sin (p * 2.9299f + 0.3f)
                             +  8.0f * std::sin (p * 4.2699f + 5.7f);
            r.dist  = juce::jlimit (0.0f, 1.0f,
                                    baseDist + 0.50f * distScale * std::sin (p * 0.8661f + 2.3f)
                                             + 0.30f * distScale * std::sin (p * 1.9365f + 4.9f)
                                             + 0.20f * distScale * std::sin (p * 3.1416f + 1.6f));
            r.controlsAz = r.controlsEl = r.controlsDist = true;
            break;
        }

        case 11: // Spiral
        {
            constexpr float numTurns = 1.75f;
            const float maxRadius = 1.0f * distScale;
            float theta = juce::MathConstants<float>::twoPi * numTurns * (1.0f - phase);
            float rLocal = maxRadius * (1.0f - phase);
            float localX = rLocal * std::cos (theta);
            float localY = rLocal * std::sin (theta);
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsDist = true;
            break;
        }

        case 12: // Square
        {
            const float halfSide = 0.7071f * distScale;
            const float lcx[4] = { -halfSide,  halfSide,  halfSide, -halfSide };
            const float lcy[4] = {  halfSide,  halfSide, -halfSide, -halfSide };
            float p = phase * 4.0f;
            int edge = juce::jlimit (0, 3, static_cast<int> (p));
            float t = p - static_cast<float> (edge);
            float ease = 0.5f * (1.0f - std::cos (t * juce::MathConstants<float>::pi));
            int next = (edge + 1) & 3;
            float localX = lcx[edge] + (lcx[next] - lcx[edge]) * ease;
            float localY = lcy[edge] + (lcy[next] - lcy[edge]) * ease;
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsEl = r.controlsDist = true;
            break;
        }

        case 13: // Triangle
        {
            const float circumR = 1.0f * distScale;
            const float lvx[3] = { 0.0f,  circumR * 0.8660254f, -circumR * 0.8660254f };
            const float lvy[3] = { circumR, -circumR * 0.5f, -circumR * 0.5f };
            float p = phase * 3.0f;
            int side = juce::jlimit (0, 2, static_cast<int> (p));
            float t = p - static_cast<float> (side);
            float ease = 0.5f * (1.0f - std::cos (t * juce::MathConstants<float>::pi));
            int next = (side + 1) % 3;
            float localX = lvx[side] + (lvx[next] - lvx[side]) * ease;
            float localY = lvy[side] + (lvy[next] - lvy[side]) * ease;
            float baseAzRad = juce::degreesToRadians (baseAz);
            float baseCx = baseDist * std::sin (baseAzRad);
            float baseCy = baseDist * std::cos (baseAzRad);
            float cosA = std::cos (baseAzRad);
            float sinA = std::sin (baseAzRad);
            float mapX = baseCx + localX * cosA + localY * sinA;
            float mapY = baseCy - localX * sinA + localY * cosA;
            r.dist  = std::sqrt (mapX * mapX + mapY * mapY);
            r.azDeg = juce::radiansToDegrees (std::atan2 (mapX, mapY));
            r.elDeg = baseEl;
            r.controlsAz = r.controlsEl = r.controlsDist = true;
            break;
        }

        default:
            r.azDeg = baseAz;
            r.elDeg = baseEl;
            r.dist  = baseDist;
            break;
    }

    while (r.azDeg > 180.0f)  r.azDeg -= 360.0f;
    while (r.azDeg < -180.0f) r.azDeg += 360.0f;
    r.elDeg = juce::jlimit (-90.0f, 90.0f, r.elDeg);
    r.dist  = juce::jlimit (0.0f, 1.0f, r.dist);

    return r;
}
