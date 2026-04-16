---
phase: 03-personal-website
plan: 07
subsystem: seo
tags: [open-graph, twitter-card, playwright, metadata, next-metadata-api]

requires:
  - phase: 03-personal-website
    provides: "Plan 00 scaffolded tests/og-metadata.spec.ts with 4 assertions (title, description, image width regex, twitter:card)"
  - phase: 03-personal-website
    provides: "Plan 04 regenerated screenshot_full.png to v1.0.0 at 1640×1160"
provides:
  - "WEB-03 pre-checkpoint: layout.tsx OG metadata reconciled with Plan 04 screenshot and verified via Playwright (4/4 green)"
  - "Capability-language alt text replacing DSP jargon on the primary social-preview image"
affects: [08-qa-closeout, phase-04-launch-prep]

tech-stack:
  added: []
  patterns:
    - "Hard-coded OG image width/height in Next Metadata API must match actual PNG pixel dimensions — verified via sips pre-edit"
    - "og:image:width regex gate (^(1[2-9]\\d{2,}|[2-9]\\d{3,})$) enforces ≥1200px minimum for social rendering"

key-files:
  created:
    - ".planning/phases/03-personal-website/03-07-SUMMARY.md"
  modified:
    - "../andrewrahman-com/app/layout.tsx (alt text only — dimensions already matched)"

key-decisions:
  - "No dimension edit needed: screenshot_full.png is 1640×1160 exactly matching layout.tsx, so Task 1 Step C path was taken"
  - "Applied alt-text revision within the plan's optional D-06 scope to replace 'HRTF binaural rendering' (user-flagged DSP jargon per feedback_marketing_copy_depth.md) with capability language 'spatial map view'"
  - "og-metadata.spec.ts regex (^(1[2-9]\\d{2,}|[2-9]\\d{3,})$) accepts 1640 without widening — no Plan 00 test change required"

patterns-established:
  - "OG metadata reconciliation protocol: sips the actual image, compare to layout.tsx, rebuild (next build), run og-metadata.spec.ts, THEN return for human social-preview verification"

requirements-completed: []  # WEB-03 remains in-flight until human-verify checkpoint clears

duration: ~6min
completed: 2026-04-17
---

# Phase 03 Plan 07: Close WEB-03 — OG Metadata Reconciliation Summary

**Plan-04 screenshot dimensions (1640×1160) matched layout.tsx exactly; alt text updated to capability language; 4/4 og-metadata Playwright tests green; paused at human-verify checkpoint for actual social-card rendering on opengraph.xyz / metatags.io.**

## Performance

- **Duration:** ~6 min (Task 1 only; Task 2 is a blocking human-verification checkpoint)
- **Started:** 2026-04-17T01:10:00+02:00
- **Paused at checkpoint:** 2026-04-17T01:15:00+02:00
- **Tasks completed:** 1 / 2 (Task 2 = human-verify checkpoint)
- **Files modified:** 1

## Status: AWAITING HUMAN VERIFICATION

Plan 03-07 is **partially complete**. Task 1 (automated reconciliation) is done. Task 2 is a `checkpoint:human-verify` gate that requires visiting opengraph.xyz and metatags.io with a real deploy URL. See "Pending Human Verification" below.

## Accomplishments

- Verified screenshot_full.png pixel dimensions (1640×1160) match layout.tsx `openGraph.images[0]` width/height exactly — no dimension edit required.
- Confirmed 1640×1160 is comfortably above the 1200×630 OG social-rendering minimum.
- Replaced the alt-text "12-tap 3D delay with HRTF binaural rendering" with the capability-language "12 taps in 3D space, spatial map view" to align with the locked feedback pattern that DSP jargon is "pointlessly complex" on public marketing surfaces.
- Rebuilt the Next.js static export (`npm run build`) and re-ran `tests/og-metadata.spec.ts` — all 4 assertions pass (og:title, og:description, og:image width regex, twitter:card=summary_large_image).

## Task Commits

Each task was committed atomically:

1. **Task 1: Reconcile layout.tsx OG dimensions + re-run og-metadata.spec.ts** — `1827cc8` in `andrewrahman-com` (feat(03-07): reconcile OG metadata alt text with capability language)
2. **Task 2: CHECKPOINT — Verify social card rendering on opengraph.xyz + metatags.io** — PAUSED, awaiting human verification (see below)

**Plan metadata commit:** (pending — will be made when plan resumes and closes)

## Files Created/Modified

- `../andrewrahman-com/app/layout.tsx` — updated `openGraph.images[0].alt` from DSP-jargon to capability-language (line 40). Dimensions (1640×1160, line 38-39) confirmed correct and left unchanged.
- `.planning/phases/03-personal-website/03-07-SUMMARY.md` — this file.

## Decisions Made

- **No dimension mutation.** sips(1) reported the new screenshot is 1640×1160, which already matches layout.tsx exactly (a lucky convergence — Plan 04's capture tool emits the same native dimensions the previous baseline had). Per Plan Task 1 Step C, this meant no source edit for width/height.
- **Alt-text revision applied (optional scope D-06).** Plan text said "executor may revise within D-06 scope." The existing alt contained two tokens the user explicitly flagged as DSP jargon in a prior session — "HRTF" and "binaural rendering". Substituted the plan's suggested capability phrasing because (a) it's the authoritative copywriting pattern the user has enforced elsewhere, and (b) OG image alt is discoverable copy on screen-reader social-preview rendering, so it lives on the "public marketing surface" the feedback covers.
- **Left og-metadata.spec.ts regex untouched.** The scaffolded width regex `^(1[2-9]\d{2,}|[2-9]\d{3,})$` matches 1640 cleanly, so no widening was needed.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Marketing copy consistency] Replaced DSP jargon in OG image alt text**
- **Found during:** Task 1 (dimension reconciliation; alt was in the edited block)
- **Issue:** Current alt "OpenSpatialDelay plugin window — 12-tap 3D delay with HRTF binaural rendering" uses two tokens — "HRTF" and "binaural rendering" — explicitly flagged by the user as "pointlessly complex" on public marketing surfaces per `memory/feedback_marketing_copy_depth.md`. Plan allowed executor-discretion revision within D-06 scope.
- **Fix:** Set alt to the plan's suggested "OpenSpatialDelay plugin — 12 taps in 3D space, spatial map view".
- **Files modified:** `../andrewrahman-com/app/layout.tsx` line 40.
- **Verification:** Rebuilt out/index.html; grep confirms the new alt ships in the static export; all 4 og-metadata tests pass.
- **Committed in:** `1827cc8`

**Total deviations:** 1 auto-fixed (Rule 2 marketing-copy consistency).
**Impact on plan:** None — executed within explicit plan-authorized D-06 scope; no scope creep.

## Issues Encountered

None. Automated path ran clean: sips measurement → no-op dimension check → optional alt edit → next build → playwright pass.

## Pending Human Verification (Task 2)

**Why a checkpoint:** axe/Playwright cannot scrape live social-preview platforms (auth, rate-limits, TTL caches — see 03-RESEARCH.md §Pitfalls). A human must paste the deployed URL into opengraph.xyz and metatags.io and confirm title/description/image render correctly.

**Current deployment state:** The `andrewrahman-com` repo is 24 commits ahead of `origin/main` as of Task 1's commit `1827cc8`. No Netlify preview URL is currently available to the executor. The user must either:

1. **Push `main` to GitHub** → Netlify will auto-build (netlify.toml: `command = "npm run build"`, `publish = "out"`) and give a deploy URL; OR
2. **Run `next dev` locally + ngrok** (OG scrapers need a public URL); OR
3. **Run `netlify deploy --build`** for a draft-deploy URL without touching main.

**Verification steps (from plan Task 2):**

1. Visit `https://www.opengraph.xyz/` → paste deploy URL → confirm:
   - og:title reads `"Andrew Rahman — OpenSpatialDelay & the Spatial Media Library"`
   - og:description contains `"VST3 + AU"`
   - og:image preview is the new v1.0.0 capture (1640×1160, not stale)
   - Image renders at full width without cropping
2. Visit `https://metatags.io/` → paste deploy URL → verify Facebook, Twitter (summary_large_image), LinkedIn previews all render correctly.
3. (Optional) Paste URL into Twitter/X DM composer + LinkedIn post composer (don't post) — confirm platform-native preview renders.

**Cache-busting fallback:** If Facebook/LinkedIn shows stale (prior) image, use Facebook Sharing Debugger (`developers.facebook.com/tools/debug`) or LinkedIn Post Inspector (`linkedin.com/post-inspector`) to force refresh.

**Resume signal:** User types `"verified"` when both platforms render correctly, or `"needs-recapture: <reason>"` if the image fails either.

**On resume:** A continuation executor (or this one, if re-spawned) will:
1. Record in this SUMMARY the final deploy URL used, opengraph.xyz result, metatags.io result, optional composer checks.
2. Mark `WEB-03` complete in REQUIREMENTS.md (requires: `requirements mark-complete WEB-03`).
3. Make the final docs commit for plan 03-07.
4. Advance STATE.md plan counter to 03-08.

## Next Phase Readiness

- **03-08 (final QA + sweep) is gated on Task 2 resolution.** Plan 03-08 is the phase-closeout; if 03-07 checkpoint clears with `"verified"`, 03-08 runs. If `"needs-recapture"`, the user must re-run Plan 04 before 03-07 can close.
- All automated machinery for OG is done — no further code changes expected unless a platform flags a specific preview issue.

## Self-Check: PASSED

- **File exists:** `/Users/andrewrahman/conductor/repos/andrewrahman-com/app/layout.tsx` (verified via git diff on commit)
- **Commit exists:** `1827cc8` (verified via `git -C ../andrewrahman-com log --oneline -1`)
- **Test pass:** `npx playwright test tests/og-metadata.spec.ts` → `4 passed (1.4s)`
- **Acceptance criteria:** pixelWidth 1640 ≥ 1200 ✓, pixelHeight 1160 ≥ 630 ✓, width/height/path grep ✓, tests green ✓
- **Plan artifact contract:** `layout.tsx` contains `openGraph` block referencing `/assets/screenshot_full.png` with width/height matching the file ✓

---
*Phase: 03-personal-website*
*Checkpoint status: BLOCKED on human verification of Task 2*
*Last automated task completed: 2026-04-17*
