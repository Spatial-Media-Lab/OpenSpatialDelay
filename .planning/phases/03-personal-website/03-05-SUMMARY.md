---
phase: 03-personal-website
plan: 05
subsystem: web-email-capture
tags: [next.js, sender.net, csp, playwright, tdd, gdpr, privacy-policy]

# Dependency graph
requires:
  - phase: 03-personal-website
    plan: 00
    provides: "D-14 disposition LOCKED as Option B (Phase 3 absorbs Sender.net migration)"
  - phase: 03-personal-website
    plan: 04
    provides: "DownloadCTA Link pattern + Hero CTA layout (post-screenshot-swap state)"
provides:
  - "components/EmailCaptureSection.tsx — shared inline/standalone D-14 section"
  - "Homepage #get-osd inline section between DownloadCTA and PatreonCTA"
  - "/get-osd/ route reduced to backlink shim (single line: <EmailCaptureSection standalone headingLevel=\"h1\" />)"
  - "Nav + Hero + DownloadCTA Download OSD CTAs anchor-scroll via href=\"#get-osd\""
  - "_headers CSP migrated: tally.so → cdn.sender.net + *.sender.net (script/frame/connect/form)"
  - "privacy/page.tsx processor swapped: Tally Technologies SRL → UAB Sender.lt + 2026-04-17 changelog entry"
  - ".env.example migrated: NEXT_PUBLIC_TALLY_FORM_ID → NEXT_PUBLIC_SENDER_FORM_ID"
  - "tests/homepage-content.spec.ts +2 tests (#get-osd presence + anchor CTA hrefs)"
  - "tests/smoke.spec.ts updated (hero CTA regex + backlink-shim heading assertion)"
  - "tests/privacy-content.spec.ts renamed+replaced processor test + new negative-Tally assertion"
affects: [03-06, 03-07, 03-08]

# Tech tracking
tech-stack:
  added: []  # No new deps — Sender.net universal.js is CDN-loaded at runtime, not npm
  patterns:
    - "Shared dual-mode section component (standalone=article w/ h1, inline=section w/ h2) — preserves axe page-has-heading-one on both routes"
    - "Client-side useEffect <script> injector with cleanup (CDN form loader)"
    - "Anchor-scroll CTA (href=\"#get-osd\") replaces Next <Link href=\"/route/\"> for inline-section scroll — leverages Tailwind default scroll-smooth on <html>"
    - "Negative-assertion Playwright pattern for processor migration (no legacy name renders + no legacy subdomain renders)"
    - "Changelog-without-legacy-name: processor change paragraph deliberately omits 'Tally' so negative-assertion stays green (git log = audit trail)"

key-files:
  created:
    - "../andrewrahman-com/components/EmailCaptureSection.tsx (55 lines, shared section)"
  modified:
    - "../andrewrahman-com/app/page.tsx (inline EmailCaptureSection + 2 Hero/DownloadCTA anchor swaps + Link import removed)"
    - "../andrewrahman-com/app/get-osd/page.tsx (68 lines → 5 lines shim)"
    - "../andrewrahman-com/components/Nav.tsx (Nav Download OSD Link → <a href=\"#get-osd\">)"
    - "../andrewrahman-com/_headers (CSP: 4 directives migrated Tally → Sender.net subdomains)"
    - "../andrewrahman-com/app/privacy/page.tsx (processor paragraph + jurisdiction blurb + 2026-04-17 changelog entry)"
    - "../andrewrahman-com/tests/homepage-content.spec.ts (+2 tests)"
    - "../andrewrahman-com/tests/smoke.spec.ts (hero CTA regex + shim heading)"
    - "../andrewrahman-com/tests/privacy-content.spec.ts (renamed processor test + added negative-Tally test)"
    - "../andrewrahman-com/.env.example (TALLY_FORM_ID → SENDER_FORM_ID)"

key-decisions:
  - "D-14 Option B executed as planned: zero Tally references remain in app/, components/, _headers, or .env.example (only the negative-assertion test guards remain)"
  - "Plan 03-05 went beyond the plan's listed files: .env.example added as Rule 2 auto-fix (stale env-var docs would mislead deployers)"
  - "SENDER_SCRIPT URL = https://cdn.sender.net/accounts_resources/universal.js — sourced from Phase 2 Plan 02-05 (incomplete but URL documented in plan + 03-RESEARCH.md §A1). Live HTTP verification NOT possible in this sandbox (outbound network blocked — curl -I returned 'Couldn't connect to server' for both cdn.sender.net and www.sender.net); URL is the Sender.net documented pattern. User should validate in production on first real deploy with NEXT_PUBLIC_SENDER_FORM_ID set."
  - "Nav.tsx keeps Link import for the home 'Andrew Rahman' link; page.tsx Link import dropped (no remaining <Link> usage after two CTA rewrites)"
  - "Changelog paragraph deliberately omits 'Tally' so negative-assertion test stays green (same pattern Plan 03-02 established for Gmail → spatialmedialab.org migration per STATE.md note)"
  - "Task 2 TDD gate enforced: test(03-05) RED commit (8d2fde3) landed before feat(03-05) GREEN commit (d666772)"

requirements-completed: []  # WEB-01 still pending — closes with Plan 03-06 (a11y) + Plan 03-04 (assets, already done)

# Metrics
duration: 6min
completed: 2026-04-16
---

# Phase 3 Plan 05: D-14 EmailCaptureSection + Sender.net Migration Summary

**D-14 Option B executed end-to-end: shared EmailCaptureSection component drives homepage inline #get-osd section + /get-osd shim + 3 anchor CTA rewrites + full Sender.net CSP/processor/env-var swap; zero Tally references remain anywhere in the site repo (only negative-assertion test guards).**

<post-task-note>
SENDER_SCRIPT verified as https://cdn.sender.net/accounts_resources/universal.js — URL sourced from (a) Phase 2 Plan 02-05-PLAN.md lines 58/71/87/233/239 which references "cdn.sender.net form bundle" + CSP `https://cdn.sender.net` script-src directive, (b) 03-RESEARCH.md §A1 which documents this as the Sender.net embed CDN, (c) 03-RESEARCH.md §Evidence point 1 citing sender.net/help "universal.js" pattern. Live HTTP verification WAS NOT possible in the execution environment: `curl -I https://cdn.sender.net/accounts_resources/universal.js` returned "Couldn't connect to server" (sandbox has no outbound network access — same failure on `https://www.sender.net/`). Rule 3 deviation: proceeded with the phase-2-and-research-documented URL; user must validate at first production deploy with NEXT_PUBLIC_SENDER_FORM_ID set.
</post-task-note>

## Option B Executed (as planned)

D-14 disposition was locked as Option B in Plan 03-00 (see 03-00-SUMMARY.md §D-14 Disposition). This plan:

1. Created `components/EmailCaptureSection.tsx` with Sender.net `universal.js` loader + dual-mode render (inline section / standalone article) + conditional headingLevel prop.
2. Embedded `<EmailCaptureSection />` inline on homepage between `<DownloadCTA />` and `<PatreonCTA />`.
3. Rewrote 3 Download OSD CTAs (Nav sticky + Hero + DownloadCTA section) to anchor `href="#get-osd"` from the prior `<Link href="/get-osd/">` pattern.
4. Reduced `/get-osd/page.tsx` from 68-line Tally loader to 5-line shim that re-renders `<EmailCaptureSection standalone headingLevel="h1" />`. Backlink safety preserved; axe `page-has-heading-one` stays green because the standalone mode keeps a single `<h1>`.
5. Migrated `_headers` CSP: removed `https://tally.so`/`https://widgets.tally.so`; added `https://cdn.sender.net` (script-src), `https://*.sender.net` (frame/connect/form), `https://api.sender.net` (connect).
6. Swapped processor paragraph in `app/privacy/page.tsx` from "Tally.so (Tally Technologies SRL, Belgium)" to "Sender.net (UAB Sender.lt, Vilnius, Lithuania)"; updated jurisdiction blurb ("Belgium" → "Lithuania"); swapped external privacy-policy link; appended 2026-04-17 changelog paragraph.
7. Updated `tests/privacy-content.spec.ts`: renamed processor test to UAB Sender.lt; appended negative test asserting zero `Tally Technologies SRL` / `tally.so` references.
8. Updated `tests/homepage-content.spec.ts`: +2 tests (inline `#get-osd` section presence + Download OSD CTAs use `#get-osd` anchor).
9. Updated `tests/smoke.spec.ts`: rewrote hero-CTA regex `/\/get-osd/` → `/^#get-osd$/`; replaced Tally-placeholder assertion with backlink-shim heading assertion.
10. Fixed `.env.example` (Rule 2 auto-fix): `NEXT_PUBLIC_TALLY_FORM_ID=REPLACE_ME` → `NEXT_PUBLIC_SENDER_FORM_ID=REPLACE_ME`.

## Performance

- **Duration:** ~6 min
- **Started:** 2026-04-16T22:53:56Z
- **Completed:** 2026-04-16T23:00:14Z
- **Tasks:** 3 of 3 complete (+ 1 Rule 2 auto-fix)
- **Files changed:** 10 (1 created + 9 modified, all in sibling repo + 1 more in plugin repo after this commit)
- **Commits:** 5 in `../andrewrahman-com`

## Accomplishments

- Shared `EmailCaptureSection` component (55 lines) supports both modes with single `headingLevel` prop — single source of truth for email capture UI.
- 3 Download OSD CTAs consistently anchor-scroll to the inline section; Tailwind's default `scroll-smooth` on `<html>` handles the transition with zero JS.
- `/get-osd/` route preserved as 5-line shim (down from 68-line Tally page) — every pre-existing backlink, privacy-policy reference, and outbound link keeps returning HTTP 200 with the same h1.
- CSP migrated with no `'unsafe-inline'` additions (defensive — script-src stays strict 'self' + single subdomain).
- Processor paragraph + jurisdiction blurb ("Belgium" → "Lithuania") both updated in one edit; changelog entry added without reintroducing the legacy name (keeps negative-assertion green).
- `.env.example` migrated in same plan (Rule 2 catch); deploy docs now match runtime expectation.
- TDD gate enforced for Task 2: RED commit `8d2fde3` (3 failing tests) → GREEN commit `d666772` (all 23 homepage-content + smoke tests pass).

## Task Commits

Per-task atomic commits in sibling `../andrewrahman-com` repo:

1. **Task 1** (create component): `890461a` — `feat(03-05): create EmailCaptureSection component (Sender.net universal.js loader)` (1 file, 55 insertions)
2. **Task 2 RED**: `8d2fde3` — `test(03-05): add failing tests for D-14 inline #get-osd section + shim route` (2 files, 20 insertions/7 deletions)
3. **Task 2 GREEN**: `d666772` — `feat(03-05): wire EmailCaptureSection inline + reduce /get-osd/ to shim (D-14)` (3 files, 13 insertions/75 deletions)
4. **Task 3** (CSP + privacy migration): `fdcec41` — `feat(03-05): migrate CSP + privacy processor Tally -> Sender.net (D-14 Option B)` (3 files, 30 insertions/15 deletions)
5. **Rule 2 auto-fix**: `986a898` — `chore(03-05): migrate .env.example env var TALLY_FORM_ID -> SENDER_FORM_ID` (1 file, 2 insertions/2 deletions)

**Plan metadata commit (openspatialdelay repo):** pending — created by final-commit step below (SUMMARY.md + STATE.md + ROADMAP.md).

## TDD Gate Compliance

Task 2 was marked `tdd="true"`. Gate sequence verified in `git -C ../andrewrahman-com log`:

1. RED gate: commit `8d2fde3` — subject prefix `test(03-05)`, diff is tests-only (`tests/homepage-content.spec.ts`, `tests/smoke.spec.ts`).
2. GREEN gate: commit `d666772` — subject prefix `feat(03-05)`, diff is implementation-only (`app/page.tsx`, `app/get-osd/page.tsx`, `components/Nav.tsx`).
3. REFACTOR gate: none needed (GREEN passed cleanly; no cleanup commit required).

RED verification: after `git stash` of impl and rebuild, `npx playwright test tests/homepage-content.spec.ts:146 :151 tests/smoke.spec.ts:3 :33` returned `3 failed, 1 passed` — matches expected RED state (2 new homepage-content tests + rewritten smoke hero CTA test all fail against un-updated source; shim-heading test passes because old Tally page still has its own h1).

GREEN verification: after `git stash pop` and rebuild, `npx playwright test tests/homepage-content.spec.ts tests/smoke.spec.ts` returned `23 passed (3.0s)`.

Tasks 1 and 3 were marked `tdd="false"`; standard single-commit flow applied.

## Files Created/Modified

Sibling repo `../andrewrahman-com/`:

- `components/EmailCaptureSection.tsx` (NEW, 55 lines) — shared component (`'use client'`, useEffect CDN script loader, conditional wrapper element + class, `headingLevel` prop, env-var placeholder state).
- `app/page.tsx` (MODIFIED) — `+import { EmailCaptureSection }`; `-import Link`; inserted `<EmailCaptureSection />` between `<DownloadCTA />` and `<PatreonCTA />`; Hero CTA Link→`<a href="#get-osd">`; DownloadCTA button Link→`<a href="#get-osd">`.
- `app/get-osd/page.tsx` (MODIFIED, 68→5 lines) — pure shim rendering `<EmailCaptureSection standalone headingLevel="h1" />`; dropped `'use client'` + useEffect + TALLY_FORM_ID + noscript block.
- `components/Nav.tsx` (MODIFIED) — Download OSD `<Link href="/get-osd/">` → `<a href="#get-osd">`. Home `<Link href="/">` Andrew Rahman link preserved.
- `_headers` (MODIFIED) — CSP: `script-src https://cdn.sender.net`; `frame-src https://cdn.sender.net https://*.sender.net`; `connect-src 'self' https://api.sender.net https://*.sender.net`; `form-action 'self' https://*.sender.net`.
- `app/privacy/page.tsx` (MODIFIED) — processor paragraph swap (3 sentences rewritten); jurisdiction blurb Belgium→Lithuania; external privacy-policy URL tally.so/privacy→www.sender.net/privacy-policy; 2026-04-17 changelog entry appended.
- `tests/homepage-content.spec.ts` (MODIFIED) — +2 tests (13→15 tests total in file; was 21/21, now 23/23 including one smoke file).
- `tests/smoke.spec.ts` (MODIFIED) — hero CTA regex `/\/get-osd/` → `/^#get-osd$/`; third test body replaced (Tally-placeholder → backlink-shim heading).
- `tests/privacy-content.spec.ts` (MODIFIED) — processor test renamed + assertion Tally Technologies SRL→UAB Sender.lt; +1 negative-Tally test (9→10 tests).
- `.env.example` (MODIFIED) — `NEXT_PUBLIC_TALLY_FORM_ID` → `NEXT_PUBLIC_SENDER_FORM_ID` + comment updated to point at Sender.net dashboard.

Plugin repo `/Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/`:

- `03-05-SUMMARY.md` (NEW, this file).

## Decisions Made

- **Task 3 .env.example fix** (Rule 2 auto-fix) — stale env-var docs would mislead deployers. Added to plan scope rather than deferred.
- **Nav.tsx Link import retained** — the home "Andrew Rahman" link still uses `<Link href="/">` for Next client-side routing efficiency. Only the Download OSD CTA converted to `<a href="#get-osd">`.
- **page.tsx Link import dropped** — both CTAs that previously used Link are now `<a href="#get-osd">`, leaving Link unused. Removed to keep the build warning-free.
- **Heading level split** — inline homepage uses default `<h2>` (Hero owns the `<h1>`); standalone `/get-osd` explicitly passes `headingLevel="h1"`. Keeps axe `page-has-heading-one` green on both routes (Plan 06 scope).
- **Changelog text discipline** — new 2026-04-17 paragraph mentions "Sender.net (UAB Sender.lt, Vilnius, Lithuania)" but NOT the legacy name. Follows the pattern Plan 03-02 established for the Gmail→spatialmedialab.org migration (STATE.md decision record). Keeps the negative-assertion test (`no legacy Tally references remain in privacy policy`) green; git log is the change audit trail.
- **SENDER_SCRIPT URL not live-verified** — the sandbox has no outbound network. URL is sourced from Phase 2 Plan 02-05 plan text and 03-RESEARCH.md §A1 + §Evidence point 1. Documented as a post-task-note above. Rule 3 deviation (missing live verification flagged explicitly rather than silently proceeding).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Critical] Migrate .env.example to SENDER_FORM_ID**
- **Found during:** Task 3 verification sweep (grep for `tally` across repo after CSP/privacy edits).
- **Issue:** `.env.example` still documented `NEXT_PUBLIC_TALLY_FORM_ID=REPLACE_ME` with a "Tally form ID" comment. This would mislead anyone deploying from the template after D-14 Option B landed.
- **Fix:** Swapped to `NEXT_PUBLIC_SENDER_FORM_ID=REPLACE_ME` and updated the comment to point at the Sender.net dashboard path.
- **Files modified:** `../andrewrahman-com/.env.example`.
- **Commit:** `986a898`.

**2. [Rule 1 - Bug] Update smoke.spec.ts hero-CTA href regex for #get-osd**
- **Found during:** Task 2 Step A (review of existing smoke.spec.ts before adding new tests).
- **Issue:** The plan Step A only specified replacing the Tally-placeholder test, but the very first smoke test (`homepage renders hero headline + Download CTA pointing at /get-osd/`) had its own `toHaveAttribute('href', /\/get-osd/)` assertion that would have failed once the hero CTA was rewritten to `#get-osd`. Plan wording did not explicitly cover this. Without this fix, the full smoke suite would have gone red on GREEN commit.
- **Fix:** Updated hero-CTA test title + regex to `/^#get-osd$/` in the same RED commit as the new tests.
- **Files modified:** `../andrewrahman-com/tests/smoke.spec.ts` (one extra test already covered by the plan's files_modified list).
- **Commit:** `8d2fde3` (folded into Task 2 RED — no separate commit).

**3. [Rule 3 - Blocker → mitigated] Cannot live-verify SENDER_SCRIPT URL**
- **Found during:** Task 1 action step (plan requires `curl -I` HTTP/2 200 check before committing SENDER_SCRIPT).
- **Issue:** Execution environment has no outbound network access (`curl: (7) Failed to connect to server` for both `cdn.sender.net` and `www.sender.net`). Cannot satisfy the plan's live-verification requirement verbatim.
- **Mitigation:** Used the Sender.net documented URL pattern (confirmed in Phase 2 Plan 02-05-PLAN.md references and 03-RESEARCH.md §A1 + §Evidence). Documented the skipped live check explicitly in both the `<post-task-note>` at the top of this SUMMARY and this Deviations section. User to re-verify on first production deploy.
- **Files modified:** None (no code change beyond committing the documented URL).
- **Commit:** `890461a` (Task 1) committed with URL as-planned.

### Authentication Gates

None — Sender.net dashboard access was not required (the plan's URL-verification flow was planned for local `curl -I`, not dashboard login). The `NEXT_PUBLIC_SENDER_FORM_ID` env var is a deployment-time setting, not a build-time requirement; the placeholder card in EmailCaptureSection renders cleanly without it.

## Issues Encountered

- **Playwright serves from `out/` export** — the first test run after source edits failed because `npx serve out` was serving the pre-edit static bundle. Resolved by running `npm run build` before each test cycle. Surface noted: the webServer launches fresh per test run (no running server on port 3000 between invocations), so each edit cycle requires `build` → `test`.

## Deferred Issues

- **a11y tests (3 failures on `tests/a11y.spec.ts`)** — `/`, `/privacy`, `/get-osd` all fail axe color-contrast checks (`--text-dim #6d7279` on `--bg-void #06090f` = 4.11:1, WCAG AA requires 4.5:1). This is explicitly Plan 03-06 scope per 03-00-SUMMARY.md §Next Phase Readiness: "Plan 03-06 (a11y audit) — unblocked; tests/a11y.spec.ts is the entry point. Expected initial failures on `--text-dim` on `--bg-void` / `--bg-panel` borderline pairs — Plan 06 will nudge tokens." Not regressed by Plan 05 (pre-existing).

## User Setup Required

- **`NEXT_PUBLIC_SENDER_FORM_ID`** — deploy-time environment variable. Source: Sender.net Dashboard → Forms → select form → ID on embed code. Until set, both the inline homepage section and the `/get-osd` shim render the "Sender.net form ID not yet configured" placeholder card. No subscriber capture until this is populated.
- **Sender.net embedded form + DOI template** — must exist in the Sender.net dashboard (Phase 2 D-09 Option A: download buttons embedded in DOI confirmation email). If not already created, user follows `02-CONTEXT.md` Sender.net flow to provision.

## Next Phase Readiness

- **Plan 03-06 (a11y audit)** — unblocked. The 3 expected a11y failures are the entry point; Plan 06 will nudge `--text-dim` to achieve 4.5:1 on `--bg-void`. Plan 05 did not introduce new a11y regressions (inline `<EmailCaptureSection />` uses the same tokens as surrounding DownloadCTA/PatreonCTA).
- **Plan 03-07 (OG metadata verification)** — unblocked (independent of this plan).
- **Plan 03-08 (launch checklist)** — unblocked. Email-capture infrastructure complete pending the Sender.net dashboard config.

## Threat Flags

None — this plan removed threat surface (Tally embed) rather than adding it. CSP tightened: `script-src` now allows exactly ONE external origin (`cdn.sender.net`) vs. the previous two (`tally.so` + `widgets.tally.so`). No new network endpoints, auth paths, or schema changes.

## Self-Check: PASSED

Verified post-write (2026-04-16T23:00Z):

- `test -f /Users/andrewrahman/conductor/repos/andrewrahman-com/components/EmailCaptureSection.tsx` → FOUND
- `grep -q 'cdn.sender.net' /Users/andrewrahman/conductor/repos/andrewrahman-com/_headers` → FOUND
- `grep -q 'UAB Sender.lt' /Users/andrewrahman/conductor/repos/andrewrahman-com/app/privacy/page.tsx` → FOUND
- `grep -q 'NEXT_PUBLIC_SENDER_FORM_ID' /Users/andrewrahman/conductor/repos/andrewrahman-com/.env.example` → FOUND
- `grep -c 'tally' /Users/andrewrahman/conductor/repos/andrewrahman-com/_headers` → 0 (Tally purged)
- `grep -c 'Tally Technologies' /Users/andrewrahman/conductor/repos/andrewrahman-com/app/privacy/page.tsx` → 0 (Tally purged from privacy)
- `git -C ../andrewrahman-com log --oneline | grep 890461a` → FOUND: feat(03-05): create EmailCaptureSection component
- `git -C ../andrewrahman-com log --oneline | grep 8d2fde3` → FOUND: test(03-05): add failing tests
- `git -C ../andrewrahman-com log --oneline | grep d666772` → FOUND: feat(03-05): wire EmailCaptureSection inline + shim
- `git -C ../andrewrahman-com log --oneline | grep fdcec41` → FOUND: feat(03-05): migrate CSP + privacy
- `git -C ../andrewrahman-com log --oneline | grep 986a898` → FOUND: chore(03-05): migrate .env.example
- Playwright final run: `51 passed, 3 failed` (3 failures are pre-existing a11y tests deferred to Plan 06, not regressions from Plan 05)
- Build: `next build` clean on all 4 routes (`/`, `/_not-found`, `/get-osd`, `/privacy`) with Sender.net migration applied

---
*Phase: 03-personal-website*
*Plan: 05*
*Completed: 2026-04-16*
