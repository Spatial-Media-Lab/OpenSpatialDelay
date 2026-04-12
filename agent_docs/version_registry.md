# Version Registry

Single source of truth for version numbers. Never reuse a version number.

## Build command

```bash
bash scripts/build_version.sh <commit-hash> v1.0.Z O10Z
```

The `O10Z` code is a short identifier used in the plugin display name (e.g., O130 = "OpenSpatialDelay v1.0.30").

## Consumed versions

Registry collapsed to v1.0.0 on 2026-04-02 after issue #77.

| Version | Code | Description | Commit |
|---------|------|-------------|--------|
| v1.0.0 | O100 | Baseline (includes issues #76 + #77) | 3ef9c79 |
| v1.0.1 | O101 | Fix dry signal attenuation at 0% wet (issue #97) | a0f39c5 |
| v1.0.2 | O102 | Call updateHostDisplay() on config changes (issue #94) | dbf19ce |
| v1.0.3 | O103 | Fix direction toggle for Bounce, Line, Random (issue #100) | 9dd6266 |
| v1.0.5 | O105 | Reset phase vocoder on preset change chirp (issue #99) | a9df52a |
| v1.0.6 | O106 | Transport fade-in to prevent scrub/seek click (issue #103) | 7901a0a |
| v1.0.7 | O107 | 4-layer tape wobble emulation (issue #92) | 893acb2 |
| v1.0.8 | O108 | Fix Ableton crash + automation reset (issue #122) | 0d3cc53 |
| v1.0.9 | O109 | Re-sign bundles after plist patching for Ableton VST3 visibility (issue #120) | — |
| v1.0.10 | O110 | Reduce automatable params to 64 for Ableton auto-populate (issue #122) | 4adfd1d |
| v1.0.14 | O114 | Shared LookAndFeel + visibility throttle for multi-instance crash (issue #131) | — |
| v1.0.17 | O117 | Shared FFT cache for multi-instance vDSP stability (issue #131) | e8ee6ec |
| v1.0.20 | O120 | Default Input to Stereo on stereo tracks, disable on mono (issue #155) | 376b5fe |
| v1.0.21–v1.0.29 | O121–O129 | Consumed by E15b double-undo investigation (issue #182) | various |
| v1.0.30 | O130 | Current fix version for issue #182 | — |

## Next available

**v1.0.31 / O131**
