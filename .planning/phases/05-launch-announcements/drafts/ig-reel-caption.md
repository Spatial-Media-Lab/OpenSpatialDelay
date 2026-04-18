# Instagram Reel — OpenSpatialDelay v1.0.0 Launch

---

## PRE-LAUNCH CHECKLIST (T-1 morning, 2026-04-27)

- [ ] Open Instagram → Edit Profile → Website field → set to https://andrewrahman.com/get-osd
- [ ] Verify CONT-02 status with Phase 4 by T-2 (2026-04-26). If not ready, trigger descope branch at bottom of this file.
- [ ] Publish Reel at 2026-04-28 10:25 CET per launch-day-runbook step 6.
- [ ] Optional: cross-post to @spatialmedialab after publishing.

---

## Reel video spec

| Property | Value |
|----------|-------|
| Length | 15–30 seconds (discovery sweet-spot per 2026 IG algorithm) |
| Aspect | 9:16 vertical |
| Audio | CONT-02 video with CONT-01 binaural audio (30s) overlaid. Add "Audio: headphones on" text overlay at 0:00–0:02. |
| Cover frame | screenshot_full.png from andrewrahman-com/public/assets/ with text overlay "Free spatial delay · VST3 + AU · out today" |
| Export | 1080×1920, H.264, ≤ 30fps, under 100MB |

---

## Caption

**First line (hook, ≤ 140 chars, visible in feed before 'more' truncation):**

> Each echo has a 3D position. Free spatial delay, out today. VST3 + AU · mac + Windows.

**Full caption body (≤ 220 chars after the hook):**

> OpenSpatialDelay v1.0.0 — the first tool in the Spatial Media Library pipeline. Stereo out of the box, grows into binaural and surround. GPL-3.0. Headphones on. Link in bio.

**Hashtag line (exactly 5 — 2026 IG cap):**

#SpatialAudio #DolbyAtmos #AudioPlugin #BinauralAudio #FreePlugin

---

## Reel composition walkthrough

1. **0:00–0:02** — Cover frame + "Audio: headphones on" overlay + plugin UI screenshot_full.png
2. **0:02–0:05** — Quick pan across 12-tap spatial map view (screenshot_spatial_map.png)
3. **0:05–0:25** — CONT-01 30s binaural audio plays; overlay live plugin UI capture or tap-map animation
4. **0:25–0:30** — End card: "Link in bio · andrewrahman.com/get-osd · GPL-3.0 · v1.0.0"

---

## Descope branch — if CONT-02 not ready by T-2

If Phase 4 CONT-02 is not ready by 2026-04-26, publish a **static IG feed post** (not a Reel) using:

- **Image:** screenshot_full.png (square-cropped 1:1 for feed)
- **Audio:** none (feed posts don't carry audio)
- **Caption:** identical to Reel caption above — hook + body + 5 hashtags
- **CTA:** still "Link in bio"

This still hits ANNC-04 acceptance — the demo-content slot is fulfilled by CONT-03 high-res screenshot instead of CONT-02 video.

Downgrade must be logged in 05-04-SUMMARY.md under "Descope branch triggered: yes/no".

---

## Notes on stereo framing (Rule 1)

OSD works in stereo today and grows with the mix into binaural, surround, or Atmos tomorrow. The caption body includes "stereo out of the box" to signal this explicitly. Do not frame as binaural-only or immersive-only in the Reel overlay text.

## Distribution note

- **Binary downloads:** andrewrahman.com/get-osd (bio-link target)
- **Source code:** github.com/Spatial-Media-Lab/OpenSpatialDelay
- **Support:** patreon.com/AndrewRahman
- Do NOT put any URL in the caption body — bio-link is the only CTA.
