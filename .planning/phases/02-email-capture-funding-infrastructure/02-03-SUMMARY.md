---
phase: 02-email-capture-funding-infrastructure
plan: 03
subsystem: legal-content
tags: [privacy-policy, gdpr, ccpa, uk-dpa, tally, nextjs, playwright, regression-tests]

# Dependency graph
requires:
  - phase: 02-email-capture-funding-infrastructure
    provides: "Plan 02 site scaffold — Next.js shell at andrewrahman-com repo with app/privacy/page.tsx skeleton, 9 H2 headings, and smoke.spec.ts (commit 20a81cf)"
provides:
  - "Fully-authored GDPR + CCPA + UK DPA privacy policy prose at andrewrahman.com/privacy (9 sections, no placeholders)"
  - "Andrew Rahman (natural person) as confirmed Data Controller with andrewjrahman@gmail.com as rights-request contact"
  - "Tally Technologies SRL (Belgium) explicitly named as data processor under GDPR Art. 28 with link to tally.so/privacy"
  - "Effective-dated (2026-04-16) legal content suitable for DIST-02 public-link requirement"
  - "Playwright content-regression suite (privacy-content.spec.ts) locking in 6 critical clauses so future edits cannot silently remove controller name, contact email, processor name, retention clause, jurisdictional coverage, or article citations"
affects:
  - "02-04-PLAN (hosting bake-off + DNS cutover) — privacy URL now has live content; Tally consent-checkbox link and Patreon footer link both have a real destination"
  - "02-05-PLAN (Tally form) — consent checkbox can now link to andrewrahman.com/privacy with real content"
  - "02-06-PLAN (Patreon) — Patreon footer can link to andrewrahman.com/privacy with real content"
  - "Phase 05 (launch newsletters) — Section 9 'Changes to this policy' commits controller to updating effective date and notifying list before newsletter processor is added"

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Content-regression testing: Playwright assertions on critical legal strings so silent removal fails CI"
    - "Three-jurisdiction policy authoring: GDPR (EU) + CCPA/CPRA (California) + UK GDPR/UK DPA, explicit article citations rather than generic rights language"

key-files:
  created:
    - "[site repo] tests/privacy-content.spec.ts — 6 Playwright regression tests (controller name, contact email, processor name, 3-jurisdiction coverage, retention clause, GDPR article enumeration)"
  modified:
    - "[site repo] app/privacy/page.tsx — replaced 9 `[Prose pending Plan 03.]` placeholders + the data-controller placeholder + the `[DATE TO BE CONFIRMED]` placeholder with final prose; also re-worded in-body GitHub link text to avoid Playwright strict-mode collision with footer link"

key-decisions:
  - "D-16 resolved (user override): Data Controller = Andrew Rahman (natural person), NOT Spatial Media Lab as plan default. Rationale: privacy policy governs personal newsletter email; site is personal identity; Patreon is personal identity; data chain must match. Plugin/source attribution to Spatial Media Lab is a separate PLUGIN-SIDE boundary and does not appear in the privacy policy body."
  - "Effective date = 2026-04-16 (today, per the YYYY-MM-DD convention in the plan)."
  - "In-body GitHub link text changed from 'github.com/Spatial-Media-Lab' to 'OpenSpatialDelay source code on GitHub' to prevent Playwright strict-mode role-query collision with the identical footer link — keeps smoke.spec.ts untouched (per user directive) while preserving SML attribution."

patterns-established:
  - "Data Controller line is a standalone top-of-page paragraph with explicit 'Data Controller:' label — makes the designation grep-able and regression-testable rather than buried inside the Who-we-are prose."
  - "Every legal claim is asserted by a Playwright content test so downstream edits cannot silently break compliance (T-02-10 mitigation)."

requirements-completed: [DIST-02]

# Metrics
duration: ~15 min
completed: 2026-04-16
---

# Phase 2 Plan 03: Privacy Policy Prose Summary

**Full GDPR + CCPA + UK DPA privacy policy authored at andrewrahman.com/privacy with Andrew Rahman (personal) as Data Controller, Tally Technologies SRL as named processor, and 6 Playwright regression tests locking in the critical clauses.**

## Performance

- **Started:** 2026-04-16T07:49Z (approx, GMT+2 = 09:49 local)
- **Completed:** 2026-04-16T07:53Z
- **Duration:** ~15 min
- **Tasks:** 2 executed (Task 1 pre-resolved by user; Task 2 and Task 3 executed)
- **Files modified/created:** 2 (one modified, one created in site repo)

## Accomplishments

- Replaced all 9 `[Prose pending Plan 03.]` placeholders with tailored prose describing the actual Phase 2 data flow (Tally form → Tally dashboard → manual newsletter starting Phase 5+).
- Policy explicitly addresses GDPR, CCPA/CPRA, and UK GDPR/UK DPA with article-level citations (GDPR Art. 6(1)(a), 7(3), 12(3), 15, 16, 17, 18, 20, 21; UK GDPR equivalents; CCPA/CPRA rights to know / delete / opt-out / non-discrimination).
- Named Tally Technologies SRL (Belgium) as data processor under GDPR Art. 28 with link to `tally.so/privacy`.
- Retention policy matches D-15 verbatim: "indefinitely until you ask us to delete" with unsubscribe-equals-deletion clause.
- Effective date populated with 2026-04-16 (today).
- 6-test Playwright content-regression suite passes; combined with Plan 02 smoke tests, total suite now runs 10 tests, all green.
- Built HTML (`out/privacy/index.html`) verified to contain every critical string and zero placeholders.

## Task Commits

Single squashed commit per the plan's execution rules (not one-per-task):

1. **Tasks 2 + 3 combined: authored prose + added content-regression tests** — `c84864c` (feat) in site repo at `AndrewRahman/andrewrahman-com`, pushed to origin/main.

**OSD-repo metadata commit:** [added after this SUMMARY is written].

_Note: The execution-rules header explicitly mandated a single commit in the site repo with the message `feat(privacy): author full GDPR+CCPA+UK DPA prose`, which is what was created._

## Files Created/Modified

- `[site repo] app/privacy/page.tsx` — 9 GDPR section prose + Data Controller line + effective date; in-body GitHub link text re-worded to avoid Playwright strict-mode collision with footer link.
- `[site repo] tests/privacy-content.spec.ts` — 6 Playwright content-regression tests.

## Decisions Made

- **D-16 final answer: Andrew Rahman (natural person).** The plan's default recommendation was Spatial Media Lab; the user overrode this on 2026-04-16, recorded in 02-CONTEXT.md commit `6fc53c4`. Rationale: the privacy policy governs emails collected via andrewrahman.com, stored in Tally, used in Andrew's personal newsletter — the site, the Patreon (D-21), and the newsletter are all under personal identity, so the Data Controller must match that chain. The Spatial Media Lab boundary is PLUGIN-SIDE (GPL-3.0 LICENSE, legal notices, source-code attribution) and intentionally does not appear in the privacy policy. Contact email for rights requests: `andrewjrahman@gmail.com`.
- **Effective date = today (2026-04-16)** per the plan's YYYY-MM-DD convention. Section 9 ("Changes to this policy") commits the controller to bumping this date on any material change and notifying the list before a new processor is introduced — provides a verifiable audit trail (T-02-08 mitigation).
- **Policy does not claim EU-only storage absolutely** — phrased as "To the best of our knowledge, EU form submissions are stored within the EEA" and "We do not intentionally transfer your data outside the EEA or the UK to jurisdictions that lack an adequacy decision." This honours T-02-11 by not asserting more than Plan 04's Tally-region UAT will confirm.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical / User Override] Data-Controller designation overridden from plan default**
- **Found during:** Pre-Task-2 (Task 1 pre-resolved by user 2026-04-16)
- **Issue:** Plan frontmatter declared `contains: "Spatial Media Lab"` on `site/app/privacy/page.tsx` and `pattern: "Data Controller.*Spatial Media Lab"` in key_links — reflecting the planner's recommended default. The user explicitly overrode this after re-examining the identity chain (site = personal, Patreon = personal per D-21, newsletter = personal) and chose Andrew Rahman (natural person) as Data Controller.
- **Fix:** Authored the Data Controller line and the Who-we-are section naming "Andrew Rahman" as Data Controller rather than "Spatial Media Lab". Updated the privacy-content Playwright test to assert `Andrew Rahman` + `Data Controller:` instead of the plan-spec's generic controller assertion.
- **Files modified:** `site/app/privacy/page.tsx`, `site/tests/privacy-content.spec.ts`
- **Verification:** Test `names Andrew Rahman as the confirmed data controller (D-16)` passes; `grep "Data Controller:" out/privacy/index.html` returns 1; `grep "Andrew Rahman" out/privacy/index.html` returns 1.
- **Committed in:** `c84864c` (site repo).

**2. [Rule 1 - Bug, caused by this task's changes] Smoke test strict-mode collision with new in-body GitHub link**
- **Found during:** Task 3 verification (`npm run test`)
- **Issue:** The newly-authored Who-we-are section linked the text "github.com/Spatial-Media-Lab" to the GitHub organisation. The existing Plan 02 smoke test `footer present on every page with github + privacy links` uses `page.getByRole('link', { name: /github\.com\/Spatial-Media-Lab/ }).toBeVisible()` in Playwright strict mode, which failed because my new policy link AND the footer link both matched the same accessible name.
- **Fix:** Re-worded the in-body link's accessible text to "OpenSpatialDelay source code on GitHub". `href` still points to `https://github.com/Spatial-Media-Lab/OpenSpatialDelay`, so Spatial Media Lab attribution (D-02) is preserved. `smoke.spec.ts` was NOT modified (per user directive in the execution-rules block).
- **Files modified:** `site/app/privacy/page.tsx` (link text only).
- **Verification:** All 10 tests now pass (4 smoke + 6 privacy-content).
- **Committed in:** `c84864c` (same commit as Task 2 prose; squashed per execution rules).

---

**Total deviations:** 2 auto-fixed (1 user-driven override of planner default, 1 regression caused by this task's own changes).
**Impact on plan:** Both fixes were mandatory — deviation 1 reflects an explicit user decision that was already recorded in CONTEXT.md before execution began; deviation 2 is a Rule-1 self-caused regression whose fix was the minimum-scope edit consistent with the "do not modify smoke.spec.ts" directive. No scope creep.

## Issues Encountered

None. Playwright test collision was discovered on first full test run and resolved immediately.

## Legal-review Follow-ups Identified During Task 1 Checkpoint

The user resolved Task 1 outside this agent run; the only follow-up recorded is:

- **Plan 04 UAT checkpoint for Tally region setting (T-02-11):** Plan 04's manual UAT **must** confirm Tally's EU data-region is active before DNS cutover. If Tally submissions land in the US by default, the Section-4 clause "EU form submissions are stored within the EEA" must be softened before the site goes live under the custom domain. This is already captured in the Plan 03 threat model; surfaced here so Plan 04 cannot forget it.

## Confirmation for Plan 04 Readiness

- `andrewrahman.com/privacy` now has live, fully-authored content (currently served from the dev scaffold; will ride to production as part of Plan 04's hosting cutover).
- The Tally consent-checkbox link destination and the Patreon footer link destination both have a real page to point at.
- Plan 04 may proceed with the Netlify vs Vercel bake-off (D-18) and DNS cutover (D-19) without blocking on privacy-copy work.

## User Setup Required

None — no external service configuration required. Tally consent-checkbox URL wiring and Patreon footer URL wiring are handled in Plans 05 and 06 respectively, both of which now have a live URL to point at.

## Next Phase Readiness

- DIST-02 is functionally complete (prose authored, all 9 GDPR sections filled, three jurisdictions covered, rights-contact published, processor named). Final DIST-02 closure will occur in Plan 04 when the privacy URL is live under the `andrewrahman.com` custom domain.
- Plan 04 (hosting bake-off + DNS) is unblocked.
- Plan 05 (Tally form) is unblocked — consent checkbox can now link to the authored policy.
- Plan 06 (Patreon) is unblocked — page footer can now link to the authored policy.

## Self-Check

**Files verified to exist:**
- FOUND: `/Users/andrewrahman/conductor/repos/andrewrahman-com/app/privacy/page.tsx`
- FOUND: `/Users/andrewrahman/conductor/repos/andrewrahman-com/tests/privacy-content.spec.ts`
- FOUND: `/Users/andrewrahman/conductor/repos/andrewrahman-com/out/privacy/index.html`

**Commit verified to exist:**
- FOUND: `c84864c` in `AndrewRahman/andrewrahman-com` (pushed to origin/main)

**Acceptance criteria:**
- All 9 `[Prose pending Plan 03.]` placeholders removed: PASS
- `[Data-controller designation pending user confirmation.]` removed: PASS
- `[DATE TO BE CONFIRMED BEFORE PUBLISHING]` removed (replaced with 2026-04-16): PASS
- Critical strings in built HTML (Andrew Rahman, andrewjrahman@gmail.com, Tally Technologies SRL, CCPA/CPRA, UK GDPR, GDPR Art. 15/17/20, indefinitely until you ask us to delete, Spatial Media Library, supervisory authority, tally.so/privacy, Data Controller:): ALL PRESENT
- `npm run build` exit 0: PASS
- `npm run test` exit 0 with 10 tests passing (4 smoke + 6 privacy-content): PASS

## Self-Check: PASSED

---
*Phase: 02-email-capture-funding-infrastructure*
*Completed: 2026-04-16*
