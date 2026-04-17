# Phase 5: Launch Announcements — Research

**Researched:** 2026-04-17
**Domain:** Multi-channel indie-plugin launch coordination (WordPress/Mailchimp blog+newsletter, LinkedIn, Instagram, KVR Audio, cold-email outreach, Berlin-local DM outreach)
**Confidence:** HIGH for publishing infrastructure, LinkedIn/IG 2026 norms, Berlin-contact handles; MEDIUM for SML send-path ownership + KVR approval turnaround; LOW for exact KVR form field specs (form is behind developer auth).

## Summary

Phase 5 is a content-and-coordination phase, not a technical-build phase. The research answered the 10 planner-flagged unknowns with mostly HIGH-confidence findings. Three findings materially change the plan shape:

1. **SML publishes on WordPress + Mailchimp, and Andrew already has publishing authority on the SML WordPress.** The 2024 post `spatialmedialab.org/ambisonic-to-atmos-template/` was written *by Andrew himself* — same author role can be used for the OSD launch post. No WordPress admin dependency. Send-path for the newsletter is Mailchimp (confirmed by the subscription form's "information will be transferred to Mailchimp" disclosure); ownership of the Mailchimp audience is the one remaining unknown — likely either Andrew or co-founder Timo Bittner / board member Basel Naouri, but needs a direct confirmation. D-08a is not a show-stopper.

2. **LinkedIn's "link in first comment" tactic is dead in 2026.** D-03 / per-channel-format (from CONTEXT.md) and the `influencer-outreach-v1.0.md` hook-line mention-ing "link in first comment" are based on outdated guidance. Current data: external links reduce reach ~60% whether in the post body OR in the first comment; the comment-link workaround is now specifically penalised. Planner must swap the LinkedIn format choice to *text-only storytelling with a bio-link prompt* (or a native LinkedIn document/PDF carousel — 3× engagement). This is a non-trivial copy-strategy pivot, not just wording.

3. **Phase 4 (Demo Content) is the only hard blocker.** Phase 5 critical-path deliverables (blog embed, IG Reel, KVR screenshots) all depend on CONT-01/02/03 assets. Phase 4 has no plans, no research, no CONTEXT — with 11 days to Apr 28, Phase 4 must be planned and executed in parallel with Phase 5 plan-writing or the launch date slips to the May 5 fallback.

Secondary findings: KVR requires a free developer account, accepts a new-product listing with a sensibly-formatted press release + images (no hard spec published; human editorial review required); Berlin contacts all have verified socials (Hainbach = Bluesky + IG, Kirn = Bluesky + Mastodon, Horstmann = LinkedIn + email, AES Germany chair = Ulli Scuda — no dedicated Berlin professional-section chair exists; the D-07 "AES Berlin chair" is best routed to Ulli Scuda or the Berlin student-section faculty Sporer/Weigelt); IG hashtag cap is now 5 (effective Dec 2025) and Reel sweet-spot is 15–30s for discovery.

**Primary recommendation:** Plan 05-00 should (a) add Phase 4 as a hard precondition at the top, (b) assign an early "confirm SML Mailchimp owner + send-path" task as a day-1 blocker-check, (c) rewrite the LinkedIn format spec to text-only-or-document-carousel (no "link in comment"), and (d) re-scope the AES Berlin DM in D-07 to Ulli Scuda (with student-section fallback).

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Launch Cadence**
- **D-01 Coordinated D-day burst:** All 5 channels (blog, newsletter, LinkedIn, IG, KVR) publish within a 2-hour window on a single day. Not rolling-week.
- **D-02 Day-of-week = Tuesday.** Highest B2B engagement; avoids Monday-email-backlog and Friday-attention-drift.
- **D-03 Target launch date = Tuesday April 28, 2026.** Driven by hard constraint: all public URLs/assets must be live before Superbooth (Berlin, May 7–10, 2026). Fallback date: Tuesday May 5, 2026 (overlaps with Andrew's travel window — avoid if possible). Planner must confirm Phase 3 site + Phase 4 demo content are ready in time.
- **D-04 User travel constraint:** Andrew is traveling for most of May 2026. Launch day itself (Apr 28) is pre-travel. Anything requiring Andrew's email response during May must NOT be triggered by the launch.

**Review-Pitch Outreach (Segments 1-5)**
- **D-05 Send at T-0 launch morning.** Cold-send all review-pitch emails on launch day itself. No pre-launch embargo process.
- **D-05a Scope of T-0 batch:** Segment 1 (10 contacts), Segment 2 (9), Segment 3 (8), Segment 4 (7 minus Berlin-local trio), Segment 5 (selective).

**Guest/Speaker-Pitch Outreach (Segment 7)**
- **D-06 Delay to late May / early June 2026.** Send AFTER Andrew's May travel period ends.
- **D-06a Decouple from launch day.** Guest-pitch is NOT part of the launch-day blast.

**Superbooth Berlin-Local Outreach (special case)**
- **D-07 Pre-Superbooth DMs to Berlin-local contacts.** Before May 7, send lightweight "see you at Superbooth" intros to Hainbach, Peter Kirn (CDM), Eric Horstmann (Immersive Lab), AES Germany Berlin section chair.
- **D-07a Tone:** short, personal, face-to-face-first. Relationship-first move.
- **D-07b Send timing:** within the next 1-2 weeks (starting now, through launch day).

**Newsletter Source (ANNC-02)**
- **D-08 Use separate spatialmedialab.org newsletter.** NOT the osd-confirmed Sender.net list at andrewrahman.com/get-osd.
- **D-08a Open question for planner:** Confirm who on the SML board manages the newsletter and the send-path.

### Claude's Discretion

- **Positioning hero angle** — Per-channel positioning. Blog: capability + free. LinkedIn: Berlin-indie-dev story. IG: audible demo. Core capability sentence consistent across all channels.
- **Per-channel content format split** — Planner designs (blog = depth + DSP story; newsletter = teaser + links; LinkedIn = personal narrative; IG = demo clip + carousel; KVR = product-metadata-first).
- **KVR listing category/tags** — Planner picks. Suggest primary `Delay`, secondary `Spatial`, plus `Free`, `VST3`, `AU`, `macOS`, `Windows`.
- **Email template styling** — Planner decides plain-text vs branded HTML per mode. Plain-text-looking for review-pitch; light branded HTML for guest-pitch.

### Deferred Ideas (OUT OF SCOPE)

- YouTube channel for Andrew (v2 / COMM-01)
- Own podcast (v2)
- Merchandise (v2)
- Product Hunt launch (v2 COMM-02)
- Launch-day livestream (deferred due to May travel)
- Press-release distribution service (PRWeb / PR Newswire — doesn't fit indie-dev positioning)
- Sound-design/film-scoring segment follow-up research (separate research round)
- AES Berlin formal speaker slot (Superbooth DM is the opener; formal talk is late-May/June guest-pitch)

</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| ANNC-01 | SML blog post with demo embed at `spatialmedialab.org` | WordPress platform confirmed; Andrew has author privileges (proven by his 2024 authored post); existing post structure template identified; permalink style = flat slug (`/ambisonic-to-atmos-template/`). Recommended mirror structure documented below. |
| ANNC-02 | SML newsletter email sent to subscribers | Mailchimp confirmed as ESP; subscribe form is on homepage; audience-owner TBD (likely board-level — Timo Bittner / Basel Naouri / Andrew). Discovery task required. |
| ANNC-03 | LinkedIn post published | 2026 algorithm research complete — no external links body OR first comment; pivot to text-only-story or document-carousel format. Character sweet-spot 1,301–2,500. Tuesday/B2B timing aligns with D-02. |
| ANNC-04 | Instagram post / Reel | Reel format recommended (15–30s discovery sweet-spot); 5-hashtag cap as of Dec 2025; first-line-before-"more" is the real caption; carousel cover-frame conventions documented. |
| ANNC-05 | KVR Audio product listing | Submission channel confirmed (KVR Developer Account or email to contactus@kvraudio.com); human editorial review; no fee; no published hard specs for screenshot size so plan to supply flexible set. |
| ANNC-06 (also scoped in per CONTEXT.md) | Review-pitch outreach (T-0) + Guest-pitch outreach (late May) + Berlin-local Superbooth DMs | Contact list is complete in `influencer-outreach-v1.0.md` (34 individuals + 15 communities). Berlin-local handles verified below. Template conventions documented. |

</phase_requirements>

## Project Constraints (from CLAUDE.md)

These are load-bearing directives from `CLAUDE.md` + the project memory that constrain all launch copy. Planner MUST enforce them in every copy-writing task.

1. **Capability-level language only, no DSP-implementation jargon.** Per `feedback_marketing_copy_depth.md`: FFT size, HRTF technical paper names, partition sizes, convolution block details are "pointlessly complex" in public-facing surfaces. Say "binaural rendering" or "each echo has a 3D position", not "4096-sample partitioned convolution with overlap-add".
2. **First-person Andrew voice** on Patreon / blog / LinkedIn / IG caption (established in Patreon seed posts 1–4).
3. **SML-plural framing.** OpenSpatialDelay is "the first tool in the Spatial Media Library pipeline". Do not position OSD as the whole company.
4. **Cross-link stack canonical:** GitHub, SML, andrewrahman.com, Patreon. Repeat in blog + newsletter + LinkedIn + Patreon cross-post (from Patreon Post 4).
5. **VST3 + AU only — no other formats mentioned anywhere.** (CLAUDE.md Conventions.)
6. **GPL-3.0 only. Never dual-license or commercial-license language.** (Project transitioned to GPL-only on 2026-04-14; see PROJECT.md and 01-REVIEW.md.)
7. **Versioning discipline:** v1.0.0 is the launch version. Don't invent v1.0.1 or v1.1 copy. `build_version.sh` and `agent_docs/version_registry.md` remain DSP-work concerns — not relevant to Phase 5 copy.
8. **Evidence-artifact pragmatism:** Per `feedback_evidence_artifact_ceremony.md`, don't demand screenshots/proof files that are ceremonial for solo-creator launch tasks. "Live URL resolves 200 OK + user confirmation" is sufficient acceptance evidence.

## Architectural Responsibility Map

Launch channels are not software tiers, but the same mapping principle applies — map each channel capability to its platform tier.

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Blog post publish + embed | SML WordPress (CMS) | Audio host (?) for demo clip | [VERIFIED: spatialmedialab.org footer] WordPress is the platform of record; media is either uploaded to WP Media Library or embedded via iframe from YouTube/SoundCloud/Bandcamp. |
| Newsletter send | SML Mailchimp | — | [VERIFIED: mailchimp.com disclosure on spatialmedialab.org homepage] Mailchimp is the sole send-path. |
| LinkedIn post | LinkedIn (Andrew's personal profile) | SML company page (optional cross-post) | [ASSUMED] First-person Andrew voice points to personal profile; SML page cross-post is amplification, not primary. |
| Instagram Reel + carousel | Instagram (Andrew's personal or OSD handle) | SML IG `@spatialmedialab` (from SML footer) | [VERIFIED: SML footer lists IG] SML IG exists and can cross-post. |
| KVR product listing | KVR Audio | Forum thread in Effects + VR/Immersive | [VERIFIED: kvraudio.com/submissions] Listing and forum thread are separate surfaces. |
| Review-pitch batch email | Andrew's personal inbox (andrew@spatialmedialab.org — per Phase 3 email migration) | — | [VERIFIED: ROADMAP.md 03-02-PLAN reference] Email migrated 2026-04-16. |
| Berlin-local DMs | Per-contact native platform (Bluesky / IG / LinkedIn / email) | — | [VERIFIED by per-contact research below] Each contact has a preferred platform; no single DM surface. |

## Standard Stack

### Core

| Library / Platform | Version | Purpose | Why Standard |
|--------------------|---------|---------|--------------|
| WordPress | [current, not verified] | SML blog CMS (ANNC-01) | [VERIFIED: "Proudly powered by WordPress" footer on spatialmedialab.org] Already in use; Andrew has authoring privileges proven by 2024 post. |
| Mailchimp | [current] | SML newsletter ESP (ANNC-02) | [VERIFIED: disclosure on spatialmedialab.org subscribe form] Already wired to SML homepage form. |
| LinkedIn native post | 2026 algorithm | LinkedIn launch post (ANNC-03) | [VERIFIED: dataslayer.ai 2026 algorithm report, multiple Feb-2026 sources] 2026 algorithm favours text-only and document carousels; penalises external links 60%. |
| Instagram Reels | 2026 format | IG demo asset (ANNC-04) | [VERIFIED: multiple 2026 IG-algorithm sources] 15–30s sweet-spot for discovery; 5-hashtag cap since 2025-12-18. |
| KVR Developer Account | current | KVR Product Database listing (ANNC-05) | [VERIFIED: kvraudio.com/submissions, kvraudio.com/developer_application.php] Free account required to add/manage listings. |

### Supporting

| Tool | Purpose | When to Use |
|------|---------|-------------|
| Google Drive / GitHub Releases | Binary hosting for download CTA | [VERIFIED: 2024 SML post links to Google Drive] Existing SML convention links to Google Drive; for OSD, GitHub Releases (`v1.0.0` already published per `project_version_e15b.md`) is the source of truth — link Drive or direct Release assets. |
| SoundCloud / YouTube / Bandcamp embed | Audio demo hosting for blog embed | Blog post embed (ANNC-01) needs a web-playable asset; the 30s CONT-01 binaural demo can be hosted on any of these. |
| Gearspace / r/spatialaudio / KVR Effects Forum / VI-CONTROL | Community-forum cross-posts | Same-day as launch; amplification (Segment 6 destinations from `influencer-outreach-v1.0.md`). |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| WordPress for blog | Export to MDX in `andrewrahman.com` repo | Rejected — SML brand + existing audience + D-08-adjacent rationale. SML is the right surface. |
| Mailchimp newsletter | Use Sender.net list from Phase 02 | Rejected by D-08 — OSD Sender list starts at 0 subs; SML has established audience. |
| LinkedIn "link in first comment" | Text-only LinkedIn post + bio-link CTA | [VERIFIED: dataslayer.ai + blog.linkboost.co 2026 reports] The "link in first comment" workaround is now penalized. Drop this tactic. |
| IG feed post only | IG Reel + carousel | Reel recommended per 2026 algorithm; carousel for screenshot variety. Can do both on same day. |
| KVR product listing only | Product listing + KVR news submission (via contactus@kvraudio.com) | Doing both is the norm for new products — the listing is permanent, the news post is the launch moment. |

**Installation / account setup:**

```
# KVR Developer Account (if not existing)
Apply at https://www.kvraudio.com/developer_application.php
(Free; tied to existing KVR member account)

# Mailchimp access (one of)
Confirm with SML board (D-08a) whether Andrew has audience-admin or needs a delegated send
```

**Version verification:** SML blog post cadence verified by `/news/` index (most recent post: 2025-10-12); Mailchimp form active (verified 2026-04-17); Hainbach handles verified from hainbachmusik.com impressum (2026-04-17).

## Architecture Patterns

### Launch-Day Sequencing Diagram

```
[T-7 to T-1 days: pre-launch staging]
  │
  ├─ D-08a: Confirm SML Mailchimp owner ──▶ Andrew or board contact
  ├─ D-07b: Berlin DMs sent (Hainbach, Kirn, Horstmann, AES)
  ├─ Phase 4 CONT-01/02/03 assets produced & hosted
  ├─ KVR Developer Account confirmed (apply if needed)
  └─ Blog draft + newsletter draft + LinkedIn post + IG caption + KVR metadata ready
  │
[T-0 Launch Morning, Tue Apr 28 2026, ~10:00 CET]
  │
  ├─ 1. Publish SML blog post ──────────▶ canonical URL now live
  ├─ 2. KVR product listing submitted ──▶ (may take hours to appear)
  ├─ 3. KVR news submission ────────────▶ (editorial queue)
  ├─ 4. Send SML Mailchimp newsletter ──▶ links to blog post + download
  ├─ 5. Publish LinkedIn post ──────────▶ links to andrewrahman.com or SML blog via bio
  ├─ 6. Publish IG Reel + carousel ─────▶ bio link updated
  ├─ 7. Post Gearspace VR/Immersive + KVR Effects + r/spatialaudio + VI-CONTROL
  └─ 8. Review-pitch batch emails sent (T-0 per D-05) ─▶ all 34 segment-1-5 contacts
  │
[T+1 to T+10: Superbooth window]
  │
  └─ Andrew face-to-face w/ Berlin contacts (follow-through on D-07 DMs)
  │
[T+~30: late May / early June]
  │
  └─ Guest-pitch wave to Segment 7 (D-06)
```

### Recommended Project Structure (artefact layout)

```
.planning/phases/05-launch-announcements/
├── 05-CONTEXT.md                  # (exists)
├── 05-DISCUSSION-LOG.md            # (exists)
├── 05-RESEARCH.md                  # (this file)
├── 05-00-PLAN.md                   # master plan — sequencing + preconditions
├── 05-01-PLAN.md                   # SML blog post (ANNC-01)
├── 05-02-PLAN.md                   # SML newsletter send (ANNC-02)
├── 05-03-PLAN.md                   # LinkedIn post (ANNC-03)
├── 05-04-PLAN.md                   # Instagram Reel + carousel (ANNC-04)
├── 05-05-PLAN.md                   # KVR listing + news submission (ANNC-05)
├── 05-06-PLAN.md                   # Review-pitch batch (ANNC-06 mode A)
├── 05-07-PLAN.md                   # Berlin-local DMs (D-07)
├── 05-08-PLAN.md                   # Guest-pitch wave (ANNC-06 mode B, scheduled late May)
└── drafts/
    ├── sml-blog-post.md            # mirrors 2024 Ambisonic-to-Atmos-template structure
    ├── sml-newsletter.md           # Mailchimp-ready HTML or plain-text
    ├── linkedin-post.md            # text-only, no-link-in-comment format
    ├── ig-reel-caption.md          # first-line + caption body
    ├── ig-carousel-caption.md      # alternate if carousel is separate post
    ├── kvr-listing-metadata.md     # name, description, tags, screenshots
    ├── kvr-news-submission.md      # press-release style
    ├── review-pitch-template.md    # plain-text-looking cold email
    ├── guest-pitch-template.md     # light-branded-HTML cold email
    ├── berlin-dm-hainbach.md       # Bluesky DM draft
    ├── berlin-dm-kirn.md           # Bluesky / Mastodon DM draft
    ├── berlin-dm-horstmann.md      # LinkedIn message draft
    └── berlin-dm-aes.md            # email to Ulli Scuda + optional Berlin student section fallback
```

### Pattern 1: SML Blog Post — Mirror the Ambisonic-to-Atmos Template Post

**What:** SML's existing 2024 "New Template just dropped!" post by Andrew is the best structural precedent. Replicate its shape.

**When to use:** ANNC-01 artefact for launch day.

**Template structure (derived from `/ambisonic-to-atmos-template/`):**

```markdown
# New plugin just dropped!   ← H1 (mirrors prior Andrew-authored post tone)

## OpenSpatialDelay v1.0.0 — Free spatial delay for VST3 and AU ← H2

[Intro paragraph — capability sentence: "Each echo has a 3D position in space.
Binaural HRTF, object-based positioning, free and GPL-3.0." — 1–2 sentences]

[Image 1: screenshot_full.png — plugin UI]

[2–3 paragraphs — the "why I built this" first-person story; Berlin-solo-dev
framing; SML pipeline framing — "first tool in the Spatial Media Library".
No DSP jargon per CLAUDE.md.]

[Image 2: a secondary screenshot — e.g., parameter detail or a factory preset
view from Phase 3 screenshot inventory]

[Embedded audio demo — 30s CONT-01 binaural clip. SoundCloud/Bandcamp
<iframe> or <audio> tag. MUST work headphones-only note.]

[Paragraph — "What's inside" at capability level: 70 factory presets,
VST3+AU, macOS+Windows, 6 HRTF profiles. GPL-3.0. Free download.]

[Image 3: third screenshot or a link-card-style download block]

## Download

**[Download OpenSpatialDelay v1.0.0 here!]** ← direct link to
github.com/.../releases/tag/v1.0.0 (or Google Drive mirror, matching the 2024 post convention)

## Links

- Project home & source: [github.com/...] 
- Support development on Patreon: [patreon.com/spatialmedialab]
- Andrew's site: [andrewrahman.com]
- Spatial Media Lab: [spatialmedialab.org]

[Tags: Spatial Audio, Binaural, Dolby Atmos, VST3, AU, Free, GPL, OpenSpatialDelay]
[Categories: Announcements, Software]

---
Published [DATE] By Andrew
```

**Example:**
```
Source: [VERIFIED: spatialmedialab.org/ambisonic-to-atmos-template/ — Andrew-authored, May 23, 2024]

H1: "New Template just dropped!"
H2: "Ambisonic to Dolby Atmos conversion template"
Body: ~900-1000 words, 3 images, Google Drive download CTA, external links
to IEM SimpleDecoder, Fiedler Atmos Composer, Flux Audio Elixir, FabFilter,
ToneBoosters, Google Docs, artist sites. Tags: Ambisonics, Dolby Atmos,
Reaper, Fiedler Audio, Carolina Eyck. Categories: Announcements, Software.
Byline: "Published May 23, 2024 By Andrew". Comments enabled.
```

### Pattern 2: LinkedIn Text-Only Launch Post (NEW 2026-era convention)

**What:** Reddit-style storytelling post, no external link in body or first comment.

**When to use:** ANNC-03.

**Template:**
```
[Hook line — first 210 chars visible before "see more" on desktop, 140 on mobile]

I spent three years building a spatial delay plugin. Today it ships — free.

[Story body — 1,000–1,800 chars total; Berlin-indie-dev voice; mentions:
- What OSD does in one capability sentence
- Why free + GPL-3.0
- That it's the first tool in the SML pipeline
- No technical jargon

Ends with: "Link in my profile / bio" or "DM me if you want the link" —
NOT "link in first comment"]

#SpatialAudio #DolbyAtmos #AudioPlugin #IndieDev #OpenSource
```

**Format alternative (higher engagement per 2026 data):** Native LinkedIn document carousel (PDF) — document posts get ~3× engagement per dataslayer.ai Feb 2026 report. Export a 5–7 page PDF from Figma/Canva with screenshots + capability one-liners, attach as document. Planner decides between text-only and document-carousel.

### Pattern 3: Instagram Reel + Carousel Combo

**What:** Reel as discovery surface (15–30s), carousel as depth surface (3–5 static frames).

**When to use:** ANNC-04.

**Reel spec:**
- Length: 15–30 seconds (discovery sweet-spot per 2026 Instagram algorithm reports)
- Aspect: 9:16 vertical
- Audio: headphones-required binaural demo (CONT-01/02 asset from Phase 4)
- Cover frame: a single UI screenshot with hook text overlay
- Caption first-line (the "real" caption): one capability sentence — "Each echo has a 3D position. Free, today, VST3+AU."
- Full caption: 100–220 chars, single CTA: "Link in bio"
- Hashtags: max 5 — `#SpatialAudio #DolbyAtmos #AudioPlugin #BinauralAudio #FreePlugin` (the 30-tag era is over as of 2025-12-18)

**Carousel spec (optional same-day second post):**
- 3–5 screenshots from Phase 3 asset inventory (`screenshot_full.png`, `screenshot_presets.png`, etc.)
- First frame = cover with hook text
- Caption: 100–220 chars same discipline
- Same hashtag set

### Pattern 4: Review-Pitch Cold Email — Plain-Text-Looking Template

**What:** Per D-05, plain-text-looking (no HTML chrome) cold email with a download link, not an attached binary.

**When to use:** ANNC-06 mode A, batch-sent at T-0.

**Template:**
```
Subject: Free spatial delay launched today — thought you might like it

Hi {first_name},

I just released OpenSpatialDelay v1.0.0 — a free, GPL-3.0 spatial delay
plugin where each echo has a 3D position in space. Binaural HRTF + object-
based positioning, VST3 + AU, mac + Windows.

I built it as a solo dev in Berlin; it's the first tool in the Spatial Media
Library pipeline. I saw you covered {Sound Particles inDelay / dearVR /
Space Controller / specific work}, so I thought it might fit your beat.

Demo + download: {andrewrahman.com/get-osd}
Source: {github.com/...}

Happy to answer any questions. No rush — I'll be quiet during May while
travelling through {...}, then fully back on email in June.

— Andrew
Spatial Media Lab • Berlin
```

Character count: ~700–900. Subject ≤ 60 chars. Personalisation slot (`{specific work}`) is mandatory — generic blasts hit spam filters in 2026.

**GDPR note:** Andrew is Berlin-based and will be emailing EU contacts. GDPR requires (a) legitimate-interest basis (public editorial figures covering relevant tools = satisfies this), (b) clear sender identity, (c) opt-out. The signature line "— Andrew, Spatial Media Lab, Berlin" + the soft opt-out "no rush, I'll be quiet during May" line serves both. Do NOT add a `List-Unsubscribe` header or "unsubscribe" footer to a 1:1 cold email — that implies a marketing list and actually weakens the GDPR legitimate-interest defence. Reference: `mailshake.com/blog/gdpr-compliant-cold-email/`.

### Pattern 5: Guest-Pitch Email — Light-Branded-HTML Template (late May)

**What:** Per D-05 discretion-point, light branded HTML with an HTML signature + one inline screenshot. Higher-ceremony than review-pitch.

**When to use:** ANNC-06 mode B, sent late May / early June post-travel.

Defer template draft to 05-08-PLAN.

### Pattern 6: Berlin-Local DM Templates (per platform)

| Contact | Preferred platform | Message shape |
|---------|---------------------|---------------|
| Hainbach (Stefan Paul Goetsch) | Bluesky `@hainbach.bsky.social` OR Instagram `@hainbach101` | Bluesky DM is quieter — preferred for a relationship-first ask. ≤ 400 chars. "Hi Hainbach — I'm Berlin-based too, launching a free spatial delay (GPL, VST3+AU) before Superbooth. Coffee at the Messe? Would love to show you in person." |
| Peter Kirn | Bluesky `@pkirn.bsky.social` OR Mastodon `@pkirn@mastodon.social` | Bluesky is Peter's explicit preference (per his Nov-2024 CDM post); Mastodon is the fallback. Short DM, not a pitch — "Hi Peter — Berlin solo dev, free GPL spatial delay shipping before Superbooth. If you're around the Messe, happy to show you the plugin in person. Not a pitch — just a Berlin-local heads-up." |
| Eric Horstmann | LinkedIn DM (https://www.linkedin.com/in/eric-horstmann-036a1927/) — 500+ connections, actively used | Alternative: email contact@immersive-lab.com. LinkedIn is the faster read. "Hi Eric — fellow Berlin spatial-audio person. Launching a free GPL spatial delay plugin Apr 28, before Superbooth. Happy to pop by Immersive Lab for a coffee + demo if you're curious." |
| AES Germany (re-scoped from "AES Berlin chair") | Email: Ulli Scuda (Germany chair), with cc Cornelius Wilkening (secretary) OR Anna Leschanowsky (vice-chair) | [VERIFIED: aes.org/community/section] No dedicated Berlin professional-section chair exists — Germany section covers it; Berlin has a *student* section (faculty: Thomas Sporer, Thorsten Weigelt). Planner decision: send to Ulli Scuda (professional audience) and optionally Sporer/Weigelt (student-section, Berlin-local). Email: ralph.kessler@masterpinguin.de is listed as AES Germany treasurer contact — use as the general AES Germany gateway if Ulli Scuda's direct email isn't discoverable. Short message: "Hello — I'm releasing a free GPL spatial delay plugin (VST3+AU) on Apr 28. I'll be at Superbooth; would the AES Germany community be interested in a brief demo or future meetup presentation?" |

### Anti-Patterns to Avoid

- **"Link in first comment" on LinkedIn** — dead in 2026. Use bio-link or go link-free. Multiple Feb-2026 algorithm reports (dataslayer.ai, linkboost.co) confirm LinkedIn now buries author's first comment specifically to kill this workaround.
- **External links in LinkedIn post body** — ~60% reach reduction. Drive traffic via profile bio or DM.
- **30-tag Instagram posts** — cap is 5 as of Dec 18, 2025. Anything above 5 is ignored or treated as spam signal.
- **Long launch-day blog post with DSP paper citations** — violates CLAUDE.md voice rule. Keep to capability language; skip partition sizes and Gardner-Martin paper refs.
- **"Buy" or "purchase" language anywhere.** Free / GPL-3.0 / download / get — never buy.
- **Mentioning AAX, VST2, or any format besides VST3 and AU.** CLAUDE.md rule.
- **Launching without confirming Phase 4 assets exist.** The blog embed, Reel, and KVR screenshots ALL depend on CONT-01/02/03. See Environment Availability section.
- **Launching without confirming SML Mailchimp audience access.** D-08a must close before Apr 28.
- **Attaching a zipped binary to cold emails.** Reviewers want hosted download links (spam filters + attachment size). Source: cold-email 2026 best-practice reports + industry convention.
- **Sending a press release to journalists with a story-less subject line.** Per 2026 subject-line research: video-mentioning subjects get 2–3× open rates; personalized subjects get 22% lift. "Press Release: OpenSpatialDelay v1.0.0" is worse than "Free spatial delay launched today — thought you might like it".

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Blog CMS for SML post | Next.js custom blog in `andrewrahman.com` repo | Use existing WordPress on spatialmedialab.org | Infrastructure exists, audience exists, Andrew already has publish authority. |
| Newsletter sending | Custom transactional email via Sender.net / Resend | Use existing SML Mailchimp | D-08 locked; Mailchimp has the existing audience. |
| Press release distribution | PRWeb / PR Newswire subscription ($200+) | Targeted email list from `influencer-outreach-v1.0.md` | Explicitly deferred per CONTEXT.md; doesn't fit indie-dev positioning. |
| Cold-email automation | Mailshake / Lemlist / Saleshandy SaaS | Batch-sent from Andrew's personal Gmail via BCC or individual sends | 34 contacts is below any automation threshold; personal-from addresses dodge 2026 spam filters. |
| Launch-day social scheduling | Buffer / Hootsuite / Later | Manual posting at T-0 (Andrew is online that morning) | Scale of 5 channels is below automation benefit; manual posting allows last-minute tone adjustment. |
| KVR listing content-writer tool | AI copy SaaS | Plain markdown draft mirroring existing KVR listings (e.g., ValhallaSupermassive's structure) | KVR editorial reviews humans' text better than generated text; tone matters for approval. |

**Key insight:** Every channel in Phase 5 has a zero-custom-build path. The effort goes into *copy quality* and *sequencing discipline*, not tooling.

## Runtime State Inventory

This is a content/coordination phase, not a rename/refactor phase. Runtime State Inventory is not applicable in the strict sense. However, the analog is the **external-service account inventory** that must be resolved before launch day:

| Category | Items Found | Action Required |
|----------|-------------|------------------|
| External accounts (authoring) | SML WordPress — Andrew has author/editor role (proven by 2024 `/ambisonic-to-atmos-template/` byline). | None — verify login still valid. |
| External accounts (sending) | SML Mailchimp — owner uncertain. Form is active (verified 2026-04-17). | **D-08a blocker:** Andrew must confirm audience access or delegate send to owner. |
| External accounts (KVR) | KVR Developer Account — status unknown. | **Discovery task:** check kvraudio.com/devs logged in; apply at kvraudio.com/developer_application.php if no account. Likely needed — not required to assume existing. |
| External accounts (social) | LinkedIn (Andrew's personal), Instagram (Andrew's personal or OSD dedicated — TBD), Bluesky (Andrew's? TBD), Mastodon (TBD). | Plan 05-03 / 05-04 must verify each handle exists and has access, before launch week. |
| Link targets | andrewrahman.com/get-osd (live per Phase 3), spatialmedialab.org (live), github.com/.../releases/tag/v1.0.0 (live per `project_version_e15b.md` memory — Apr 12 squash), patreon.com/spatialmedialab (live per Phase 2). | Verify all five URLs resolve 200 OK on Apr 27 (T-1 precheck). |

**Nothing else to inventory.** This is not a rename phase.

## Common Pitfalls

### Pitfall 1: Launching without Phase 4 assets in hand

**What goes wrong:** Blog embed, Reel, IG carousel, and KVR screenshots all depend on Phase 4 (CONT-01/02/03). If Phase 4 slips, Phase 5 has no blocking assets to publish.

**Why it happens:** Phase 4 has no plans, no research, no CONTEXT as of 2026-04-17. Phase 5 is planned first (because launch-day sequencing is the hardest thinking). Naïve sequencing assumes Phase 4 will land in time.

**How to avoid:** Plan 05-00 must list Phase 4 completion as a hard precondition in its opening paragraph. Don't start copy drafting that references the demo clip until CONT-01 is at least in a reviewable state.

**Warning signs:** Apr 20 arrives and Phase 4 still has no 04-00-PLAN.md. At that point, triage — either pull the launch date to May 5 (within Superbooth overlap) or descope ANNC-04 (IG Reel) to a static-image post.

### Pitfall 2: Assuming LinkedIn "link in first comment" works

**What goes wrong:** Launch day Andrew posts the LinkedIn narrative, drops the OSD URL in the first comment, and LinkedIn buries the comment — audience never sees the CTA.

**Why it happens:** "Link in first comment" was the indie-launch gospel 2021–2024. Multiple 2026 algorithm reports confirm the workaround is now explicitly penalized; the platform hides author's first comment specifically.

**How to avoid:** Use text-only posts with "Link in bio" CTA, OR post a native LinkedIn document (PDF carousel) — documents get 3× engagement per Feb 2026 data. Update the LinkedIn profile bio URL to point at `andrewrahman.com/get-osd` one day before launch.

**Warning signs:** Post goes out, reach caps at ~200 impressions, comment count is zero. At that point, LinkedIn has suppressed the link-in-comment. Recover by editing the post to remove CTA-in-comment and append "Link in bio" to the body.

### Pitfall 3: Mailchimp send delay / audience-owner blocker

**What goes wrong:** Launch morning, Andrew opens Mailchimp, discovers his role on the SML audience is "limited" or he's not on the audience at all, scrambles to get Timo Bittner or Basel Naouri to send on 2-hour notice, misses the D-01 2-hour window.

**Why it happens:** D-08a is an open question. SML board-member roles in the Mailchimp account weren't verified in this research round — outside the scope of web research.

**How to avoid:** Plan 05-02 Task 1 = "Log into SML Mailchimp. Verify Andrew has Manager or Admin role on the audience. If not, coordinate with audience owner to either grant access or schedule the send for Apr 28 10:00 CET." This is a 15-minute task but a 2-week calendar risk if the owner is Timo/Basel and they're travelling.

**Warning signs:** By Apr 22 (T-6), this task has no "✓ access confirmed" status. Escalate.

### Pitfall 4: KVR editorial-queue backlog

**What goes wrong:** KVR news submission sits in the editorial queue for 2–5 days. Launch-day news hit doesn't appear on KVR until Wed/Thu — by which point LinkedIn + IG traction has already faded.

**Why it happens:** KVR posts are human-reviewed (per kvraudio.com/submissions). No published SLA.

**How to avoid:** Submit the KVR *product listing* (database entry) T-1 day (Apr 27) so it's live on Apr 28. Submit the KVR *news item* Apr 28 morning — even if it posts on Apr 29/30, the product listing has already caught early KVR traffic. Treat KVR as a **discovery surface that compounds over weeks**, not a launch-day spike.

**Warning signs:** Apr 28 evening, KVR listing still not visible. Check kvraudio.com/developer dashboard for rejection reason (typically: missing required field).

### Pitfall 5: GDPR-flagged cold email batch (review-pitch)

**What goes wrong:** 34 cold emails from `andrew@spatialmedialab.org` to EU reviewers, sent within a 30-minute window, flagged by Gmail/Outlook as a "newsletter spray" → deliverability tanked.

**Why it happens:** New sending domain + 34 sends in short succession + generic greetings = spam-filter profile.

**How to avoid:** (a) Space sends over 2 hours (1 every 3–4 minutes), (b) personalize the `{specific work}` slot in every email, (c) ensure SPF+DKIM+DMARC are set on spatialmedialab.org (if not, switch to `andrewjrahman@gmail.com` which has reputation), (d) keep reply-to = sending address, (e) no attachments, (f) no `unsubscribe` footer on what is a 1:1 cold email.

**Warning signs:** Bounce rate > 5% on the batch, or a 0% reply rate within 24h (should be 3–11% per 2026 benchmarks).

### Pitfall 6: Berlin DMs timed wrong (too early or too late)

**What goes wrong:** DM to Hainbach goes out Apr 17 — too early; Hainbach hasn't firmed Superbooth schedule. Or Apr 27 — too late; his days are fully booked. Either way, no coffee meeting.

**Why it happens:** Berlin-local practitioners firm Superbooth schedules 2–3 weeks out.

**How to avoid:** Send Berlin DMs in the Apr 21–Apr 25 window — 2 weeks before Superbooth but after most practitioners have started blocking their Messe days. That's also still pre-launch, preserving the "see you there" framing.

**Warning signs:** Zero replies by Apr 27. Follow up once with a "still happy to grab coffee if you have a window" nudge during Superbooth itself.

## Code Examples

This phase has no code. Blog post, newsletter, and social copy drafts are the artefacts. Sample structures are documented in Pattern 1–6 above.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| "Link in first comment" on LinkedIn | Text-only post + bio-link OR native document carousel | Algorithm change early 2026 | Don't do the comment-link workaround. |
| 30 hashtags on Instagram | Maximum 5 hashtags | 2025-12-18 (hard cap rolled out) | Keep tag lists ≤ 5. |
| Press-release distribution services (PRWeb etc.) | Direct outreach to ~30 targeted reviewers | Ongoing indie-dev norm | Skip PR services; they don't fit free/GPL positioning. |
| Embargo-gated pre-launch reviewer outreach | T-0 cold-send with hosted download links | Superseded by D-05 decision for this launch | Simpler; lower reply rate accepted. |
| Attached plugin binaries in outreach emails | Hosted download link (GitHub Release) | Long-standing; 2026 spam filters strict on attachments | Links, never attachments. |

**Deprecated / outdated in older indie-launch playbooks:**

- "Auto-DM new followers" on Twitter/X — legacy tactic, zero-value in 2026.
- "Post on r/VSTs as the primary launch destination" — subreddit is now saturated with free-plugin promos; Gearspace VR/Immersive and r/spatialaudio are better targeted per Segment 6 analysis.
- "Send Press Release to MusicRadar / Attack" — these publications now pull stories from KVR news feed and social buzz, not from direct PR emails; listing on KVR is the better upstream investment.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Andrew has Author/Editor role on SML WordPress sufficient to publish directly (inferred from 2024 authored post byline). | Standard Stack / Pattern 1 | LOW — if role was downgraded, Andrew submits-for-review and Timo/Basel approve; adds 0–24h to the pipeline but doesn't block launch. |
| A2 | SML Mailchimp audience is owned/accessible by Andrew or a co-founder who can send on Apr 28. | Architecture / Pitfall 3 | MEDIUM — if owner is unreachable during the Apr 28 window, newsletter slips (D-08a must resolve this). |
| A3 | GitHub Release v1.0.0 download URL is the single source of truth for binary distribution on launch day. | Patterns 1, 4 | LOW — already published per `project_version_e15b.md` (squashed to 768c248 on 2026-04-12). |
| A4 | andrewrahman.com/get-osd resolves on launch day (Phase 3 deploy). | Patterns 1, 4, 5 | LOW — per ROADMAP 03-09 plan, site launch is queued; required precondition regardless. |
| A5 | KVR Developer Account submission is still the correct product-listing path in 2026 (not deprecated for a newer portal). | Stack, ANNC-05 | LOW — verified 2026-04-17 via kvraudio.com/submissions and kvraudio.com/developer_application.php; both active. |
| A6 | LinkedIn "link in first comment" penalty is symmetric regardless of post format (text vs document). | LinkedIn pitfall | MEDIUM — sources (dataslayer.ai, linkboost.co) report the penalty against text posts; document carousels may behave differently. Fallback: test with a scout post T-3 days and observe reach. |
| A7 | 3.43% reply rate for cold outreach holds for audio-plugin-reviewer niche. | Review-pitch template | LOW — reply rate is a benchmark, not a promise; actual rate will depend on personalisation quality. |
| A8 | AES Germany Berlin-professional section does not exist (only student section). | Berlin DMs / D-07 | LOW — verified 2026-04-17 via aes.org/community/section; Ulli Scuda is Germany-wide chair. |
| A9 | Hainbach actively reads Bluesky DMs. | Pattern 6 | LOW — handle confirmed via hainbachmusik.com/impressum (2026-04-17), but engagement frequency unknown; IG `@hainbach101` is a fallback. |
| A10 | Mailchimp "Send Time Optimization" or "Timewarp" is not needed — Tuesday 10:00 CET fixed send suffices for launch-burst timing. | Launch sequencing | LOW — optimisation features save a few percentage points of open rate; the D-01 2-hour-window discipline dominates. |

**If any A-item resolves against assumption:** Update this research in `05-RESEARCH.md` and the planner should revise the corresponding plan before execution.

## Open Questions

1. **Who owns the SML Mailchimp audience?**
   - What we know: Mailchimp confirmed as the ESP; form active on spatialmedialab.org; SML board = Andrew + Timo Bittner (president) + Basel Naouri.
   - What's unclear: Role assignments inside Mailchimp.
   - Recommendation: Andrew logs in, checks his role, escalates to Timo/Basel if limited. This is Plan 05-02 Task 1.

2. **Does Andrew already have a KVR Developer Account?**
   - What we know: Free to apply; plugin submissions accepted; no fee.
   - What's unclear: Whether Andrew has one from prior work.
   - Recommendation: Check first; apply if missing. This is Plan 05-05 Task 1.

3. **What is the actual SML Mailchimp subscriber count?**
   - What we know: SML has existed since 2017; subscribe form is prominent on homepage; post cadence suggests an active community.
   - What's unclear: Raw subscriber count.
   - Recommendation: Andrew pulls the number during Plan 05-02 Task 1. This does NOT block the send — it just informs expected reach. If the number is < 50, reconsider whether cross-posting to OSD Sender list (even at whatever sub count it has by then) adds value.

4. **Andrew's personal vs OSD-branded Instagram handle — which one?**
   - What we know: Phase 3 research doesn't specify; Andrew may have a personal IG that has followed him from prior work.
   - What's unclear: Whether an `@openspatialdelay` or similar handle should be created pre-launch.
   - Recommendation: Use Andrew's personal + cross-post to `@spatialmedialab` (which exists per SML footer). Defer dedicated OSD handle to v2. Reduces moving parts.

5. **Apr 27 T-1 pre-check: do all four link targets resolve 200?**
   - What we know: SML is live, github release is published, andrewrahman.com deploy is Phase 3 concern, Patreon is live.
   - What's unclear: Nothing in principle — but operational pre-check catches CDN cache issues, URL typos, 301 loops.
   - Recommendation: Plan 05-00 final task = run `curl -I` against all 5 candidate URLs T-1 morning.

6. **KVR taxonomy: what is the exact category list in 2026?**
   - What we know: Broad guidance is `Delay` primary, `Spatial` secondary.
   - What's unclear: Whether "Spatial" is still a canonical tag or has been replaced by "Immersive" or "3D Audio" in a 2025+ taxonomy rev.
   - Recommendation: Plan 05-05 Task 2 = browse current KVR top-level plugin-type and tag pages to lock category choice. 15-minute task.

## Environment Availability

Phase 5 depends on external services, not local tooling. Probe availability below.

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| SML WordPress publish access | ANNC-01 | [ASSUMED yes — 2024 post byline verifies] | [n/a] | Submit-for-review; Timo Bittner / Basel Naouri approves. |
| SML Mailchimp audience access | ANNC-02 | [UNKNOWN — D-08a] | [n/a] | Delegate send to audience owner; 24–48h coordination overhead. |
| andrewrahman.com deploy (live) | Blog/newsletter CTA target | [VERIFIED as in progress per Phase 3] | [n/a] | — |
| GitHub Release v1.0.0 download | All download CTAs | [VERIFIED live per project_version_e15b.md] | v1.0.0 | Google Drive mirror (matches 2024 SML post convention). |
| Patreon page (live) | Cross-link | [VERIFIED live per Phase 2] | — | — |
| KVR Developer Account | ANNC-05 | [UNKNOWN] | — | Apply at kvraudio.com/developer_application.php (free, < 1 day turnaround typically). Alternative: email press release to contactus@kvraudio.com. |
| LinkedIn account active | ANNC-03 | [ASSUMED yes — Andrew is actively publishing] | — | — |
| Instagram account access | ANNC-04 | [UNKNOWN] | — | Request from Andrew; create dedicated OSD handle if preferred (not recommended — adds a cold-start problem). |
| Bluesky account | Berlin DM to Kirn + Hainbach | [UNKNOWN — assumption only] | — | Email fallback for Hainbach (no direct email on his site; use contact form at hainbachmusik.com); Mastodon fallback for Kirn. |
| Mastodon account | Berlin DM backup for Kirn | [UNKNOWN] | — | Bluesky direct; or email editor@createdigitalmusic.com. |
| Email on andrew@spatialmedialab.org | Review-pitch batch (ANNC-06) | [VERIFIED configured per Phase 3 03-02-PLAN commit 2026-04-16] | — | Fallback: andrewjrahman@gmail.com (higher spam reputation but dilutes SML brand). |
| SPF / DKIM / DMARC on spatialmedialab.org | Cold-email deliverability | [UNKNOWN — not verified in Phase 3 research] | — | **MEDIUM risk** — if unconfigured, batch send of 34 emails may land in spam. Fallback: send from andrewjrahman@gmail.com. |

**Missing dependencies with no fallback:** None are hard blockers — every gap has a workaround.

**Missing dependencies with fallback:**
- SML Mailchimp access → delegate to owner.
- KVR Developer Account → email submission route.
- Bluesky/Mastodon accounts → email fallbacks for Kirn and Hainbach.
- Unverified SPF/DKIM/DMARC → switch to Gmail-from.

**Planner action:** Plan 05-00 should include a "T-7 pre-check checklist" that closes every [UNKNOWN] row by Apr 21.

## Validation Architecture

> Nyquist validation is not configured in `.planning/config.json`; defaulting to "enabled" per research agent protocol. Phase 5 is a content/coordination phase — validation is URL-resolution and human-confirmation, not unit tests.

### Test Framework

| Property | Value |
|----------|-------|
| Framework | Manual + Playwright (existing in `andrewrahman-com` repo per STATE.md Phase 3 notes) + curl smoke checks |
| Config file | `andrewrahman-com/playwright.config.ts` (external site checks) + no new config for Phase 5 |
| Quick run command | `curl -I {url}` per target link |
| Full suite command | Manual inspection + existing Playwright `@external` suite (4 tests on sml-about.spec.ts) runs as part of Phase 3 |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| ANNC-01 | SML blog post is live and embeds or links to the audio demo | smoke | `curl -I https://spatialmedialab.org/{slug}/` | ❌ Wave 0 — URL exists only post-publish |
| ANNC-01b | Audio embed plays in browser | manual-only | open URL in Chrome + Safari, confirm audio element | — |
| ANNC-02 | SML newsletter email sent | manual-only | Mailchimp campaign "Sent" status + verify receipt at Andrew's email | — |
| ANNC-03 | LinkedIn post published | smoke | open LinkedIn URL, screenshot post | — |
| ANNC-04 | IG Reel published | smoke | open IG URL, confirm Reel playable | — |
| ANNC-05 | KVR listing live | smoke | `curl -I https://kvraudio.com/product/openspatialdelay-...` (URL TBD post-submission) | — |
| ANNC-06 | Review-pitch batch sent | manual-only | Gmail sent folder: 34 sends with individual timestamps | — |
| ANNC-06b | Guest-pitch batch sent (late May) | manual-only | Gmail sent folder | — |
| D-07 | Berlin-local DMs sent | manual-only | Screenshot of each DM send | — |
| — | All launch-day link targets resolve 200 (T-1 pre-check) | smoke | `for u in $URLS; do curl -I $u; done` | ❌ Wave 0 — shell one-liner, no file needed |

### Sampling Rate

- **Per task commit:** `curl -I` against any URL touched.
- **Per wave merge:** No applicable — this phase has no waves in the DSP sense.
- **Phase gate:** All 10 rows green + Andrew's manual "launch complete, Superbooth ready" confirmation before `/gsd-verify-work`.

### Wave 0 Gaps

- [ ] `scripts/phase5-smoke.sh` — a tiny shell script that runs `curl -I` against: spatialmedialab.org/{new-post-slug}, andrewrahman.com/get-osd, github.com/.../releases/tag/v1.0.0, patreon.com/spatialmedialab, and the KVR listing URL. (10 lines, no framework.)

*(Otherwise: no test-infrastructure gaps — this phase's validation is fundamentally "did the URL resolve and did the human confirm".)*

Per `feedback_evidence_artifact_ceremony.md`: don't demand screenshot/proof files per channel beyond "URL resolves + Andrew confirms". Launch-day ceremony = send + confirm, not artifact production.

## Sources

### Primary (HIGH confidence)

- **spatialmedialab.org homepage** (WebFetch 2026-04-17) — confirmed WordPress + Mailchimp + /news/ blog path + board members.
- **spatialmedialab.org/news/** — confirmed post cadence + permalink structure + Andrew's authoring role.
- **spatialmedialab.org/ambisonic-to-atmos-template/** — confirmed precedent post structure (H1/H2, 3 images, download CTA, tags, categories, byline "By Andrew").
- **spatialmedialab.org/about/** — confirmed board members (Andrew, Basel Naouri, Timo Bittner).
- **hainbachmusik.com/impressum** — confirmed Hainbach handles: Bluesky `@hainbach.bsky.social`, IG `@hainbach101`, YouTube, Facebook, Reddit, Spotify.
- **aes.org/community/section/** — confirmed AES Germany chair (Ulli Scuda); no dedicated Berlin professional chapter chair; Berlin *student* section faculty = Sporer, Weigelt.
- **kvraudio.com/submissions** (WebSearch verification) — confirmed free Developer Account path + editorial review + email alternative contactus@kvraudio.com.
- **linkedin.com/in/eric-horstmann-036a1927/** — confirmed Eric Horstmann LinkedIn (500+ connections, Berlin).
- **immersive-lab.com** — confirmed Horstmann's company contact: contact@immersive-lab.com, Alt-Buch 45-51, 13125 Berlin.
- **cdm.link + bsky.app/profile/did:plc:yuj6elbct6dvcgcjnvynbiev + mastodon.social/@pkirn** — confirmed Peter Kirn's Bluesky + Mastodon handles.
- **dataslayer.ai/blog/linkedin-algorithm-february-2026-whats-working-now** — LinkedIn 2026 algorithm: document posts 3× engagement, external links -60% reach.
- **blog.linkboost.co/linkedin-algorithm-changes-2026/** — confirmed "link in first comment" is now specifically penalised.
- **evergreenfeed.com/blog/instagram-reels-algorithm/** + **trustypost.ai/blog/instagram-reel-caption-length-2026-best-practices-examples-that-get-watched/** — IG 2026: 15–30s Reel sweet-spot, 5-hashtag cap (Dec 2025), first-line-before-"more" is the real caption.

### Secondary (MEDIUM confidence)

- **mailshake.com/blog/gdpr-compliant-cold-email/** — GDPR cold-outreach best-practice guidance.
- **weezly.com/blog/cold-email-subject-lines/** + **prospeo.io/s/subject-line-for-a-cold-email** — 2026 cold-email subject conventions (benchmark 3.43% reply rate, 22% lift from personalization).
- **publishpress.com/knowledge-base/contributor/** + **wordpress.org/documentation/article/roles-and-capabilities/** — WordPress author/contributor role capabilities.
- **mailchimp.com/help/create-and-send-regular-email/** — Mailchimp send workflow.
- **audiomediainternational.com/aes-announces-berlin-convention-committee/** — historical AES Berlin convention committee context.

### Tertiary (LOW confidence — marked for validation)

- **KVR screenshot/image size specifications** — not published; real form is auth-gated. Plan accordingly with a flexible asset set (supply multiple resolutions).
- **SML Mailchimp subscriber count** — not publicly visible. Must be confirmed inside the Mailchimp dashboard at Plan 05-02 Task 1.
- **LinkedIn document carousel vs text-only post performance delta** specifically for audio-plugin-indie-dev launches — general 3× engagement figure holds but domain-specific data is unavailable.
- **Whether Andrew has an existing KVR Developer Account** — not discoverable from public sources.

## Metadata

**Confidence breakdown:**

- SML infrastructure (WordPress + Mailchimp + authoring role): **HIGH** — verified directly from spatialmedialab.org + existing 2024 post.
- LinkedIn 2026 algorithm rules: **HIGH** — multiple 2026-dated sources agree.
- IG 2026 format norms: **HIGH** — multiple 2026-dated sources agree on the 5-hashtag cap and 15–30s Reel length.
- Berlin contact handles: **HIGH** — verified at each contact's canonical source.
- KVR submission mechanics: **MEDIUM** — public guidance is shallow; Developer-Account form is auth-gated; assume standard indie-plugin flow.
- Review-pitch email conventions: **MEDIUM** — 2026 cold-email best-practice exists; audio-plugin-specific data is implicit from industry convention rather than from a cited case study.
- Launch-day sequencing optimal ordering: **MEDIUM** — derived by first-principles reasoning + scattered indie-dev blog guidance; no canonical source.
- GDPR for cold-email batch: **MEDIUM** — mailshake guidance is the anchor; legitimate-interest basis is defensible but not free of risk.

**Research date:** 2026-04-17
**Valid until:** 2026-05-08 (post-Superbooth; launch window closes). LinkedIn algorithm guidance in particular drifts fast — re-verify if launch slips past May.
