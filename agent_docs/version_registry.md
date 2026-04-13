# Version Registry

Single source of truth for version numbers. Never reuse a version number.

## Build command

```bash
bash scripts/build_version.sh <commit-hash> v1.0.Z O10Z
```

The `O10Z` code is a short identifier used in the plugin display name (e.g., O101 = "OpenSpatialDelay v1.0.1").

## Current release

| Version | Code | Description | Commit |
|---------|------|-------------|--------|
| v1.0.0 | O100 | **Release v1.0.0** — all fixes issues #76–#200 squashed into one commit | 768c248 |

## Post-release patches

| Version | Code | Description | Commit |
|---------|------|-------------|--------|
| v1.0.1 | O101 | Fix Immersive HRTF bass boost — correct low-shelf filter profile index (issue #198) | 70cc5d8 |
| v1.0.2 | O102 | Filter graph UX: EQ8-style grid lines + extended nipple drag range (issue #196) | 14e8ac3 |

## Next available

**v1.0.3 / O103** (use for any post-release patches)

## Archive — internal patch builds (v1.0.1–v1.0.36)

These were iterative development builds squashed into the v1.0.0 release commit on 2026-04-12.
The pre-squash history is preserved at tag `v1.0.0-dev-baseline` (3ef9c79).

| Version | Code | Description | Commit |
|---------|------|-------------|--------|
| v1.0.0 (baseline) | O100 | Baseline (includes issues #76 + #77) | 3ef9c79 |
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
| v1.0.31 | O131 | Issue #182 retry (failed — no undo stack, minimal fixes) | a73554f |
| v1.0.32 | O132 | Issue #182: gesture hygiene + internal undo stack + v1.0.30 bug fixes | — |
| v1.0.33 | O133 | Issue #182: ComboBox undo capture + tempoSync fix + preset name on undo | — |
| v1.0.34 | O134 | Issue #182: manual gestures for trajectory/inputChannel + preset name sync | 76fd6cd |
| v1.0.35 | O135 | (Same as v1.0.34, tested in Notion DB as v1.0.35) | — |
| v1.0.36 | O136 | Issue #182: guard captureUndoState/notifyHostStateChanged + mod knobs + trajectory shape manual gestures | — |
