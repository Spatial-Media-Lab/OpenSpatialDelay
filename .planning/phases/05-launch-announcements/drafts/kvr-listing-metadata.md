# KVR Audio Product Listing — OpenSpatialDelay v1.0.0

Submission target date: 2026-04-27 (T-1). Route: KVR Developer Account dashboard. Fallback: email to contactus@kvraudio.com.

---

## Core metadata fields

| Field | Value |
|-------|-------|
| Product name | OpenSpatialDelay |
| Version | 1.0.0 |
| Developer | Spatial Media Lab |
| Developer contact | Andrew Rahman, andrew@spatialmedialab.org |
| License | GPL-3.0 |
| Price | Free |
| Formats | VST3, AU |
| Operating systems | macOS (arm64 — Sequoia and newer, per README system requirements), Windows (10 and 11) |
| Download URL | https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0 |
| Source URL | https://github.com/Spatial-Media-Lab/OpenSpatialDelay |
| Product homepage | https://andrewrahman.com/get-osd |
| Primary category | Delay |
| Tags | Delay, Spatial, Free, VST3, AU, macOS, Windows |

---

## Description body (400–700 words)

Each echo has a 3D position in space.

OpenSpatialDelay v1.0.0 is a free spatial delay plugin for VST3 and AU on macOS and Windows, released under GPL-3.0. It is the first tool in the Spatial Media Library pipeline — an open-source collection of spatial audio tools developed under Spatial Media Lab.

**It works in stereo today, and grows with your mix into binaural, surround, or Atmos whenever you're ready.**

Load it on any stereo bus in Logic Pro, Ableton Live, or Reaper and it responds exactly like a delay should — 12 taps, each independently positionable, with its own timing, filter, feedback, and level. The five stereo modes (Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein) cover every stereo scenario: classic ping-pong, wide pair spacing, mid/side processing, and coincident mic configurations. The **Stereo Ping-Pong** preset (Classic Delays category) puts two taps at ±90° azimuth for the textbook effect. The **Wide Stereo** preset (Surround Production category) places two taps at a ±30° standard stereo pair, tempo-synced at 1/4 note — ideal for widening any bus without breaking mono compatibility.

Then, when the session calls for it, one dropdown takes you further:

**Three algorithm families — stereo, binaural, surround:**

- **Stereo (5 modes)** — Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein. Works on any stereo bus, no DAW configuration needed.
- **Binaural (6 modes)** — Five measured HRTF profiles (KU100, CIPIC, HUTUBS, MIT KEMAR, SADIE) plus a CPU-lite Simple mode for headphone spatial delay without HRTF coloration. No surround output bus required — plug it into any stereo track with headphones on.
- **Surround / Immersive (7 algorithms)** — Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP — covering Quad, 5.1, 7.1, Dolby Atmos 7.1.4, and domes up to 13.1. Ambisonics supports encoding up to 6th order.

The same 12 taps, the same UI, the same presets travel across all three families. Start stereo today; step up the day the session demands it.

**What's in the box:**

- 12 independently-positioned delay taps, each with its own 3D placement, filter, feedback, level, and trajectory
- A trajectory engine that moves each tap through space — linear sweeps, orbits, pendulums, random paths, or any of 13 named shapes — so your delays animate without manual automation
- ADM-OSC send and receive, so OSD connects to Spat Revolution, Panoramix, Iannix, and TouchDesigner out of the box
- 70 factory presets spanning stereo production, headphone pieces, ambient beds, cinematic whooshes, rhythmic counter-lines, Atmos sessions, and dome installations
- No trial timer, no licence server, no unlock sequence — GPL-3.0 means free to download, free to use in commercial work, free to fork

**Quality proof points:**

OpenSpatialDelay v1.0.0 is a stable release, not a beta. The codebase carries 162+ Catch2 unit tests that run in CI on every commit — you can verify the green badge on GitHub. The full source is available under GPL-3.0 at github.com/Spatial-Media-Lab/OpenSpatialDelay, so the implementation is inspectable and forkable.

The plugin is actively developed under Spatial Media Lab. Patreon backers fund the development time and help steer the roadmap for the next tools in the pipeline.

**Get started:**

Download the VST3 and AU binaries for macOS arm64 and Windows x64 at **https://andrewrahman.com/get-osd**. The source, CI configuration, and HRTF files are at **https://github.com/Spatial-Media-Lab/OpenSpatialDelay**. Follow the work and support the pipeline at **https://patreon.com/AndrewRahman**. The Spatial Media Lab blog at **spatialmedialab.org** carries the full launch story.

---

## System requirements

- **macOS:** arm64 (Apple Silicon), Sequoia (15.x) or newer. Intel Mac is not supported in v1.0.0 — Apple Silicon only.
- **Windows:** Windows 10 or Windows 11, x64.
- **DAW:** Any VST3 or AU host. Tested against Reaper, Logic Pro, and Ableton Live (specific DAW versions documented in the README).
- **macOS install note:** Apple's Gatekeeper flags unsigned free plugins on recent macOS. Run the `xattr` command documented in the README to clear the quarantine attribute — it is a single terminal line.

---

## Screenshots (upload 3–6 PNGs)

Supply in this order; upload as many as the dashboard allows (minimum 3):

1. **screenshot_full.png** — Plugin UI full view (hero image)
2. **screenshot_spatial_map.png** — Spatial map view showing 12 delay taps positioned in 3D space
3. **screenshot_presets.png** — 70 factory presets panel (or use screenshot_right_panel.png if presets panel alone is not available)
4. **screenshot_bottom_panel.png** — HRTF profile selector and binaural rendering controls
5. **signal-flow.png** — Signal flow architectural diagram
6. **screenshot_shimmer.png** or **screenshot_elevation_map.png** — Alternate view if a sixth slot is available

All PNG assets are in the `docs/` directory of the GitHub repository.

---

## Listing-tone checklist (self-review before submit)

Work through this checklist at T-1 before pasting into the KVR dashboard:

- [ ] No "buy" or "purchase" language anywhere in the listing body
- [ ] No DSP jargon: no FFT, no phase vocoder, no paper names (Laroche, Dolson, Röbel, Gardner, Martin), no partition sizes, no STFT, no "2048-sample"
- [ ] No AAX or VST2 mentions (VST3 and AU only)
- [ ] No "commercial license" language — GPL-3.0 is the only license mentioned
- [ ] No claims of "zero-latency mode" (does not exist)
- [ ] No claim of "ConstantPower for stereo" (it is surround-only)
- [ ] No claim of "cross-feedback" or "stereo-width control" (not implemented)
- [ ] Specific numbers used throughout: 12 taps, 7 surround algorithms, 5 HRTF profiles, 70 presets, 162 tests
- [ ] Stereo named first: stereo use case is the FIRST capability in the description, not buried
- [ ] Three-families architecture correct: "5 stereo modes, 6 binaural modes, 7 surround algorithms"
- [ ] Tag line matches exactly: Delay, Spatial, Free, VST3, AU, macOS, Windows
- [ ] Verify KVR taxonomy: confirm "Spatial" is still a canonical 2026 tag; if renamed to "Immersive" or "3D Audio", update the tag line here and in kvr-news-submission.md before submit
- [ ] Links tested at T-1 — all URLs return 200 OK:
  - https://andrewrahman.com/get-osd
  - https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0
  - https://github.com/Spatial-Media-Lab/OpenSpatialDelay
  - https://spatialmedialab.org
  - https://patreon.com/AndrewRahman
- [ ] Screenshots uploaded: minimum 3 PNGs from the list above
