---
phase: 05-launch-announcements
plan: 00
subsystem: launch-coordination
tags: [launch, bash, checklist, runbook, smoke-script, dkim, spf, mailchimp, kvr]

# Dependency graph
requires:
  - phase: 03-website-rebuild
    provides: andrewrahman.com/get-osd landing page + Sender.net capture flow (URL target of smoke script)
  - phase: 02-email-capture-funding
    provides: Patreon/AndrewRahman live (Phase 5 CTA target) + privacy policy (review-batch fallback address sender identity)
provides:
  - scripts/phase5-smoke.sh — one-command URL smoke over all 5 hard launch-day URLs (exit 0 on all-pass, supports KVR_URL + SML_BLOG_POST_URL env overrides)
  - launch-precheck.md — 16-checkbox T-7-through-T-1 precondition checklist across 6 sections + 4 escalation paths
  - launch-day-runbook.md — 8-step T-0 sequence inside the 2-hour 10:00–12:00 CET window + 4 contingency branches
  - T-11 precheck decisions: Mailchimp owner=Andrew (resolved), SPF=verified, DKIM+DMARC=pending HITL at jackhost.net, KVR Developer Account=no-account (apply + fallback drafted), Phase 4=plan committed by 2026-04-20
affects: [05-02-newsletter, 05-04-instagram, 05-05-kvr-press, 05-06-review-outreach, 05-07-berlin-dm, 05-08-forum-posts]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "URL smoke pattern: bash + curl -fsIL --max-time 10 with hard vs. env-optional URL split"
    - "Precheck-walk pattern: checkbox lines with dated note sub-bullets as in-file audit trail"
    - "Runbook pattern: minute-anchored T-0 ordering + always-paired contingency branches"

key-files:
  created:
    - scripts/phase5-smoke.sh
    - .planning/phases/05-launch-announcements/launch-precheck.md
    - .planning/phases/05-launch-announcements/launch-day-runbook.md
  modified:
    - .planning/phases/05-launch-announcements/launch-precheck.md (T-11 precheck walk decisions)

key-decisions:
  - "Phase 4 (demo content) NOT descoped — Andrew committed to plan 04-00 by 2026-04-20. IG Reel remains the primary 05-04 asset format."
  - "SML Mailchimp audience owner = Andrew (self). 05-02 newsletter drafts for direct send by Andrew — no delegation required."
  - "SPF verified present on spatialmedialab.org: v=spf1 a include:spf.jackhost.net -all."
  - "DKIM + DMARC absent at default._domainkey / google._domainkey. Andrew handling setup at jackhost.net as HITL. Target: verified by 2026-04-25 (T-3)."
  - "From-address decision deferred to T-3: 05-06 will draft BOTH andrew@spatialmedialab.org AND andrewjrahman@gmail.com variants. Final selection after test-send Authentication-Results inspection."
  - "KVR Developer Account = no account yet. Andrew applying at kvraudio.com/developer_application.php. 05-05 drafts BOTH Developer DB listing metadata AND contactus@kvraudio.com press-release email so launch is not blocked on application turnaround."

patterns-established:
  - "HITL gate deferral: when a blocker requires DNS/external account work outside the executor's control, document as dated note + split downstream plans into dual variants so the launch day is not gated on a single resolution path."
  - "Precheck ticking discipline: a decision or mitigation is NOT the same as the blocker being clear — only tick [x] when the underlying precondition is verified true."

requirements-completed: []  # 05-00 is the Wave-0 coordination scaffold; ANNC-01..06 are delivered by downstream plans 05-01..05-08. Plan frontmatter was optimistic — actual requirement completion will be stamped by each individual delivery plan.

# Metrics
duration: ~90min (including checkpoint pause for precheck walk)
completed: 2026-04-17
---

# Phase 05 Plan 00: Launch Orchestration Scaffold Summary

**Three coordination artefacts (smoke script + precheck checklist + T-0 runbook) plus T-11 precheck-walk decisions that closed 3 of 4 [UNKNOWN] rows and defined HITL paths for the remaining 2 gates (DKIM/DMARC at jackhost.net, KVR Developer Account application).**

## Performance

- **Duration:** ~90 min (including human-verify checkpoint)
- **Started:** 2026-04-17T15:43:22Z (Phase 5 execute-phase kickoff)
- **Completed:** 2026-04-17T16:00:00Z (checkpoint resolved)
- **Tasks:** 4 (3 auto + 1 checkpoint:human-verify)
- **Files modified:** 3 created

## Accomplishments

- Shipped `scripts/phase5-smoke.sh` — one-command URL smoke covering the 5 hard launch-day URLs with env-optional KVR_URL + SML_BLOG_POST_URL overrides; executable, documented, exit-code correct.
- Shipped `launch-precheck.md` — 16 checkboxes across 6 sections (hard blockers, email deliverability, external accounts, URL smoke, Berlin DM window, owner/cadence) plus 4 escalation paths; structured for daily walk from 2026-04-21 (T-7) onward.
- Shipped `launch-day-runbook.md` — minute-anchored 8-step T-0 sequence (10:00–12:00 CET) with pre-window checklist, post-window tasks, and 4 contingency branches (WordPress fail, Mailchimp fail, LinkedIn throttle, KVR rejection).
- Walked the precheck with Andrew live on 2026-04-17 (T-11): closed Mailchimp-owner row (= Andrew self), closed SPF row (verified), defined DKIM+DMARC HITL target (2026-04-25 T-3), defined KVR no-account dual-path (apply + press-release fallback), confirmed Phase 4 commitment (plan 04-00 by 2026-04-20).

## Task Commits

Each task was committed atomically:

1. **Task 1: Write scripts/phase5-smoke.sh — URL curl loop** — `412b4b3` (feat)
2. **Task 2: Write launch-precheck.md — T-7 through T-1 precondition checklist** — `83c6954` (feat)
3. **Task 3: Write launch-day-runbook.md — T-0 8-step sequence** — `86be2dd` (feat)
4. **Task 4: Checkpoint — walk launch-precheck.md with Andrew, close [UNKNOWN] rows** — `a81ecab` (docs)

**Plan metadata:** _pending this commit_ (docs: complete plan)

## Files Created/Modified

- `scripts/phase5-smoke.sh` — Executable bash smoke script hitting 5 hard launch URLs via `curl -fsIL`, with env-optional KVR_URL + SML_BLOG_POST_URL appends, PASS/FAIL per-URL output, summary line, exit 0 only if all hard URLs pass.
- `.planning/phases/05-launch-announcements/launch-precheck.md` — Andrew's daily-walk checklist from T-7 through T-0. Created Task 2, updated Task 4 with 6 dated decision notes from the T-11 walk.
- `.planning/phases/05-launch-announcements/launch-day-runbook.md` — Andrew's 2026-04-28 runbook: pre-window tab setup, 8 minute-anchored steps, post-window tasks, 4 contingency branches, evidence-requirement note.

## Decisions Made

**T-11 precheck walk (2026-04-17) — captured as dated notes in launch-precheck.md:**

1. **Phase 4 (Demo Content) — NOT descoped.** Andrew committed to plan 04-00 by 2026-04-20. 05-04 IG plan assumes Reel as primary asset format. Checkbox remains open (pending plan artefact).
2. **SML Mailchimp audience owner — resolved.** Andrew (self) confirmed as audience owner. 05-02 newsletter drafts for direct Andrew send. Checkbox ticked `[x]`.
3. **SPF on spatialmedialab.org — verified.** `v=spf1 a include:spf.jackhost.net -all` confirmed. Checkbox ticked `[x]`.
4. **DKIM + DMARC on spatialmedialab.org — pending HITL.** Neither `default._domainkey` nor `google._domainkey` selectors return records; DMARC also absent. Andrew setting up at jackhost.net as a separate HITL task; target verified by 2026-04-25 (T-3). Checkbox remains open (pending).
5. **Review-batch from-address — deferred to T-3.** 05-06 will draft BOTH `andrew@spatialmedialab.org` AND `andrewjrahman@gmail.com` variants. Final choice made at T-3 based on test-send Authentication-Results header inspection. Checkbox remains open (pending).
6. **KVR Developer Account — no account yet.** Andrew applying at kvraudio.com/developer_application.php. 05-05 drafts BOTH the Developer DB listing metadata AND the `contactus@kvraudio.com` press-release fallback so launch isn't blocked on application turnaround. Checkbox remains open (pending).

**Remaining open rows (andrewrahman.com prod deploy, GitHub v1.0.0 release, LinkedIn/IG/Bluesky login checks, Mastodon fallback, URL smoke schedule, Berlin DM window)** stay [ ] and will be walked during the daily T-7 cadence starting 2026-04-21.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Corrected requirements-completed mis-tag in plan frontmatter**
- **Found during:** State updates (post-SUMMARY)
- **Issue:** 05-00-PLAN.md frontmatter listed `requirements: [ANNC-01..ANNC-06]`. Running `requirements mark-complete` per the executor protocol ticked all six ANNC requirements in REQUIREMENTS.md and flipped the traceability table to Complete. But 05-00 ships a coordination scaffold (smoke + precheck + runbook) — ANNC-01..06 are delivered by downstream plans 05-01..05-08 (blog post, newsletter, LinkedIn, IG Reel, KVR listing, review outreach). Marking them complete from 05-00 creates a lie in the traceability.
- **Fix:** Reverted ANNC-01..06 to `[ ]` in REQUIREMENTS.md requirement list + `Pending` in traceability table. Updated this SUMMARY's `requirements-completed: []` with explanation. Downstream plans will mark their own requirement IDs complete on their own SUMMARY commits.
- **Files modified:** `.planning/REQUIREMENTS.md`, `.planning/phases/05-launch-announcements/05-00-SUMMARY.md`
- **Verification:** `grep "ANNC-01" REQUIREMENTS.md` shows `[ ]` and `Pending`.
- **Committed in:** (upcoming metadata commit)

---

**Total deviations:** 1 auto-fixed (Rule 1 correction of requirement traceability)
**Impact on plan:** No scope change — only accounting accuracy. The three artefacts (smoke + precheck + runbook) all match the PLAN.md acceptance criteria exactly. The T-11 precheck walk followed the `how-to-verify` steps in Task 4 and resolved or deferred every [UNKNOWN] row per the resume-signal contract.

## Issues Encountered

None during execution. Two gates moved from `unknown` to `pending HITL` rather than `confirmed` — this is explicitly the expected outcome of a precheck walk T-11 days before launch, not an issue. Both have dual-path mitigations (from-address variants, KVR listing+email fallback) drafted into the relevant downstream plan scope.

## Known Stubs

None. All artefacts are complete; pending items are external HITL gates documented in-file with resolution dates.

## HITL Items Deferred (downstream gates)

The following are documented as OPEN in launch-precheck.md and will be resolved on the T-7 daily cadence or at the stated target date:

- **DKIM + DMARC at jackhost.net** — target 2026-04-25 (T-3). Resolves from-address decision for 05-06.
- **KVR Developer Account application outcome** — target 2026-04-24 (T-4). Resolves 05-05 delivery path (dashboard vs. email fallback).
- **Phase 4 plan 04-00** — target 2026-04-20. Confirms 05-04 IG Reel asset availability.
- **andrewrahman.com production deploy** — Phase 3-09 gate, not a Phase 5 task.
- **GitHub v1.0.0 release** — Phase 0 gate (binary + metadata attached), not a Phase 5 task.
- **Account login verifications (LinkedIn, IG, Bluesky, Mastodon)** — T-7 through T-1 daily walk.

## Next Phase Readiness

**Ready for downstream execution:**
- Wave 1 plans (05-01 SML blog, 05-02 newsletter, 05-03 LinkedIn) are unblocked — all depend on Wave 0 coordination artefacts which now exist.
- 05-02 can proceed against Andrew as audience owner.
- 05-05 (KVR) proceeds with dual-path scope (DB listing + contactus fallback).
- 05-06 (review outreach) proceeds with dual from-address drafting; final selection at T-3.

**Still blocked / pending:**
- 05-04 (IG Reel) awaits Phase 4 plan 04-00 landing by 2026-04-20 (Andrew committed).
- 05-06 final from-address choice gated on 2026-04-25 DKIM verification test-send.
- 05-05 final delivery route gated on KVR Developer Account application outcome.

## Self-Check: PASSED

Verifying claims:

- `scripts/phase5-smoke.sh` — FOUND (committed 412b4b3)
- `.planning/phases/05-launch-announcements/launch-precheck.md` — FOUND (committed 83c6954, updated a81ecab)
- `.planning/phases/05-launch-announcements/launch-day-runbook.md` — FOUND (committed 86be2dd)
- Commit 412b4b3 — FOUND in git log
- Commit 83c6954 — FOUND in git log
- Commit 86be2dd — FOUND in git log
- Commit a81ecab — FOUND in git log (HEAD)

All artefacts and commits verified on disk + in git history.

---

*Phase: 05-launch-announcements*
*Plan: 00 (Wave 0 coordination scaffold)*
*Completed: 2026-04-17*
