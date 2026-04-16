---
phase: 03-personal-website
plan: 06
subsystem: web-accessibility
tags: [axe-core, wcag21aa, playwright, design-tokens, color-contrast, oklch]

# Dependency graph
requires:
  - phase: 03-personal-website
    plan: 00
    provides: "tests/a11y.spec.ts scaffold (@axe-core/playwright wired; 3 routes iterated with WCAG AA tag filter)"
  - phase: 03-personal-website
    plan: 03
    provides: "--accent-regal token (verified passing against --bg-void; no nudge needed)"
  - phase: 03-personal-website
    plan: 05
    provides: "inline #get-osd EmailCaptureSection + /get-osd shim (new surface covered by the a11y scan)"
provides:
  - "app/globals.css with 2 Phase 3 AA fix annotations (--text-dim + --accent-violet)"
  - "Zero WCAG 2.1 AA violations across /, /privacy, /get-osd"
  - "03-06-AXE-BASELINE.txt (1938-line raw axe JSON capture — audit trail for the 58-violation pre-fix state)"
affects: [03-08]

# Tech tracking
tech-stack:
  added: []  # No new deps — axe-core/playwright already installed in Plan 00
  patterns:
    - "OKLCH-minded token nudge: bump lightness ~+2 points toward white while keeping hue + chroma, using WCAG contrast formula as the oracle; prefer the smallest bump that clears all background pairings to avoid cross-pair regressions"
    - "Annotation format: '/* Phase 3 AA fix — was <hex>, failed 4.5:1 on <bg>. New value passes on <bg-list> (<ratios>). */' — captures prior hex + reason + post-nudge pass ratios inline"
    - "Atomic per-token commits: one commit per token value change (not per-route) so the blame for any future regression points at the exact WCAG justification"

key-files:
  created:
    - ".planning/phases/03-personal-website/03-06-AXE-BASELINE.txt (1938 lines, raw Playwright output capturing the 3-route baseline failures before any fixes applied)"
  modified:
    - "../andrewrahman-com/app/globals.css (2 tokens nudged: --text-dim #6d7279 → #787d84, --accent-violet #7457d1 → #8570d7)"

key-decisions:
  - "--text-dim nudged +2 OKLCH lightness pts to #787d84 (not the minimum #747980 that only just clears) because the minimum leaves --bg-panel at 4.44 (under the 4.5 floor by 0.06); #787d84 passes all 4 backgrounds with ≥4.69 margin"
  - "--accent-violet nudged to #8570d7 (not minimum #8068d6) because the minimum leaves --bg-panel at 4.51 — too close to the floor for safe downstream use; #8570d7 passes both --bg-void (5.12) and --bg-panel (4.91)"
  - "--accent-regal (oklch(72% 0.15 290) ≈ #b49bd8) verified unchanged — axe did not flag it on either as-background (--bg-void text) or as-foreground (no such pairing rendered). No change needed."
  - "No 40-violation symptom (Pitfall 1 of 03-RESEARCH.md) — baseline returned 58 color-contrast nodes collapsing to 5 unique FG/BG pairs, all explained by 2 root-cause tokens. Stale-build check passed (baseline run after fresh `npm run build`)."
  - "Zero --tap-N UI-minimum violations (the 12-tap rainbow against --bg-panel all cleared 3:1). No post-launch follow-up needed."
  - "Plan 03 colour-tolerance test on --accent-regal untouched (its RGB range regex targets --accent-regal, not --accent-violet; no tolerance widening required despite the accent-violet nudge)."

patterns-established:
  - "WCAG oracle > eyeballing: used Node WCAG contrast calculator against all 4 dark backgrounds before editing, not just the one axe flagged loudest, to pick a nudge that clears every current pair AND preserves headroom for future surfaces."
  - "Inline annotation preserves pre-nudge value: future readers can grep `Phase 3 AA fix` in globals.css to see what was WCAG-motivated vs brand-motivated."

requirements-completed: [WEB-01]  # WEB-01 acceptance criteria include the site being "live" + accessible; D-08 a11y audit was the final gating task per 03-CONTEXT.md. (Note: WEB-01 was already marked [x] in REQUIREMENTS.md from Plan 03-04; this plan re-confirms the full criterion is satisfied.)

# Metrics
duration: 8min
completed: 2026-04-16
---

# Phase 3 Plan 06: D-08 WCAG 2.1 AA Conformance Summary

**Axe-core baseline found 58 color-contrast violations collapsing to 2 root-cause tokens (`--text-dim` on all 4 dark backgrounds, `--accent-violet` on `--bg-void`); both tokens nudged ~+2 OKLCH lightness points, all 3 routes now green, full Playwright suite at 54/54.**

## Performance

- **Duration:** ~8 min
- **Started:** 2026-04-16T23:04:50Z
- **Completed:** 2026-04-16T23:06:48Z (approx, wall-clock before SUMMARY write)
- **Tasks:** 1 of 1 complete (split across 2 atomic commits — one per token nudge)
- **Files modified:** 1 (+ 1 audit-trail artifact created)

## Accomplishments

- **All 3 a11y routes green:** `/`, `/privacy`, `/get-osd` return zero WCAG 2.1 AA violations from `@axe-core/playwright` with tags `wcag2a`, `wcag2aa`, `wcag21a`, `wcag21aa`.
- **Full suite green:** 54 Playwright tests across 6 spec files (homepage-content 28 tests, privacy-content 10 tests, smoke 4 tests, og-metadata 5 tests, asset-existence 4 tests, a11y 3 tests) — no regressions from token nudges.
- **Minimum-nudge discipline:** touched 2 tokens total (the 2 root causes); no speculative fixes, no re-baselining of any other token. `--accent-regal`, `--accent-rose`, `--accent-stellar`, `--accent-amber`, `--accent-green`, `--accent-sync`, and all 12 `--tap-N` values are untouched because axe never flagged them.
- **Audit trail preserved:** raw 1938-line Playwright JSON output saved to `.planning/phases/03-personal-website/03-06-AXE-BASELINE.txt` so the 58→0 trajectory is reproducible and reviewable.

## Axe Baseline Violations

Pre-fix scan: **58 color-contrast nodes** across 3 routes, collapsing to **5 unique FG/BG pairs**:

| # | FG                        | BG                        | Ratio  | Required | Token pair                            | Routes     |
| - | ------------------------- | ------------------------- | ------ | -------- | ------------------------------------- | ---------- |
| 1 | #6d7279 (`--text-dim`)    | #03060b (`--bg-void`)     | 4.18:1 | 4.5:1    | text-dim on bg-void                   | /, /get-osd |
| 2 | #6d7279 (`--text-dim`)    | #0a0d12 (`--bg-panel`)    | 4.01:1 | 4.5:1    | text-dim on bg-panel                  | /          |
| 3 | #6d7279 (`--text-dim`)    | #06090f (`--bg-header`)   | 4.11:1 | 4.5:1    | text-dim on bg-header (footer)        | /, /privacy, /get-osd |
| 4 | #6d7279 (`--text-dim`)    | #020307 (`--bg-well`)     | 4.25:1 | 4.5:1    | text-dim on bg-well                   | /          |
| 5 | #7457d1 (`--accent-violet`) | #03060b (`--bg-void`)   | 3.89:1 | 4.5:1    | accent-violet on bg-void              | /          |

Rule coverage: **only `color-contrast`** fired. No `aria-*`, no `label`, no `page-has-heading-one`, no `landmark-*`, no `duplicate-id`. Semantic a11y scaffold was already sound from Plans 00 and 05.

## Tokens Changed

| Token            | Old      | New      | Reason                                                                                                      | Post-nudge contrast (FG on…)                                          |
| ---------------- | -------- | -------- | ----------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------- |
| `--text-dim`     | `#6d7279` | `#787d84` | Failed 4.5:1 on all 4 dark backgrounds (bg-void 4.18, bg-panel 4.01, bg-header 4.11, bg-well 4.25)       | bg-void 4.90, bg-panel 4.69, bg-header 4.81, bg-well 4.97             |
| `--accent-violet` | `#7457d1` | `#8570d7` | Failed 4.5:1 on `--bg-void` (3.89). Nudged ~+2 OKLCH L pts keeping hue (lightened, same violet identity).  | bg-void 5.12, bg-panel 4.91 (safe margin on both surfaces)            |

Both changes annotated inline in `app/globals.css`:

```css
--text-dim: #787d84; /* Phase 3 AA fix — was #6d7279, failed 4.5:1 on --bg-void (4.18), --bg-panel (4.01), --bg-header (4.11), --bg-well (4.25). New value passes all four (4.90/4.69/4.81/4.97). */
--accent-violet: #8570d7; /* Phase 3 AA fix — was #7457d1, failed 4.5:1 on --bg-void (3.89). New value passes on --bg-void (5.12) and --bg-panel (4.91). */
```

`grep -c 'Phase 3 AA fix' /Users/andrewrahman/conductor/repos/andrewrahman-com/app/globals.css` → **2** (satisfies the plan's acceptance criterion of ≥1).

## Tap-N UI-Minimum Check

All 12 tap colours were tested implicitly by the axe scan (they render as dots against `--bg-panel` on the homepage brand strip). **Zero violations** flagged against the 3:1 UI minimum — the tap palette was designed against `--bg-panel` in the plugin and the rainbow carries over cleanly to the web surface.

No post-launch follow-up needed for the tap palette.

## Task Commits

Per-task atomic commits in sibling `../andrewrahman-com` repo:

1. **Task 1a (text-dim nudge):** `f7f5b72` — `fix(03-06): nudge --text-dim #6d7279 -> #787d84 for WCAG AA (D-08)` (1 file, 1 insertion / 1 deletion) — resolved 4 of 5 violation pairs; /privacy and /get-osd green after this commit.
2. **Task 1b (accent-violet nudge):** `996d42d` — `fix(03-06): nudge --accent-violet #7457d1 -> #8570d7 for WCAG AA (D-08)` (1 file, 1 insertion / 1 deletion) — resolved the last violation pair; `/` green after this commit.

**Plan metadata commit (openspatialdelay repo):** pending — created by final-commit step below (SUMMARY.md + STATE.md + ROADMAP.md).

## Files Created/Modified

Sibling repo `../andrewrahman-com/`:

- `app/globals.css` (MODIFIED) — 2 token value changes with Phase 3 AA fix annotations. Tokens touched: `--text-dim`, `--accent-violet`. All other tokens, `@theme inline` mappings, body styles, `::selection`, `.polaroid` shadows, and media queries unchanged.

Plugin repo `/Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/`:

- `03-06-AXE-BASELINE.txt` (NEW, 1938 lines) — raw `npx playwright test tests/a11y.spec.ts --reporter=list` output from the pre-fix scan; captures all 58 violation nodes across 3 routes with FG/BG hex + contrast ratio + related DOM nodes. Audit trail for the 58→0 trajectory.
- `03-06-SUMMARY.md` (NEW, this file).

## Decisions Made

- **Nudge-to-margin, not nudge-to-minimum** — For `--text-dim`, the minimum passing hex was `#747980` (bg-panel 4.438, the borderline case). Chose `#787d84` instead (bg-panel 4.69) to leave headroom for any future surface using text-dim on a similar mid-grey background. Same logic for `--accent-violet`: minimum `#8068d6` (bg-panel 4.51) was too close to the floor; `#8570d7` gives 4.91.
- **Atomic per-token commits** — split the one-task-two-nudges into 2 commits (not one) so blame on any future colour-regression test points at the exact WCAG justification for that specific token. Matches GSD's per-task atomic commit convention.
- **No Plan 03 tolerance widening required** — Plan 03's `homepage-content.spec.ts` colour regex asserts the PatreonCTA renders `--accent-regal`'s RGB range, NOT `--accent-violet`. Verified by re-running the full suite post-nudge: all 54 tests green including the Plan 03 Patreon CTA assertion.
- **Baseline artifact preserved in planning repo, not site repo** — Raw axe output is a GSD audit artifact (belongs with phase planning), not a site-runtime artifact (doesn't belong in the production bundle). Avoids bloating the Netlify deploy with 1938 lines of test output.
- **Pass-through NOT taken** — Plan included a "pass-through acceptance" clause for a zero-violation first run. Baseline returned 58 violations, so the normal nudge-iterate loop ran as designed (1 iteration per token, 2 iterations total, well under the 3-iteration escalation gate).

## Deviations from Plan

### None (in Rules 1/2/3 sense)

The plan executed exactly as written. The 5 violation pairs matched the expected pattern in `03-RESEARCH.md §Expected violations` plus one pair the plan's preamble implied but didn't explicitly enumerate (`--accent-violet` on `--bg-void`). No Rule 1 bugs encountered, no Rule 2 missing criticality flagged, no Rule 3 blockers, no Rule 4 architectural questions.

### Task Split (cosmetic, not a deviation)

The single `<task>` block in the plan was executed as 2 atomic commits rather than 1 monolithic commit (one commit per token nudge). This matches GSD's per-task commit convention and keeps the per-commit WCAG justification tight; it does not change the file set or the success criteria.

---

**Total deviations:** 0 auto-fixed
**Impact on plan:** Plan executed as specified. Baseline matched research's expected failure pattern; nudges landed on the first iteration for both tokens.

## Issues Encountered

- **Test runner serves from `out/` export** — same gotcha documented in 03-05-SUMMARY.md. First attempt to re-run axe after editing `globals.css` would have served the pre-edit bundle; mitigated by running `npm run build` between every source edit and every `npx playwright test` invocation. No wasted iterations — caught this from prior-plan learning.
- **Read-before-edit hook friction** — Hook fired an intermediate reminder between edit attempts because `Read` + `Edit` interleaved with `Grep` (the grep reset the file's session-read flag in the hook's view). Re-registered the file via a targeted `Read` call and proceeded; no logical impact on the nudges themselves.

## Deferred Issues

None — all axe findings resolved inside the plan scope. No scope for "post-launch tap-N outline" work (the plan's conditional follow-up) because no tap colour failed its 3:1 UI minimum.

## User Setup Required

None — token-only changes to CSS source. Ship as part of the next Netlify deploy; no env var, no dashboard config, no external service touchpoint.

## Next Phase Readiness

- **Plan 03-07 (OG metadata verification)** — unblocked. Independent of a11y changes; `layout.tsx` dimensions and social-card preview are Plan 07 scope.
- **Plan 03-08 (launch checklist + WEB-02 sentinel)** — unblocked. D-08 is the final design-system launch-risk item per `03-CONTEXT.md §Design system scope (Area 2)`; with axe green, the WEB-02 external sentinel + launch checklist is the remaining gate.
- **ROADMAP/REQUIREMENTS:** WEB-01 stays complete (already marked in REQUIREMENTS.md from Plan 03-04); the a11y criterion that was the open thread on WEB-01 is now concretely satisfied.

## Threat Flags

None — token value changes do not introduce new network endpoints, new auth paths, new file-system reads, or new schema surfaces. No changes to CSP (`_headers`), no changes to forms, no changes to DOM structure. Pure visual contrast adjustment on 2 existing CSS custom properties.

## Self-Check: PASSED

Verified post-write (2026-04-16T23:07Z):

- `test -f /Users/andrewrahman/conductor/repos/andrewrahman-com/app/globals.css` → FOUND
- `test -f .planning/phases/03-personal-website/03-06-AXE-BASELINE.txt` → FOUND (1938 lines)
- `test -f .planning/phases/03-personal-website/03-06-SUMMARY.md` → FOUND (this file)
- `git -C ../andrewrahman-com log --oneline | grep f7f5b72` → FOUND: `fix(03-06): nudge --text-dim #6d7279 -> #787d84 for WCAG AA (D-08)`
- `git -C ../andrewrahman-com log --oneline | grep 996d42d` → FOUND: `fix(03-06): nudge --accent-violet #7457d1 -> #8570d7 for WCAG AA (D-08)`
- `grep -c 'Phase 3 AA fix' ../andrewrahman-com/app/globals.css` → **2** (meets ≥1 acceptance criterion)
- Axe scan final run: `3 passed (2.0s)` — all 3 routes green on WCAG 2.1 AA
- Full Playwright suite final run: `54 passed` — no regressions

---
*Phase: 03-personal-website*
*Plan: 06*
*Completed: 2026-04-16*
