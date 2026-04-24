---
phase: 02
phase_name: email-capture-funding-infrastructure
status: clean
depth: standard
files_reviewed: 1
reviewed_at: 2026-04-24
findings:
  critical: 0
  warning: 0
  info: 3
  total: 3
---

# Phase 02 Code Review

**Phase 2 code review complete — 1 source file reviewed, 0 critical/warning findings, 3 informational observations.**

## Scope

Phase 2 (email capture infrastructure) produced no source code changes in the openspatialdelay repo itself — all site code lives in `andrewrahman-com` (a separate repository). The one code artifact in this repo is:

- `docs/phase-02-evidence/patreon-graphics/_generate.py` — Python script that procedurally generates Patreon tier cards, cover banner, and post cover using Pillow.

## Findings

### INFO: Duplicate FONTS path assignment (dead code)

**File:** `docs/phase-02-evidence/patreon-graphics/_generate.py:26-27`

```python
FONTS = Path(__file__).parents[2] / ".." / "fonts"  # repo root /fonts  ← dead code
FONTS = (ROOT.parents[2] / "fonts").resolve()        ← actual value used
```

The first assignment is immediately overwritten. Both resolve to the same directory. No runtime impact — the second line is correct. Remove the first line.

---

### INFO: Tautological conditional in `icon_astronaut`

**File:** `docs/phase-02-evidence/patreon-graphics/_generate.py:186`

```python
a = 255 if i == 0 else 255
```

Both branches evaluate to `255`, making the condition dead. The variable `a` is then unused in the following line (which uses the direct `accent` tuple). No visual or runtime impact.

---

### INFO: Redundant `draw_tap_strip` call in `make_post1_cover`

**File:** `docs/phase-02-evidence/patreon-graphics/_generate.py:528-540`

`draw_tap_strip(combined, active_idx=0, ...)` is called with `active_idx=0` (which draws all 12 bars at `alpha=70`), then a manual loop immediately redraws all 12 bars at `alpha=255`, completely overwriting the first call. The first call is a no-op. The logic works correctly because the second pass wins.

---

## Summary

No bugs or security issues. The script is a standalone offline image generator with no external inputs, no shell exec, and no user-supplied data. The three `INFO` items are code clarity issues only — they don't affect output correctness or security. Safe to ship as-is.
