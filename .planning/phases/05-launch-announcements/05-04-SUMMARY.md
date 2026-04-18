---
plan: 05-04
phase: 05-launch-announcements
status: complete
completed: 2026-04-18
requirement: ANNC-04
---

# Summary: 05-04 Instagram Reel + Carousel

## What was built

Two draft files committed and approved by Andrew:

- `drafts/ig-reel-caption.md` — Reel caption (hook + body + 5 hashtags), video spec (15–30s, 9:16, CONT-02+CONT-01), composition walkthrough (0:00–0:30 timeline), descope branch to static feed post if CONT-02 slips by T-2, pre-launch bio-update checklist.
- `drafts/ig-carousel-caption.md` — Optional companion carousel (3–5 frames with overlay text, alt-text per frame, zodiac 12-frame variant documented per Rule 6).

## Decisions

- **Reel or descoped static post:** TBD at T-2 (2026-04-26) — depends on Phase 4 CONT-02 readiness. Descope branch documented in `ig-reel-caption.md`.
- **Zodiac variant:** Available in `ig-carousel-caption.md`. Decision deferred to publish time (5-frame primary is the default).
- **Bio-link:** andrewrahman.com/get-osd — update at T-1 (2026-04-27).
- **Hashtag count confirmed:** exactly 5 (#SpatialAudio #DolbyAtmos #AudioPlugin #BinauralAudio #FreePlugin).

## Publish timeline (pending)

| Step | When | Action |
|------|------|--------|
| T-2 (2026-04-26) | Confirm CONT-02 status | Trigger descope if not ready |
| T-1 (2026-04-27) | Bio-link update | Instagram Edit Profile → Website → andrewrahman.com/get-osd |
| T-0 (2026-04-28 10:25 CET) | Publish Reel | Per launch-day-runbook step 6 |
| T-0+30 (approx 10:55 CET) | Optional carousel | Same-day companion if desired |

## Self-Check: PASSED

- Both files exist and committed (e6b9f9c, 7a01dab)
- Automated acceptance criteria: all pass
- Andrew approved drafts: 2026-04-18
- Stereo-first framing (Rule 1): ✓ ("stereo out of the box, grows into binaural and surround")
- Hashtag count: exactly 5 ✓
- No DSP jargon: ✓
- Zodiac: present in carousel only, clearly optional ✓ (Rule 6)
