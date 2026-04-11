# OpenSpatialDelay Version Registry

**Do not reuse version numbers.** Every testable build must use a unique `vX.Y.Z / OXYZ` identifier per the versioned-build mandate in `CLAUDE.md`.

Registry reset: 2026-04-02, collapsed after issue #77.

## Consumed versions

| Version | Plugin Code | Purpose | Commit |
|---------|-------------|---------|--------|
| v1.0.0 | O100 | baseline (includes issues #76 + #77) | 3ef9c79 |
| v1.0.1 | O101 | fix dry signal attenuation at 0% wet (issue #97) | a0f39c5 |
| v1.0.2 | O102 | call updateHostDisplay() on config changes (issue #94) | dbf19ce |
| v1.0.3 | O103 | fix direction toggle for Bounce, Line, Random (issue #100) | 9dd6266 |
| v1.0.5 | O105 | reset phase vocoder on preset change chirp (issue #99) | a9df52a |
| v1.0.6 | O106 | transport fade-in to prevent scrub/seek click (issue #103) | 7901a0a |
| v1.0.7 | O107 | 4-layer tape wobble emulation (issue #92) | 893acb2 |
| v1.0.8 | O108 | fix Ableton crash + automation reset (issue #122) | 0d3cc53 |
| v1.0.9 | O109 | re-sign bundles after plist patching for Ableton VST3 visibility (issue #120) | — |
| v1.0.10 | O110 | reduce automatable params to 64 for Ableton auto-populate (issue #122) | 4adfd1d |
| v1.0.14 | O114 | shared LookAndFeel + visibility throttle for multi-instance crash (issue #131) | — |
| v1.0.17 | O117 | shared FFT cache for multi-instance vDSP stability (issue #131) | e8ee6ec |
| v1.0.20 | O120 | default Input to Stereo on stereo tracks, disable on mono (issue #155) | 376b5fe |

## Next available

**v1.0.21 / O121**
