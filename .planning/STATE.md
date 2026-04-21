---
gsd_state_version: 1.0
milestone: v1.0
milestone_name: milestone
status: paused
stopped_at: "Phase 02 Plan 05 (Sender.net) — domain auth complete 2026-04-22 (ns1 synced; SPF+DKIM+DMARC all green on Sender); groups osd-unconfirmed/osd-confirmed created; embedded form published (FORM_ID pending capture). Design pivot 2026-04-22: DOI email is confirm-button-only; downloads live on /get-osd/ post-confirm page; Sender post-confirm redirect URL = https://andrewrahman.com/get-osd/. Remaining: build DOI automation per revised body, data-region check, email support@sender.net for account verification. Site code already on andrewrahman-com origin/main (DownloadForm, DownloadButtons, /get-osd/, lib/release.ts, /assets/zips). Production flip gated on Plan 02-04 Netlify deploy. See .planning/phases/02-email-capture-funding-infrastructure/02-05-SENDING-DOMAIN-RESEARCH.md §12–§13. Phase 05 Wave 2 also paused for HITL — see .planning/phases/05-launch-announcements/PAUSE-T-10-v3.md."
last_updated: "2026-04-22T00:15:00.000Z"
last_activity: 2026-04-22
progress:
  total_phases: 5
  completed_phases: 1
  total_plans: 30
  completed_plans: 24
  percent: 80
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-04-14)

**Core value:** Each delay echo occupies a distinct spatial position, creating immersive 3D soundscapes that move through space around the listener.
**Current focus:** Phase 05 — launch-announcements

## Current Position

**Active blocker — Phase 02 Plan 05 (Sender.net email-capture)**
Status: DOI automation build pending (no longer DNS-blocked).
Decision (2026-04-21): Option A — sender domain = `andrewrahman.com`, sender identity = `hey@andrewrahman.com`, forwarder = ImprovMX → Gmail. DNS host = Netfirms (Plan 02-04 Netlify cutover deferred).
Progress 2026-04-21 → 2026-04-22:
  - ImprovMX configured; Netfirms DNS records added + stale records removed; mail forwarding verified end-to-end.
  - Netfirms ns1 sync completed ~30 min after record addition (much faster than Netfirms' 4–8h warning).
  - Sender.net: SPF + DKIM + DMARC all green; groups `osd-unconfirmed` + `osd-confirmed` created; embedded signup form published (FORM_ID pending capture).
**Design pivot 2026-04-22** — after reading `andrewrahman-com` origin/main fresh (local clone was 93 commits behind): the DOI email is confirm-button-only (no download buttons); downloads live on `/get-osd/` (post-DOI landing page with `DownloadButtons` component); Sender's post-confirm redirect URL = `https://andrewrahman.com/get-osd/`. Site code (DownloadForm, DownloadButtons, /get-osd/, lib/release.ts, self-hosted /assets/*.zip) already on origin/main — remaining code work scopes to CSP/privacy-page/test-spec spot-checks, not a rewrite.
Resume file: `.planning/phases/02-email-capture-funding-infrastructure/02-05-SENDING-DOMAIN-RESEARCH.md §12–§13` (session logs + resume checklist).
Next session: (1) build DOI automation in Sender per revised body in SENDER-SETUP.md Step 3; (2) set post-confirm redirect URL to /get-osd/; (3) data-region check; (4) email `support@sender.net` requesting account verification.
Downstream dep: production flip (Netlify env var + live E2E UAT) waits on Plan 02-04 Netlify deploy/cutover. Local clone of `andrewrahman-com` is 93 behind + 2 ahead of origin/main — needs git rebase/reset before further code work; not blocking Sender setup.

**Secondary — Phase 05 (launch-announcements)**
Plan: 8 of 9 (Wave 1 complete; Wave 2 + 3 pending)
Status: Paused — see `.planning/phases/05-launch-announcements/PAUSE-T-10-v3.md` for resume
Last activity: 2026-04-18

Progress: [████████░░] 80% (24/30 plans complete)

## Performance Metrics

**Velocity:**

- Total plans completed: 4
- Average duration: --
- Total execution time: 0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01 | 4 | - | - |

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

None yet.

### Blockers/Concerns

- Website rebuild is the #1 schedule risk. Research recommends time-boxing to 2 days or falling back to updating the existing builder page (Phase 3).
- Patreon must have 2+ posts before any public link appears in announcements or website (Phase 2 gate before Phase 5).
- **[03-07 CHECKPOINT-PAUSED]** Task 2 is a `checkpoint:human-verify` gate on opengraph.xyz + metatags.io social-preview rendering. Needs a deploy URL (Netlify push of `andrewrahman-com` main — currently 24 commits ahead of origin, or `netlify deploy --build` for draft). Cannot fabricate verdict — axe/Playwright cannot scrape live social platforms. Blocks 03-07 close + 03-08 start.
- **[02-06 CLOSED 2026-04-17]** Patreon LAUNCHED and live at `https://patreon.com/AndrewRahman` (HTTP/2 301 → www.patreon.com/AndrewRahman). 4-screenshot evidence requirement waived by user override (non-load-bearing ceremony for solo-creator context — see 02-06-SUMMARY.md Decisions §A). DIST-03 complete. Creator-verified 7 incognito checks at launch time + self-verified reader test.
- **[02-05 BLOCKED 2026-04-19]** Sender.net DOI gated behind account verification → sending-domain authentication required → no direct DNS access to `spatialmedialab.org` (DNS at InterNetX AutoDNS under Timo's account). Research doc produced with three options and ordered resumption checklist. DIST-01 remains in-progress. Primary decision input needed: pivot to `andrewrahman.com` + ImprovMX forwarder, or wait on Timo, or run both in parallel.

## Session Continuity

Last session: 2026-04-19T16:30:00.000Z
Stopped at: Phase 02 Plan 05 blocked on sending-domain decision after discovering jackhost-Plesk has no DNS editor for spatialmedialab.org (DNS actually hosted at InterNetX AutoDNS). Research doc complete; Andrew researching before deciding on Option A (pivot to andrewrahman.com + ImprovMX forwarder) vs Option B (ask Timo to add records).
Resume files:
  - PRIMARY: .planning/phases/02-email-capture-funding-infrastructure/02-05-SENDING-DOMAIN-RESEARCH.md (active blocker)
  - Phase 05 pause (secondary, unchanged): .planning/phases/05-launch-announcements/PAUSE-T-10-v3.md
