---
phase: 03-personal-website
plan: 00
subsystem: testing
tags: [playwright, axe-core, wcag, opengraph, next.js, sender.net, tally]

# Dependency graph
requires:
  - phase: 02-email-capture-funding-infrastructure
    provides: "Sender.net decision (D-28) — Phase 3 will consume Sender.net wiring if Phase 2 ships, otherwise Phase 3 absorbs migration (D-14 disposition)"
provides:
  - "@axe-core/playwright ^4.11.1 installed as devDependency in andrewrahman-com"
  - "tests/a11y.spec.ts — WCAG 2.1 AA axe scan skeleton (3 route iterations) for Plan 06 to consume"
  - "tests/og-metadata.spec.ts — WEB-03 Open Graph + twitter:card assertions (4 tests) for Plan 07"
  - "tests/asset-existence.spec.ts — filesystem presence + deletion gates (14 tests) for Plan 04"
  - "D-14 disposition LOCKED: Option B — Phase 3 absorbs Sender.net migration"
  - "v1.0.0 AU plugin status LOCKED: installed — Wave 2 screenshot capture unblocked"
  - "Phase 2 Tally → Sender.net triage: Tally references remain, zero Sender.net refs (Phase 2 plans 02-05/02-06 still incomplete)"
affects: [03-04, 03-05, 03-06, 03-07]

# Tech tracking
tech-stack:
  added: ["@axe-core/playwright@^4.11.1"]
  patterns:
    - "Playwright + axe-core WCAG 2.1 AA scan (WCAG_AA_TAGS array, AxeBuilder chain, violations serialised in expect message)"
    - "Playwright meta-tag assertion via locator('head > meta[property=\"og:*\"]') + toHaveAttribute"
    - "Playwright filesystem-presence test using Node fs APIs (existsSync + statSync) without page fixture"

key-files:
  created:
    - "../andrewrahman-com/tests/a11y.spec.ts (WCAG 2.1 AA axe scan — 3 tests)"
    - "../andrewrahman-com/tests/og-metadata.spec.ts (WEB-03 — 4 tests)"
    - "../andrewrahman-com/tests/asset-existence.spec.ts (13 presence + 1 deletion-gate = 14 tests)"
  modified:
    - "../andrewrahman-com/package.json (+@axe-core/playwright devDep)"
    - "../andrewrahman-com/package-lock.json (dependency tree update)"

key-decisions:
  - "D-14 Option B: Phase 3 absorbs Sender.net migration (Tally refs still in get-osd/page.tsx, _headers, privacy/page.tsx; zero Sender.net refs anywhere in app/ or components/). Plan 05 will create components/EmailCaptureSection.tsx, migrate _headers CSP, update privacy/page.tsx processor name to UAB Sender.lt, and swap /get-osd/page.tsx body."
  - "v1.0.0 AU plugin installed: OpenSpatialDelay v1.0.component AND OpenSpatialDelay v1.0.0.component both present in ~/Library/Audio/Plug-Ins/Components/ — Wave 2 (Plan 04) screenshot capture is unblocked, no build_version.sh invocation needed."
  - "Landed Tasks 1–4 as a single Wave 0 commit per plan instruction (Task 1 step 4: 'Do NOT commit yet — Tasks 2, 3, 4 will land with this one in a single Wave 0 commit')."

patterns-established:
  - "WCAG_AA_TAGS constant: ['wcag2a','wcag2aa','wcag21a','wcag21aa'] — shared across future a11y tests"
  - "Red-on-first-run is intended: asset-existence goes RED until Plan 04 lands; a11y goes RED until Plan 06 fixes borderline token contrast"
  - "Three spec files live beside existing tests/*.spec.ts — no testDir change; picked up automatically by playwright.config.ts"

requirements-completed: []  # Plan 00 provides test infrastructure for WEB-01 + WEB-03; Plans 06 (a11y) and 07 (OG) mark them complete. Mark-complete deferred.

# Metrics
duration: 7min
completed: 2026-04-16
---

# Phase 3 Plan 00: Wave 0 Validation Infrastructure + D-14/AU Gates Summary

**Three Playwright spec files (a11y WCAG 2.1 AA + OG metadata + filesystem presence) scaffolded, @axe-core/playwright installed, D-14 locked as Option B (absorb Sender.net migration), and v1.0.0 AU plugin confirmed installed — Wave 2 screenshot capture gate released.**

## D-14 Disposition (decided Wave 0)

**Option B: Phase 3 absorbs Sender.net migration.**

**Evidence (triage from plan Task 1 step 3):**

- `grep -l 'TALLY\|tally.so'` across the three canonical files returned **all three** (`app/get-osd/page.tsx`, `_headers`, `app/privacy/page.tsx`).
- `grep -lr 'sender.net\|SENDER_FORM_ID\|EmailCaptureSection' app/ components/` returned **zero matches**.
- `app/get-osd/page.tsx` still imports `NEXT_PUBLIC_TALLY_FORM_ID` and loads `https://tally.so/widgets/embed.js`.
- `_headers` CSP still whitelists `https://tally.so https://widgets.tally.so`.
- `app/privacy/page.tsx` (lines 83–95) still names "Tally.so (Tally Technologies SRL, Belgium)" as the processor.
- Phase 2 STATE.md shows plans 02-05 (Sender.net wiring) and 02-06 (Patreon) still incomplete as of 2026-04-16T20:51:29.727Z.

**Implication for downstream plans:**

Plan 03-05 (per its plan-phase mapping) will:

1. Create `components/EmailCaptureSection.tsx` with Sender.net universal.js loader.
2. Migrate `_headers` CSP from `https://tally.so` / `https://widgets.tally.so` / `frame-src https://*.tally.so` to the Sender.net equivalents (`cdn.sender.net` per research A1, pending Sender.net dashboard confirmation at plan execution time).
3. Update `app/privacy/page.tsx` processor name to "UAB Sender.lt, Vilnius, Lithuania" (per Phase 2 STATE.md) and add changelog entry.
4. Swap `app/get-osd/page.tsx` body to `<EmailCaptureSection standalone />` (keeps URL as a thin shim for inbound backlinks per research §Open Question 4).
5. Rewrite `/get-osd/` links in `app/page.tsx` (Hero CTA, DownloadCTA) and `components/Nav.tsx` to `#get-osd` anchor.

## v1.0.0 AU Plugin Status

**Installed.** `ls ~/Library/Audio/Plug-Ins/Components/ | grep -E 'OpenSpatialDelay v1\.0(\.component|$)'` returned `OpenSpatialDelay v1.0.component`. Additionally, `OpenSpatialDelay v1.0.0.component`, `v1.0.1.component`, and `v1.0.2.component` are also present (user's A/B test environment). Wave 2 Plan 04 screenshot capture is **unblocked** — no `bash scripts/build_version.sh 768c248 v1.0.0 O100` invocation needed. Executor of Plan 04 must still verify the plugin window title reads `OpenSpatialDelay v1.0` (not v0.9 or v1.0.x) per Pitfall 2 in 03-RESEARCH.md before capturing.

## Phase 2 Tally/Sender.net Triage (informational)

| Surface | State | Action needed |
|---------|-------|---------------|
| `app/get-osd/page.tsx` | Tally `useEffect` loader + `NEXT_PUBLIC_TALLY_FORM_ID` | Plan 05 swaps body to shared `<EmailCaptureSection>` |
| `_headers` | CSP whitelists `tally.so` + `widgets.tally.so` | Plan 05 migrates to Sender.net domain(s) |
| `app/privacy/page.tsx` | "Tally.so (Tally Technologies SRL, Belgium)" processor | Plan 05 swaps to "UAB Sender.lt, Vilnius, Lithuania" + changelog |
| `app/` + `components/` | Zero Sender.net references | Plan 05 creates EmailCaptureSection + env var `NEXT_PUBLIC_SENDER_FORM_ID` |

Last relevant Phase 2 commit in site repo: `61519c3 revert: remove tally-embed test (tool changed to Sender.net)` (nothing newer). Confirms Phase 2 stopped before wiring Sender.net.

## Performance

- **Duration:** ~7 min
- **Started:** 2026-04-16T21:48Z (approx, from phase execution start)
- **Completed:** 2026-04-16T21:52Z
- **Tasks:** 4 of 4 complete
- **Files modified:** 5 (2 modified + 3 created, all in sibling repo)

## Accomplishments

- `@axe-core/playwright@^4.11.1` installed as devDependency in `andrewrahman-com` (1 of 2 added packages; verified via `node_modules/@axe-core/playwright/package.json` presence).
- `tests/a11y.spec.ts` scaffolded with WCAG 2.1 AA axe scan iterating `/`, `/privacy`, `/get-osd` (3 tests, listed by `npx playwright test --list`).
- `tests/og-metadata.spec.ts` scaffolded with 4 WEB-03 assertions (og:title, og:description, og:image + og:image:width, twitter:card).
- `tests/asset-existence.spec.ts` scaffolded with 14 tests (13 filesystem-presence assertions + 1 `sml-logo.png` deletion gate per D-11).
- D-14 disposition locked as Option B with full evidence trail.
- v1.0.0 AU plugin presence confirmed; Wave 2 capture session gate released.
- 21 new Playwright tests total (3 + 4 + 14); all listed, all intentionally red until later waves populate content.

## Task Commits

All four tasks landed in one sibling-repo commit per plan instruction (Task 1 step 4):

1. **Tasks 1–4 (combined):** install `@axe-core/playwright`, scaffold three spec files — `5e14b30` in `../andrewrahman-com` (test): "test(03-00): scaffold Phase 3 Wave 0 validation infrastructure"

**Plan metadata commit (openspatialdelay repo):** pending — created by final-commit step below (SUMMARY.md + STATE.md + ROADMAP.md).

## Files Created/Modified

Sibling repo `/Users/andrewrahman/conductor/repos/andrewrahman-com/`:

- `package.json` — added `@axe-core/playwright: ^4.11.1` to devDependencies.
- `package-lock.json` — dependency tree update (2 packages added per npm output).
- `tests/a11y.spec.ts` (NEW, 17 lines) — axe-core WCAG 2.1 AA scan across 3 routes.
- `tests/og-metadata.spec.ts` (NEW, 29 lines) — og:title / og:description / og:image / twitter:card assertions.
- `tests/asset-existence.spec.ts` (NEW, 38 lines) — 13 presence + 1 deletion-gate tests.

Plugin repo `/Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/`:

- `03-00-SUMMARY.md` (NEW, this file).

## Decisions Made

- **D-14 → Option B** (absorb Sender.net migration into Phase 3 plan 05). Rationale: zero Sender.net wiring exists in the site repo; Phase 2 plans 02-05/02-06 remain incomplete; launch deadline cannot wait on upstream Phase 2.
- **v1.0.0 AU already installed** — skipped the conditional `build_version.sh` step in Task 1; recorded presence instead.
- **Single Wave 0 commit** covering all 4 tasks per explicit plan instruction in Task 1 step 4. No per-task commits were created in the sibling repo because the plan deliberately sequences atomicity at the *wave* granularity for Wave 0 only.
- **Requirements mark-complete deferred** — Plan 00's frontmatter lists WEB-01 + WEB-03, but Plan 00 only provides *test infrastructure* for them. WEB-01 closes with Plan 06 (a11y audit passes) + Plan 04 (asset swap); WEB-03 closes with Plan 07 (manual OG verification). Marking complete here would be premature.

## Deviations from Plan

None - plan executed exactly as written. Zero Rule-1/2/3 auto-fixes. All acceptance criteria passed on first attempt:

- `grep -q '"@axe-core/playwright"' package.json` exits 0.
- `test -f node_modules/@axe-core/playwright/package.json` returns 0.
- `test -f tests/a11y.spec.ts && grep -q 'AxeBuilder' && grep -q 'wcag2aa'` all pass.
- `grep -c "for (const path of \['/', '/privacy', '/get-osd'\])"` returns 1.
- `npx playwright test tests/a11y.spec.ts --list` lists 3 tests, exit 0.
- `grep -c "test.describe('Open Graph metadata (WEB-03)'"` returns 1.
- `grep -q "twitter:card"` exits 0; `grep -q "summary_large_image"` exits 0.
- `npx playwright test tests/og-metadata.spec.ts --list` lists 4 tests, exit 0.
- `grep -c "screenshot_full.png"` returns ≥1 (actually 1); `grep -c "'andrew.jpg'"` returns 1.
- `grep -q "sml-logo.png is DELETED"` exits 0.
- `npx playwright test tests/asset-existence.spec.ts --list` lists 14 tests, exit 0.
- Combined `--list` across all three files: `Total: 21 tests in 3 files` — matches plan verification section (3+4+14=21).

## Issues Encountered

None.

## User Setup Required

None — all automation runs without external service configuration. Plan 05 will introduce `NEXT_PUBLIC_SENDER_FORM_ID` as a new environment variable; that's Plan 05's user-setup, not Plan 00's.

## Next Phase Readiness

- **Plan 03-01 (Hero stats + 6 features)** — unblocked; no dependency on Wave 0 beyond the now-installed test runner infra.
- **Plan 03-04 (Asset swaps + screenshots)** — unblocked; v1.0.0 AU confirmed installed. `tests/asset-existence.spec.ts` will go from red → green as assets land.
- **Plan 03-05 (D-14 EmailCaptureSection)** — disposition locked as Option B; Plan 05 executor must now produce Sender.net component + CSP migration + processor name swap + get-osd shim + link anchors. Exact `universal.js` URL to be verified against the Sender.net dashboard at Plan 05 Task 1 execution time (research A1 flagged as ASSUMED).
- **Plan 03-06 (a11y audit)** — unblocked; `tests/a11y.spec.ts` is the entry point. Expected initial failures on `--text-dim` on `--bg-void` / `--bg-panel` borderline pairs (per 03-RESEARCH.md §Expected violations) — Plan 06 will nudge tokens.
- **Plan 03-07 (OG metadata verification)** — unblocked; `tests/og-metadata.spec.ts` should already pass against current `app/layout.tsx` (metadata already WEB-03-compliant per research §5). Plan 07 is primarily the manual `opengraph.xyz` preview checkpoint.

## Self-Check: PASSED

Verified post-write:

- `test -f /Users/andrewrahman/conductor/repos/andrewrahman-com/tests/a11y.spec.ts` → FOUND
- `test -f /Users/andrewrahman/conductor/repos/andrewrahman-com/tests/og-metadata.spec.ts` → FOUND
- `test -f /Users/andrewrahman/conductor/repos/andrewrahman-com/tests/asset-existence.spec.ts` → FOUND
- `test -f /Users/andrewrahman/conductor/repos/andrewrahman-com/node_modules/@axe-core/playwright/package.json` → FOUND
- `git -C ../andrewrahman-com log --oneline | grep 5e14b30` → FOUND: "test(03-00): scaffold Phase 3 Wave 0 validation infrastructure"

---
*Phase: 03-personal-website*
*Plan: 00*
*Completed: 2026-04-16*
