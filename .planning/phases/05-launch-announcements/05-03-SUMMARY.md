# Plan 05-03 — LinkedIn Launch Post (ANNC-03) — Summary

**Status:** complete
**Requirement:** ANNC-03
**Delivered:** 2026-04-18

## Deliverable

- **`.planning/phases/05-launch-announcements/drafts/linkedin-post.md`** — single file containing two ship-ready formats and a pre-launch checklist. Format selection deferred to T-1 (2026-04-27) per Andrew's decision.

## Commits

| Commit | Purpose |
| ------ | ------- |
| `db76307` | Initial draft — Format A (text-only, 2,128 chars) + Format B (7-page document carousel + 689-char companion caption) + pre-launch checklist + deviations log. |
| `1a97d47` | Align Simple-mode wording with approved blog (replaced "CPU-lite fallback" with "Simple mode for basic binaural without HRTF coloration" in both formats). |
| _this commit_ | Plan closure (SUMMARY + STATE + ROADMAP). |

## Two formats preserved in draft

**Format A — Text-only storytelling post** (~2,204 chars after Simple-mode fix, still in LinkedIn's 1,301–2,500 sweet spot):
- Hook land in first 210 chars (pre-"see more"): "I've been wanting a delay like this for years. Seven weeks ago I sat down in my Berlin apartment and started building it. Today it ships — free."
- Leads with stereo producer framing in paragraph 2.
- Names all five stereo modes inline (Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein).
- Calls out "Stereo Ping-Pong" and "Wide Stereo" by name.
- Three-families architecture in paragraph 4 (5 stereo / 6 binaural / 7 surround).
- "Link in bio" CTA (no URLs in body or first comment per 2026 LinkedIn algorithm research).
- Closes with Superbooth May 7–10 hook.

**Format B — Native document carousel** (7-page PDF + 689-char companion caption):
- Page 1 cover, Page 2 "Stereo today, spatial tomorrow", Page 3 "12 taps in 3D", Page 4 "Three algorithm families", Page 5 "70 presets + ADM-OSC", Page 6 full-bleed screenshot_full.png, Page 7 download CTA.
- ~3× engagement per dataslayer.ai Feb 2026 data.
- Requires 60–90 min PDF design (Figma or Canva) by T-2 evening.
- Companion caption repeats the stereo-first framing in short-form.

## Key decisions captured

1. **Format selection deferred to T-1 (2026-04-27).** Both formats are ship-ready as spec. Andrew picks A or B after deciding whether to invest the 60–90 min PDF design time. If PDF isn't done by T-2 evening, default to A.

2. **No URLs in post body or first comment.** LinkedIn's 2026 algorithm demotes both (2 independent reports cited in 05-RESEARCH.md Pitfall 2). Bio-link (`andrewrahman.com/get-osd`) is the compliant CTA. Pre-launch checklist has the bio-link update as a T-1 task.

3. **No zodiac references** (supplement Rule 6). LinkedIn audience skews serious; zodiac is Instagram-only.

4. **Seven-week timeline framing** (supplement Rules 5 + 8). Hook preserves the "I've been wanting for years" emotional anchor but corrects the build claim to "seven weeks ago I sat down in my Berlin apartment and started building it."

5. **CPU-lite → Simple-mode wording alignment** (consistency with approved blog). Both formats updated at `1a97d47`.

## Pre-launch checklist (tracked in the draft file)

- [ ] T-1 morning (2026-04-27): LinkedIn profile → Contact info → Website = `https://andrewrahman.com/get-osd`. Verify in incognito.
- [ ] T-1 morning: confirm bio/headline mentions "Berlin".
- [ ] T-2 evening (2026-04-26): decide A vs B. If Format B, ensure PDF design is complete.
- [ ] T-0 (2026-04-28 10:20 CET): post per launch-day-runbook step 5. NO URLs in body, NO URLs in first-comment slot.

## Acceptance checks (all pass)

- [x] Draft file exists with Format A and Format B populated
- [x] Format A within LinkedIn optimal range (~2,204 chars)
- [x] Leads with stereo value — NOT immersive
- [x] Three-families architecture named (not "7 algorithms")
- [x] Preset anchors named: Stereo Ping-Pong, Wide Stereo
- [x] No zodiac references (supplement Rule 6)
- [x] Voice consistent with approved blog + newsletter
- [x] Distribution path: andrewrahman.com/get-osd (bio-link) for binary; GitHub for source
- [x] NO URLs in post body or first-comment slot
- [x] Two commits (draft + revision) + this closure

## Supplement compliance record

Draft file itself contains a "Deviations from plan template" section at the bottom traceably linking each rewrite to the supplement rule it honors. This audit trail is preserved in the committed draft.

## Next

Plan 05-04 Instagram (Reel + carousel — zodiac activates here, 12-slide structural joke × 12 taps × 12 zodiac signs). Plan 05-05 KVR listing. Plan 05-07 Berlin DMs.
