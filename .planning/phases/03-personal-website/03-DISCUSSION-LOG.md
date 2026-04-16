# Phase 3: Personal Website — Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in `03-CONTEXT.md` — this log preserves the alternatives considered and the reasoning.

**Date:** 2026-04-16
**Phase:** 03-personal-website
**Areas discussed:** Feature pitch & copy (B-2, B-3), Design system scope, Assets/identity/IA (B-1, B-4…B-8)
**Session continuity:** Resumed from `03-DISCUSS-CHECKPOINT.json` (Area 1 partially done in prior session; Areas 2 & 3 started fresh in this one).

---

## Area 1 — Feature pitch & copy (B-2, B-3)

### Q1a — Hero grid structure (prior session)

| Option | Description | Selected |
|--------|-------------|----------|
| Keep 4-stat grid | Structure stays, stats get replaced. | ✓ |
| Collapse to 3-stat | Trim to top 3 to emphasise each more. | |
| Expand to 5-stat | Add one more capability number. | |

**User's choice:** Keep the 4-stat grid structure; replace the numbers inside it.
**Notes:** Same session concluded the old `±12 semitones / 2048-sample FFT / GPL-3.0` trio was wrong for v1.0.0.

---

### Q1b — Which 4 hero stats?

| Option | Description | Selected |
|--------|-------------|----------|
| 12 / 7 / 13 / 5 (Recommended) | `12 echoes · 7 spatialization algorithms · 13 trajectory shapes · 5 HRTF profiles` | partial |
| 12 / 7 / 13 / Open-source | Drops HRTF, adds Free & open-source | |
| 12 / 7 / 5 / Cross-platform | Drops trajectories, adds VST3+AU macOS+Windows | |
| Let me nominate | Freeform | |

**User's choice:** Modified the recommended slate twice. Accepted 12/7 and 5, renamed "12 echoes" → **"12 Delay Taps"**, and rejected "13 trajectory shapes" as "not that impressive". Also asked for clarification on "Simple" in the spatialization list (answered: not in the 7 spat algorithms; it's in the 6 binauralization modes).
**Notes:** Clarification on "Simple" routed to `03-CONTEXT.md` Specifics section. The 7 spatialization algorithms were verified against `Source/PluginProcessor.cpp:1801-1807`.

---

### Q1c — 4th stat replacement

| Option | Description | Selected |
|--------|-------------|----------|
| Free & open-source (Recommended) | Positioning in plain English | |
| VST3 + AU, macOS + Windows | Cross-platform compatibility | |
| OSC in + out (ADM-OSC) | Power-user signal | |
| Something else (type it) | Freeform | ✓ |

**User's choice:** `"Probably X Presets"` → verified `NUM_FACTORY_PRESETS = 70` in `Source/PresetData.cpp:73` → locked as **"70 factory presets"**.
**Notes:** Final hero stats: `12 Delay Taps · 7 spatialization algorithms · 5 HRTF profiles · 70 factory presets`.

---

### Q2a — Feature card count (prior session)

| Option | Description | Selected |
|--------|-------------|----------|
| 6 cards (even) | Up from current 5 | ✓ |
| Keep 5 cards | Current state | |
| 4 cards | Tighter | |

**User's choice:** 6 cards. Even number; user flagged some current cards as "pointlessly complex" and wanted room to add depth.

---

### Q2b — Which 6 capabilities get cards?

| Option | Description | Selected |
|--------|-------------|----------|
| Capabilities-forward (Recommended) | 12 taps · Trajectories · HRTFs · 7 algorithms · Doppler (simplified) · Open-source | |
| Producer-forward | 12 taps · Trajectories · 70 presets · Doppler · Binaural · Open-source | |
| Pro-user forward | 12 taps · Trajectories · HRTFs · 7 algorithms · OSC in/out (ADM-OSC) · Open-source | ✓ |
| Let me mix my own 6 | Freeform | |

**User's choice:** **Pro-user forward** slate. Explicit targeting of film/VR/live spatial-audio practitioners.
**Notes:** The pitch-shifted Doppler card from the current live site is dropped from the hero page → moved to Deferred Ideas. Card #4 (7 algorithms) requires verifying the OSC mapping details in `Source/OSCController.*` at plan-time.

---

### Q3/Q4 — Hero tone + copy-pass depth (prior session)

| Question | Answer |
|----------|--------|
| Hero copy tone | Keep producers-first/sound-designers-second framing (no rewrite). |
| B-3 copy-edit depth for this phase | Hero headline + the 6 new feature cards + Download CTA only. Full-page review deferred. |

---

## Area 2 — Design system scope

### Q1 — Style guide home

| Option | Description | Selected |
|--------|-------------|----------|
| Plugin repo (Recommended) | Longest-lived surface | |
| Site repo (andrewrahman-com) | Newest, most visible | ✓ (with migration plan) |
| New SpatialCore repo | Clean separation | migration target |

**User's choice:** **Site repo for now, SpatialCore repo later.** Explicitly: `"For now it will live in the Site repo, but later, which it is solidified and verified, will move to the SpatialCore repo."` Migration intent documented in D-09.

---

### Q2 — Token format

| Option | Description | Selected |
|--------|-------------|----------|
| Markdown + JSON (Recommended) | Human + machine | ✓ |
| Markdown only | Simplest; drift prone | |
| Markdown + JSON + Storybook | Heaviest | |

**User's choice:** Markdown + JSON. (Decision applies when the style-guide phase actually happens — see scope decision below.)

---

### Q3 — Canonical source for drifted tokens

| Option | Description | Selected |
|--------|-------------|----------|
| Site values win (Recommended) | Newest, most visible | |
| Plugin values win | Longest-lived | ✓ (with accessibility override) |
| Pick new OKLCH-normalised values | Most work | |

**User's choice:** **Plugin canonical by default, BUT accessibility requirements override.** Triggered follow-up scope question below. Captured as D-09 alongside the migration plan.

---

### Q4 — Phase 3 scope for the style guide (NEW — raised by user)

**User surfaced a strategic concern:** `"launch deadline is May 1st. Is this something we could deal with after launch?"`

| Option | Description | Selected |
|--------|-------------|----------|
| Accessibility audit only (Recommended) | WCAG-AA check, fix failures, defer formal style guide | ✓ |
| Audit + style-guide doc (original Area 2) | Accessibility + DESIGN-SYSTEM.md + design-tokens.json + Patreon retrofit | |
| Full audit + site + Patreon + plugin v1.0.1 patch | All aligned pre-launch | |
| Defer entirely | No Phase 3 design-system work at all | |

**User's choice:** **Accessibility audit only** for Phase 3. Formal style-guide authoring, Patreon retrofit, SpatialCore migration all deferred to a post-launch phase.
**Notes:** 15-day window to launch drove this narrowing. WEB-01/02/03 don't require a formal style guide; the site already looks right; drift is ~20% in supporting tokens only.

---

### Q5 — Retrofit scope (before the narrowing above)

| Option | Description | Selected |
|--------|-------------|----------|
| Site + Patreon now; plugin v1.1 (Recommended) | Middle scope | ✓ (conditional on Q4) |
| Everything aligned now | Plugin patch release | |
| Style guide only, no retrofits | Minimal | |

**User's choice:** Site + Patreon generator now, plugin v1.1. Since Q4 deferred the whole style-guide work, this decision becomes the plan for the future post-launch phase, not Phase 3. Captured in D-09.

---

## Area 3 — Assets, identity & IA (B-1, B-4…B-8)

B-1, B-4, B-8 were mostly mechanical once other decisions landed — no branching needed. Four decisions required interactive selection.

### B-5 — Patreon CTA colour

| Option | Description | Selected |
|--------|-------------|----------|
| oklch(72% 0.15 290) (Recommended) | Light regal lavender ~#b49bd8 | ✓ |
| oklch(68% 0.19 295) | Deeper regal purple ~#a080d8 | |
| oklch(76% 0.12 295) | Pastel purple ~#c2b0dc | |
| Claude's discretion in-tool | Executor picks, user reviews | |

**User's choice:** `oklch(72% 0.15 290)`. Light regal lavender, warm-and-welcoming over nightclub-purple.

---

### B-6 — Email migration

| Option | Description | Selected |
|--------|-------------|----------|
| Marketing migrates, legal stays Gmail (Recommended) | Split: visible email = SML, GDPR controller = Gmail | |
| Everything migrates to SML | Full migration, privacy-policy version bump | ✓ |
| Everything stays Gmail | Defer | |

**User's choice:** **Full migration** including the privacy-policy data-controller field. Requires a privacy-policy version bump + changelog entry.
**Notes:** Test assertions in `tests/privacy-content.spec.ts` and `tests/homepage-content.spec.ts` must be updated in lockstep.

---

### B-7 — Get-OSD scroll vs route

| Option | Description | Selected |
|--------|-------------|----------|
| Planner investigates + decides (Recommended) | Research Sender.net redirect flow, decide at plan-time | ✓ |
| Keep /get-osd/ route, no change | Lowest risk | |
| Force inline scroll, replace route | Commit now, risk blocker | |

**User's choice:** Planner investigates. **Critical correction from user:** `"we have switched to Sender.net and no longer are using Tally"` — the backlog item B-7 mentioned Tally, but Phase 2 superseded Tally with Sender.net (see claude-mem S135). Research must target Sender.net's post-submission redirect behaviour.

---

### B-8 — Headshot rights

| Option | Description | Selected |
|--------|-------------|----------|
| Yes, I own the rights (Recommended) | Republish SML About headshot | ✓ |
| Verify first | Planner-confirms-before-downloading | |
| Different photo | User supplies later | |

**User's choice:** Rights clean. Own headshot on own org's site. Proceed with download.

---

## Claude's Discretion (areas user explicitly left flexible)

- Exact **headline copy** for each of the 6 feature cards, Hero headline, and Download CTA headline — B-3 execution.
- Whether to nudge the B-5 `oklch(72% 0.15 290)` by ≤2 lightness points if the accessibility audit demands it.
- Micro-copy on the "+1 CPU-lite mode" footnote/tooltip attached to the "5 HRTF profiles" hero stat.

---

## Deferred Ideas (captured for post-Phase-3 phases)

- **Post-launch design-system consolidation phase:** Formal DESIGN-SYSTEM.md + design-tokens.json (starting in site repo), Patreon-generator token retrofit, plugin alignment in v1.1, SpatialCore-repo migration after site stabilises.
- **Post-launch full-page copy review:** All sections not touched by the new feature pitch — Pipeline, Patreon headline, About Andrew, SysReq framing.
- **Pitch-shifted Doppler as a dedicated feature surface:** still in the plugin, not on the landing hero. Revisit for blog post, demo video, KVR listing.

---

## Session meta

- **Stale-doc fallout (not a decision, but context for planner):** During the prior session, same-day stale-doc cleanup corrected algorithm counts (7 not 8), HRTF counts (5 measured + 1 CPU-lite Woodworth, not 6 HRTFs), and trajectory lists across README, SPECIFICATION.md, PROJECT.md, and `memory/reference_spatialization.md`. Committed to main as `b2a865b`. All hero-stat and feature-card copy in Phase 3 MUST reflect this corrected state.
- **WEB-02 verdict:** complete off-site. `spatialmedialab.org/about` already lists Andrew (co-founder 2017), Basel Naouri (joined 2025), Timo Bittner (co-founder 2017, current president). REQUIREMENTS.md WEB-02 row should be flipped to Complete; no Phase 3 work needed for WEB-02.
