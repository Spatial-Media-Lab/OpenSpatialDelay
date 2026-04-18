---
plan: 05-05
phase: 05-launch-announcements
status: complete
completed: 2026-04-18
requirement: ANNC-05
---

# Summary: 05-05 KVR Audio Product Listing + News Submission

## What was built

- `drafts/kvr-listing-metadata.md` — Full KVR Product Database listing: 13 metadata fields, 400-word stereo-first description body, system requirements, screenshot upload order, self-review checklist.
- `drafts/kvr-news-submission.md` — Press-release for T-0 KVR news queue: headline, 5-paragraph body, About SML, media contact, screenshot guidance. Contains `{SML_BLOG_POST_URL}` placeholder — replace with live blog URL at T-0.

## Decisions

- **Download URL in KVR listing:** `andrewrahman.com/get-osd` (email wall). KVR policy prefers friction-free links; Andrew accepted the risk of editorial rejection. Fallback: swap to `github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0` if listing is rejected.
- **macOS system requirement:** arm64 only (Apple Silicon), Sequoia 15.x+. Intel Mac not supported in v1.0.0. Confirmed by Andrew.
- **Submission route:** KVR Developer Account dashboard (T-1), or email contactus@kvraudio.com if account not yet approved.

## Submission timeline (pending)

| Step | When | Action |
|------|------|--------|
| T-1 (2026-04-27) | Afternoon | Paste kvr-listing-metadata.md into KVR Developer dashboard. Upload screenshots. Submit. |
| T-0 (2026-04-28 10:05–10:10 CET) | Launch morning | Paste kvr-news-submission.md into KVR News form. Replace {SML_BLOG_POST_URL}. Submit. |

## Self-Check: PASSED

- Both files exist and committed (30b4be6, badac48)
- Stereo-first (Rule 1): ✓
- Three-families architecture (Rule 2): ✓ ("5 stereo modes, 6 binaural modes, 7 surround algorithms")
- Preset anchors (Rule 3): ✓ ("Stereo Ping-Pong", "Wide Stereo" named by name)
- No DSP jargon (Rule 4): ✓
- No zodiac (Rule 6): ✓
- Andrew approved: 2026-04-18
