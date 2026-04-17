---
phase: 05-launch-announcements
plan: 01
subsystem: launch-copy
tags: [blog-post, wordpress, markdown, launch, announcement, sml, voice-review, copy]

# Dependency graph
requires:
  - phase: 05-launch-announcements
    provides: 05-00 launch-coordination scaffold (runbook paste-step, precheck URL smoke) + canonical cross-link stack frozen in 05-CONTEXT
  - phase: 03-personal-website
    provides: Hero stats D-01 (12 taps / 7 algorithms / 5 HRTFs / 70 presets) + screenshot asset inventory (screenshot_full, screenshot_spatial_map, signal-flow)
  - phase: 02-email-capture-funding-infrastructure
    provides: Patreon About voice (Berlin solo-dev first-person) + Post-04 cross-link pattern + andrewrahman.com/get-osd email-wall endpoint
provides:
  - .planning/phases/05-launch-announcements/drafts/sml-blog-post.md — 1100-word WordPress-ready Markdown draft of the canonical v1.0.0 announcement post (ANNC-01)
  - Approved voice template for downstream launch channels (05-02 newsletter, 05-04 IG caption, 05-05 KVR press, 05-06 review-pitch) — "each echo has a 3D position" opener + 7-week build timeline + future-plugin list (panners/choruses/reverbs/synthesizers) + Patreon-coffee-a-month CTA framing all originate here
  - T-0 WordPress paste-step artifact for launch-day runbook step 2
affects: [05-02-newsletter, 05-04-instagram, 05-05-kvr-press, 05-06-review-outreach, 05-07-berlin-dm, 05-08-forum-posts]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Capability-level voice lock: zero DSP jargon on public surfaces (FFT / phase-vocoder / partition / paper-names all blocked at acceptance-grep layer)"
    - "Two-pass draft → red-line → revise → approve pattern for long-form Andrew-voice copy (original + revision commits both preserved for audit)"
    - "Download-path split: binary via email-wall (andrewrahman.com/get-osd), source via GitHub — Release tag URL deliberately de-emphasized per Andrew's T-11 red-lines"

key-files:
  created:
    - .planning/phases/05-launch-announcements/drafts/sml-blog-post.md
  modified: []

key-decisions:
  - "Timeline locked: OSD started March 6 2026 + 7-week build — NOT the '3 years' phrasing from original draft. Red-lined to match the true recent-build narrative."
  - "7-algorithm bullet corrected: dropped 'headphones' from the 'pick one for speakers/headphones/dome' framing — binaural on headphones uses the HRTF-convolution path, not the panning-algorithm selector. This is a substantive technical correction, not just a voice tweak."
  - "Simple mode phrasing: 'CPU-lite' → 'basic binaural spatialization without the coloration of HRTFs' — lands the what-it-is without claiming CPU advantage as the primary feature."
  - "Future-plugin slate: 'panners, choruses, reverbs, synthesizers' — explicitly NOT 'convolution reverbs / moving-source simulators' (those overclaim the DSP framework's actual near-term scope). Voice red-line #8 from Andrew."
  - "Licensing/Patreon CTA merged into single bullet with coffee-a-month framing — not a separate sales pitch. Tools stay free; Patreon funds the pipeline."
  - "Dropped 'every SML plugin free forever' claim — future plugins may not all be GPL-3.0; Patreon backers steer direction rather than locking in a forever-free commitment."
  - "Download CTA restructure (BIGGEST structural change): andrewrahman.com/get-osd is the PRIMARY binary path (email-wall gated download); GitHub Release is positioned as source-only for build-from-source users. Consequence: the exact '/releases/tag/v1.0.0' deep-link URL is NOT in the final draft — base repo URL is present for source access."
  - "Closer: @spatialmedialab social tag + 'very long time since I've wanted this thing' Andrew-voice framing + 'See you at Superbooth!' exclamation — replaces planner's functional-but-flat CTA close."

patterns-established:
  - "Voice red-line ledger: when Andrew red-lines a planner draft, capture each edit in the revise-commit message body (18 edits enumerated in 9f9ee88 body) so downstream copy plans (05-02 .. 05-08) inherit the voice corrections verbatim without a second review pass."
  - "Acceptance-grep relaxation for human-approved drafts: when a human copy-review intentionally removes a structural URL that the planner's automated grep required, the deviation is documented in the SUMMARY rather than re-litigated in the draft (Rule 4 architectural — affects downstream 05-02 onward which now use get-osd not release-tag URL)."

requirements-completed: [ANNC-01]

# Metrics
duration: ~50min (Task 1 draft + Task 2 red-line revision round)
completed: 2026-04-17
---

# Phase 05 Plan 01: SML Blog Post Summary

**1100-word WordPress-ready Markdown announcement post for OpenSpatialDelay v1.0.0 — opens with "each echo has a 3D position in space", 7-week-build Berlin-solo-dev voice, get-osd primary download path, approved by Andrew after 18 voice red-lines.**

## Performance

- **Duration:** ~50 min (initial draft ~20 min, human review cycle + revision ~30 min)
- **Started:** 2026-04-17T18:22Z (approximate — a88d430 timestamp)
- **Completed:** 2026-04-17T17:11:50Z (9f9ee88 commit timestamp)
- **Tasks:** 2 (1 draft + 1 human-verify checkpoint resolved)
- **Files modified:** 1 (sml-blog-post.md created + revised in place)

## Accomplishments

- Canonical v1.0.0 launch blog post written, revised, and approved — ready for paste-in at T-0 (2026-04-28 10:00 CET) into the SML WordPress.
- Established the voice template downstream launch channels (05-02 through 05-08) will inherit: capability-level language, Berlin-solo-dev first-person, March-6 7-week-build timeline, future-plugin list (panners/choruses/reverbs/synthesizers), Patreon-coffee-a-month framing.
- Locked 18 substantive voice and technical corrections into the draft in a single review round — no drift, no second-pass required.

## Task Commits

Each task was committed atomically:

1. **Task 1: Draft sml-blog-post.md (initial WordPress-ready draft)** — `a88d430` (feat)
2. **Task 1 revision: Apply T-11 voice red-lines (18 edits)** — `9f9ee88` (revise)
3. **Task 2: Human-verify checkpoint** — resolved by Andrew with "blog approved" after revision applied; no additional commit (checkpoint is a gate, not a write-step).

**Plan metadata commit:** will be applied as this SUMMARY + STATE + ROADMAP closure commit.

## Files Created/Modified

- `.planning/phases/05-launch-announcements/drafts/sml-blog-post.md` (created a88d430, revised 9f9ee88) — 1100-word WordPress-ready Markdown draft. Structure: H1 "New plugin just dropped!" + 3 H2s (v1.0.0 + Listen + What's inside) + 3 image placeholders (screenshot_full, screenshot_spatial_map, signal-flow) + 1 audio embed placeholder (CONT-01) + Download CTA + Links + Tags + Categories + Byline.

## Decisions Made

See frontmatter `key-decisions` — 8 locked decisions from the red-line round, summarized:

1. Timeline = March 6 2026 start, 7-week build (not "3 years").
2. 7-algorithm bullet does NOT list headphones (binaural = HRTF path, not algorithm-selector).
3. Simple mode = "basic binaural without HRTF coloration" (not "CPU-lite").
4. Future-plugin slate = panners/choruses/reverbs/synthesizers (scope-realistic).
5. Patreon CTA = coffee-a-month framing, merged into licensing bullet.
6. Dropped "every SML plugin free forever" — Patreon backers steer direction.
7. Download path split: get-osd = primary binary; GitHub = source only.
8. Closer: @spatialmedialab tag + Superbooth reference.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 4 — Architectural, human-approved at checkpoint] Dropped exact release-tag URL from Download CTA**

- **Found during:** Task 2 (Andrew copy-review checkpoint)
- **Issue:** Plan's acceptance criteria required the literal string `github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0` to appear in the draft. During the red-line round Andrew restructured the Download CTA to prioritize the email-wall path (`andrewrahman.com/get-osd`) as the primary binary download, with GitHub positioned as source-only. The exact `/releases/tag/v1.0.0` deep-link was removed in favor of the base repo URL, because the email-wall is the revenue-relevant capture path for Phase 2's Sender.net infrastructure.
- **Fix:** Accepted the restructure as an architectural decision from the human reviewer (checkpoint-resolved, not an auto-fix). Base URL `github.com/Spatial-Media-Lab/OpenSpatialDelay` present for source access; release-tag deep-link absent.
- **Files modified:** .planning/phases/05-launch-announcements/drafts/sml-blog-post.md (9f9ee88)
- **Verification:** 12 of 13 original acceptance greps still pass; the release-tag grep is the single intentional drop. Downstream plans 05-02 .. 05-08 inherit the new get-osd-primary pattern.
- **Committed in:** 9f9ee88 (revision commit)

**2. [Rule 2 — Missing critical, technical accuracy] Corrected 7-algorithm bullet (dropped "headphones")**

- **Found during:** Task 2 (Andrew copy-review checkpoint)
- **Issue:** Planner's draft said "pick the right one for speakers, headphones, or a dome" for the 7 spatialization algorithms (Ambisonics / ConstantPower / DBAP / KNN / MDAP / VBAP / VBIP). This is technically wrong — headphone spatialization in OSD uses the HRTF-convolution path (the 5 measured HRTF profiles + Simple mode), not the panning-algorithm selector. Shipping this claim on the canonical launch post would create a technical credibility hit with the spatial-audio readership.
- **Fix:** Removed "headphones" from the algorithm bullet; kept speakers and dome (which are the actual valid targets for those algorithms). HRTF path stays in its own bullet.
- **Files modified:** .planning/phases/05-launch-announcements/drafts/sml-blog-post.md (9f9ee88)
- **Verification:** Revised draft reviewed by Andrew and approved ("blog approved").
- **Committed in:** 9f9ee88 (revision commit, edit #2 of 18)

---

**Total deviations:** 2 tracked (1 architectural / human-approved, 1 technical accuracy / human-caught). The remaining 16 of the 18 edits in commit 9f9ee88 are voice-level refinements (tone, phrasing, small factual cleanups) and are fully enumerated in the commit message body rather than itemized here.

**Impact on plan:** Draft improved materially in the review cycle. The release-tag-URL drop is a scope-positive restructure (email-wall capture = Phase 2 revenue lever); the algorithm-bullet correction is a credibility-positive technical fix. No scope creep; all changes tighten the plan's stated goal.

## Issues Encountered

- **Planner's URL acceptance-grep was too literal.** The automated acceptance test for `/releases/tag/v1.0.0` enforced a specific deep-link that the human reviewer re-prioritized during the checkpoint. Pattern-established: future launch-copy plans should grep for "GitHub repo URL present" at the org-level, not deep-link level, to give the human reviewer latitude on the final download funnel structure.
- No other issues; the draft passed on first pass through all other 12 acceptance greps (word count, H1/H2 counts, image count, voice-literal "each echo has a 3D position", byline, tags, zero DSP jargon).

## User Setup Required

None — the draft is a Markdown artifact in the planning tree. At T-0 (2026-04-28 10:00 CET), Andrew copy-pastes into the SML WordPress block editor per launch-day-runbook step 2; uploads the 3 PNG screenshots from `andrewrahman-com/public/assets/` into WP Media Library; fills the CONT-01 audio embed iframe; and clicks Publish.

## Deferred Items

Tracked for handoff to downstream plans and launch-day runbook:

- **CONT-01 audio host decision (T-0 paste-step):** The draft has `[AUDIO EMBED — CONT-01]` as a placeholder. Andrew selects SoundCloud iframe vs. Bandcamp iframe vs. WP Media Library `<audio controls>` at paste-time based on where CONT-01 finally lands. Not blocking for T-1 (the placeholder does not break the Markdown); resolved in the T-0 runbook step 2.
- **Three image asset confirmations (T-1):** The draft references three `/wp-content/uploads/2026/04/` URLs (screenshot_full.png, screenshot_spatial_map.png, signal-flow.png). Andrew confirms at T-1 whether the final three images are those exact three or a substitution (e.g., screenshot_presets.png swapped for signal-flow.png). Swap is a one-URL edit in the draft; no blocker.
- **Byline date fallback (T-3):** Byline is locked to "Published April 28, 2026 By Andrew". If Phase 4 (demo content) slips and launch shifts to the May 5 fallback, the byline date updates at that point. Runbook covers the fallback branch.

## Next Phase Readiness

**Ready to proceed to 05-02 (newsletter draft).** The blog post is the canonical anchor URL that 05-02, 05-04, 05-05, 05-06, 05-07, and 05-08 all link to. Voice template is locked; downstream plans inherit:

- Opener: "each echo has a 3D position in space"
- Timeline: "started March 6 2026, 7 weeks"
- Future-plugin slate: panners / choruses / reverbs / synthesizers
- Patreon framing: coffee-a-month, not sales pitch
- Primary download: andrewrahman.com/get-osd (email-wall)
- Source download: github.com/Spatial-Media-Lab/OpenSpatialDelay

**No blockers** for 05-02. The SML WordPress blog post URL (`spatialmedialab.org/open-spatial-delay-v1-0-0/` or similar slug) is not yet resolvable — it becomes live at T-0 — but newsletter draft can reference it with a placeholder `{SML_BLOG_POST_URL}` which the launch-day runbook step 4 substitutes in.

## Self-Check: PASSED

- Draft file at `.planning/phases/05-launch-announcements/drafts/sml-blog-post.md` — FOUND (1100 words, 7407 bytes)
- Commit a88d430 (original draft) — FOUND in `git log`
- Commit 9f9ee88 (revision) — FOUND in `git log` (verified via `git show --stat`)
- 12 of 13 planner acceptance greps pass; 1 intentional deviation (release-tag URL) documented above
- Zero DSP jargon in final draft (confirmed via `grep -ciE '(FFT|phase vocoder|partition|Laroche|Dolson|Röbel|Gardner|Martin|2048-sample|STFT|AAX|VST2|commercial license|proprietary|buy|purchase)'` → returns 0)

---
*Phase: 05-launch-announcements*
*Plan: 01 (SML blog post — ANNC-01)*
*Completed: 2026-04-17*
