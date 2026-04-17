---
status: locked
phase: 05-launch-announcements
decided: 2026-04-18
decided_by: andrew
supersedes: none
applies_to: [05-03, 05-04, 05-05, 05-06, 05-07, 05-08]
---

# Phase 5 Locked Framing — Stereo-First + Three Families + Tongue-in-Cheek Zodiac

Written post-05-02-rewrite to prevent the factual + framing drift that caught us in the initial 05-01 + 05-02 drafts. Every downstream plan in this phase must honor these rules. No exceptions without explicit Andrew override.

## Rule 1 — Lead with stereo, everywhere

**Why:** Andrew's 2026-04-18 directive: "there are thousands, if not millions, more stereo users than spatial users. We need to make it crystal clear everywhere in every post in every newsletter and especially on the site that it can be used in stereo as well and grow with you into spatial."

**How to apply:**
- Every public-facing surface (blog, newsletter, LinkedIn, IG, KVR, pitches) must name **stereo use** as a primary capability within the first paragraph or first 3 bullets.
- Do NOT frame OSD as "an immersive plugin that also happens to work in stereo."
- DO frame OSD as "a spatial delay that works in stereo today and grows with your mix into binaural, surround, or Atmos tomorrow."
- The word "grow" / "grows with you" / "step up" is the approved metaphor.

## Rule 2 — Three algorithm families, not "7 algorithms"

**Why:** Prior drafts claimed "7 spatialization algorithms" covers stereo-to-dome. That's factually wrong. The 7 surround algorithms are surround-only. Stereo output uses a separate family of 5 modes; Binaural uses a third family of 6.

**Correct architecture (verified 2026-04-18 against `Source/PluginProcessor.cpp:5075`, `Source/PluginEditor.cpp:3617`):**

| Family | Count | Modes |
| ------ | ----- | ----- |
| **Stereo** | 5 | Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein |
| **Binaural** | 6 | 5 measured HRTFs (KU100 / CIPIC / HUTUBS / MIT KEMAR / SADIE) + 1 Simple (Woodworth ITD, no HRTF convolution) |
| **Surround / Immersive** | 7 | Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP |

**Output formats supported:** 23 total — Stereo, Binaural, Quad, 5.0, 5.1, 7.0, 7.1, 9.1, Octaphonic, SML 13.1, 5.1.2/5.1.4/7.1.2/7.1.4/7.1.6/9.1.4/9.1.6 Atmos, and Ambisonics 1OA through 6OA.

**How to apply:**
- When naming algorithms, use the three-families structure, not a flat count.
- It is fine to use abbreviated language on short-form surfaces (LinkedIn, IG, KVR listing), e.g. "5 stereo modes, 6 binaural modes, 7 surround algorithms" or "three algorithm families — stereo, binaural, surround".
- When naming stereo modes, use: Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein.
- When naming HRTF profiles, use: KU100, CIPIC, HUTUBS, MIT KEMAR, SADIE (+ Simple).
- When naming surround algorithms, use: Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP.

## Rule 3 — Anchor stereo use with concrete presets

**Why:** Claims are stronger with named anchors. OSD ships two explicit stereo presets — name them.

- **Preset #1: Stereo Ping-Pong** (Classic Delays category) — 2 taps at ±90° azimuth. Classic psychoacoustic effect.
- **Preset #35: Wide Stereo** (Surround Production category) — 2 taps at ±30° standard stereo pair, tempo-synced 1/4 note.

**How to apply:**
- Blog, newsletter, KVR listing must name at least one of these.
- LinkedIn + IG can name them where space allows — they signal "this plugin knows what stereo producers already use."

## Rule 4 — Stereo use cases (sourced from code, not speculation)

The stereo pipeline supports these real use cases, verified in `Source/PluginProcessor.cpp` and `Tests/PreReleaseTests.cpp`:

1. **Stereo ping-pong** (Preset #1, taps at ±90°)
2. **Wide stereo pair** (Preset #35, ±30° mic pair, tempo-synced)
3. **Mid/side encoding** (MS Encode mode — L = M+S, R = M−S; vinyl-safe-bass domain processing)
4. **Headphone spatial delay on stereo track** (select Binaural output + any HRTF on any stereo bus; no DAW surround setup required)
5. **Per-tap L/R channel routing** (verified in `PreReleaseTests.cpp` — route taps to L-only, R-only, or both)

**How to apply:** any copy that lists "stereo use cases" pulls from this list. Do NOT invent use cases. Do NOT claim things that aren't in the list (see Rule 5).

## Rule 5 — Facts we MUST NOT claim

These were specifically flagged by the 2026-04-18 codebase research:

- ❌ "Zero-latency mode" — does not exist. 2048-sample latency is unconditional (phase vocoder FFT, PDC-compensated on dry path). CLAUDE.md gotcha #2 covers this.
- ❌ "ConstantPower for stereo" — ConstantPower is surround-only. Stereo's default pan is Equal Power.
- ❌ "Height on stereo output" — elevation only attenuates on stereo. Only Binaural renders true spatial height.
- ❌ "Cross-feedback" or "stereo-width control" — not implemented.
- ❌ "7 algorithms for stereo" — the 7 are surround-only (Rule 2).
- ❌ "Three years of development" — OSD was built over seven weeks starting 2026-03-06 with Claude Code as a pairing partner. The "wanting it for years" framing is fine; the "building it for years" framing is false.

## Rule 6 — Zodiac: tongue-in-cheek, Instagram only

**Why:** Andrew's 2026-04-18 directive: "Zodiac is tongue-in-cheek and just used for marketing materials, not important for the product itself."

**How to apply:**
- 05-04 Instagram carousel — 12 slides × 12 taps × 12 zodiac signs is a structural joke. Do it.
- 05-01 blog, 05-02 newsletter, 05-03 LinkedIn, 05-05 KVR listing, 05-06 review pitches, 05-07 Berlin DMs, 05-08 guest pitches — **NO zodiac references**. Those audiences skew serious; zodiac signals unseriousness.
- Do NOT rename any factory preset to a zodiac sign. Product clarity > marketing cleverness.
- Do NOT add zodiac iconography to the plugin UI.

## Rule 7 — Distribution path (re-affirmed)

Per 05-01 revision (commit `9f9ee88`):
- **Binary downloads** live at `andrewrahman.com/get-osd` (email-wall). The primary "Download" CTA in all copy points here.
- **Source code** lives at `github.com/Spatial-Media-Lab/OpenSpatialDelay`. Framed as source/build-from-source path, NOT as "the download link."
- **Support link:** `patreon.com/AndrewRahman`.
- **SML org:** `spatialmedialab.org`.
- **Creator home:** `andrewrahman.com`.

These URLs stay consistent across all downstream artefacts.

## Rule 8 — Voice / timeline / personal framing

Per 05-01 revision, the Andrew-voice baseline:

- "I've been wanting a delay like this for years."
- Timeline: "I started on March 6, 2026, and seven weeks later — today — it ships." (NOT "three years" — that's wrong.)
- "The delay I wanted, for everyone."
- "See you at Superbooth!" (with exclamation).
- First-person, warm, direct. No vulnerable framings like "unpaid" or "alone at a kitchen table" on surfaces other than the blog (blog keeps it; short-form drops it).
- Capability-level language only. No DSP jargon (FFT size, paper names, partition method). No commercial-product vocabulary ("buy", "purchase", "upgrade").

## Plans this file locks

- 05-03 LinkedIn launch post (not yet drafted) — must honor Rules 1–8.
- 05-04 Instagram (not yet drafted) — must honor Rules 1–8 **plus** exercise Rule 6 (zodiac carousel).
- 05-05 KVR listing (not yet drafted) — must honor Rules 1–8. KVR audience is ~80% stereo-music producers — stereo framing is MOST important here.
- 05-06 review-pitch outreach (not yet drafted) — must honor Rules 1–8. Individual personalisations may reference reviewer's existing stereo-tool reviews (e.g. Valhalla, Soundtoys, FabFilter) to anchor OSD as "stereo-native plus more."
- 05-07 Berlin DMs (not yet drafted) — must honor Rules 1–8. Local-relationship tone; lead with the hook most relevant to each recipient's body of work.
- 05-08 guest-pitch drafts (not yet drafted) — must honor Rules 1–8.
