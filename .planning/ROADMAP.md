# Roadmap: OpenSpatialDelay v1.0 Release Plan

## Overview

Five phases turn a shipped plugin into a public product. The sequence is non-negotiable: legal and repo readiness first (the foundation everything else links to), then email capture and Patreon infrastructure (CTAs need destinations), then the personal website (the hub that connects it all), then demo content production (required for the website and every announcement), and finally the launch sequence — blog, newsletter, social, KVR. Each phase unblocks the next.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [ ] **Phase 1: Repo & License Readiness** - GPL transition complete, repo public, release published, install docs written
- [ ] **Phase 2: Email Capture & Funding Infrastructure** - Tally form live, privacy policy published, Patreon seeded and ready
- [ ] **Phase 3: Personal Website** - Landing page live on Netlify, SML About Us updated, Open Graph configured
- [ ] **Phase 4: Demo Content** - 30s audio demo, video reel, and high-res screenshots produced
- [ ] **Phase 5: Launch Announcements** - Blog post, newsletter, LinkedIn, Instagram, KVR listing published

## Phase Details

### Phase 1: Repo & License Readiness
**Goal**: The codebase is clean GPL-3.0, the GitHub repo is public, the release is downloadable, and installation instructions are accurate
**Depends on**: Nothing (first phase)
**Requirements**: REPO-01, REPO-02, REPO-03, REPO-04
**Success Criteria** (what must be TRUE):
  1. `git grep -i "commercial licen"` returns zero results outside CHANGELOG across all source headers, README, manual PDF, About dialog, CLAUDE.md, and licensing.md
  2. GitHub repo is publicly visible and the v1.0.0 Release page shows macOS and Windows binaries ready to download
  3. A documented list of supported OS versions, CPU architectures, and DAWs exists and can be referenced by the install page
  4. Installation instructions include the macOS Sequoia xattr command and are accurate for all supported platforms
**Plans**: 2 plans
Plans:
- [x] 01-01-PLAN.md — License cleanup: update 11 files from dual-license to GPL-3.0 only
- [x] 01-02-PLAN.md — Release metadata update and README system requirements/installation review

### Phase 2: Email Capture & Funding Infrastructure
**Goal**: Visitors who want to follow the project have two working paths — email capture via Tally and direct Patreon support — before the website goes live
**Depends on**: Phase 1
**Requirements**: DIST-01, DIST-02, DIST-03
**Success Criteria** (what must be TRUE):
  1. Submitting the Tally form with optional email + GDPR consent checkbox redirects to the GitHub Releases page
  2. A privacy policy page is publicly accessible and linked from the Tally form
  3. The Patreon page is published with 2+ posts and language explaining that supporters are funding a spatial audio tools pipeline, not just one plugin
**Plans**: TBD

### Phase 3: Personal Website
**Goal**: The personal landing page is live and functions as the central hub — visitors can learn about the plugin, hear a demo, and reach every downstream destination (download, Patreon, Tally, SML)
**Depends on**: Phase 2
**Requirements**: WEB-01, WEB-02, WEB-03
**Success Criteria** (what must be TRUE):
  1. The landing page loads at a Netlify URL and contains: hero section, 30-second audio demo embed, download CTA, plugin screenshots, system requirements, and GPL-3.0 license statement
  2. Pasting the landing page URL into Twitter/X or LinkedIn shows a correct social preview card (title, image, description) via Open Graph meta tags
  3. The SML About Us page names Andrew and all current board members
**Plans**: TBD
**UI hint**: yes

### Phase 4: Demo Content
**Goal**: A 30-second binaural audio clip, a video reel, and high-res screenshots exist as finished assets ready to embed in the website and attach to every announcement
**Depends on**: Phase 1
**Requirements**: CONT-01, CONT-02, CONT-03
**Success Criteria** (what must be TRUE):
  1. A 30-second binaural audio demo clip is produced and exported in a web-embeddable format
  2. A video demo or Reel exists (screen recording or trajectory animation) suitable for Instagram and the landing page
  3. High-resolution plugin screenshots are captured at a quality appropriate for the KVR Audio listing and social posts
**Plans**: TBD

### Phase 5: Launch Announcements
**Goal**: The plugin is publicly announced across all channels — blog, newsletter, LinkedIn, Instagram, and KVR Audio — with working links to the website and download
**Depends on**: Phase 3, Phase 4
**Requirements**: ANNC-01, ANNC-02, ANNC-03, ANNC-04, ANNC-05
**Success Criteria** (what must be TRUE):
  1. The SML blog post is live and embeds or links to the audio demo
  2. The SML newsletter email is sent with the download and landing page links
  3. A LinkedIn post is published with the link in the first comment
  4. An Instagram post or Reel is published using the video demo content
  5. A KVR Audio product listing for OpenSpatialDelay is live and links to the GitHub release
**Plans**: TBD

## Progress

**Execution Order:**
Phases execute in dependency order. Phase 4 (Content) depends on Phase 1 only and can run concurrently with Phase 2 and Phase 3 if desired, but Phase 5 requires both Phase 3 and Phase 4 complete.

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Repo & License Readiness | 0/2 | Planning complete | - |
| 2. Email Capture & Funding Infrastructure | 0/? | Not started | - |
| 3. Personal Website | 0/? | Not started | - |
| 4. Demo Content | 0/? | Not started | - |
| 5. Launch Announcements | 0/? | Not started | - |
