# Glow Trail Visual Quality Regression

**Version:** v0.9
**Date:** 2026-03-19
**Status:** Deferred — to be revisited in a future version
**GitHub Issue:** #19 (closed as deferred)

## Summary

The glow trail (trajectory path visualization on the spatial map) lost visual quality during the v0.9 trajectory system rewrite. The trail used to look significantly better prior to the rewrite but now appears less polished.

## Symptoms

- Trail rendering is functional but visually degraded compared to pre-rewrite state
- The trail path sometimes goes the opposite direction of the animated object
- Overall glow effect is less refined — brightness, falloff, and smoothness are all reduced

## Root Cause

Unknown. The trajectory system was rewritten from scratch during v0.9 (origin-point architecture, 13 shapes, direction fixes across 22 GitHub issues). The glow trail rendering code was modified multiple times during this process. The specific change that caused the regression has not been isolated.

## Relevant Code

- `Source/PluginEditor.cpp` — `SpatialMapComponent::paint()`, specifically the trail drawing section
- Trail uses proximity-based glow: brightness increases near the animated dot position
- Elevation encoded as opacity (0.3–1.0) + line thickness (1.0–5.5px)
- Sample count: 240 points along the trajectory path

## Suggested Investigation

1. Compare current trail rendering code against the pre-trajectory-rewrite version (commit before `d5afdb4`)
2. Check if the `evaluateTrajectory()` function returns the same path geometry as the old `computeTrajectory()` when used for trail rendering
3. Review alpha/brightness curves — the old code may have had different base/peak values
4. Test with a simple Orbit trajectory to isolate rendering from shape math

## User Notes

User described the old trail as "much much better" and wants it restored. The trail direction inversion was partially fixed but the overall visual quality remains below the pre-rewrite baseline.
