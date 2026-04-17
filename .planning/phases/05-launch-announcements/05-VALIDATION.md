---
phase: 5
slug: launch-announcements
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-04-17
---

# Phase 5 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.
>
> **Note:** Phase 5 is a content / coordination phase (launch announcements, outreach). Its validation surface is URL resolution + human confirmation, not unit tests. This file documents the lightweight smoke + manual protocol per RESEARCH.md Validation Architecture.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Manual confirmation + `curl -I` smoke checks + existing `andrewrahman-com` Playwright `@external` suite (from Phase 3) |
| **Config file** | none — `scripts/phase5-smoke.sh` shell script is the only new artefact (Wave 0) |
| **Quick run command** | `bash scripts/phase5-smoke.sh` |
| **Full suite command** | `bash scripts/phase5-smoke.sh` + manual link-by-link open in Chrome + Safari |
| **Estimated runtime** | ~10 seconds for smoke; ~10 minutes manual walk |

---

## Sampling Rate

- **After every task commit:** `curl -I` any URL touched in the task (or n/a if task is draft-only)
- **After every plan wave:** `bash scripts/phase5-smoke.sh` once all plans in wave claim their URLs are live
- **Before `/gsd-verify-work`:** All smoke rows return 200 + Andrew confirms each channel by screenshot or live open
- **Max feedback latency:** ~10 seconds (curl smoke)

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| TBD | TBD | TBD | ANNC-01 | — | SML blog post URL resolves 200 | smoke | `curl -fsI https://spatialmedialab.org/{slug}/` | ❌ W0 (URL exists only post-publish) | ⬜ pending |
| TBD | TBD | TBD | ANNC-01 | — | Audio demo plays in browser | manual-only | Open URL in Chrome + Safari, confirm `<audio>` plays | — | ⬜ pending |
| TBD | TBD | TBD | ANNC-02 | — | SML newsletter campaign sent | manual-only | Mailchimp dashboard shows "Sent" + receipt in Andrew's inbox | — | ⬜ pending |
| TBD | TBD | TBD | ANNC-03 | — | LinkedIn post published | smoke | Open LinkedIn post URL; screenshot | — | ⬜ pending |
| TBD | TBD | TBD | ANNC-04 | — | Instagram Reel / post published | smoke | Open IG URL; confirm Reel plays | — | ⬜ pending |
| TBD | TBD | TBD | ANNC-05 | — | KVR product listing live | smoke | `curl -fsI https://kvraudio.com/product/...` (URL TBD after submission) | — | ⬜ pending |
| TBD | TBD | TBD | ANNC-06 | — | Review-pitch batch sent (T-0) | manual-only | Gmail sent folder: 34 sends with timestamps on launch day | — | ⬜ pending |
| TBD | TBD | TBD | ANNC-06 | — | Guest-pitch batch sent (late May) | manual-only | Gmail sent folder after travel | — | ⬜ pending |
| TBD | TBD | TBD | D-07 | — | Berlin-local Superbooth DMs sent | manual-only | Screenshot of each DM send | — | ⬜ pending |
| TBD | TBD | TBD | — | — | All launch-day link targets resolve 200 (T-1 pre-check) | smoke | `bash scripts/phase5-smoke.sh` — one-liner curl loop | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

*Task IDs will be filled in once PLAN.md files exist.*

---

## Wave 0 Requirements

- [ ] `scripts/phase5-smoke.sh` — shell script running `curl -I` against: `spatialmedialab.org/{new-post-slug}`, `andrewrahman.com/get-osd`, `github.com/.../releases/tag/v1.0.0`, `patreon.com/spatialmedialab`, KVR listing URL. ~10 lines, no framework dependency.

*(No test-framework install needed — this phase's validation is fundamentally "did the URL resolve and did the human confirm".)*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Audio embed plays in browser | ANNC-01 | Headless `curl` cannot exercise media element | Open SML blog URL in Chrome and Safari; click play; confirm audio output |
| Mailchimp campaign delivered | ANNC-02 | Mailchimp send status is only queryable via dashboard login | Log into Mailchimp; verify campaign status = "Sent"; confirm test-send received in Andrew's inbox |
| LinkedIn post visible + no link in first comment | ANNC-03 | LinkedIn has no public API for post state | Open post URL logged out; confirm visible without login; confirm no first-comment link (per 2026 algorithm research) |
| IG Reel discoverable via hashtag | ANNC-04 | IG discovery is algorithmic + variable | Search one of the 5 hashtags; confirm Reel appears in results within 24h |
| KVR listing approved | ANNC-05 | KVR editorial review is human, no published SLA | Refresh KVR listing URL daily post-submission; email `contactus@kvraudio.com` if >5 days without response |
| Review-pitch batch sent with no GDPR issue | ANNC-06 | Cold-email compliance is per-jurisdiction; no automated linter | Pre-send, confirm each email has: plain-text body, legitimate-interest opt-out sentence, unsubscribe path |
| Berlin DMs delivered on chosen channel | D-07 | Bluesky / IG / LinkedIn DM delivery not easily checkable via API | Screenshot each send; confirm "delivered" status in each platform's UI |
| Launch-day coverage roll-up | all | No automated aggregator | 7 days post-launch, walk each channel, log engagement snapshot in `.planning/phases/05-launch-announcements/launch-day-results.md` |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` smoke verify OR explicit manual-only justification (above)
- [ ] Sampling continuity: every plan wave triggers `phase5-smoke.sh`
- [ ] Wave 0 covers: `scripts/phase5-smoke.sh` checked in
- [ ] No watch-mode flags
- [ ] Feedback latency < 15s for smoke
- [ ] `nyquist_compliant: true` set in frontmatter once all plan tasks are mapped

**Approval:** pending
