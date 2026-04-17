---
phase: 02-email-capture-funding-infrastructure
plan: 06
subsystem: patreon-creator-page
status: partial
tags: [patreon, funding, creator-page, seed-posts, tier-configuration, brand-identity, partial, checkpoint-paused]

# Dependency graph
requires:
  - phase: 02-email-capture-funding-infrastructure
    provides: "02-CONTEXT.md D-21 (identity = Andrew Rahman personally, not SML) · D-22 (SML ≠ Spatial Media Library disambiguation) · D-23 (4 tiers $3/$10/$25/$100) · D-24 (free-forever GPL; future tools may be patron-first) · D-25 (Berlin one-liner, low-urgency) · D-26 (cross-links set) · D-27 (3 seed posts required) · D-28 (seed-post visibility map: public / patron-only / public) · D-29 (Claude drafts; user revises for voice)"
  - phase: 02-email-capture-funding-infrastructure
    provides: "02-UI-SPEC.md Patreon Page tier table + Patreon Seed Posts copy baselines"
provides:
  - ".planning/phases/02-email-capture-funding-infrastructure/drafts/ — 6 Markdown drafts (page-about, tier-copy, post-01-welcome, post-02-technical, post-03-roadmap, post-04-project-links) as in-repo source of truth for what is pasted into the Patreon dashboard"
  - "docs/phase-02-evidence/patreon-graphics/ — branded graphics bundle (avatar 500×500, cover 2500×1000 centre-safe, 4 tier images 460×200 matched to site design-system rainbow palette, post-01 cover 1200×675). Includes _generate.py (deterministic, Pillow-based) so revisions are reproducible."
  - ".planning/phases/02-email-capture-funding-infrastructure/02-06-PATREON-SETUP.md — consolidated top-to-bottom paste-ready walkthrough the user works through in-browser (single-doc replacement for the Task 3/4 checkpoint step-list)"
affects:
  - "02-05 Tally/Sender.net thank-you page: depends on `patreon.com/andrewrahman` resolving publicly once the page is launched (Task 4 Step D) — until launch, the thank-you Patreon link falls back to Patreon's not-yet-launched state per RESEARCH.md Pitfall 3"
  - "Phase 5 re-visit: annual billing deferred here (Patreon gates until 3 months live + $200/mo) — re-enable flagged against D-30 when eligibility hits"
  - "Phase verifier: MUST flag this plan as partial / checkpoint-paused until user returns Task 4 resume signal (curl -sI `patreon.com/andrewrahman` showing HTTP 200/301 + 4 screenshots in docs/phase-02-evidence/ + reader-test passes)"

# Tech tracking
tech-stack:
  added:
    - "Pillow (Python) — used only by docs/phase-02-evidence/patreon-graphics/_generate.py for deterministic graphics regeneration; not a runtime dependency of the plugin or site"
  patterns:
    - "Drafts-as-source-of-truth: the 6 Markdown files in drafts/ are the canonical copy; Patreon dashboard state is downstream. Post-launch divergence is caught by a periodic manual diff (T-02-28 mitigation)"
    - "Space-exploration tier naming: Stargazer → Astronaut → Commander → Mission Control. Mirrors the 3D/spatial nature of the Library; tier images use the site design-system rainbow palette (tap 7 / tap 6 / tap 10 / tap 12)"

key-files:
  created:
    - ".planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-page-about.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-tier-copy.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-01-welcome.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-02-technical.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-03-roadmap.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-04-project-links.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/02-06-PATREON-SETUP.md"
    - "docs/phase-02-evidence/patreon-graphics/avatar-500x500.png"
    - "docs/phase-02-evidence/patreon-graphics/avatar-andrew-rahman.jpg"
    - "docs/phase-02-evidence/patreon-graphics/cover-2500x1000.png"
    - "docs/phase-02-evidence/patreon-graphics/post-01-cover-1200x675.png"
    - "docs/phase-02-evidence/patreon-graphics/tier-1-stargazer.png"
    - "docs/phase-02-evidence/patreon-graphics/tier-2-astronaut.png"
    - "docs/phase-02-evidence/patreon-graphics/tier-3-commander.png"
    - "docs/phase-02-evidence/patreon-graphics/tier-4-mission-control.png"
    - "docs/phase-02-evidence/patreon-graphics/_generate.py"
  modified: []

key-decisions:
  - "Task 1 (draft 6 in-repo Markdown files) delivered against plan baseline (5 files) PLUS one additional file: patreon-post-04-project-links.md. Rationale: Patreon 2026 UI only accepts 6 platform-gated social link slots (YouTube/Instagram/Twitter/Facebook/Twitch/TikTok) — it rejects GitHub / spatialmedialab.org / andrewrahman.com / privacy URLs. D-26 cross-links cannot live in the social slots, so Post 4 is pinned to the top of the page as the cross-links home. Documented in-line at 02-06-PATREON-SETUP.md Step 7."
  - "Tier naming swapped from baseline Supporter/Patron/Partner/Founder to space-exploration theme (Stargazer/Astronaut/Commander/Mission Control) per S3198. Prices and benefits unchanged from D-23; labels only. Each tier maps to one slot of the 12-tap rainbow palette (tap 7 Stargazer, tap 6 Astronaut, tap 10 Commander, tap 12 Mission Control) so the Patreon cards read as a natural extension of andrewrahman.com."
  - "Berlin one-liner (D-25) placement adapted: Patreon 2026 UI exposes no sidebar / footer copy slot on creator pages. D-25 was tagged low-urgency with a drop-if-no-slot instruction — instead of dropping entirely, the Berlin line is folded into the pinned public Post 4, keeping the pointer to spatialmedialab.org events discoverable above the fold."
  - "Billing-plan selection (Step C — choose Standard 10% plan) deferred until post-launch. Patreon's 2026-04-16 dashboard gates Payouts / Earnings settings behind the Launch action; the billing-plan screen only appears after step 10. Documented in Step 3 of 02-06-PATREON-SETUP.md; the Standard plan choice will be made during the launch flow."
  - "Technical Post 2 rewritten against agent_docs/architecture.md ground truth: HRTF profile count corrected from 6 → 5 (KU100/CIPIC/HUTUBS/MIT KEMAR/SADIE); phase-vocoder spec expanded (2048-point STFT, 4× overlap, Laroche-Dolson phase locking, Röbel transient detection, ±12 semitones, equal-power crossfade). The original plan draft had speculative specs that did not match the shipped plugin."
  - "Graphics bundle established at docs/phase-02-evidence/patreon-graphics/ (avatar, cover, 4 tier cards, post-01 cover) using a deterministic Pillow generator. Cover composition is centre-safe: desktop creator-card overlays the left ~40% and mobile crops to centre, so the generator places the wordmark and attribution strip in the always-visible zone (S146/S147)."
  - "D-23 partial delivery: annual billing omitted at launch — Patreon enforces 3-month + $200/mo eligibility window (RESEARCH.md Pitfall 4). Monthly tiers ship in full; annual re-enables post-eligibility via Phase 5 revisit (D-30). About section carries a soft-mention setting user expectation."

# Metrics
duration: N/A (agent-side: drafts + graphics + setup-guide authored across 6 prior commits; this execution run records + partial-summaries only)
completed: 2026-04-17
---

# Phase 2 Plan 06: Patreon Creator Page (Partial — In-Repo Drafts + Graphics + Setup-Guide Complete) Summary

**Six in-repo Markdown drafts (page-about, tier-copy, post-01-welcome, post-02-technical, post-03-roadmap, post-04-project-links), a branded graphics bundle (avatar + centre-safe cover + 4 space-themed tier cards + post-01 cover, with deterministic Pillow regenerator), and a consolidated 02-06-PATREON-SETUP.md top-to-bottom walkthrough are all committed to the repo as the canonical source for what gets pasted into patreon.com/andrewrahman. Task 3 substantive dashboard work (creator account + vanity URL + 4 tiers + About + 4 seed posts uploaded as drafts) is done per the setup-guide progress tracker; Task 4 (preview → LAUNCH → incognito verification → screenshots) blocks on the user completing Steps 9–11 of 02-06-PATREON-SETUP.md in-browser. DIST-03 not yet satisfied — page remains in pre-launch state until the user clicks Launch and returns the Task 4 resume signal.**

## Plan Status

**Partial — do not mark 100% complete. Checkpoint-paused on Task 4 human-action gate.**

| Task | Description | Status | Owner |
| ---- | --- | --- | --- |
| 1 | Draft 5 in-repo Markdown files (page-about, tier-copy, 3 seed posts) | **Complete** (plus Post 4 added for D-26 cross-links per live-state adaptation) | Agent (prior commits 35fe5fe + 1a9d266 + 3cafd02 + cfba575) |
| 2 | User revises drafts for voice (D-29, D-11) | **Complete** (revisions landed via commits 1a9d266 / e8ae5d2 / 3cafd02 / cfba575 — tier renames, tech-post corrections, Post 4 addition, cover redesign) | User (via prior agent-assisted revisions) |
| 3 | Create Patreon creator account + vanity URL + tiers + About (dashboard) | **Substantively complete** — Steps 1,2,4,5,8 of 02-06-PATREON-SETUP.md checked off (avatar, cover, vanity URL, 4 tiers with tier images, About section, 4 seed posts uploaded as drafts). Step 3 (billing) deferred until post-launch per Patreon's gating; Step 6 (sidebar) dropped per 2026 UI; Step 7 (cross-links) adapted to pinned Post 4. | User (via dashboard + 02-06-PATREON-SETUP.md walkthrough) |
| 4 | Publish posts + LAUNCH page + incognito verification + screenshots | **Blocked on user** — Steps 9 (preview), 10 (click Launch), 11 (incognito verify), 12 (reader test), 13 (resume signal) of 02-06-PATREON-SETUP.md pending; 4 screenshots (`patreon-vanity-url.png`, `patreon-tiers.png`, `patreon-launch-confirmation.png`, `patreon-page-live.png`) not yet saved under docs/phase-02-evidence/ | User |

The plan closes only after the user completes Task 4 Steps A–F of the plan (= Steps 9–13 of 02-06-PATREON-SETUP.md) and posts the Task 4 resume signal:

```
Patreon launched
<paste output of: curl -sI https://patreon.com/andrewrahman | head -1>
```

plus confirmation the 4 screenshots exist and the "what is this Patreon for?" reader test passes. Phase verifier should flag this plan as incomplete until that signal arrives. DIST-03 (≥ 2 public posts + pipeline framing visible in logged-out browser) is **not** yet satisfied.

## Performance

- **Started:** 2026-04-15T23:26:00Z (original Task 1 kickoff per plan commit 35fe5fe)
- **Partial-complete:** 2026-04-17T10:03:00Z (this agent run — reconciling prior manual work with plan success-criteria)
- **Duration:** ~1.5 days across multiple sessions (agent + user dashboard work + graphics regeneration passes)
- **Tasks complete (agent + user, agent-side authorship):** 1 of 4 fully complete (Task 1); Tasks 2 + 3 substantively complete pending user launch
- **Commits touching this plan:** 6 (35fe5fe drafts + 9c8ff52 setup-guide + 1a9d266 tier rename + e8ae5d2 tier-image redesign + 3cafd02 live-state adaptation + cfba575 cover redesign)
- **Files created:** 6 drafts + 9 graphics + 1 setup-guide = 16

## Accomplishments

**Task 1 — Draft Patreon copy as in-repo Markdown (commit 35fe5fe + later revision commits)**

All 6 Markdown drafts committed under `.planning/phases/02-email-capture-funding-infrastructure/drafts/`:

- `patreon-page-about.md` — tagline, About section, source-code callout, future-plugins soft-mention, annual-billing soft-mention, Berlin one-liner, 4 cross-links (D-26). Both brand names present (Spatial Media Library ×3, Spatial Media Lab ×3).
- `patreon-tier-copy.md` — 4 tiers Stargazer / Astronaut / Commander / Mission Control at $3/$10/$25/$100 with monthly-only billing per RESEARCH.md Pitfall 4. Header preamble notes the space-theme rationale.
- `patreon-post-01-welcome.md` — PUBLIC anchor post with Why Patreon / Pipeline Vision / Source Code / CTA sections.
- `patreon-post-02-technical.md` — PATRON-ONLY ($3+) HRTF / Phase Vocoder / Trajectory Engine deep-dive. Corrected from plan baseline to match agent_docs/architecture.md ground truth (5 HRTF profiles not 6; 2048-point STFT with 4× overlap, Laroche-Dolson phase locking, Röbel transient detection; equal-power crossfade).
- `patreon-post-03-roadmap.md` — PUBLIC pipeline-themes post (spatial processing / binaural rendering / immersive mixing) with no specific promises and why-open-source framing.
- `patreon-post-04-project-links.md` — PUBLIC pinned post carrying the D-26 cross-links (source code, spatialmedialab.org, andrewrahman.com, privacy) after Patreon's social-link slots turned out to be platform-gated (YouTube / Instagram / Twitter / Facebook / Twitch / TikTok only).

Task 1 automated verify (plan §verify) re-ran green this session: all 5 canonical draft files present, `Spatial Media Library` ≥ 1 in every draft, `Spatial Media Lab` ≥ 1 in every post, `HRTF` in technical post, `PATRON-ONLY` visibility tag on technical post.

**Task 1 extension — Branded graphics bundle (commits 1a9d266, e8ae5d2, cfba575)**

`docs/phase-02-evidence/patreon-graphics/`:

| # | File | Spec | Purpose |
|---|------|------|---------|
| 1 | avatar-500x500.png | 500×500 | Profile avatar (circle next to creator name) — centre-cropped from Andrew's headshot |
| 2 | cover-2500x1000.png | 2500×1000 | Page header banner — centre-safe composition (void bg + DM Sans wordmark + rose underline + cyan attribution + 12-tap rainbow strip); accounts for Patreon desktop creator-card overlay (~40% left) and mobile centre-crop |
| 3 | tier-1-stargazer.png | 460×200 | Tier 1 — tap 7 light blue star cluster + constellation lines |
| 4 | tier-2-astronaut.png | 460×200 | Tier 2 — tap 6 cyan visor astronaut silhouette |
| 5 | tier-3-commander.png | 460×200 | Tier 3 — tap 10 violet chevrons + command star with orbit arc |
| 6 | tier-4-mission-control.png | 460×200 | Tier 4 — tap 12 rose Earth + rocket exhaust (matches site's `--accent-rose` Patreon block) |
| 7 | post-01-cover-1200x675.png | 1200×675 | Post 1 thumbnail — plugin screenshot + branded overlay + 12-tap strip |
| 8 | _generate.py | — | Deterministic Pillow-based regenerator (seeded, self-contained) |

All graphics share the andrewrahman.com visual system (`#03060b` void, 12-tap rainbow from `Source/PluginEditor.cpp`, DM Sans Bold display, JetBrains Mono accents) so the Patreon page reads as a natural extension of the creator-home site.

**Task 1 extension — Consolidated setup guide (commits 9c8ff52, 3cafd02)**

`.planning/phases/02-email-capture-funding-infrastructure/02-06-PATREON-SETUP.md` is a single-document top-to-bottom walkthrough the user works through in-browser. Each of the 13 steps has a ✅ / ❌ / 📸 / 📋 marker, every paste-block is exactly the text that goes into Patreon, and the live-progress tracker at the top is the authoritative state for Task 3/4 completion. This replaces the original plan's Task 3+4 step-list, which would have been redundant for the user to work through while the plan execution was in-flight.

**Task 2 — User revisions (commits 1a9d266 + e8ae5d2 + 3cafd02 + cfba575)**

User-driven revisions that landed via commits (D-29 satisfied — user had voice-review gate on all published copy before dashboard paste):

- Tier naming from Supporter/Patron/Partner/Founder → Stargazer/Astronaut/Commander/Mission Control (space-theme, tracks Library's 3D character)
- Technical post DSP claims corrected against agent_docs/architecture.md ground truth
- Post 4 added to carry D-26 cross-links after platform-gated social slots discovered
- Cover image redesigned for centre-safe composition after first desktop-overlay test showed Patreon's creator-card covering the left 40%

**Task 3 — Dashboard work (per 02-06-PATREON-SETUP.md live progress tracker)**

Per the progress tracker at top of `02-06-PATREON-SETUP.md` (updated 2026-04-16 17:42 CEST):

- [x] Step 1 — Creator account created, avatar uploaded, cover uploaded
- [x] Step 2 — Vanity URL chosen (user to confirm exact string via Task 4 resume signal)
- [ ] Step 3 — **Billing plan deferred** — Patreon gates Payouts settings behind Launch; Standard 10% selection will happen inline with Step 10
- [x] Step 4 — 4 monthly tiers (Stargazer / Astronaut / Commander / Mission Control) created with tier images
- [x] Step 5 — About section pasted and revised for voice
- [x] Step 6 — **Dropped** — no sidebar slot in 2026 Patreon UI; Berlin one-liner folded into Post 4
- [ ] Step 7 — **Adapted** — social slots are platform-gated; Post 4 is pinned and carries D-26 cross-links; selection of which personal socials to connect is a later user decision
- [x] Step 8 — 4 seed posts uploaded (as drafts); Post 4 confirmed live and pinned

## Known Stubs / Deferrals

- **Annual billing** — deferred (Patreon enforces 3-month + $200/mo eligibility window; RESEARCH.md Pitfall 4). Monthly tiers ship in full; annual re-enables post-eligibility in Phase 5 per D-30. About section carries a soft-mention so expectation is set.
- **Billing plan (Step 3)** — deferred until post-launch per Patreon's dashboard gating. Standard 10% will be selected inline with Launch.
- **Social link slot selection (Step 7)** — adapted, not completed. Social slots only accept YouTube/Instagram/Twitter/Facebook/Twitch/TikTok; none of those are currently in scope. User revisits later. Post 4 carries the cross-links in the meantime.

## Deviations from Plan

### Auto-applied (Rule 2 — missing critical functionality) and live-state adaptations

**1. [Rule 2 — Missing D-26 cross-link carrier]** Patreon 2026 UI's social slots are platform-gated; raw URLs to GitHub, spatialmedialab.org, andrewrahman.com, and privacy are rejected. Added a 6th draft (`patreon-post-04-project-links.md`) as a pinned public post that carries all 4 D-26 cross-links. Commit 3cafd02.

**2. [Rule 2 — Missing graphics bundle]** Plan did not specify cover / avatar / tier images; Patreon's launch-readiness checklist requires them. Added `docs/phase-02-evidence/patreon-graphics/` with 7 graphics + deterministic Pillow generator. Cover composition is centre-safe for desktop creator-card overlay + mobile centre-crop (commits 1a9d266, e8ae5d2, cfba575).

**3. [Live-state adaptation — D-25 Berlin one-liner]** Patreon 2026 UI exposes no sidebar / footer copy slot on creator pages. D-25 was tagged low-urgency with drop-if-no-slot instruction. Instead of dropping, line is folded into pinned Post 4. Documented in 02-06-PATREON-SETUP.md Step 6. Commit 3cafd02.

**4. [Live-state adaptation — Billing plan Step 3]** Patreon 2026 UI gates Payouts settings behind Launch. Standard 10% selection deferred to launch flow (Step 10). Documented in 02-06-PATREON-SETUP.md Step 3. Commit 3cafd02.

**5. [Rule 1 — Technical Post 2 factual correction]** Original draft claimed 6 HRTF profiles (`KEMAR/CIPIC 20/21/27/40/MIT KEMAR`); ground truth per agent_docs/architecture.md is 5 (`KU100/CIPIC/HUTUBS/MIT KEMAR/SADIE`). Phase vocoder spec under-specified; rewritten to include 2048-point STFT, 4× overlap, Laroche-Dolson phase locking, Röbel transient detection, equal-power crossfade. Commit 1a9d266 area.

## Threat Flags

None. Plan's threat_model was fully scoped; no new network endpoints, auth paths, or trust-boundary changes introduced beyond what the register already covered. T-02-26 (2FA on creator account) surfaced in 02-06-PATREON-SETUP.md Step 1 as a user-side action; Task 4 resume signal should confirm.

## Next Steps (User)

Work through `.planning/phases/02-email-capture-funding-infrastructure/02-06-PATREON-SETUP.md` Steps 9–13:

1. **Step 9 — Preview check:** Walk through each post in Patreon preview mode. Confirm Markdown rendering (bold, links, bullets), confirm Post 2 public teaser shows correctly above the paywall, confirm no draft-metadata artifacts left in bodies.
2. **Step 10 — LAUNCH:** Patreon → Page settings → Launch checklist → verify all pre-flight items checked → click **Launch page** → screenshot confirmation → save as `docs/phase-02-evidence/patreon-launch-confirmation.png`.
3. **Step 11 — Incognito verification:** Fresh browser (no Patreon cookies) → visit `patreon.com/andrewrahman` → confirm 7 checks from plan Task 4 Step E (page loads, tagline, About, 4 tiers, 3 visible posts + 1 locked, 3 links, Berlin line via Post 4) → screenshot as `docs/phase-02-evidence/patreon-page-live.png`.
4. **Step 12 — Reader test:** Read About + Post 1 end-to-end once. Answer: "what is this Patreon for?" → must come out as "funding the Spatial Media Library pipeline of spatial audio tools, of which OpenSpatialDelay is the first."
5. **Step 13 — Return resume signal:** Reply with:
   ```
   Patreon launched
   <paste output of: curl -sI https://patreon.com/andrewrahman | head -1>
   ```
   plus confirmation the 4 screenshots exist (`patreon-vanity-url.png`, `patreon-tiers.png`, `patreon-launch-confirmation.png`, `patreon-page-live.png`) plus reader-test pass.

After the resume signal is posted, the next executor run will update 02-05-PLAN Tally thank-you verification (the link `patreon.com/andrewrahman` will now resolve to a launched page), flip DIST-03 to Complete in REQUIREMENTS.md, and close out this plan by advancing STATE.md to Plan 02-07.

## Self-Check: PASSED

- [x] `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-page-about.md` — FOUND
- [x] `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-tier-copy.md` — FOUND
- [x] `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-01-welcome.md` — FOUND
- [x] `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-02-technical.md` — FOUND
- [x] `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-03-roadmap.md` — FOUND
- [x] `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-04-project-links.md` — FOUND
- [x] `.planning/phases/02-email-capture-funding-infrastructure/02-06-PATREON-SETUP.md` — FOUND
- [x] `docs/phase-02-evidence/patreon-graphics/` (9 files incl. `_generate.py`) — FOUND
- [x] Task 1 automated verify command (plan §verify) — PASSES (`VERIFY_PASS` printed)
- [x] Commit 35fe5fe (Task 1 initial drafts) — FOUND in git log
- [x] Commit 9c8ff52 (setup guide) — FOUND in git log
- [x] Commit 1a9d266 (tier rename + graphics) — FOUND in git log
- [x] Commit e8ae5d2 (tier image redesign) — FOUND in git log
- [x] Commit 3cafd02 (live-state adaptation + Post 4) — FOUND in git log
- [x] Commit cfba575 (cover redesign) — FOUND in git log
- [ ] Task 4 resume signal — PENDING (user-side, blocks plan close)
- [ ] `docs/phase-02-evidence/patreon-vanity-url.png` — PENDING (user screenshot)
- [ ] `docs/phase-02-evidence/patreon-tiers.png` — PENDING (user screenshot)
- [ ] `docs/phase-02-evidence/patreon-launch-confirmation.png` — PENDING (user screenshot)
- [ ] `docs/phase-02-evidence/patreon-page-live.png` — PENDING (user screenshot)

All agent-authored artifacts confirmed present on disk; all referenced commits confirmed in git log. Remaining items are user-owned and block plan close.
