---
phase: 02-email-capture-funding-infrastructure
plan: 01
status: complete
completed: 2026-04-15
commits:
  - 3f8746f  # PROJECT.md Brand Identity section + GPL-3.0-only
  - d7a6bcb  # REQUIREMENTS.md DIST-01 rewrite + anti-feature row removal
key-files:
  created: []
  modified:
    - .planning/PROJECT.md
    - .planning/REQUIREMENTS.md
deviations:
  - task: Task 2 (ROADMAP.md)
    expected: Rewrite Phase 2 success criterion 1 from optional-email to required-email
    actual: No edit needed — ROADMAP.md Phase 2 criterion 1 already contained the required-email wording and thank-you-page-with-downloads flow from the planning sprint earlier tonight (2026-04-15 ~23:27, commit `Plans TBD replaced with all 6 plan summaries`).
    impact: None — acceptance criteria for Task 2 all pass against current file state. No additional commit.
---

## Summary

Plan 02-01 aligns the three OpenSpatialDelay planning docs with the Phase 2 CONTEXT.md decisions (D-01, D-03, D-04, D-05, D-06) before any external-facing Phase 2 artifact is produced.

## What changed

### Task 1 — PROJECT.md (commit `3f8746f`)

Replaced the project-description paragraph's license clause:

- **Before:** `Dual-licensed GPL-3.0 / Commercial (Spatial Media Lab) — transitioning to GPL-3.0 only.`
- **After:** `Licensed GPL-3.0. The first product in the Spatial Media Library pipeline, developed under Spatial Media Lab.`

Added a new `## Brand Identity` section immediately after the project description, with bold-bulleted definitions of:

- **Spatial Media Lab (SML)** — the open-source organization
- **Spatial Media Library** — the pipeline brand (three separate words, never abbreviated)
- **OpenSpatialDelay (OSD)** — the first product in the pipeline
- **Andrew Rahman** — the personal creator identity on `patreon.com/andrewrahman` and `andrewrahman.com`

Plus a disambiguation rule: every externally-published Phase 2 artifact MUST disambiguate "Spatial Media Lab" from "Spatial Media Library" on first reference.

**Lines changed:** +12 / -1.

### Task 2 — ROADMAP.md (no commit needed)

Acceptance criteria already satisfied:

- `grep -q "required email + GDPR consent checkbox"` → **present** on ROADMAP.md:42
- `grep -q "thank-you screen with working macOS + Windows download buttons"` → **present**
- `grep -q "optional email"` → **absent** (count: 0)

The required-email framing was written into the Phase 2 block during the earlier planning sprint when the "Plans: TBD" field was replaced with all six plan summaries. No further edit needed.

### Task 3 — REQUIREMENTS.md (commit `d7a6bcb`)

Rewrote DIST-01:

- **Before:** `Tally email capture form live — optional email, GDPR consent checkbox, redirect to GitHub Releases`
- **After:** `Tally email capture form live — required email, GDPR consent checkbox, thank-you screen with manual macOS + Windows download buttons linking to the v1.0.0 GitHub Release assets`

Deleted the contradictory anti-features table row:

```
| Download gate / mandatory email | Destroys GPL plugin adoption; community shares direct links |
```

This row contradicted D-04 ("email IS the gate now"). Every other anti-feature row preserved.

**Lines changed:** +1 / -2.

## Downstream impact — unblocks

| Downstream plan | What was blocked before | What it can now proceed with |
|-----------------|------------------------|------------------------------|
| 02-02 (Next.js shell) | Site layout copywriting cannot reference a stable "Spatial Media Library" pipeline brand | PROJECT.md now defines SML / Spatial Media Library / OSD / Andrew Rahman canonically — all site copy can reference the glossary |
| 02-03 (Privacy prose) | Data-controller disambiguation between SML the org vs Andrew Rahman the individual was underspecified | Brand Identity block clarifies who owns which surface; privacy-prose drafting has a stable glossary |
| 02-04 (Hosting + DNS) | N/A | No direct dependency |
| 02-05 (Tally form copy) | Form copy referencing "optional email" would have contradicted REQUIREMENTS.md and the Tally configuration | DIST-01 now reads required-email; Tally copy can enforce required-email without doc conflict |
| 02-06 (Patreon pitch) | Pipeline-pitch language needed "Spatial Media Library" canon | Canon defined in PROJECT.md — Patreon About/tier copy can cite the pipeline brand directly |

## Verification

| Check | Result |
|-------|--------|
| `grep -q "Spatial Media Library" .planning/PROJECT.md && grep -q "Brand Identity" .planning/PROJECT.md && grep -q "Licensed GPL-3.0" .planning/PROJECT.md && ! grep -q "Dual-licensed GPL-3.0 / Commercial" .planning/PROJECT.md` | PASS |
| `grep -c "Spatial Media Library" .planning/PROJECT.md` | 4 (expected ≥2) |
| `grep -q "required email + GDPR consent checkbox" .planning/ROADMAP.md && grep -q "thank-you screen with working macOS + Windows download buttons" .planning/ROADMAP.md && ! grep -q "optional email + GDPR consent checkbox" .planning/ROADMAP.md` | PASS |
| `grep -q "DIST-01.*required email" .planning/REQUIREMENTS.md && ! grep -q "Download gate / mandatory email" .planning/REQUIREMENTS.md && ! grep -q "DIST-01.*optional email" .planning/REQUIREMENTS.md` | PASS |
| `grep -c "DIST-01" .planning/REQUIREMENTS.md` | 2 (expected ≥2) |

All three `<verify>` blocks pass. All `<success_criteria>` items satisfied.

## Self-Check: PASSED
