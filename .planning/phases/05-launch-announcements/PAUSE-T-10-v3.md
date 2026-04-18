---
status: paused
phase: 05-launch-announcements
paused_at: 2026-04-18 (T-10, evening)
paused_by: andrew
supersedes: PAUSE-T-10-v2.md
reason: Wave 1 complete (7/9 plans) + Rule 8a identity framing locked. Pausing before Wave 2 (05-06 review pitches) for HITL items. Resume with /gsd-execute-phase 5.
---

# Phase 5 Pause — T-10 v3 (2026-04-18 evening)

## Where we stand

| Plan | Status | Wave | Key Commits |
| ---- | ------ | ---- | ----------- |
| 05-00 Launch orchestration | ✅ Complete | — | 5 commits |
| 05-01 SML blog post | ✅ Complete | — | a88d430 → 9f9ee88 → b7d53b4 |
| 05-02 SML newsletter | ✅ Complete | — | 97832aa → 69b315c → 6369206 |
| 05-03 LinkedIn | ✅ Complete (rewritten Rule 8a) | — | db76307 → e3137c4 → **364719f** |
| 05-04 Instagram | ✅ Complete (this session) | 1 | e6b9f9c, 7a01dab, 4bfd471 |
| 05-05 KVR listing | ✅ Complete (rewritten Rule 8a) | 1 | 30b4be6, badac48, fe4697c, **cd272be** |
| 05-06 Review pitches | ⏸ Wave 2 — NOT STARTED, paused for HITL | 2 | — |
| 05-07 Berlin DMs | ✅ Complete (this session, Rule 8a applied) | 1 | 69d146e, 778f721 |
| 05-08 Guest pitches | ⏸ Wave 3 | 3 | — |

**Completion: 7/9 (78%)**

## Major decision this session — Rule 8a (Andrew identity framing)

Andrew is **NOT** a "software developer" / "audio dev" / "solo dev". Identity = **"Berlin-based spatial media expert and co-founder of Spatial Media Lab"**. Locked in `05-CONTEXT-SUPPLEMENT-2026-04-18.md` (commit `9f74d9e`). Applies retroactively + forward.

**Timo Bittner (SML co-founder) — when to name:**
- Social broadcast (LinkedIn, IG, blog body, newsletter): IN
- Direct emails (KVR news, AES, review pitches): OUT
- DM to peer Andrew knows personally: OUT
- DM to person Andrew doesn't know: IN

**Andrew's actual platforms:** LinkedIn ✓, Email ✓, Instagram ✓. **NO Bluesky, NO Mastodon.** Plan templates that specified those have been re-routed.

## Open HITL items Andrew owns

- [ ] **DKIM + DMARC setup at jackhost.net** — target 2026-04-25 (T-3). Blocks 05-06 from-address decision (andrew@spatialmedialab.org vs andrewjrahman@gmail.com fallback).
- [ ] **05-06 contact-knowledge audit** — Of the 34 review-pitch contacts in `influencer-outreach-v1.0.md`, flag any Andrew knows personally (those flip Timo OUT). Default for unknown contacts = Timo IN.
- [ ] **andrewrahman.com bio rewrite** — page.tsx + v2.tsx line 651: "composer, producer, engineer, and audio software developer based in Berlin" → spatial media expert + SML co-founder framing. Separate andrewrahman-com PR.
- [ ] **Patreon About section rewrite** — same framing change. Patreon UI edit.
- [ ] **Phase 4 (Demo Content)** — target plan 04-00 by 2026-04-20 (still on critical path for CONT-01/02 assets).
- [ ] **KVR Developer Account application** — `kvraudio.com/developer_application.php`. 05-05 drafts include press-release fallback regardless.
- [ ] **Daily launch-precheck walk** — starts 2026-04-21 (T-7). See `launch-precheck.md`.

## Outstanding tasks at T-0

Per existing runbook (`launch-day-runbook.md`, `launch-precheck.md`):
- Apr 21–25: Send 4 Berlin DMs (drafts approved, ready to send)
- Apr 27 (T-1): Update LinkedIn + IG bio-links to andrewrahman.com/get-osd; submit KVR listing
- Apr 28 (T-0): Publish blog 10:00 CET → KVR news 10:05 → newsletter 10:10 → LinkedIn 10:20 → IG Reel 10:25 → review pitches 10:45–12:00 → community cross-posts

## To resume

```
/gsd-execute-phase 5
```

Or to skip ahead:

```
/gsd-execute-phase 5 --wave 2     # Wave 2: 05-06 review pitches
/gsd-execute-phase 5 --wave 3     # Wave 3: 05-08 guest pitches (DO NOT SEND BEFORE 2026-05-25)
```

## Notes for the next session

- **05-06 needs Andrew's contact-knowledge audit BEFORE the executor runs** — otherwise the executor will default Timo IN for everyone and you'll have to red-line per contact.
- **05-08 guest-pitch wave** is independent of 05-06 and can run in any order; it's a Wave 3 because 05-06 is its prerequisite per plan structure (review-pitch template references guest-pitch dual-pitch contacts like Michael G Wagner + Oliver Kadel).
- **Session pattern continues:** Andrew reviews every draft with line-level red-lines. Budget for 3–5 revision cycles per plan.

## Session artefacts produced (2026-04-18 evening)

**Committed (.planning/):**
- `05-CONTEXT-SUPPLEMENT-2026-04-18.md` (Rule 8a addendum)
- `drafts/ig-reel-caption.md`, `drafts/ig-carousel-caption.md` (05-04)
- `drafts/kvr-listing-metadata.md`, `drafts/kvr-news-submission.md` (05-05, news rewritten)
- `drafts/berlin-dm-{hainbach,kirn,horstmann,aes}.md` (05-07, all rewritten)
- `berlin-dm-log.md` (4-row send log)
- `drafts/linkedin-post.md` (rewritten — 05-03 reopened for Rule 8a fix)
- `05-04-SUMMARY.md`, `05-05-SUMMARY.md`, `05-07-SUMMARY.md`
- This file (PAUSE-T-10-v3.md)

**Committed (Andrew memory):**
- `feedback_andrew_self_positioning.md` — never call Andrew a "software developer"; tinkerer/spatial media expert framing
