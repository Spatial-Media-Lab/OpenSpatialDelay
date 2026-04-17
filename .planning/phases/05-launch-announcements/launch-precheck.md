# Phase 5 Launch Precheck — T-7 through T-1

**Target launch:** Tuesday 2026-04-28, ~10:00 CET. Fallback: Tuesday 2026-05-05 (overlaps Andrew's travel — avoid if possible).

This file is Andrew's daily walk-through from T-7 (2026-04-21) onward. Every `[UNKNOWN]` row from
`05-RESEARCH.md §Environment Availability` has a checkbox here. Appending a dated note under
each checkbox (e.g. `2026-04-22: Mailchimp role = Manager confirmed`) keeps the audit trail in-file.

## Hard blockers (any one unresolved = delay launch to 2026-05-05)

- [ ] Phase 4 (Demo Content) has a 04-00-PLAN.md OR an explicit descope decision by 2026-04-20. Current status as of 2026-04-17: NO PLANS. Escalate at 2026-04-20 standup if unchanged.
  - 2026-04-17: Andrew committed to plan 04-00 by 2026-04-20. Not descoped. IG Reel assumed as primary asset in 05-04.
- [x] SML Mailchimp audience owner identified — Andrew (self) OR Timo Bittner (president) OR Basel Naouri (board). Role confirmed = Manager or Admin on the audience.
  - 2026-04-17: Andrew (self) confirmed as audience owner. 05-02 drafts for direct send.
- [ ] andrewrahman.com is live on Netlify production domain (Phase 3-09 gate).
- [ ] GitHub release v1.0.0 downloadable — both macOS and Windows binaries attached.

## Email-deliverability preconditions (gates the 34-contact review batch in 05-06)

- [x] SPF record exists on spatialmedialab.org (check: `dig TXT spatialmedialab.org | grep spf1`)
  - 2026-04-17: SPF verified — `v=spf1 a include:spf.jackhost.net -all`.
- [ ] DKIM record exists on spatialmedialab.org (check: `dig TXT default._domainkey.spatialmedialab.org` or inspect a test send's Authentication-Results header)
  - 2026-04-17: DKIM NOT found at `default._domainkey` or `google._domainkey` selectors. DMARC also missing. Andrew handling DKIM+DMARC setup at jackhost.net as separate HITL. Target: verified by 2026-04-25 (T-3). Do NOT tick — pending.
- [ ] Decision: if SPF+DKIM not verified by 2026-04-25 (T-3), review-pitch batch sends from andrewjrahman@gmail.com instead of andrew@spatialmedialab.org. Document the chosen from-address in 05-06 outreach log.
  - 2026-04-17: 05-06 will draft BOTH from-address variants. Final selection at T-3 based on test-send Authentication-Results inspection.

## External-account preconditions

- [ ] KVR Developer Account: login-check at https://www.kvraudio.com/ — if no existing account, apply at https://www.kvraudio.com/developer_application.php (free; < 1-day turnaround). Fallback: email press release to contactus@kvraudio.com.
  - 2026-04-17: No account yet. Andrew applying at kvraudio.com/developer_application.php. 05-05 drafts BOTH Developer DB listing AND contactus@kvraudio.com press-release fallback.
- [ ] LinkedIn: Andrew logged in + bio-link field accessible (pre-launch bio will be updated to andrewrahman.com/get-osd on 2026-04-27 T-1).
- [ ] Instagram: Andrew's personal IG login verified. SML IG handle `@spatialmedialab` is a cross-post option (confirmed in RESEARCH sources).
- [ ] Bluesky: Andrew's handle exists (verify or create — target send to Hainbach and Peter Kirn requires Bluesky DM).
- [ ] Mastodon fallback for Kirn (mastodon.social/@pkirn) if Bluesky fails.

## URL smoke (run bash scripts/phase5-smoke.sh at T-1 and T-0)

- [ ] T-1 morning (2026-04-27): `bash scripts/phase5-smoke.sh` returns 0, all 5 hard URLs pass.
- [ ] T-0 morning (2026-04-28, before 10:00 CET): re-run; all pass.
- [ ] Post-publish (2026-04-28 after step 1): `SML_BLOG_POST_URL=https://spatialmedialab.org/{slug}/ bash scripts/phase5-smoke.sh` passes.
- [ ] Post-listing (any day Apr 28-May 3): `KVR_URL=https://www.kvraudio.com/product/{slug} bash scripts/phase5-smoke.sh` passes.

## Berlin-local D-07 DM timing window

- [ ] Berlin DMs to Hainbach, Peter Kirn, Eric Horstmann, Ulli Scuda sent in the 2026-04-21 through 2026-04-25 window (per RESEARCH Pitfall 6).
- [ ] See 05-07-PLAN.md for per-contact drafts.

## Precheck owner + cadence

- Owner: Andrew Rahman.
- Review cadence: daily walk through this file 2026-04-21 (T-7) onward.
- Log: append a daily dated line under each item ("2026-04-22: Mailchimp role = Manager confirmed") so the audit trail is in-file.

## Escalation paths

- **Phase 4 unplanned on 2026-04-20** → surface at standup; either scope a minimal 04-00-PLAN.md (single binaural demo clip + 1 Reel cut-down) by T-5, OR descope ANNC-04 (IG Reel) to a static screenshot + caption post.
- **Mailchimp access impossible** → delegate send; if owner unreachable, send from Andrew's Gmail ListMonk (degraded plaintext) to top 20 personal-network recipients, accept loss of the full SML audience.
- **andrewrahman.com not live by T-1** → fall back to Netlify preview URL in all CTAs, OR slip to 2026-05-05.
- **KVR Developer Account rejected** → email press release directly to contactus@kvraudio.com with subject "OpenSpatialDelay v1.0.0 — free GPL spatial delay (VST3/AU, mac+Win)".
