---
gsd_state_version: 1.0
milestone: v1.0
milestone_name: milestone
status: ready_to_plan
stopped_at: "Phase 02 Plan 05 — Sender design pass mid-pivot. Site-side A2 CSS injection implemented (+184 lines in andrewrahman-com/components/DownloadForm.tsx, dirty working tree) but scope exceeds ideal. User direction (final this session): move bulk of styling to Sender account-level Brand Settings (applies to ALL forms + emails, minimises site coupling to Sender class names). Chrome confirmed open+logged-in at https://app.sender.net/forms/builder/bkRxov. Chrome drive helpers pre-written + verified at /tmp/osdchrome/{run.sh, click.sh, js.js}. Sender ToS fetched — zero free-tier branding clauses. `next dev` may still be running on :3000."
last_updated: "2026-04-24T16:27:27.612Z"
last_activity: 2026-04-24 -- Phase --phase execution started
progress:
  total_phases: 5
  completed_phases: 2
  total_plans: 31
  completed_plans: 27
  percent: 40
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-04-14)

**Core value:** Each delay echo occupies a distinct spatial position, creating immersive 3D soundscapes that move through space around the listener.
**Current focus:** Phase --phase — 02

## Current Position

Phase: 03
**Active task — Phase 02 Plan 05 (Sender.net email-capture)** — `02-05-SUMMARY.md` written 2026-04-24 (status: partial).
Status: Ready to plan
Active task (paused end-of-session 2026-04-24 late-afternoon): **Sender form design pass — pivoted A2 → A1.** A2 (iframe CSS injection) implemented +184 lines in andrewrahman-com components/DownloadForm.tsx, but scope exceeds ideal — user direction: move bulk of styling to Sender account-level Brand Settings (applies to ALL forms + emails, zero site coupling to Sender class names), then strip the site-side A2 code back to minimal. A1 starting point: Chrome open + logged in at https://app.sender.net/forms/builder/bkRxov; Chrome drive helpers verified working at /tmp/osdchrome/{run.sh,click.sh,js.js}. Sender ToS fetched → zero free-tier branding clauses → attribution-hide via site-side color-match trick is ToS-safe. **Resume file: `02-05-SENDER-DESIGN-HANDOFF.md`** (paste-ready design tokens + Brand Settings walkthrough + Form Design walkthrough + Email templates spot-check + post-dashboard site cleanup plan).
UX wart accepted, deferred post-launch: 2-emails/2-clicks. Reduction-to-1-link path (point Email #1 confirm URL at `/get-osd/`) documented in `02-05-RESEARCH-LOG.md`; not implemented now.
User decision 2026-04-24: ship full andrewrahman-com site with finalised styling BEFORE Plan 02-04 Netlify account migration + andrewrahman.com DNS cutover. Minimises change-surface during the deploy step.
Decision (2026-04-21): Option A — sender domain = `andrewrahman.com`, sender identity = `hey@andrewrahman.com`, forwarder = ImprovMX → Gmail. DNS host = Netfirms (Plan 02-04 Netlify cutover deferred).
Progress 2026-04-21 → 2026-04-22:

  - ImprovMX configured; Netfirms DNS records added + stale records removed; mail forwarding verified end-to-end.
  - Netfirms ns1 sync completed ~30 min after record addition (much faster than Netfirms' 4–8h warning).
  - Sender.net: SPF + DKIM + DMARC all green; groups `osd-unconfirmed` + `osd-confirmed` created; embedded signup form published (FORM_ID = `bkRxov`, captured 2026-04-22).
  - **2026-04-22 session (00:15 → 01:15):** DOI workflow `OSD DOI — confirm subscription` built end-to-end. Trigger → Email step (with `{$double-optin-link}` confirm button, Sender free-tier default styling) → 1min Delay → Condition (Workflow email activity: clicked link) → Yes branch: Move to `osd-confirmed`; No branch empty. Workflow saved in Paused state. Account auto-flagged for review when Activate was clicked (Sender banner: ≤1h auto-approval). Form's "Redirect after submit" unchecked — correct UX (inline success; `/get-osd/` is post-confirm only). Post-DOI redirect URL field not yet located — almost certainly unlocks with the DOI toggle (currently greyed out on form's Publishing Settings with "only available on verified accounts" warning).

**Design pivot 2026-04-22** — after reading `andrewrahman-com` origin/main fresh (local clone was 93 commits behind): the DOI email is confirm-button-only (no download buttons); downloads live on `/get-osd/` (post-DOI landing page with `DownloadButtons` component); Sender's post-confirm redirect URL = `https://andrewrahman.com/get-osd/`. Site code (DownloadForm, DownloadButtons, /get-osd/, lib/release.ts, self-hosted /assets/*.zip) already on origin/main — remaining code work scopes to CSP/privacy-page/test-spec spot-checks, not a rewrite.
**Palette correction (2026-04-22):** Phase 03-03 D-12 `--accent-regal` lavender "Patreon CTA only" token is ABANDONED — Session 3 swapped Patreon CTA to `--accent-green` (`#3BCE6C`), Session 7 extended the through-line to §5 Pipeline. Current CTA hierarchy: primary cyan `#80d8ff` / secondary green `#3BCE6C` / tertiary dim. See `~/.claude/projects/.../memory/project_andrewrahman_site_cta_palette.md`. `globals.css:28` stale `/* Patreon CTA only */` comment earmarked for cleanup during local-clone rebase.
**Session 2026-04-22 08:20 → 09:00 (~40min):** Sender account review cleared. Enabled DOI toggle on form `bkRxov`. Spent session investigating where the post-DOI redirect URL lives in Sender. **Finding: it doesn't exist.** Sender's DOI panel has only email-metadata fields; the form's "Redirect after submit" empirically fires at step 1 pre-DOI (tested with `/?sender-test=1` tracer URL — browser redirected immediately after form submit, before any confirmation email); and the confirm-button click always lands on Sender's fixed hosted Success view ("Oh thank you! / We are glad to have you on board"). Four Sender help docs implicitly confirm: post-confirm behaviour goes through automations, not redirects. Path A chosen (follow-up email on Yes branch). Test setting reverted — "Redirect after submit" back to unchecked.
**Session 2026-04-22 14:17 → 14:25 (~8min):** Ran `/gsd-execute-phase 3` to try to clear Plan 03-09 (DIST-01 cross-phase closeout). Plan 03-09 Task 1 precondition gate **FAILED** on 2 of 4 checks: (1) `02-05-SUMMARY.md` does not exist yet — Sender walkthrough still in Paused state per §15; (2) `andrewrahman.com` still resolves to the old Weebly frameset (DNS not flipped to Netlify, `/get-osd/` and `/privacy/` both 404). Site commits (`fce5663`, `a4ef090`) confirmed present on `../andrewrahman-com` main; Plan 03-07 has no `checkpoint-paused` status, so checks 3 + 4 pass. No files touched, no Netlify actions attempted. **New blocker surfaced:** Netlify free-tier build credits were exhausted by the pre-launch build churn, so the production deploy requires creating a fresh Netlify account. **Scoping decision (2026-04-22):** new-account setup stays under **Plan 02-04** (currently `status: partial`), NOT in Phase 3 — 02-04's `user_setup` block already lists "Create Netlify account (free Starter plan) if not already" as step 1 of its dashboard checklist, so this is resumption of the same scope, not new work. Phase 3 is content/design (D-02 struck infra out of it) and Plan 03-09 is a consumer of the deploy, not the home for it. Resume order: finish Plan 02-04 (new Netlify account → Import site repo → env vars → Netfirms DNS flip → `scripts/verify-production.sh` green → 02-04 `status: partial` → `complete`), then resume Plan 02-05 Sender walkthrough (§15 Path A), then Plan 03-09 precondition gate re-runs cleanly. Plan 03-09 file untouched; 03-09-SUMMARY.md not yet created.
**Session 2026-04-24 (~2hrs across worktrees):** DOI flow validated end-to-end on localhost:3000 (full path: form submit → osd-unconfirmed → confirm email → click → osd-confirmed + download email → /get-osd/). Form embed fixed on andrewrahman-com — root cause: loading `https://cdn.sender.net/accounts_resources/universal.js` directly crashes with `Cannot read properties of undefined (reading 'q')`. Fix in DownloadForm.tsx commit `e9c6b27`: Sender's official IIFE bootstrap (sets up `window.Sender` queue + `.l` timestamp + `.on` listener; calls `sender('9b01e3bbeb8393')` to register account ID) before script tag injection. New env var `NEXT_PUBLIC_SENDER_ACCOUNT_ID` added alongside existing `NEXT_PUBLIC_SENDER_FORM_ID`. Three new planning docs replace the prior Path A handoff narrative: `02-05-MAILING-LIST-ANALYSIS.md` (provider matrix + decision rationale; rebuts "Path A broken" framing), `02-05-RESEARCH-LOG.md` (F1–F11 confirmed facts, X1–X6 failed approaches), `02-05-RESEARCH-PLAN.md` (research methodology). Local main was 3 commits ahead with stale Path A pessimism; reset to origin/main 2026-04-24 13:18 GMT+2 (backup branch `backup/2026-04-23-path-a-investigation` preserves the discarded work). `02-05-SUMMARY.md` written same session (status: partial).
Resume file (PRIMARY): `.planning/phases/02-email-capture-funding-infrastructure/02-05-SENDER-DESIGN-HANDOFF.md` (paste-ready design tokens + full walkthrough + post-dashboard cleanup plan). Supporting: `02-05-SUMMARY.md`, `02-05-RESEARCH-LOG.md`, `02-05-MAILING-LIST-ANALYSIS.md`.
Next session: (1) reload localhost:3000 + Playwright re-screenshot the embedded form; verify cyan button + dark borders + DM Sans title + no red attribution render correctly after the v2 CSS rewrite; (2) if any element still shows Sender-default colours, add JS inline-style fallback (specificity 1000); (3) opportunistically strip rules from DownloadForm.tsx that Sender dashboard now delivers natively; (4) commit both repos local + flip 02-05-SUMMARY status partial→complete; (5) resume Sender account-level Brand Settings (logo upload, light-theme colours, "from" identity) — Plan 02-08 scope; (6) then kick off Plan 02-04 Netlify migration → DNS cutover → production E2E UAT; (7) Plan 03-09 precondition gate now passes SUMMARY check.
Downstream dep: production flip (Netlify env var + live E2E UAT) waits on Plan 02-04 Netlify deploy/cutover.

**Secondary — Phase 05 (launch-announcements)**
Plan: Not started
Status: Paused — see `.planning/phases/05-launch-announcements/PAUSE-T-10-v3.md` for resume
Last activity: 2026-04-24

Progress: [████████░░] 80% (24/30 plans complete)

## Performance Metrics

**Velocity:**

- Total plans completed: 12
- Average duration: --
- Total execution time: 0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01 | 4 | - | - |
| 02 | 8 | - | - |

**Recent Trend:**

- Last 5 plans: --
- Trend: --

*Updated after each plan completion*
| Phase 02 P03 | 15min | 2 tasks | 2 files |
| Phase 03 P00 | 7min | 4 tasks | 5 files |
| Phase 03 P01 | 5min | 3 tasks | 2 files |
| Phase 03 P02 | 3min | 3 tasks | 5 files |
| Phase 03 P03 | 3min | 2 tasks | 4 files |
| Phase 03 P04 | 5min | 3 tasks | 13 files |
| Phase 03 P05 | 6min | 3 tasks | 10 files |
| Phase 03 P06 | 8min | 1 tasks | 2 files |
| Phase 03 P08 | 1min 26s | 2 tasks | 2 files |
| Phase 05 P00 | 90min | 4 tasks | 3 files |
| Phase 05 P01 | ~50min | 2 tasks | 1 files |

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table.
Recent decisions affecting current work:

- [Init]: GPL-3.0 only (drop commercial license) — JUCE GPL tier compatible, revenue via Patreon
- [Init]: Patreon as funding model — funds entire spatial audio tools pipeline
- [Init]: ~~Tally for email capture — free tier, no code needed, redirect to GitHub Releases~~ — **SUPERSEDED 2026-04-16 by [Phase 02] Sender.net decision below.**
- [Init]: Website + mailing list first — enables social posts immediately
- [Phase 02]: Data Controller = Andrew Rahman (natural person), not Spatial Media Lab (D-16 final, overriding plan default 2026-04-16). Rationale: privacy policy governs personal newsletter email collection; SML boundary remains plugin-side only.
- [Phase 02]: Privacy policy effective date = 2026-04-16; Section 9 commits controller to updating effective date on any material change + notifying list before adding a newsletter processor (T-02-08 mitigation).
- [Phase 02] (2026-04-16): **Email capture + newsletter tool = Sender.net (UAB Sender.lt, Vilnius, Lithuania)**. Overrides Init decision (Tally) and D-10 (defer newsletter to Phase 5). Rationale: Tally is a form builder only — no newsletter send, no double-opt-in email, no file delivery via confirmation. User's flow requires email field → DOI confirmation email with download buttons → subscriber added to list. Sender.net free-forever (2,500 subs / 15k emails/mo) natively supports this pattern plus embeddable forms, unlimited automations, and EU-based data processing (cleaner privacy-policy story than US ESPs). Selected after reviewing Tally, Kit, MailerLite, EmailOctopus, Brevo, Mailchimp, CleverReach, Rapidmail. Trade-off accepted: Sender branding appears in free-tier email footers. Newsletter flow = Option A (download buttons embedded directly in the DOI confirmation email alongside the confirm button).
- [Phase 03]: D-14 Option B: Phase 3 absorbs Sender.net migration (no Sender.net refs exist in site repo; Tally still active in get-osd, _headers, privacy)
- [Phase 03]: v1.0.0 AU plugin confirmed installed — `v1.0.0.component` is the shipped screenshot target; legacy `v1.0.component` is a pre-release iteration from before the 2026-04-12 squash (NOT the same binary). Wave 2 screenshot capture unblocked.
- [Phase 03]: Plan 03-01 complete — Hero stats locked (D-01: 12/7/5+1/70), FEATURES 5→6 (D-03), hero + DownloadCTA copy polished, ADM-OSC claim verified against Source/PluginProcessor.cpp
- [Phase 03]: Plan 02: Effective date bumped to 2026-04-17 (current day) rather than leaving baseline 2026-04-16; regex test tolerant either way
- [Phase 03]: Plan 02: Privacy changelog paragraph deliberately omits legacy Gmail from rendered text — git log is the audit trail, keeps Playwright negative-assertion green
- [Phase 03]: Plan 02: scripts/verify-production.sh migrated alongside privacy/page.tsx (Rule 1) — post-deploy content check directly coupled to page copy, atomic commit prevents broken-deploy state
- [Phase 03]: D-12 scoped recolour: --accent-regal added, --accent-rose retained; 3 Patreon call-sites swapped (PatreonCTA + Hero + Nav); AboutAndrew SectionLabel untouched
- [Phase 03]: Chromium returns oklch() sources as lab() not rgb() in getComputedStyle; Playwright rendered-style regex must match lab() form for wide-gamut tokens
- [Phase 03]: Plan 03-04: screenshot_tool CLI replaces REAPER human checkpoint — off-screen editor render via cmake target, 70 factory presets map deterministically to the 10 plugin-UI PNGs. signal-flow.png preserved as architectural diagram.
- [Phase 03]: Plan 03-04: sml-logo.svg + andrew.jpg live; sml-logo.png deleted (D-11); AboutAndrew monogram placeholder replaced with D-15 locked Image; Pipeline logo src .png→.svg. 14/14 asset-existence + 15/15 homepage-content tests green.
- [Phase 03]: Plan 03-05 D-14 Option B executed: EmailCaptureSection created; homepage inline #get-osd section rendered between DownloadCTA and PatreonCTA; /get-osd/ reduced to 5-line shim; Nav + Hero + DownloadCTA Download OSD CTAs use href='#get-osd'; _headers CSP migrated Tally→Sender.net (cdn.sender.net + *.sender.net); privacy processor Tally Technologies SRL→UAB Sender.lt with 2026-04-17 changelog; .env.example TALLY_FORM_ID→SENDER_FORM_ID (Rule 2 auto-fix); zero Tally refs remain; 51/54 Playwright green (3 a11y failures deferred to Plan 06 as documented in 03-00-SUMMARY)
- [Phase 03]: Plan 03-06 — D-08 WCAG 2.1 AA closed: axe-core baseline showed 58 violations collapsing to 2 root-cause tokens; --text-dim nudged #6d7279 → #787d84 and --accent-violet nudged #7457d1 → #8570d7 (both ~+2 OKLCH lightness pts, annotated in globals.css with /* Phase 3 AA fix */). All 3 routes green, full 54-test Playwright suite green, no --tap-N UI-minimum failures.
- [Phase 03]: Plan 03-07 Task 1 — screenshot_full.png (1640×1160) matched layout.tsx exactly, so no dimension edit was required; alt text updated from "HRTF binaural rendering" to capability language "spatial map view" per feedback_marketing_copy_depth.md (Rule 2). 4/4 og-metadata Playwright tests green. Task 2 (human-verify on opengraph.xyz + metatags.io) paused — needs deploy URL.
- [Phase 03]: Plan 03-08: WEB-02 flipped to Complete via external-site Playwright guard on spatialmedialab.org/about. sml-about.spec.ts (4 tests, @external, retries: 2) lives in andrewrahman-com repo; REQUIREMENTS.md checkbox + traceability row flipped in openspatialdelay repo. WEB-01/WEB-03 untouched. CONTEXT.md D-02 scope-strike now reflected in requirements tracker.
- [Phase 05]: Plan 05-00: T-11 precheck walk — Mailchimp owner=Andrew (resolved), SPF verified (v=spf1 a include:spf.jackhost.net -all), DKIM+DMARC pending HITL at jackhost.net target T-3, KVR Developer Account=no-account (dual path: apply + contactus@kvraudio.com fallback), Phase 4 plan 04-00 committed by 2026-04-20 (not descoped)
- [Phase 05]: Plan 05-01: SML blog post approved after 18 voice red-lines. Primary download path = andrewrahman.com/get-osd (email-wall); GitHub positioned as source-only. Release-tag deep-link URL (/releases/tag/v1.0.0) deliberately absent from final draft — intentional deviation from planner grep.
- [Phase 02] (2026-04-19): Plan 02-05 blocked on sending-domain decision. Findings: (1) Sender.net requires double opt-in behind account verification, which requires SPF/DKIM/DMARC on the sending domain; (2) `spatialmedialab.org` DNS is hosted at InterNetX AutoDNS (nameservers `a-d.ns14.net`) NOT at jackhost.net — the jackhost Plesk panel has no DNS editor because DNS was never delegated to jackhost; (3) AutoDNS credentials presumed held by Timo Bittner (Plesk `Systembenutzer = timobittner.de`). Research doc `.planning/phases/02-email-capture-funding-infrastructure/02-05-SENDING-DOMAIN-RESEARCH.md` enumerates three options: A) pivot sending domain to `andrewrahman.com` via ImprovMX forwarder (recommended — aligns with D-16 Data-Controller=Andrew natural-person framing), B) ask Timo to add the three DNS records on `spatialmedialab.org` one-time, C) use Sender shared domain (rejected — poor deliverability under Gmail/Yahoo 2024 sender rules). User researching before committing.

### Pending Todos

- **[02-05 BRAND-SETTINGS]** Go through Sender.net account-level **Brand settings** page (logo upload, colour theme, fonts) before next marketing email goes out. Captured 2026-04-22 during DOI build — skipped to keep momentum on the DOI automation itself. Notes from screenshot: accent colour already near-cyan (close to `#80d8ff`), headline font = Fira Sans, paragraph font = DM Sans (DM Sans matches OSD spec ✅). Targets: upload OSD wordmark/logo PNG, pick a preset or tune `--accent-stellar` / `--bg-void` / DM Sans across headline + paragraph, save. Applies globally to all future Sender emails — better than per-email styling. May be folded into the active 2026-04-24 Sender form design pass if the same Sender dashboard surfaces are touched.
- **[02-05 SINGLE-LINK DOI OPTIMISATION]** Post-launch, evaluate collapsing the 2-email/2-click funnel to 1-click by retargeting Email #1's confirm button URL from current target to `https://andrewrahman.com/get-osd/`. Eliminates Email #2 entirely. Keeps DOI legally valid (UWG §7 / DSGVO Art. 7 — labelled Confirm button still satisfies "clear affirmative act"). Trade-off: visual separation between consent action and download CTA collapses. Defer until post-launch traffic data is available. Pattern documented in `02-05-RESEARCH-LOG.md` "Recommended Architecture: Two-Automation Chain".

### Blockers/Concerns

- Website rebuild is the #1 schedule risk. Research recommends time-boxing to 2 days or falling back to updating the existing builder page (Phase 3).
- Patreon must have 2+ posts before any public link appears in announcements or website (Phase 2 gate before Phase 5).
- **[03-07 CHECKPOINT-PAUSED]** Task 2 is a `checkpoint:human-verify` gate on opengraph.xyz + metatags.io social-preview rendering. Needs a deploy URL (Netlify push of `andrewrahman-com` main — currently 24 commits ahead of origin, or `netlify deploy --build` for draft). Cannot fabricate verdict — axe/Playwright cannot scrape live social platforms. Blocks 03-07 close + 03-08 start.
- **[02-06 CLOSED 2026-04-17]** Patreon LAUNCHED and live at `https://patreon.com/AndrewRahman` (HTTP/2 301 → www.patreon.com/AndrewRahman). 4-screenshot evidence requirement waived by user override (non-load-bearing ceremony for solo-creator context — see 02-06-SUMMARY.md Decisions §A). DIST-03 complete. Creator-verified 7 incognito checks at launch time + self-verified reader test.
- **[02-05 PROGRESS 2026-04-24]** Sender.net DOI flow VALIDATED WORKING end-to-end on localhost:3000. DOI toggle ON, account verified, form `bkRxov` published, automation `OSD DOI — confirm subscription` active. Form embed working on andrewrahman-com via IIFE bootstrap (DownloadForm.tsx commit `e9c6b27`; account `9b01e3bbeb8393`). 02-05-SUMMARY.md written (status: partial). Two follow-on items: (a) Sender form design pass to match site dark theme — active this session; (b) production flip blocked on Plan 02-04 Netlify migration. DIST-01 remains in-progress pending production E2E UAT (waits on (b)).
- **[02-04 BLOCKED 2026-04-22]** Production Netlify deploy blocked on free-tier build credit exhaustion. User must create a fresh Netlify account before andrewrahman.com DNS can be cut from Netfirms parking → Netlify. Owner: Plan 02-04 (status: partial).

## Session Continuity

Last session: 2026-04-24T16:45:00.000Z
Stopped at: Phase 02 Plan 05 — Sender design pass mid-pivot. Site-side A2 CSS injection implemented (+184 lines in andrewrahman-com/components/DownloadForm.tsx, dirty working tree) but scope exceeds ideal. User direction (final this session): move bulk of styling to Sender account-level Brand Settings (applies to ALL forms + emails, minimises site coupling to Sender class names). Chrome confirmed open+logged-in at https://app.sender.net/forms/builder/bkRxov. Chrome drive helpers pre-written + verified at /tmp/osdchrome/{run.sh, click.sh, js.js}. Sender ToS fetched — zero free-tier branding clauses. `next dev` may still be running on :3000.
Resume files:

  - PRIMARY: .planning/phases/02-email-capture-funding-infrastructure/02-05-SENDER-DESIGN-HANDOFF.md (paste-ready design tokens + full Chrome-drive walkthrough + post-dashboard cleanup plan)
  - SUPPORTING: 02-05-SUMMARY.md (current state of plan — status: partial)
  - SUPPORTING: 02-05-RESEARCH-LOG.md (F1–F11 facts, X1–X6 failed approaches)
  - SUPPORTING: 02-05-MAILING-LIST-ANALYSIS.md (provider matrix, decision rationale)
  - MEMORY: ~/.claude/projects/.../memory/reference_chrome_drive_mechanism.md (helper script source of truth)
  - Phase 05 pause (secondary, unchanged): .planning/phases/05-launch-announcements/PAUSE-T-10-v3.md

Uncommitted working tree:

  - openspatialdelay: M STATE.md, ?? 02-05-SUMMARY.md, ?? 02-05-SENDER-DESIGN-HANDOFF.md (0 commits ahead of origin)
  - andrewrahman-com: M components/DownloadForm.tsx (0 commits ahead of origin)
  - User explicit: NO COMMITS this session — "form looks awful right now, you're definitely not done"
