# Architecture Research

**Domain:** Indie audio plugin launch infrastructure (static site + survey gate + Patreon + GitHub public repo + social)
**Researched:** 2026-04-14
**Confidence:** HIGH (component relationships are well-understood; platform-specific mechanics confirmed)

---

## Standard Architecture

### System Overview

```
                          DISCOVERY LAYER
    ┌─────────────────────────────────────────────────────────┐
    │  LinkedIn / Instagram    SML Blog    GitHub (public)    │
    │  (social posts)          (blog post) (GPL README/code)  │
    └─────────┬──────────────────┬──────────────────┬─────────┘
              │                  │                  │
              ▼                  ▼                  ▼
                          LANDING LAYER
    ┌─────────────────────────────────────────────────────────┐
    │              Personal Website (static)                  │
    │  ┌─────────────────────────────────────────────────┐    │
    │  │  Hero section: plugin name, tagline, screenshot │    │
    │  │  Demo video embed (YouTube)                     │    │
    │  │  Feature list                                   │    │
    │  │  [Get it free →] CTA → Tally survey             │    │
    │  │  [Support on Patreon →] CTA → Patreon page      │    │
    │  └─────────────────────────────────────────────────┘    │
    └─────────────────────┬───────────────────────────────────┘
              ┌───────────┴──────────┐
              ▼                      ▼
    ┌──────────────────┐   ┌─────────────────────┐
    │  Tally Survey    │   │  Patreon Page       │
    │  (email capture) │   │  (membership tiers) │
    │                  │   │                     │
    │  Fields:         │   │  - Linked from site │
    │  - Name          │   │  - Linked from posts│
    │  - Email         │   │  - Linked from README│
    │  - DAW           │   │                     │
    │  - OS            │   └─────────────────────┘
    │                  │
    │  On submit:      │
    │  Redirect to     │
    │  GitHub Release  │
    └────────┬─────────┘
             │
             ▼
    ┌──────────────────┐
    │  GitHub Releases │
    │  (download page) │
    │                  │
    │  - macOS zip     │
    │  - Windows zip   │
    │  - PDF manual    │
    └──────────────────┘
```

### Component Responsibilities

| Component | Responsibility | Owned By |
|-----------|----------------|----------|
| Personal website (static) | Central landing hub — converts discovery traffic into survey completions and Patreon visits | Andrew (personal domain) |
| Tally survey | Email capture gate — collects audience data before surfacing download link | Tally.so (hosted) |
| GitHub Releases | Authoritative download location — hosts plugin binaries + manual | GitHub (Spatial-Media-Lab org) |
| Patreon page | Funding mechanism — converts supporters from website + social | Patreon (hosted) |
| SML website | Organisation presence — blog post for SEO and newsletter announcement | SML (builder-based) |
| Social (LinkedIn + Instagram) | Traffic driver — pushes discovery audience to personal website | Direct post |
| Demo video (YouTube) | Proof-of-concept — embedded on website, shared natively on social | YouTube (embedded) |
| GitHub repo (public) | Source of truth — GPL source, README as product page for developer audience | GitHub (Spatial-Media-Lab org) |

---

## User Journey Map

### Primary Journey: Discovery to Download

```
1. DISCOVERY
   User sees post on LinkedIn or Instagram
        │
        ▼
2. LANDING
   Clicks link → Personal website
   Reads feature list, watches demo video
        │
        ▼
3. INTENT
   Clicks [Get it free →]
        │
        ▼
4. EMAIL GATE (Tally survey)
   Submits name, email, DAW, OS
        │
        ▼
5. DOWNLOAD
   Tally redirect → GitHub Releases page
   Downloads macOS or Windows zip
        │
        ▼
6. INSTALL
   Follows quarantine/SmartScreen instructions from README
        │
        ▼
7. USE
   Loads in DAW, tries presets
```

### Secondary Journey: Developer / Open-Source Path

```
1. DISCOVERY
   GitHub search, KVR, Bedroom Producers Blog, Reddit r/SpatialAudio
        │
        ▼
2. GITHUB REPO (public)
   README has download link + Patreon link
   GPL-3.0 license visible
        │
        ▼
3. BRANCHES
   Download binary  →  same GitHub Releases download
   Fork/build       →  clone + cmake instructions
   Support project  →  Patreon link from README
```

### Tertiary Journey: Discovery via SML Ecosystem

```
1. DISCOVERY
   User follows SML website / newsletter
        │
        ▼
2. SML BLOG POST
   "Introducing OpenSpatialDelay" post
   Links to personal website landing page
        │
        ▼
3. Joins primary journey at LANDING step
```

---

## Content Flow Between Platforms

```
Demo Video (YouTube)
    │
    ├──embed──► Personal website (hero section)
    └──share──► LinkedIn post / Instagram Reel
                        │
                        └──link──► Personal website

Personal website
    ├──CTA──► Tally survey ──redirect──► GitHub Releases
    └──CTA──► Patreon page

SML blog post
    └──link──► Personal website

GitHub README
    ├──link──► GitHub Releases (download section)
    └──link──► Patreon (support section)
    └──link──► Personal website (optional)
```

### Content Reuse Map

| Source Asset | Reused In |
|---|---|
| Plugin screenshot (`docs/assets/screenshot.png`) | Personal website hero, social posts, SML blog post |
| Demo video (to be created) | Personal website embed, LinkedIn post, Instagram Reel |
| Short description (1-2 sentences from RELEASE_PLAN) | Social captions, SML blog intro, README tagline, Tally confirmation page |
| Release notes (`docs/RELEASE_NOTES_v1.0.0.md`) | GitHub Release body, SML blog detail section |
| User manual PDF | GitHub Release attachment, website download section link |
| Feature list (from README) | Website feature section, Patreon page about section |

---

## Dependency Graph and Build Order

Components have hard dependencies (cannot be built without the prior step) and soft dependencies (order matters for quality but not for technical function).

### Hard Dependencies

```
GitHub repo must be PUBLIC
    before:
        → GitHub Releases download link works for public users
        → Windows CI (free minutes require public repo)
        → GitHub Releases page is accessible without login

Tally survey must be LIVE and redirect URL SET
    before:
        → Personal website [Get it free] CTA goes live

GitHub Releases v1.0.0 must be PUBLISHED
    before:
        → Tally redirect URL is valid
        → README download links work

Personal website must be LIVE
    before:
        → Social posts go out (link in bio / post link)
        → SML blog post links to it
```

### Soft Dependencies (ordering for quality)

```
Demo video should be READY
    before:
        → Personal website goes live (embed it, not placeholder)
        → Social posts (video content drives engagement)

SML blog post should be DRAFTED
    before:
        → Newsletter goes out (newsletter links to blog post)

Patreon page should be LIVE and configured
    before:
        → Any link to it goes public (avoid dead links)
```

### Recommended Build Order

```
Phase A — Infrastructure (no external dependencies, can parallelize)
  A1. Make GitHub repo public + trigger Windows CI build
  A2. Publish GitHub Release v1.0.0 with macOS artifacts
      (Windows artifacts added once CI completes)
  A3. Set up Patreon page (tiers, about copy, link to website)

Phase B — Capture layer (depends on A1/A2)
  B1. Create Tally survey
      - Set redirect URL to GitHub Releases page
      - Test end-to-end: submit → redirect → can download

Phase C — Website (depends on B1 for working CTA)
  C1. Build personal website
      - Embed YouTube demo video (can use placeholder until video ready)
      - Wire [Get it free] → Tally URL
      - Wire [Support on Patreon] → Patreon URL
  C2. Publish website

Phase D — Content layer (depends on C2 for link destination)
  D1. Record and publish demo video → update website embed
  D2. Publish SML blog post (links to personal website)
  D3. Update SML About Us + newsletter

Phase E — Announce (depends on C2 + D1 for assets)
  E1. LinkedIn post (link to personal website)
  E2. Instagram post (link in bio → personal website)
  E3. Post to KVR, Gearspace, Reddit
```

---

## Architectural Patterns

### Pattern 1: Survey-Gated Download

**What:** Download link lives behind a Tally form submission. The form's "on submit" action redirects to the GitHub Releases page. Users who know the GitHub Releases URL can bypass the gate — this is acceptable (open-source) and expected for the developer audience.

**When to use:** When email capture matters more than download friction, and when the download artifact is inherently public (GPL-3.0 means gatekeeping is soft by design).

**Trade-offs:**
- Pro: Builds email list from day one without a mailing list platform
- Pro: Captures DAW/OS data for future platform priorities
- Con: One extra click vs. direct download — acceptable friction for free software
- Con: Tally free tier has limits on submissions (100/month on free plan); verify tier before launch

**Implementation:** Tally redirect-on-completion → GitHub Releases URL. No automation platform (Make/Zapier) needed for v1.0 — Tally natively stores submissions in its dashboard.

### Pattern 2: Hub-and-Spoke Website

**What:** Personal website is the single canonical destination for all traffic. Social posts, SML blog, GitHub README all link to it. The website then routes users to the appropriate downstream destination (survey, Patreon, demo video).

**When to use:** When you control multiple platforms but want a single source of truth that can be updated without editing every outbound link.

**Trade-offs:**
- Pro: One URL to share everywhere; one place to update CTAs
- Con: Adds one redirect hop for GitHub-discovered users (minimal impact)
- Con: Website going down breaks all traffic flows (use reliable static host: Netlify, Vercel, or GitHub Pages)

### Pattern 3: Thin Patreon Integration

**What:** Patreon is linked to (not embedded in) the website and README. The website has a dedicated CTA button. README has a "Support" section. No Patreon API or widget — just outbound links.

**When to use:** For v1.0 where the goal is presence + discoverability, not complex membership gating.

**Trade-offs:**
- Pro: Zero integration complexity
- Con: No social proof widget showing member count on website (Patreon widgets require API setup)
- Note: Patreon member count can be cited manually in copy once known ("X supporters on Patreon")

---

## GitHub Repo Public-Readiness Checklist

The repo is already well-prepared (based on docs audit in RELEASE_PLAN). Key checks:

### Security / Secrets

| Check | Status | Action |
|---|---|---|
| No `.env` files committed | Verify with `git log --diff-filter=A --name-only \| grep -iE '\.env\|secret\|key'` | Already planned in release plan Phase 3 |
| No credentials in history | Same scan | Run before making public |
| `.gitignore` excludes build artifacts, secrets | Likely in place | Verify covers `/build/`, `*.key`, `*.p12` |
| `LICENSE` file at root | Present (GPL-3.0 / Commercial dual) | Done |

### Discoverability

| Check | Status | Action |
|---|---|---|
| GitHub repo description set | Unknown | Set to: "Free, open-source spatial delay plugin — 12 taps in 3D space, VST3+AU, macOS+Windows" |
| GitHub topics/tags set | Unknown | Add: `audio-plugin`, `vst3`, `spatial-audio`, `juce`, `cpp`, `au`, `binaural`, `hrtf`, `open-source` |
| README has screenshot | Present (`docs/assets/screenshot.png`) | Already in README |
| README links to download | Present (GitHub Releases link) | Done |
| README links to Patreon | Not yet | Add "Support" section |
| GitHub Releases description is polished | Yes (RELEASE_NOTES_v1.0.0.md) | Done |
| README mentions macOS notarization gap clearly | Yes (`xattr -cr` instructions) | Done |

### Maintenance Signals

| Check | Status | Action |
|---|---|---|
| Issues enabled | Presumably yes | Confirm |
| Issue templates | Unknown | Optional for v1.0 — can add later |
| CONTRIBUTING.md | Not mentioned | Not needed for v1.0 |
| Code of conduct | Not mentioned | Optional for v1.0 |

---

## Integration Points

### External Services

| Service | Integration Pattern | Notes |
|---|---|---|
| Tally.so | Standalone form, redirect-on-submit to GitHub Releases | No API/webhook needed for v1.0; submissions viewable in Tally dashboard |
| Patreon | Outbound links only (website + README) | No API; Patreon page copy links back to website |
| GitHub Releases | Direct URL (public after repo goes public) | Canonical download URL; freeze this URL in Tally redirect |
| YouTube | Embed `<iframe>` on personal website | Upload unlisted first, switch to public on launch day |
| LinkedIn | Post with external link to website | LinkedIn preview card auto-populated from website og:tags |
| Instagram | Link in bio → website; post with screenshot | No clickable links in caption — bio link is only entry point |
| SML website (builder) | Blog post links to personal website | SML blog post is SEO asset; newsletter points to it |

### Internal Boundaries

| Boundary | Communication | Notes |
|---|---|---|
| Social → Personal website | Outbound link in post/bio | og:image + og:title must be set on website for social preview cards |
| Personal website → Tally | Button/link to Tally form URL | Use UTM parameters: `?utm_source=website&utm_medium=cta` to track in Tally |
| Tally → GitHub Releases | Redirect-on-submit | Test this redirect before website goes live |
| GitHub README → Patreon | Hyperlink in Support section | Place below download instructions, not above |
| SML blog → Personal website | Hyperlink in post body + CTA | SML blog is secondary; personal website is primary destination |

---

## Scaling Considerations

This is a launch infrastructure, not a scalable application. The primary concern is avoiding broken links and download friction at launch traffic peak.

| Scale | Architecture Adjustments |
|---|---|
| 0-1k downloads | Current architecture is fine — GitHub Releases, Tally free tier, static site on free Netlify/Vercel plan |
| 1k-10k downloads | Verify Tally submission tier limits; GitHub Releases has no limits for downloads |
| 10k+ downloads | GitHub Releases CDN handles this natively; Tally Pro tier ($29/mo) if needed for submission volume |

---

## Anti-Patterns

### Anti-Pattern 1: Download Link Before Survey

**What people do:** Put the GitHub Releases link directly on the website as the CTA.
**Why it's wrong:** Loses all email capture data. The plugin is GPL and the source is public, so the binaries will be discoverable regardless — the survey gate is the only mechanism to build a list before KVR/Reddit/blog posts distribute the direct link everywhere.
**Do this instead:** Website CTA → Tally survey → redirect to GitHub Releases. Accept that the developer/GitHub audience will bypass this — that's fine.

### Anti-Pattern 2: Launch Social Before Website is Live

**What people do:** Post on LinkedIn/Instagram pointing to the website before it's deployed.
**Why it's wrong:** Dead links on launch day destroy credibility and waste the initial traffic spike.
**Do this instead:** Website live-check before any social post goes out. The build order (Phase C before Phase E) enforces this.

### Anti-Pattern 3: Tally Redirect to Unstable URL

**What people do:** Redirect Tally to a temporary or changing URL (e.g., a CI artifact link that expires).
**Why it's wrong:** Tally submissions have no expiry — users who submitted 6 months ago and return to re-download would hit a dead link.
**Do this instead:** Redirect to the permanent GitHub Releases page (`/releases/tag/v1.0.0` or just `/releases/latest`). Use `/releases/latest` so future versions are served automatically.

### Anti-Pattern 4: No og:meta Tags on Personal Website

**What people do:** Launch website without Open Graph meta tags.
**Why it's wrong:** LinkedIn, Instagram (link in bio), and iMessage previews all depend on `og:title`, `og:description`, and `og:image`. Without them, the link preview is blank or shows the domain name only — significantly reduces click-through rate.
**Do this instead:** Set `og:title`, `og:description`, `og:image` (use `docs/assets/screenshot.png`), and `og:url` before any social posts go out.

### Anti-Pattern 5: Patreon Link Buried Below the Fold

**What people do:** Put Patreon as a footer link or in a "more" section.
**Why it's wrong:** For an open-source audio plugin by a solo developer, most users will never return to the website. The one moment of maximum goodwill is the first visit after discovering a free tool they want. Bury the Patreon and that moment is wasted.
**Do this instead:** Two CTAs in the hero section: [Get it free →] as primary, [Support on Patreon →] as secondary. Both visible without scrolling.

---

## Sources

- Tally.so feature documentation: https://tally.so/features (redirect-on-completion, email capture)
- GitHub Releases CDN behavior: standard GitHub infrastructure, HIGH confidence
- Open Graph Protocol specification: https://ogp.me/
- Release plan strategy: `docs/RELEASE_PLAN_v1.0.0.md` (internal, written by project maintainer)
- Patreon integration: outbound link pattern, no API needed for v1.0

---
*Architecture research for: OpenSpatialDelay v1.0 launch infrastructure*
*Researched: 2026-04-14*
