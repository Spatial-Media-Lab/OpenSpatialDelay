---
title: Patreon Creator Page — Step-by-Step Setup Guide
phase: 02-email-capture-funding-infrastructure
plan: 02-06
purpose: Single-document walkthrough for setting up patreon.com/andrewrahman. Work through top-to-bottom.
audience: Andrew (user) — work through this in your browser while the agent continues Phase 2
---

# Patreon Setup — Everything You Need to Paste

All copy below is the final draft from `drafts/patreon-*.md` (merged to main as commit `35fe5fe`).
**Review each section and edit for voice before pasting into Patreon (D-29 — voice revision is
your gate).** Every section marked 📋 has a paste-ready copy block.

Total time: ~45–60 min.

---

## Pre-flight

- [ ] You have a personal email address NOT tied to the Spatial Media Lab org (D-21 — page is
      under Andrew Rahman's personal identity, not SML).
- [ ] You know your authenticator app is ready for 2FA setup (T-02-26 threat mitigation).
- [ ] Create `docs/phase-02-evidence/` locally and plan to save 4 screenshots there:
      `patreon-vanity-url.png`, `patreon-tiers.png`, `patreon-launch-confirmation.png`,
      `patreon-page-live.png`.
- [ ] Graphics bundle ready at `docs/phase-02-evidence/patreon-graphics/` (see Step 0 below).

---

## Step 0 — Graphics You'll Upload

All images below are committed to the repo at `docs/phase-02-evidence/patreon-graphics/`.
Upload them verbatim into Patreon — no cropping or resizing needed (Patreon will handle
any final fit). The set uses a space-exploration visual language that matches the tier names
(Stargazer → Astronaut → Commander → Mission Control).

| # | Patreon slot | Spec | File (repo-relative) | Source |
|---|--------------|------|----------------------|--------|
| 1 | **Profile avatar** (circle next to creator name) | 500×500 square | `docs/phase-02-evidence/patreon-graphics/avatar-500x500.png` | Downloaded from spatialmedialab.org/about/ (Andrew's headshot), center-biased square crop |
| 2 | **Cover image** (page header banner) | 1600×400 | `docs/phase-02-evidence/patreon-graphics/cover-1600x400.png` | Generated — dark starfield + "SPATIAL MEDIA LIBRARY" wordmark + tagline |
| 3 | **Tier 1 image** (Stargazer) | 512×512 | `docs/phase-02-evidence/patreon-graphics/tier-1-stargazer.png` | Generated — gold star + constellation |
| 4 | **Tier 2 image** (Astronaut) | 512×512 | `docs/phase-02-evidence/patreon-graphics/tier-2-astronaut.png` | Generated — astronaut silhouette with visor + antenna |
| 5 | **Tier 3 image** (Commander) | 512×512 | `docs/phase-02-evidence/patreon-graphics/tier-3-commander.png` | Generated — rank chevrons + command star |
| 6 | **Tier 4 image** (Mission Control) | 512×512 | `docs/phase-02-evidence/patreon-graphics/tier-4-mission-control.png` | Generated — Earth + orbit ring + launch rocket |
| 7 | **Post 1 cover** (Welcome anchor post) | 1200×675 | `docs/phase-02-evidence/patreon-graphics/post-01-cover-1200x675.png` | Plugin screenshot (`docs/assets/screenshot_full.png`) + branded title overlay |

**Regenerating:** if you change the visual style, rerun
`python3 docs/phase-02-evidence/patreon-graphics/_generate.py` from the repo root
(requires PIL / Pillow). The generator is self-contained, deterministic (seeded), and reads
the headshot from `avatar-andrew-rahman.jpg` + plugin screenshot from `docs/assets/screenshot_full.png`.

**Optional personal replacement:** if you prefer a different headshot, drop a JPG named
`avatar-andrew-rahman.jpg` into the same directory and rerun the generator — it will re-crop
to 500×500 automatically.

**Where each graphic is uploaded:**
- Avatar → upload in Step 1 (Create Creator Account), profile settings.
- Cover → upload in Step 1 or Step 5 (About Section) — Patreon exposes it in both places.
- Tier images → upload alongside each tier in Step 4 (the Tier rows below now each reference their specific file).
- Post 1 cover → upload when creating Post 1 in Step 8, in the "Post image / thumbnail" slot.

---

## Step 1 — Create Creator Account

Go to **patreon.com/signup/creator**.

- [ ] Sign up with your **personal email** (not SML org email — D-21)
- [ ] **Creator name:** `Andrew Rahman`
- [ ] **Tagline** (Patreon field: "What are you creating?"):

📋 **Paste this tagline:**
```
Funding the Spatial Media Library — spatial audio tools for musicians and sound designers
```

- [ ] Enable **2FA** in Account Settings → Security immediately (T-02-26).
- [ ] **Upload profile avatar** → `docs/phase-02-evidence/patreon-graphics/avatar-500x500.png`
- [ ] **Upload cover image** → `docs/phase-02-evidence/patreon-graphics/cover-1600x400.png`

---

## Step 2 — Vanity URL

Page Settings → URL.

- [ ] Try `andrewrahman` → gives `patreon.com/andrewrahman` (first choice)
- [ ] If taken, try `andrew-rahman`
- [ ] If still taken, try `andrewjrahman`
- [ ] **Record which one you used** (you'll need it for the resume signal at the end)
- [ ] 📸 Screenshot → `docs/phase-02-evidence/patreon-vanity-url.png`

---

## Step 3 — Billing Plan

Payouts / Earnings settings.

- [ ] Select **Standard plan** (10% fee — default for new creators post 2025-08-04 per RESEARCH.md)
- [ ] Do **NOT** pick Pro or enterprise tiers — they're not justified for a new creator page

---

## Step 4 — Tiers (4 monthly, no annual)

Create exactly **4 monthly tiers**. **DO NOT enable annual billing on any tier** —
not eligible until the page is 3+ months live and earning $200+/mo (RESEARCH.md Pitfall 4).

### Tier 1 — $3/mo · Stargazer

📋 **Name:** `Stargazer`
📋 **Price:** `$3 / month`
📋 **Tier image:** upload `docs/phase-02-evidence/patreon-graphics/tier-1-stargazer.png`
📋 **Benefits (paste as bullet list):**
```
- Newsletter updates on new Spatial Media Library tools
- Public acknowledgment as a Spatial Media Library supporter
```

### Tier 2 — $10/mo · Astronaut

📋 **Name:** `Astronaut`
📋 **Price:** `$10 / month`
📋 **Tier image:** upload `docs/phase-02-evidence/patreon-graphics/tier-2-astronaut.png`
📋 **Benefits:**
```
- All Stargazer benefits
- Vote on feature priorities for OpenSpatialDelay and future Spatial Media Library tools
- Access to early builds before public release
```

### Tier 3 — $25/mo · Commander

📋 **Name:** `Commander`
📋 **Price:** `$25 / month`
📋 **Tier image:** upload `docs/phase-02-evidence/patreon-graphics/tier-3-commander.png`
📋 **Benefits:**
```
- All Astronaut benefits
- 1:1 access via Discord or email
- Early access to future patron-exclusive Spatial Media Library tools
```

### Tier 4 — $100/mo · Mission Control

📋 **Name:** `Mission Control`
📋 **Price:** `$100 / month`
📋 **Tier image:** upload `docs/phase-02-evidence/patreon-graphics/tier-4-mission-control.png`
📋 **Benefits:**
```
- All Commander benefits
- Permanent public thank-you credit in the OpenSpatialDelay About dialog
```

- [ ] 📸 After all 4 tiers are created → screenshot → `docs/phase-02-evidence/patreon-tiers.png`

---

## Step 5 — About Section

Page Settings → About → paste the full block below.

📋 **About section (paste verbatim, then revise for voice):**
```
I'm Andrew Rahman, an audio software developer based in Berlin. I build spatial audio tools under
the **Spatial Media Library** project, developed open-source through **Spatial Media Lab**.

**OpenSpatialDelay** is the first — a free GPL-3.0 spatial delay plugin for VST3 + AU on macOS and
Windows. It places each echo at a distinct 3D position around the listener, using HRTF convolution
for binaural rendering, a phase vocoder for pitch-independent time-stretching, and a trajectory
engine for per-echo Doppler simulation.

The tools are free. Your support keeps them free. Patreon funds the pipeline so I can keep
shipping spatial-audio tools without chasing licensing revenue.

---

OpenSpatialDelay source code lives at [github.com/Spatial-Media-Lab/OpenSpatialDelay](https://github.com/Spatial-Media-Lab/OpenSpatialDelay).
The Spatial Media Lab organization — the legal home of the code — is at [SpatialMediaLab.org](https://spatialmedialab.org).

---

OpenSpatialDelay is GPL and free for everyone. Some future Spatial Media Library tools will launch
on Patreon first or as patron-exclusive — those will be announced as they come. No specific
timeframes committed.

---

Annual membership at a ~15% discount is planned — coming once eligibility requirements are met
(3 months live + sustained monthly earnings).
```

---

## Step 6 — Berlin One-Liner (sidebar or footer)

Place this somewhere Patreon exposes sidebar/footer copy (D-25 — low-urgency, drop if Patreon
doesn't give you a slot):

📋 **Berlin one-liner:**
```
In Berlin? Come by **Spatial Media Lab** — we run events and collaborate with local artists.
spatialmedialab.org
```

---

## Step 7 — Cross-Links (D-26 — 4 links required)

Page settings → Links / Social. Add these **4 links** (in order of priority):

1. `https://github.com/Spatial-Media-Lab/OpenSpatialDelay` (label: "Source code")
2. `https://spatialmedialab.org` (label: "Spatial Media Lab")
3. `https://andrewrahman.com` (label: "Creator home")
4. `https://andrewrahman.com/privacy` (label: "Privacy")

**If Patreon caps social slots at 3** (it sometimes does), drop the privacy link from the
social section — instead embed it as a hyperlink inside the About text. The first 3 links must
stay in the social slots.

---

## Step 8 — Upload 3 Seed Posts

Post in **this exact publish order**. For each post: paste the body verbatim (skip frontmatter
lines like `**Visibility:** PUBLIC`), confirm preview rendering, then publish.

### Post 1 (PUBLIC, anchor post) — *"Welcome to the Spatial Media Library Patreon"*

- [ ] Visibility: **Public**
- [ ] Title: `Welcome to the Spatial Media Library Patreon`
- [ ] **Post cover image** → upload `docs/phase-02-evidence/patreon-graphics/post-01-cover-1200x675.png`

📋 **Body (paste verbatim):**
```markdown
Hi — I'm Andrew. I build spatial audio software through **Spatial Media Lab**, a small organization
based in Berlin. This Patreon funds the **Spatial Media Library** pipeline: a set of spatial audio
tools for musicians and sound designers, released as GPL open-source.

## Why Patreon

The tools are free and the source code is always public. But building them takes real time and
hardware. Patreon lets supporters directly fund the pipeline so I can keep shipping tools without
chasing licensing revenue. The first tool — **OpenSpatialDelay** — is already out. More are in
progress. Your support determines the pace.

## Pipeline Vision

The Spatial Media Library is a pipeline, not a single plugin. Spatial audio is a deep space —
binaural rendering, convolution reverbs, trajectory-driven effects, immersive mixing tools. There
is room for a dozen focused, free, open-source tools in this space, and I want to build them.

OpenSpatialDelay is proof the approach works. What comes next is what patrons help fund.

## Source Code

Source code lives at **Spatial Media Lab**:
[github.com/Spatial-Media-Lab](https://github.com/Spatial-Media-Lab). GPL-3.0. Fork it, study it,
improve it.

## Call to Action

If you use OpenSpatialDelay or care about open-source spatial audio, consider becoming a patron.
Every tier counts. $3/mo keeps one project alive; $100/mo keeps three going.

— Andrew
```

### Post 2 (PATRON-ONLY, $3+) — *"Inside OpenSpatialDelay: HRTF, Phase Vocoder, and 3D Trajectory"*

- [ ] Visibility: **Paid members only ($3+ Stargazer and above)**
- [ ] Title: `Inside OpenSpatialDelay: HRTF, Phase Vocoder, and 3D Trajectory`
- [ ] Public teaser (shows above paywall — paste into the teaser field):

📋 **Public teaser:**
```
A look at the DSP architecture behind OpenSpatialDelay — for patrons who want to know how it actually works.
```

📋 **Body (paste verbatim, patron-only):**
```markdown
OpenSpatialDelay is a spatial delay plugin where every echo lives at a distinct 3D position — the
first tool in the **Spatial Media Library** pipeline, with source code maintained under
**Spatial Media Lab**. This post walks through the DSP building blocks that make it possible.

## 1. HRTF Convolution — How Each Echo Lands at a Specific Position

An echo gets its spatial location by convolving the tap's audio with a pair of HRIRs (head-related
impulse responses) — one for the left ear, one for the right. The plugin ships with 5 HRTF
profiles loaded from SOFA files, sourced from public research datasets (KU100, CIPIC, HUTUBS,
MIT KEMAR, SADIE). Each profile is a multi-dimensional array of impulse responses indexed by
azimuth and elevation.

The convolution runs through a **partitioned convolver** — the impulse response is split into
equally-sized blocks, FFT'd up-front, and the incoming audio is convolved block-by-block in the
frequency domain. This keeps per-buffer CPU bounded for IRs that can be thousands of samples long.
Spectral envelope smoothing is applied per frequency bin (magnitude and phase smoothed separately)
to prevent comb-filtering artifacts when the position interpolates between HRIR snapshots.

## 2. Phase Vocoder — Pitch-Independent Time-Stretching

The Doppler effect in OpenSpatialDelay needs to stretch or compress the echo's playback rate
without changing pitch. Pure resampling couples the two — faster = higher pitch. The plugin uses
a **phase vocoder** built on a 2048-point STFT with 4× overlap:

- Laroche-Dolson phase locking keeps tonal content coherent across frames
- A Röbel-style spectral flux detector with an adaptive median threshold flags transients
- Phase reset on transient frames preserves attack sharpness instead of smearing it
- Range: ±12 semitones (combined with the Doppler engine)
- Latency: 2048 samples, reported to the DAW so PDC works correctly

The dry path carries a matching 2048-sample latency compensation so wet/dry mixes stay phase-
aligned. If you set the plugin to 100% dry, the output is still latency-compensated. The dry/wet
mix uses an equal-power crossfade (cos/sin) instead of linear, so perceived loudness stays
constant at all mix settings.

## 3. Trajectory Engine — Per-Echo 3D Position

Each echo has an independent position in 3D space. The trajectory engine lets you define a path
(line, circle, orbit, hand-drawn spline) that all echoes follow with configurable delay offsets
between them. The engine drives the HRTF convolver's azimuth/elevation interpolation frame-by-
frame, and separately feeds the Doppler engine an instantaneous velocity for pitch-free time-
stretch.

The **ITD/ILD model** adds interaural time and level differences on top of HRTF convolution for
close-range source simulation. Note: for the MIT KEMAR profile, ITD is baked into the HRIR
waveform itself and the ITD delay line runs as a pass-through.

## 4. What This Architecture Enables Next

The DSP kernel here — partitioned convolution, phase vocoder, trajectory engine — isn't delay-
specific. It's a toolkit for a whole pipeline of spatial effects: spatial reverbs, convolution-
based placement processors, binaural chorus, moving-source simulations. Subsequent Spatial Media
Library tools will share these primitives.

That's the technical case for the pipeline framing: we build the kernel once, then ship N tools
on top of it.

— Andrew
```

### Post 3 (PUBLIC) — *"Where the Spatial Media Library Is Going"*

- [ ] Visibility: **Public**
- [ ] Title: `Where the Spatial Media Library Is Going`

📋 **Body (paste verbatim):**
```markdown
OpenSpatialDelay is live. Here's the broader context for what the Spatial Media Library is trying
to build.

## Pipeline Themes

The Spatial Media Library is about three things:

1. **Spatial processing** — effects where position in 3D space is a first-class parameter, not a
   post-hoc pan. OpenSpatialDelay is the first example: each echo is placed, not panned.
2. **Binaural rendering** — making 3D audio work on headphones, which is how most people actually
   listen. HRTF convolution is the foundation; future tools will extend this to reverbs,
   ambisonic decoders, and close-range simulations.
3. **Immersive mixing tools** — utilities that help you work with spatial audio during a session,
   not just print it at the end. This category is wide open and will grow fastest.

## No Specific Promises

I'm not announcing specific plugins or timelines here. The Spatial Media Library is a commitment
to keep shipping in this space, at a pace set by patron support. If this Patreon funds at a level
that lets me work on it full-time, the pipeline ships fast. If it funds at a level that buys me
a few focused weekends per month, the pipeline ships slower but still ships.

Either way, everything will be GPL and free. That's non-negotiable.

## Why Open Source

Free + open-source tools outlast their authors. If I stop shipping tomorrow, OpenSpatialDelay
doesn't disappear — the source code lives under **Spatial Media Lab** at
[github.com/Spatial-Media-Lab](https://github.com/Spatial-Media-Lab). Anyone can fork it, improve
it, release their own builds. The **Spatial Media Library** exists to grow that commons, not to
lock anyone in.

If you've found OpenSpatialDelay useful, or you care about open-source spatial audio generally,
consider becoming a patron. The pace of the pipeline is the pace you set.

— Andrew
```

---

## Step 9 — Preview Check (before launching)

Before clicking Launch, flip through the preview:

- [ ] About section reads cleanly — no raw Markdown characters visible, no `**bold**` showing
      as literal asterisks
- [ ] Tagline visible in header area
- [ ] All 4 tiers visible with correct prices ($3/$10/$25/$100), no annual option
- [ ] All 4 cross-links visible (or 3 if the privacy link was folded into About)
- [ ] Post 1 rendered publicly (anyone can read end-to-end)
- [ ] Post 2 shows public teaser; rest is locked behind "support to unlock"
- [ ] Post 3 rendered publicly (anyone can read end-to-end)

---

## Step 10 — LAUNCH (RESEARCH.md Pitfall 3 — critical)

Page settings → **Launch checklist** → click **"Launch page"**.

- [ ] Without this step, the page is in "not launched" mode — public visitors see a "not launched
      yet" message regardless of how complete the setup is. **This click is mandatory.**
- [ ] 📸 Screenshot the "page is now live" confirmation → `docs/phase-02-evidence/patreon-launch-confirmation.png`

---

## Step 11 — Incognito Verification

Open a fresh **incognito/private window** (not logged into Patreon). Visit:

```
https://patreon.com/<your-vanity-URL>
```

Confirm ALL of these in the incognito view:

- [ ] Page loads (HTTP 200)
- [ ] Tagline visible in header
- [ ] About section visible with both brand names ("Spatial Media Library" AND "Spatial Media Lab")
- [ ] 4 tiers visible, no annual toggle
- [ ] Post 1 (Welcome) visible end-to-end
- [ ] Post 2 (Technical) visible as locked/teased
- [ ] Post 3 (Roadmap) visible end-to-end
- [ ] All 4 D-26 cross-links present (or 3 with privacy in About)
- [ ] Berlin one-liner visible if Patreon rendered a sidebar/footer for it
- [ ] 📸 Screenshot → `docs/phase-02-evidence/patreon-page-live.png`

---

## Step 12 — Reader Test

Read the public About + Post 1 end-to-end in the incognito view. Ask yourself:

> Can I answer "what is this Patreon for?" with
> "funding the Spatial Media Library pipeline of spatial audio tools, of which OpenSpatialDelay is the first"?

- [ ] Yes → Setup complete.
- [ ] No → Revise About copy until the answer is obvious; republish.

---

## Step 13 — Return Signal to Resume Plan 02-06

When everything above is done, reply in chat with:

```
Patreon launched

Vanity URL used: <andrewrahman | andrew-rahman | andrewjrahman | other>
Drafts revised during voice pass: <yes, list files | no, drafts as-is>
Screenshots stored at:
  docs/phase-02-evidence/patreon-vanity-url.png
  docs/phase-02-evidence/patreon-tiers.png
  docs/phase-02-evidence/patreon-launch-confirmation.png
  docs/phase-02-evidence/patreon-page-live.png

curl -sI https://patreon.com/<vanity> | head -1
<paste output, expecting HTTP/2 200 or 301>

Reader test passes: yes/no
```

A continuation agent will then verify the screenshots + curl output and write the final
02-06-SUMMARY.md to close the plan.

---

## Notes on deviations already applied in drafts

The agent made two accuracy corrections while drafting — you should sanity-check these match
what you want to say publicly:

- **HRTF profile count:** Plan said 6, actual `HRTF/` directory has 5 SOFA files — Post 2 now
  says "5 profiles (KU100, CIPIC, HUTUBS, MIT KEMAR, SADIE)".
- **Phase vocoder description:** Plan said "with WSOLA-Lite adjustments", but `agent_docs/architecture.md`
  says the phase vocoder *replaced* WSOLA — Post 2 now says "Laroche-Dolson phase locking +
  Röbel-style transient detection".

Both corrections match what's actually in the repo today.
