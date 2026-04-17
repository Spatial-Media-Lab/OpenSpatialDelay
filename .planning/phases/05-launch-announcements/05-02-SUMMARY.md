# Plan 05-02 — SML Newsletter (ANNC-02) — Summary

**Status:** complete
**Requirement:** ANNC-02
**Delivered:** 2026-04-18

## Deliverable

- **`.planning/phases/05-launch-announcements/drafts/sml-newsletter.md`** — Mailchimp-ready teaser (subject + preview + 349-word body).

## Commits

| Commit | Purpose |
| ------ | ------- |
| `97832aa` | Initial draft (immersive-first framing — obsolete). |
| `69b315c` | Full rewrite — stereo-first framing, three-families architecture, corrected post-research. |
| _this commit_ | Plan closure (SUMMARY + STATE + ROADMAP). |

## Key decisions captured

1. **Framing pivot on 2026-04-18 (T-10).** Andrew's re-plan directive: "thousands if not millions more stereo users than spatial users. Make it crystal clear everywhere that it can be used in stereo as well and grow with you into spatial." The newsletter now leads with stereo producers, not immersive specialists.

2. **Factual correction.** Prior draft claimed "7 spatialization algorithms so the same arrangement can sit in a stereo mix, a 5.1 room..." — wrong. Stereo output uses a separate family of 5 stereo modes (Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein). Corrected to name the three algorithm families truthfully.

3. **Subject + preview chosen.** "Free delay that grows with your mix — OSD v1.0.0" / "Stereo to 7.1.4 Atmos in one plugin. 12 taps, three algorithm families." Both favor "grows with you" over immersive-first language.

4. **Send-ops gate deferred.** The original plan's Task 2 checkpoint was about Mailchimp access confirmation + schedule. Per the 05-00 precheck walk, Andrew owns the SML Mailchimp audience directly, so no owner-delegation is required. Actual campaign creation, test send, and schedule happen at T-1 / T-0 per launch-day-runbook step 4 — they are operational, not drafting, so this plan can be closed before them. Ops items tracked in launch-precheck.md (T-1 step) and launch-day-runbook.md (T-0 step 4).

## Placeholders in the draft

- `{SML_BLOG_POST_URL}` — fill at T-0 step 1 after blog publish (per runbook).

## Deferred / downstream

- **T-1 (2026-04-27):** Create Mailchimp campaign, paste body with placeholder in place, send test to andrew@spatialmedialab.org + personal inbox, verify render.
- **T-0 (2026-04-28 10:15 CET):** Swap `{SML_BLOG_POST_URL}` with live blog URL, hit Send.
- **If Phase 4 slips to 2026-05-05:** update byline nowhere — newsletter has no byline date block (only the blog does). No change needed to the newsletter for fallback date.

## Acceptance checks (all pass)

- Subject ≤ 60 chars: 48 ✓
- Preview ≤ 90 chars: 72 ✓
- Body within 200–400 words: 349 ✓ (actually the counted 349 includes headers / metadata lines — body proper is closer to ~310, still in range)
- Contains literal "each echo has a 3D position" ✓
- Contains `{SML_BLOG_POST_URL}` placeholder ✓
- Contains `andrewrahman.com/get-osd` ✓
- Contains `patreon.com/AndrewRahman` ✓
- First-person voice (I just, I started, I've) ✓
- Zero forbidden DSP jargon (FFT, phase vocoder, partition, Laroche, Dolson, VST2, AAX, "buy", "purchase") ✓
- Exactly 2 CTAs (read full post + download) ✓
- Voice + algorithm-family claims consistent with 05-01 blog draft ✓

## Next

Plan 05-03 LinkedIn launch post — apply the same stereo-first framing guidance. See `05-CONTEXT-SUPPLEMENT-2026-04-18.md` for the locked framing rules that all downstream plans (05-03 through 05-08) must honor.
