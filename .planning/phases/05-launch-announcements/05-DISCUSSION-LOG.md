# Phase 5: Launch Announcements — Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in `05-CONTEXT.md` — this log preserves the alternatives considered.

**Date:** 2026-04-17
**Phase:** 05-launch-announcements
**Areas discussed:** Launch cadence, Outreach sequencing, Newsletter list source

---

## Gray area selection

User was presented 4 candidate gray areas and multi-selected 3:

| Option | Description | Selected |
|--------|-------------|----------|
| Launch cadence | Single-day D-day burst vs rolling week | ✓ |
| Outreach sequencing | Pre-launch embargo vs T-0 vs staggered | ✓ |
| Positioning hero angle | Free/GPL vs Spatial vs Berlin indie | (left to Claude's discretion) |
| Newsletter list source | OSD Sender list vs SML newsletter | ✓ |

---

## Launch cadence

### Q1 — Single-day D-day burst or rolling week?

| Option | Description | Selected |
|--------|-------------|----------|
| Single coordinated D-day | All 5 channels publish within 2-hour window on one day; pre-staged outreach goes out morning-of | ✓ |
| Rolling week (Mon-Fri) | Each channel gets its own spotlight; sustained attention; diluted signal | |
| Soft-launch Mon + big-hit Fri | Blog+newsletter Mon, everything else Fri; build then push | |

**User's choice:** Single coordinated D-day
**Notes:** Chose the Recommended option; aligns with indie-dev simplicity.

### Q2 — What day-of-week for the main hit?

| Option | Description | Selected |
|--------|-------------|----------|
| Tuesday | Highest B2B engagement; typical indie-product launch day | ✓ |
| Thursday | Strong for creative-audience content; close enough to weekend for weekend coverage | |
| Wednesday | Midweek compromise | |
| I'll pick later — Claude's discretion | Planner decides based on readiness | |

**User's choice:** Tuesday
**Notes:** Recommended option accepted.

### Q3 — Absolute earliest launch window?

| Option | Description | Selected |
|--------|-------------|----------|
| ASAP — whenever Phases 3+4 are done | No calendar anchor | |
| Target a specific 2-week window | Calendar deadline | |
| Anchor to an external event | E.g., AES Berlin, ADC, spatial-audio industry moment | |
| (Free-text) | | ✓ |

**User's choice:** Free-text response — *"We need to have all of the links live before Superbooth which is May 7-10."*
**Notes:** Superbooth 2026 (Berlin, May 7–10) is the immovable anchor. This effectively selects "Anchor to an external event" but with a specific date. Target launch date derived: Tuesday April 28, 2026 (11 days out; clears before Superbooth).

---

## Outreach sequencing

### Q1 — When do review-pitch emails go out? (Segments 1-5)

| Option | Description | Selected |
|--------|-------------|----------|
| T-2 weeks pre-launch with embargo | Private download link + embargo date; industry-standard for plugins | |
| T-0 same-day as launch | Cold send launch morning; simpler; lower reply rate | ✓ |
| T-1 week pre-launch, no embargo | Soft heads-up without embargo | |
| Staggered: anchors first, rest at T-0 | High-priority get T-2 weeks; rest batch-send at launch | |

**User's choice:** T-0 same-day as launch
**Notes:** User's May travel situation (see next question) made embargo logistics impractical.

### Q2 — When do guest-pitch emails go out? (Segment 7 podcasts + meetups)

| Option | Description | Selected |
|--------|-------------|----------|
| T-3 weeks pre-launch | Recordings land in launch month | |
| T-0 at launch | Simultaneous with review-pitch | |
| AES Berlin + ADC first, rest at T-0 | Immediate for local/conference, rest at launch | |
| (Free-text) | | ✓ |

**User's choice:** Free-text response — *"Honestly, they need to go out late May because I am traveling most of May and would not be able to answer the emails if I get a response. Sending them closer to June is ideal."*
**Notes:** Key constraint surfaced — Andrew is traveling for most of May. Guest-pitch replies trigger scheduling conversations that need an active inbox. Delay to late May / early June keeps reply-handling realistic. This also reinforces Q1's choice to skip the embargo strategy.

### Q3 — Superbooth-specific outreach?

| Option | Description | Selected |
|--------|-------------|----------|
| Yes — send "see you at Superbooth" intros to Berlin contacts | Hainbach, Kirn, Horstmann, AES Berlin chair; coffee-meet ask | ✓ |
| No — keep outreach channel-agnostic | No face-to-face angle | |

**User's choice:** Yes
**Notes:** Leverages the geographic advantage. Treated in CONTEXT.md as a separate outreach category (D-07) distinct from review-pitch and guest-pitch — lightweight, relationship-first, sent within next 1-2 weeks.

---

## Newsletter list source

### Q1 — What is the "SML newsletter" for ANNC-02?

| Option | Description | Selected |
|--------|-------------|----------|
| Same Sender.net list at andrewrahman.com/get-osd | Email to osd-confirmed group; requires DIST-01 shipped + subs | |
| Separate spatialmedialab.org newsletter | SML has its own independent newsletter | ✓ |
| No newsletter at launch — drop ANNC-02 | Defer until OSD list has >50 subs | |
| SML newsletter if exists; fallback to OSD list | Adaptive — planner checks state | |

**User's choice:** Separate spatialmedialab.org newsletter
**Notes:** Decouples ANNC-02 from DIST-01 and the Sender.net outage. Downstream implication: planner must confirm who manages the SML newsletter send-path (noted as open question D-08a in CONTEXT.md).

---

## Claude's Discretion

User left the following areas to Claude's judgment, with planner expected to make sensible defaults:

- **Positioning hero angle** (free vs spatial vs Berlin-indie) — left at discussion gray-area selection. Recommendation in CONTEXT.md: per-channel — blog leads with capability+free, LinkedIn with Berlin-indie-dev story, IG with audible demo.
- **Per-channel content format split** — planner designs each channel's unique angle.
- **KVR listing category/tags** — planner picks after reviewing current KVR taxonomy.
- **Email template styling** (plain-text vs branded HTML per outreach mode) — planner decides.

---

## Deferred Ideas

Noted for future phases / v2 milestone (not acted on in Phase 5):

- YouTube channel for Andrew (v2 / COMM-01)
- Own podcast (v2)
- Merchandise (v2 or later)
- Product Hunt launch (already v2 COMM-02; consider promoting to v1.1 if launch generates signal)
- Launch-day livestream (deferred due to May travel; revisit for v1.1)
- Press-release distribution service (doesn't fit indie-dev positioning; skip)
- Sound-design/film-scoring outreach segment follow-up research (logged in `influencer-outreach-v1.0.md` Gaps)
- Formal AES Berlin speaker slot pitch (the Superbooth DM is the relationship-opener; formal talk proposal lives in the late-May guest-pitch wave)

## Scope dependencies noted during discussion

- **Phase 4 (Demo Content) has no plans yet.** Phase 5's critical path requires CONT-01 (30s binaural audio demo), CONT-02 (video/reel), CONT-03 (high-res screenshots). With launch target Apr 28, Phase 4 must be planned and executed in 11 days. Planner should surface this as a hard precondition at the top of Plan 05-00.
- **Plan 03-09 (cross-phase DIST-01 flip)** does NOT block Phase 5 per D-08 (newsletter uses separate SML list). Noted so planner doesn't add a false dependency.
