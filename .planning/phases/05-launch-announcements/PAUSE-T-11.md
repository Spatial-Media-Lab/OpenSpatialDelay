---
status: paused
phase: 05-launch-announcements
paused_at: 2026-04-17T19:30+02:00
paused_by: andrew
reason: two new strategic inputs require re-evaluation of drafts already produced and plans still to execute
---

# Phase 5 Pause — T-11 (2026-04-17 19:30 CET)

## Current Position

| Plan | Status | Key Commits |
| ---- | ------ | ----------- |
| 05-00 Launch orchestration | ✅ Complete | 412b4b3, 83c6954, 86be2dd, a81ecab, 7445525 |
| 05-01 SML blog post | ✅ Complete (approved after 18 red-lines) | a88d430 (draft), 9f9ee88 (revision), 2dba46e (closure) |
| 05-02 SML newsletter | ⏸ Draft at 97832aa, checkpoint OPEN — Andrew about to red-line | 97832aa |
| 05-03 LinkedIn | ⏸ Not started |
| 05-04 Instagram | ⏸ Not started |
| 05-05 KVR listing | ⏸ Not started |
| 05-07 Berlin DMs | ⏸ Not started |
| 05-06 Review-pitch outreach (Wave 2) | ⏸ Not started, DKIM HITL deferred |
| 05-08 Guest-pitch drafts (Wave 3) | ⏸ Not started |

## Two new strategic inputs to incorporate

### Input 1 — Zodiac × 12 Taps

**Insight:** OpenSpatialDelay has 12 delay taps. There are 12 zodiac signs. This is a potentially strong
creative/marketing angle — naming, visual iconography, preset grouping, launch post framing,
Instagram carousel concept, Patreon reward-tier naming (already space-themed per memory).

**Potential incorporation surfaces:**
- 05-01 blog post — could add a paragraph or visual reference
- 05-02 newsletter — subject-line angle?
- 05-04 Instagram — carousel = 12 slides = 12 signs = perfect fit
- 05-05 KVR listing — positioning language
- Patreon post cross-references (already-committed Phase 2 tiers are space-themed)
- Factory-preset naming convention (future work, not launch)

**Open questions:**
- Is this a tongue-in-cheek hook or a load-bearing piece of positioning?
- Does a zodiac set of 12 presets exist or need to be created?
- Does the UI already visualise the 12 taps in a way that lends itself to 12 signs (circle? grid?)?

### Input 2 — Stereo production via Binaural + Stereo algorithm

**Insight:** The plugin isn't only for immersive/Ambisonics work. It also delivers for conventional
stereo productions via the Binaural render + Stereo-output algorithm option. Current drafts
(05-01 blog, 05-02 newsletter) under-sell this and risk alienating the larger-but-less-immersive
audience of stereo-music producers.

**Current state of the surfaces:**
- 05-01 blog: frames immersive first ("Ambisonics sessions, Atmos mixes, binaural headphone pieces").
  Stereo only appears inside the 7-algorithm bullet as "a stereo mix". Under-weight.
- 05-02 newsletter: same framing — stereo is one item in a list of five output targets.
- 05-03 LinkedIn (not yet drafted): audience skews broader; stereo framing MUST land here.
- 05-04 Instagram: audience is huge + broad; stereo-music producers dominate. Stereo framing essential.
- 05-05 KVR: KVR audience = ~80% stereo-music producers. Stereo framing is the headline.

**Implications:**
- 05-01 may need a revision pass to give stereo-production its own paragraph or bullet.
- 05-02 subject/preview/body likely need rewrite with stereo producers as a named audience.
- 05-03..05-05 planning must lead with stereo, not trail with it.

## Resume path

1. Andrew provides the details/angles for both inputs (any existing zodiac preset work? stereo-use-case examples? demo clips?).
2. Re-evaluate drafts:
   - Does 05-01 blog get a revision pass (mini-PR) or stand as is?
   - Does 05-02 newsletter get a rewrite?
3. Lock the "stereo producer" + "zodiac" framing guidance before drafting 05-03..05-05.
4. Resume `/gsd-execute-phase 5` from 05-02 checkpoint.

## Deferred items still pending

- DKIM + DMARC setup at jackhost.net (HITL) — Andrew, target T-3 (2026-04-25)
- 05-06 from-address decision — depends on DKIM outcome
- Launch precheck T-7 daily walk — starts 2026-04-21
- Phase 4 (Demo Content) plan — Andrew to create 04-00-PLAN.md by 2026-04-20
- KVR Developer Account application — Andrew, target before 2026-04-28
