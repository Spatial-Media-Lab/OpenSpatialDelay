# Launch Day Runbook — Tuesday 2026-04-28

**Window:** 10:00–12:00 CET (2 hours, per D-01). **Owner:** Andrew Rahman. **Fallback date:** 2026-05-05.

This file is the precise T-0 script Andrew follows on launch morning. The 8 numbered steps below are
copied verbatim from `05-RESEARCH.md §Launch-Day Sequencing Diagram`. Times are CET. Contingencies
for the 4 most likely failure modes are at the bottom.

## Pre-window (09:30–09:55 CET)

- [ ] `bash scripts/phase5-smoke.sh` — all 5 hard URLs PASS
- [ ] Open these browser tabs in order: WordPress admin (spatialmedialab.org/wp-admin), Mailchimp (mailchimp.com/campaigns), KVR Developer dashboard, LinkedIn post composer, Instagram web/Business Suite, Gmail draft-folder with 34 review-pitch drafts
- [ ] Confirm coffee. No coffee, no launch.

## T-0 sequence (10:00–12:00 CET, per RESEARCH Launch-Day Sequencing Diagram)

1. `10:00` — **Publish SML blog post** (WordPress → Publish). Grab canonical URL. Update bio-link fields (LinkedIn, Instagram) to `andrewrahman.com/get-osd`.
2. `10:05` — **Submit KVR product listing** via KVR Developer dashboard (or email to contactus@kvraudio.com per fallback). Editorial review is hours-to-days; OK.
3. `10:10` — **Submit KVR news item** (press-release style) via the same dashboard / email.
4. `10:15` — **Send SML Mailchimp newsletter** — campaign already prepped in 05-02; just click Send or Schedule-now.
5. `10:20` — **Publish LinkedIn post** (text-only or document-carousel, per 05-03). Rule: do NOT add link in first comment (2026 algorithm penalty).
6. `10:25` — **Publish Instagram Reel** (or Reel + carousel companion per 05-04). Caption hook first-line ≤ 140 chars before "more".
7. `10:35` — **Cross-post to forums**: Gearspace VR/Immersive, KVR Effects Forum, r/spatialaudio, VI-CONTROL (per 05-RESEARCH §Segment 6).
8. `10:45–12:00` — **Send 34-contact review-pitch batch** (05-06). Space over 2 hours (1 send every 3–4 min) per RESEARCH Pitfall 5 to dodge Gmail/Outlook spam filters.

## Post-window (12:00+ CET)

- [ ] Re-run smoke with blog + KVR URLs: `SML_BLOG_POST_URL=<url> KVR_URL=<url> bash scripts/phase5-smoke.sh`
- [ ] Patreon cross-post (seed post 5, "v1.0.0 is OUT") — OPTIONAL, non-counted channel.
- [ ] Andrew updates STATE.md resume pointer to `Phase 5 launched; monitoring 7-day window`.
- [ ] Quick engagement snapshot at T+2h and T+6h: LinkedIn impressions, Mailchimp opens, IG Reel plays, KVR listing live-yes/no.

## If anything breaks mid-window

- [ ] If WordPress Publish fails → submit-for-review route (Timo/Basel approves). Absorbs 0–24h slip. Continue with remaining steps; newsletter CTA can link directly to GitHub Release + andrewrahman.com/get-osd for 24h.
- [ ] If Mailchimp Send fails (role issue) → delegate send to audience owner. If owner unreachable, send via Andrew's gmail ListMonk (degraded plaintext) to top 20 personal-network recipients. Accept loss of the full SML audience for this launch.
- [ ] If LinkedIn algorithm throttle suspected post-publish (<200 impressions 2h in) → edit post to remove any URL and append "Link in bio" (per RESEARCH Pitfall 2).
- [ ] If KVR listing rejected → read rejection reason; fix missing field; resubmit. Product listing catches traffic whenever it goes live; news item is the optional launch-moment.

## Evidence requirements (per feedback_evidence_artifact_ceremony.md)

- Live URL + Andrew self-confirmation is sufficient per-channel. No per-step screenshots required unless a validation surface (a11y / hashtag cap / first-comment check) needs one.
- The 34-contact review-pitch batch can be verified by a Gmail "sent" folder count at 12:00 CET. No per-send screenshot.
- The one mandatory screenshot is the LinkedIn post post-publish (confirms it rendered with no first-comment link — surfaces the 2026 algorithm pitfall if Andrew forgot).

## Post-launch tail (T+1 through T+7)

- Andrew monitors replies on the 34-contact batch; auto-reply "travelling through May, replying in June" if needed.
- Berlin-local DMs (sent T-7..T-3 per `launch-precheck.md`) should have produced 1–2 Superbooth coffee confirmations by now.
- KVR listing should be live by T+3 at latest. Email contactus@kvraudio.com at T+5 if still pending.
- Phase 5 plan gate closes at T+7 when the `launch-day-results.md` snapshot is written.

## One-line summary for day-of reference

> 10:00 WordPress publish → bio-link update → 10:05 KVR listing → 10:10 KVR news → 10:15 Mailchimp send → 10:20 LinkedIn → 10:25 Instagram Reel → 10:35 forums → 10:45–12:00 34-contact review-pitch batch staggered. Contingencies documented above.
