# LinkedIn Launch Post — OpenSpatialDelay v1.0.0

## PRE-LAUNCH CHECKLIST (T-1 morning, 2026-04-27)

- [ ] Open LinkedIn profile → Edit intro → Contact info → Website field → set to `https://andrewrahman.com/get-osd`. This is the bio-link target the post's CTA depends on. Verify by viewing your own profile in incognito/public mode.
- [ ] Confirm bio/headline text mentions "Berlin" somewhere so the "Link in bio" CTA pays off visually when a reader taps through.
- [ ] Decide: Format A (text-only) OR Format B (document carousel). Research says B gets ~3× engagement; A is zero-friction if a Figma/Canva PDF isn't ready. Honest recommendation: ship A if the PDF isn't already designed by T-2 evening — B is better only when the design is good.
- [ ] Post at 2026-04-28 10:20 CET per launch-day-runbook step 5.
- [ ] DO NOT paste any URL into the first-comment slot. DO NOT add any link in the post body. (2026 LinkedIn algorithm penalises both — see `05-RESEARCH.md` Anti-Patterns + Pitfall 2. The bio-link is the compliant CTA path.)

---

## Format A — Text-only storytelling post

**Body character count: 2,128 chars** (within LinkedIn's 1,301–2,500 sweet spot). First 210 characters visible before "see more" on desktop land the hook + the stereo-first framing + the capability sentence.

```
I've been wanting a delay like this for years. Seven weeks ago I sat down in my Berlin apartment and started building it. Today it ships — free.

OpenSpatialDelay v1.0.0 is live. It's a VST3/AU delay where each echo has a 3D position in space, and — this is the part I'd want a stereo producer to hear first — it works on a plain stereo track today.

Load it on a bus in Logic or Ableton and you get five classic stereo modes (Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein), twelve independently-positioned taps, and the full preset bank — "Stereo Ping-Pong" and "Wide Stereo" are in there alongside cinematic whooshes, ambient beds, and rhythmic counter-lines. The day you take on a headphone piece, a 5.1 room, a dome, a 7.1.4 Atmos session — flip one dropdown and the same patches come with you.

Under the hood there are three algorithm families — five stereo modes, six binaural modes (five measured HRTF datasets plus a CPU-lite fallback), seven surround/immersive algorithms covering Quad through 7.1.4 Atmos and Ambisonics up to sixth order. ADM-OSC in and out so it talks to Spat Revolution, Panoramix, Iannix, TouchDesigner. 70 factory presets. No trial timer. No licence server. GPL-3.0.

I kept the whole pipeline open because it's genuinely useful that way — for students learning Ambisonics, for composers scoring their first VR piece, for mixers in Berlin and everywhere else who want to experiment with immersive without signing up for another subscription, and for stereo producers who want a delay that grows with their mix as they grow with it.

OSD is the first tool in the Spatial Media Library — a small family of spatial-audio plugins sharing one framework underneath. Panners, choruses, reverbs, synthesizers. Build the engine once, ship several tools on top of it. Solo dev, Berlin, open source — that's the whole shop.

If you're curious what a free, open-source spatial delay sounds like — Link in bio.

(Berlin folks: I'll be at Superbooth Messe May 7–10. If you want to see it running in person, find me.)

#SpatialAudio #DolbyAtmos #AudioPlugin #IndieDev #OpenSource
```

---

## Format B — Native document carousel (7 pages)

Export as a 1080×1350 PDF (7 pages). Tool: Figma or Canva. Attach via LinkedIn "Add a document" in the post composer. Document carousels average ~3× engagement vs text-only per dataslayer.ai Feb 2026 data; trade-off is ~60–90 min of design time.

### Page-by-page spec

**Page 1 (cover)** — Black background, OSD wordmark centre, tagline stack below:
> Each echo has a 3D position in space.
> Free. GPL-3.0. VST3 + AU. macOS + Windows.
> OpenSpatialDelay v1.0.0 — out today

**Page 2 — "Stereo today"**
Headline: Stereo today, spatial tomorrow
Body: Load OSD on any stereo track in Logic or Ableton and it works immediately — five classic stereo modes, twelve independently-positioned taps, "Stereo Ping-Pong" and "Wide Stereo" in the preset bank. Same patches grow with your mix into binaural, surround, or 7.1.4 Atmos.

**Page 3 — "12 taps, each in 3D"**
Headline: 12 delay taps, each with an independent 3D position
Body: One plain-English sentence on what that means for a producer: every echo has its own position, feedback, filter, and trajectory. Above, behind, four metres to the left, drifting upward at the tail — wherever the piece wants it.

**Page 4 — "Three algorithm families"**
Headline: Stereo. Binaural. Surround. One plugin.
Body: Three families — 5 stereo modes, 6 binaural modes (5 measured HRTF datasets + a CPU-lite fallback), 7 surround/immersive algorithms. Flip one dropdown, same patches, different output.
Small type grid: Stereo — Equal Power, Stereo VBAP, XY Pair, MS Encode, Blumlein. Binaural — KU100, CIPIC, HUTUBS, MIT KEMAR, SADIE, Simple. Surround — Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP.

**Page 5 — "70 presets + ADM-OSC"**
Headline: Starting points + external control
Body: 70 factory presets — from "Stereo Ping-Pong" and "Wide Stereo" for conventional mixes, to binaural-headphone specials, cinematic whooshes, ambient beds, 8-channel dome setups, and Atmos utility patches. ADM-OSC in + out so the plugin talks to Spat Revolution, Panoramix, Iannix, and TouchDesigner.

**Page 6 — screenshot**
Full-bleed `screenshot_full.png` (from `andrewrahman-com/public/assets/`). Single caption: "the plugin UI." No overlay text. Let the image breathe.

**Page 7 — download CTA**
Headline: Get OpenSpatialDelay
Body:
> Link in bio → andrewrahman.com/get-osd
> Source: github.com/Spatial-Media-Lab/OpenSpatialDelay
> Support: patreon.com/AndrewRahman
Small type: VST3 + AU • macOS + Windows • GPL-3.0

### Companion caption (posted alongside the document, 689 chars — within 400–800 target)

```
Seven weeks ago I started building OpenSpatialDelay in my Berlin apartment. Today v1.0.0 ships — free, GPL-3.0.

It's a VST3/AU delay where each echo has a 3D position in space. It works on a plain stereo track today — five classic stereo modes, twelve independently-positioned taps, "Stereo Ping-Pong" and "Wide Stereo" in the preset bank — and grows with your mix into binaural, surround, or 7.1.4 Atmos tomorrow. Same patches, different output.

Swipe through the doc for the capability tour. Link in bio when you're ready to download.

(Berlin folks at Superbooth May 7–10 — find me and I'll show you in person.)

#SpatialAudio #DolbyAtmos #AudioPlugin #IndieDev #OpenSource
```

---

## Why bio-link and not a comment URL

LinkedIn's 2026 algorithm specifically demotes posts whose author drops an external URL into the first-comment slot. Two independent Feb-2026 reports (dataslayer.ai, blog.linkboost.co) confirm the platform now suppresses the author's own first comment when it contains a URL — the exact mechanism that killed the first-comment-URL workaround indie devs relied on from ~2021–2024. Bio-link CTA is the compliant 2026 pattern.

External links in the post body are also throttled (~60% reach reduction). So: no URLs in the body, no URLs in the first-comment slot, only the bio-link CTA. See `05-RESEARCH.md` Pitfall 2 for the full citation trail.

---

## Deviations from the plan template (for traceability)

- **Stereo-first hook (supplement Rule 1):** The plan's template led with an immersive-first story paragraph. The locked 2026-04-18 supplement requires stereo framing to appear in the first paragraph / first 3 bullets on every surface. Reframed so the stereo callout lands inside the hook itself, before any immersive language.
- **Three algorithm families, not "7 spatialization algorithms" (supplement Rule 2):** The plan's Format B template named "7 spatialization algorithms" as a single count. That's factually wrong — the 7 are surround-only. Replaced with the three-families architecture (5 stereo / 6 binaural / 7 surround) on both the text post and the carousel Page 4.
- **Seven-weeks timeline, not the multi-year claim (supplement Rules 5 + 8):** The plan template opened with a multi-year development framing. Supplement forbids that claim — OSD was built over seven weeks starting 2026-03-06. Rewrote the hook to "I've been wanting a delay like this for years. Seven weeks ago I sat down in my Berlin apartment and started building it." (keeps the emotional "years" framing legitimately — years of *wanting*, seven weeks of *building*.)
- **Preset-name anchors (supplement Rule 3):** Added "Stereo Ping-Pong" and "Wide Stereo" by name in both formats. These are the two explicit stereo presets OSD ships and signal to LinkedIn producers that the plugin speaks their vocabulary.
- **No zodiac (supplement Rule 6):** LinkedIn audience is serious. Zodiac is Instagram-only. No references.
- **Distribution URLs (supplement Rule 7):** Bio-link target is `andrewrahman.com/get-osd` (binary + email). GitHub is source-only. Patreon is support-only. Consistent across both formats and the pre-launch checklist.
