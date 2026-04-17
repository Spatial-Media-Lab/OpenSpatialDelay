# Requirements: OpenSpatialDelay v1.0 Release Plan

**Defined:** 2026-04-14
**Core Value:** Each delay echo occupies a distinct spatial position, creating immersive 3D soundscapes that move through space around the listener.

## v1.0 Requirements

Requirements for the public launch. Each maps to roadmap phases.

### Repo & License

- [x] **REPO-01**: GPL-3.0 license audit complete — all dual-license references removed across source headers, README, manual PDF, About dialog, CLAUDE.md, agent_docs/licensing.md
- [x] **REPO-02**: Release v1.0.0 prepared — macOS arm64 binary + legal notices bundled in release ZIP at GitHub (Spatial-Media-Lab/OpenSpatialDelay). Repo-going-public deferred to Phase 3 per D-09; Windows binary deferred to post-Phase 3 per D-06b.
- [x] **REPO-03**: System requirements reviewed and documented (supported OS versions, CPU architectures, DAW compatibility)
- [x] **REPO-04**: Installation instructions written — includes macOS Sequoia xattr command, based on verified system requirements from REPO-03

### Website

- [x] **WEB-01**: Personal landing page live on Netlify — hero section, 30s audio demo embed, download CTA, screenshots, system requirements, license statement
- [x] **WEB-02**: SML About Us page updated with Andrew + board members
- [ ] **WEB-03**: Open Graph meta tags configured (og:title, og:image, og:description) for social link previews

### Distribution & Funding

- [ ] **DIST-01**: Sender.net email capture live on andrewrahman.com/get-osd — required email, privacy-policy link, DOI confirmation email with inline macOS + Windows download buttons linking to v1.0.0 GitHub Release assets + Patreon CTA. (Supersedes original Tally spec 2026-04-16.) Cross-phase closeout tracked in Plan 03-09.
- [x] **DIST-02**: Privacy policy page published (required for GDPR compliance)
- [x] **DIST-03**: Patreon page published with 2+ posts and patron-value framing (spatial audio tools pipeline pitch)

### Content

- [ ] **CONT-01**: 30-second binaural audio demo clip produced
- [ ] **CONT-02**: Video demo / Reel created (screen recording or trajectory animation)
- [ ] **CONT-03**: High-res plugin screenshots captured for landing page, social posts, KVR

### Announcements

- [ ] **ANNC-01**: SML blog post published with demo embed
- [ ] **ANNC-02**: SML newsletter email sent announcing OSD
- [ ] **ANNC-03**: LinkedIn post published (link in first comment)
- [ ] **ANNC-04**: Instagram post/Reel published with demo content
- [ ] **ANNC-05**: KVR Audio product listing created
- [ ] **ANNC-06**: Influencer outreach — researched contact list (spatial audio practitioners, plugin reviewers, relevant YouTubers / journalists / creators) with per-contact personalised intro emails drafted and ready to send at launch

## v2 Requirements

Deferred to future milestone. Tracked but not in current roadmap.

### Content & Community

- **COMM-01**: Full YouTube walkthrough / tutorial video
- **COMM-02**: Product Hunt launch
- **COMM-03**: Discord community server
- **COMM-04**: CONTRIBUTING.md for open-source contributors
- **COMM-05**: Multi-language landing page support

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| Plugin code changes | No new DSP features this milestone — plugin is shipped |
| AAX format support | VST3 + AU only per project constraints |
| Mobile or web versions | Desktop plugin only |
| Paid plugin tiers | Plugin is free; Patreon is the funding model |
| Auto-playing hero animations | Anti-feature per research |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| REPO-01 | Phase 1 | Complete |
| REPO-02 | Phase 1 | Complete (release prepared; public-repo flip + Windows binary deferred per D-06b/D-09) |
| REPO-03 | Phase 1 | Complete |
| REPO-04 | Phase 1 | Complete |
| WEB-01 | Phase 3 | Complete |
| WEB-02 | Phase 3 | Complete |
| WEB-03 | Phase 3 | Pending |
| DIST-01 | Phase 2 | Pending |
| DIST-02 | Phase 2 | Complete |
| DIST-03 | Phase 2 | Complete |
| CONT-01 | Phase 4 | Pending |
| CONT-02 | Phase 4 | Pending |
| CONT-03 | Phase 4 | Pending |
| ANNC-01 | Phase 5 | Pending |
| ANNC-02 | Phase 5 | Pending |
| ANNC-03 | Phase 5 | Pending |
| ANNC-04 | Phase 5 | Pending |
| ANNC-05 | Phase 5 | Pending |
| ANNC-06 | Phase 5 | Pending |

**Coverage:**
- v1.0 requirements: 19 total
- Mapped to phases: 19
- Unmapped: 0

---
*Requirements defined: 2026-04-14*
*Last updated: 2026-04-17 — Phase-1 REPO-01..04 verified complete, DIST-01 scope rewritten Tally→Sender.net, ANNC-06 added for influencer outreach*
