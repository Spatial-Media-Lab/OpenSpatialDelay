---
phase: 03-personal-website
plan: 02
subsystem: ui
tags: [nextjs, tailwind, playwright, gdpr, privacy-policy, email-migration, tdd]

# Dependency graph
requires:
  - phase: 03-personal-website/00
    provides: "Wave 0 validation infrastructure — Playwright describe-block in tests/privacy-content.spec.ts and tests/homepage-content.spec.ts extended here with 4 new tests (3 privacy + 1 homepage)"
  - phase: 03-personal-website/01
    provides: "app/page.tsx in post-hero-rewrite stable state at commit 39122d0; Task 3 layered its 2-line contact-anchor edit on top without touching the Hero/Features/DownloadCTA blocks Plan 01 finalised"
provides:
  - "Data-controller contact email on app/privacy/page.tsx migrated across all 6 sites: Data Controller line, Who-we-are section, rights-request link, unsubscribe link — both the href attribute and the body text in each"
  - "AboutAndrew contact anchor in app/page.tsx migrated (lines 558 href + 562 body text)"
  - "Privacy effective date bumped from 2026-04-16 to 2026-04-17 per GDPR changelog best practice (D-13)"
  - "Privacy 'Changes to this policy' section gains a dated <p> documenting the controller-contact migration — phrase 'Data Controller contact email updated' pinned by a Playwright test"
  - "4 new Playwright content-regression assertions: 2 negative (no Gmail on privacy / no Gmail on homepage) + effective-date regex + changelog-entry string"
  - "scripts/verify-production.sh post-deploy content check swapped from the old email to the new one (Rule 1 — would have broken the post-deploy verify workflow)"
affects: [03-03, 03-04, 03-05, 03-06, 03-07, 03-08]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "GDPR controller-change audit pattern — when the data-controller contact channel changes, (a) bump the effective date, (b) append a new <p> to 'Changes to this policy' with a date label + plain-English description, (c) do NOT include the legacy email in the rendered changelog text (git history is the audit trail; Playwright negative assertion stays green)"
    - "Playwright positive + negative assertion pair — every content migration gets BOTH a `.toContainText(new)` and a `.not.toContainText(old)` assertion so regressions surface instantly on either direction"
    - "Replace-all editing on leaf strings — Edit tool with replace_all:true used for mechanical string migration across privacy/page.tsx (6 occurrences) and app/page.tsx (2 occurrences); safer than 6 individual edits when every occurrence must change identically"

key-files:
  created:
    - ".planning/phases/03-personal-website/03-02-SUMMARY.md"
  modified:
    - "../andrewrahman-com/app/privacy/page.tsx (6 mailto sites migrated + effective date bumped to 2026-04-17 + changelog <p> appended inside 'Changes to this policy' section)"
    - "../andrewrahman-com/app/page.tsx (AboutAndrew contact anchor: href lines 558 + body text line 562)"
    - "../andrewrahman-com/tests/privacy-content.spec.ts (rights-request positive-swap + 3 new assertions: negative Gmail, effective-date regex, changelog-entry string)"
    - "../andrewrahman-com/tests/homepage-content.spec.ts (contact-email test positive-swap in 3 positions + 1 new negative Gmail assertion)"
    - "../andrewrahman-com/scripts/verify-production.sh (post-deploy content check line 54 — Rule 1 auto-fix; left alone would have broken the verify workflow at the next deploy)"

key-decisions:
  - "Effective date bumped to 2026-04-17 (the current local date; was 2026-04-16 baseline). Regex in the new Playwright assertion `/Effective: 2026-04-1[6-9]|.../` tolerates 2026-04-16-or-later, but the stricter current-day value makes the audit trail more honest about when the migration actually shipped."
  - "Changelog <p> DELIBERATELY omits the legacy 'andrewjrahman@gmail.com' string from rendered text. Plan Step C revised itself mid-spec to avoid creating the exact scenario that would break the `.not.toContainText('andrewjrahman@gmail.com')` negative assertion (which matches inside <code> tags). The git history carries the audit trail; the rendered page carries only the new address and a plain-English description of the change."
  - "scripts/verify-production.sh was migrated in Task 2's commit (Rule 1 auto-fix) even though it lives outside the stated scope (app/ and components/). The script's `check_contains \"/privacy/\" \"andrewjrahman@gmail.com\"` assertion would have failed on the very next deploy-verify run — leaving it would have effectively gated all future deploys behind the legacy email. Fixed inline and documented."

patterns-established:
  - "Changelog-entry-as-audit-artifact pattern — when a GDPR-material field changes, append a dated <p> to the 'Changes to this policy' section with phrasing 'X updated [to Y]' but DO NOT render the old value in the changelog text. The Playwright `.not.toContainText` negative assertion depends on this; git history supplies the audit trail instead."
  - "Post-deploy verification-script-as-content-gate — scripts/verify-production.sh encodes copy expectations as grep-style assertions. Whenever site content migrates, the script's content checks migrate alongside (or the post-deploy step breaks). Plan 02 hit this; future plans that edit `app/privacy/page.tsx` copy should grep `scripts/` for residual assertions before committing."

requirements-completed: [WEB-01]

# Metrics
duration: 3min
completed: 2026-04-16
---

# Phase 3 Plan 02: Email Migration (andrewjrahman@gmail.com → andrew@spatialmedialab.org) Summary

**GDPR-material data-controller contact swap across every site-facing surface — 6 migrated sites on privacy/page.tsx + 2-position swap on AboutAndrew() + bumped effective date (2026-04-17) + dated changelog paragraph + 4 new Playwright regression assertions; D-13 migration audit-complete.**

## Performance

- **Duration:** ~3 min (start 2026-04-16T22:05:14Z, end 2026-04-16T22:08:09Z)
- **Tasks:** 3 of 3
- **Files modified:** 5 (4 in sibling repo `../andrewrahman-com/` + 1 new SUMMARY here)
- **Tests added:** 4 (3 privacy-content + 1 homepage-content)
- **Final test count:** 22 passed (9 privacy + 13 homepage)

## Accomplishments

- Migrated the data-controller contact email across every rendered surface on andrewrahman.com:
  - `app/privacy/page.tsx` — 6 mailto sites (Data Controller line lines 18-21, Who-we-are link lines 32-34, How-to-exercise-your-rights link lines 172-175, How-to-unsubscribe link lines 187-190; href + body text in each).
  - `app/page.tsx` AboutAndrew() — 2 positions (href line 558 + body text line 562).
- Bumped privacy-policy effective date from `2026-04-16` to `2026-04-17` (current local date at execution time) so the audit trail reflects when the D-13 migration actually shipped.
- Appended a dated changelog <p> inside the "Changes to this policy" section pinning the "Data Controller contact email updated" string as a Playwright-asserted audit artifact. Deliberately did NOT include the legacy Gmail string in the rendered changelog so the negative assertion keeps passing; git history carries the audit trail.
- Added 4 new Playwright content-regression assertions:
  - `privacy-content.spec.ts` — 3 new: old Gmail absence (D-13 migration), effective-date regex tolerant of any 2026-04-16-or-later date, changelog-entry phrase `"Data Controller contact email updated"`.
  - `homepage-content.spec.ts` — 1 new: old Gmail absence from the homepage.
  - Existing rights-request test (privacy) + contact-email test (homepage, 3 positions) swapped from old to new address.
- Rule 1 auto-fix: `scripts/verify-production.sh:54` post-deploy content check swapped from the old email to the new one (would have broken the verify-production workflow at the next deploy).
- Site-wide `grep -rn 'andrewjrahman@gmail.com' app/ components/` returns 0 matches — D-13 migration goal achieved.
- All 22 Playwright tests green (13 homepage-content + 9 privacy-content) after final build.

## Task Commits

Each task was committed atomically in the sibling site repo `../andrewrahman-com/`:

1. **Task 1: Update privacy-content.spec.ts + homepage-content.spec.ts (RED)** — `1b26373` (test)
2. **Task 2: Migrate 6× email references in app/privacy/page.tsx + bump effective date + append changelog entry** — `8113e1e` (feat)
3. **Task 3: Migrate AboutAndrew contact link in app/page.tsx (2 positions)** — `4de7977` (feat)

**Plan metadata:** pending — created by final-commit step below (SUMMARY.md + STATE.md + ROADMAP.md + REQUIREMENTS.md in the plugin repo).

## Files Created/Modified

### Sibling repo `/Users/andrewrahman/conductor/repos/andrewrahman-com/`

- `app/privacy/page.tsx` — 8 edits: 6 email-site replacements (via `Edit replace_all:true`), 1 effective-date bump, 1 new `<p>` appended inside the "Changes to this policy" `<section>`.
- `app/page.tsx` — 2 edits in AboutAndrew() contact anchor (href + body text), both via the single `Edit replace_all:true` call.
- `tests/privacy-content.spec.ts` — rights-request assertion swapped (old → new email); 3 new tests appended inside existing describe block (file grew from 40 lines / 6 tests to 53 lines / 9 tests).
- `tests/homepage-content.spec.ts` — contact-email test swapped in 3 positions (body text, link name, mailto href); 1 new negative-assertion test appended (file grew from 103 lines / 13 tests to 109 lines / 14 tests, of which Playwright runs 13 — the describe block includes 13 test() blocks, one hero-names test that already existed plus the 12 from Plan 01 + 1 new).
- `scripts/verify-production.sh` — line 54 post-deploy content-check string swapped (Rule 1 auto-fix, see Deviations).

### Plugin repo `/Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/`

- `03-02-SUMMARY.md` (NEW, this file).

## Decisions Made

- **Effective date: 2026-04-17** (local current day at execution). The regex `/Effective: 2026-04-1[6-9]|.../` tolerates 2026-04-16-or-later, but bumping to the actual ship day makes the audit trail honest. Source of truth: system clock at execution (2026-04-17 00:05 local / 2026-04-16 22:05 UTC); local date used because GDPR policy bodies read natural language dates.
- **Changelog <p> deliberately omits the legacy Gmail string** from rendered text. Plan Step C revised itself mid-spec to avoid creating exactly the scenario that would break the negative assertion (`.not.toContainText('andrewjrahman@gmail.com')` matches inside `<code>` tags too). Plain-English "Data Controller contact email updated to andrew@spatialmedialab.org" + the git log is the audit trail.
- **scripts/verify-production.sh migrated in the same commit as privacy/page.tsx** rather than deferred. Rationale: the file encodes a grep-style content expectation that is DIRECTLY coupled to the live privacy page — if the privacy page migrates and the script doesn't, the next deploy-verify run fails because the script checks for a string the page no longer renders. This is a Rule 1 bug that the privacy-page edit itself causes; fixing them together keeps the commit atomic and self-contained.

## Test Pass Count

- `tests/privacy-content.spec.ts`: **9 passed** (6 pre-existing + 3 new; Task 2 verify run).
- `tests/homepage-content.spec.ts`: **13 passed** (12 pre-existing from Plan 01 + 1 new negative-Gmail assertion; Task 3 verify run).
- Combined end-of-plan run: **22 passed** in 2.4s (2 workers).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 — Bug] scripts/verify-production.sh post-deploy content check would have broken after privacy-page migration**

- **Found during:** Task 2 (pre-edit site-wide `grep -rn 'andrewjrahman@gmail.com'` sweep of the `andrewrahman-com/` repo)
- **Issue:** `scripts/verify-production.sh:54` hard-codes `check_contains "/privacy/" "andrewjrahman@gmail.com"` as a post-deploy content assertion. After the privacy-page migration in Task 2, the deployed site no longer contains the old email — so the script would fail on the first post-migration deploy, effectively gating future deploys behind a string that no longer exists.
- **Why Rule 1 and not deferred:** the verify script's assertion is DIRECTLY caused by the migration — Task 2 is the upstream edit that invalidates it. Leaving it would create a post-deploy regression directly downstream of this plan's changes.
- **Fix:** single-line replace of the legacy Gmail string with `andrew@spatialmedialab.org` on line 54. Keeps the script's role (post-deploy content sanity check) intact under the new data-controller contact.
- **Files modified:** `../andrewrahman-com/scripts/verify-production.sh`
- **Verification:** `grep -c 'andrewjrahman@gmail.com' scripts/verify-production.sh` returns 0; `grep -c 'andrew@spatialmedialab.org' scripts/verify-production.sh` returns 1.
- **Committed in:** `8113e1e` (Task 2 commit, explicitly noted in commit body as "Rule 1 — left alone would break the verify-production workflow").

### Scope-boundary note

`scripts/verify-production.sh` lives outside the plan's stated file list (`../andrewrahman-com/app/privacy/page.tsx`, `app/page.tsx`, `app/get-osd/page.tsx`, `tests/*`). It was migrated anyway because the assertion in it is directly downstream of the privacy-page edit — migrating one without the other creates a broken state. Documented here per GSD scope-boundary rules.

---

**Total deviations:** 1 auto-fixed (Rule 1 — bug)
**Impact on plan:** Zero scope creep. The fix is single-line, directly downstream of the in-scope privacy-page edit, and prevents a post-deploy verify regression.

## Issues Encountered

- **None blocking.** The plan executed cleanly in TDD order: Task 1 authored RED tests (committed while still failing as intended), Task 2 turned the 3 new privacy assertions green + kept all 6 pre-existing privacy assertions green, Task 3 turned the homepage assertions green.
- **Static-build caching** (observed pattern from Plan 01): ran `npm run build` once after Task 2's privacy-page edits and once after Task 3's page.tsx edit before each Playwright run, then `pkill -f "serve out -l 3000"` before the final combined run to flush any stale `npx serve out` worker left behind by `playwright.config.ts`'s `reuseExistingServer: !process.env.CI`. This is the Plan 01 documented hazard, handled the same way.
- **Read-tool file caching** (environment quirk, not a plan issue): the Read tool on files modified earlier in the session returned "File unchanged — refer to earlier result" even for offset-based re-reads. Worked around by using `Grep` with `-B/-A` for context inspection and by relying on the `Edit` tool's `replace_all:true` for mechanical migrations. No impact on correctness; noted for future sessions.

## User Setup Required

None — all automation ran without external service configuration. No secrets, no credentials, no DNS / email-provider setup. Pure content migration.

## Next Phase Readiness

- **Plan 03-03 (next wave-1 plan)** is unblocked. Per Plan 02 header `<context_note>`: Plans 01/02/03 all edit `app/page.tsx` serially (03 depends_on `[00, 01, 02]`). My AboutAndrew anchor edit in `4de7977` touched only lines 558 + 562 of a 789-line file; the Hero/Features/DownloadCTA blocks Plan 01 finalised and everything else Plan 03 will touch are unchanged. Zero merge-conflict hazard.
- **Plan 03-06 (a11y audit)** — the Playwright negative assertions added in this plan (absence of old Gmail on both privacy and homepage) are stable regression guards that Plan 06's broader a11y sweep can rely on as copy invariants.
- **Plan 03-07 / 03-08 (release-ready, deploy)** — `scripts/verify-production.sh` is now correct under the new data-controller contact, so the post-deploy verification step in the release workflow remains green at v1.0 launch.

## Self-Check: PASSED

Verified post-write:

- `test -f /Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/03-02-SUMMARY.md` — FOUND (written via Write tool).
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep 1b26373` — FOUND: "test(03-02): update email assertions for D-13 migration (RED)"
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep 8113e1e` — FOUND: "feat(03-02): migrate privacy controller contact to andrew@spatialmedialab.org (D-13)"
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep 4de7977` — FOUND: "feat(03-02): migrate AboutAndrew contact to andrew@spatialmedialab.org (D-13)"
- Site-wide sweep `grep -rn 'andrewjrahman@gmail.com' app/ components/` returns **0 matches** (tested in both repos).
- Final Playwright combined run: **22 passed** (9 privacy-content + 13 homepage-content).

---
*Phase: 03-personal-website*
*Plan: 02*
*Completed: 2026-04-16*
