# Research Summary: OpenSpatialDelay v1.0 Release Plan

**Synthesized:** 2026-04-14
**Sources:** STACK.md, FEATURES.md, ARCHITECTURE.md, PITFALLS.md

## Executive Summary

OpenSpatialDelay launches on a zero-budget infrastructure stack. A static personal website (Next.js 15 / Netlify) acts as the central hub, routing traffic from social posts and the SML blog toward an optional email capture survey (Tally) and a Patreon page, with GitHub Releases as the authoritative download endpoint. The approach mirrors how comparable open-source audio plugins (Surge XT, Odin2) have launched — KVR Audio and the existing GitHub audience are the primary discovery channels.

Sequence strictly: repo readiness first (GPL transition audit, GitHub public), then infrastructure (Release published, Tally form live, Patreon seeded), then website, then launch content (demo clip, blog post, newsletter, social). Each phase unblocks the next.

## Stack Decisions

| Tool | Purpose | Notes |
|------|---------|-------|
| Next.js 15 + React 19 + Tailwind 4 | Personal website | Static export to Netlify |
| MagicUI | UI components | Copy-paste via shadcn CLI, not an npm package |
| Impeccable | AI design skill | Not a CSS framework — steers Claude Code away from UI anti-patterns |
| Netlify (free tier) | Hosting | 100 GB bandwidth, commercial use allowed (unlike Vercel/GitHub Pages) |
| Tally.so (free tier) | Email capture | Unlimited submissions, redirect-on-submit to GitHub Releases |
| Google Sheets | Mailing list storage | Native Tally integration; defer ESP until 500+ subscribers |
| Patreon Standard | Funding | 10% + 2.9% + $0.30 per transaction; link-only integration |

## Feature Priority Matrix

### P1 — Must have on launch day

- GPL-3.0 transition complete across all files (source headers, manual PDF, About dialog, README, CLAUDE.md, licensing.md)
- GitHub repo public + GitHub Release v1.0.0 published
- macOS Sequoia xattr install instructions (ctrl-click workaround removed Nov 2024)
- Landing page: hero + 30s audio demo + download CTA + screenshots + system requirements + license
- 30-second binaural audio demo clip
- SML blog post with demo embed
- SML newsletter email
- LinkedIn announcement post
- KVR Audio product listing (primary free plugin discovery channel)

### P2 — Within 1-2 weeks post-launch

- Tally email capture (optional, not gated) with GDPR consent checkbox + privacy policy
- Patreon page with 2+ posts and patron-value framing
- Instagram Reel with 3D trajectory animation
- Open Graph meta tags for social link previews

### P3 — Defer to v2+

- Full YouTube walkthrough
- Product Hunt launch
- Discord server
- CONTRIBUTING.md
- Multi-language support

### Anti-features (do not build)

- Download gate (destroys GPL plugin adoption)
- Auto-playing hero animations
- Mandatory email gate

## Architecture — Hub and Spoke

**Hard dependency chain:** GitHub repo public -> Release published -> Tally redirect tested -> Website CTAs wired -> Social posts

**Key pattern:** Tally redirect should point to `/releases/latest` (not a pinned tag) so future versions are served automatically. Developer/GitHub audience will bypass the Tally gate — acceptable for GPL software.

## Top 5 Pitfalls

1. **Incomplete license transition** — `git grep -i "commercial licen"` must return zero results outside CHANGELOG before going public
2. **macOS Gatekeeper (Sequoia 15.1+)** — `sudo xattr -rd com.apple.quarantine` must be prominent near download link
3. **Empty Patreon at launch** — needs 2+ posts and patron-value language before any public link
4. **GDPR non-compliance on Tally** — unchecked consent checkbox + linked privacy policy required before first submission
5. **Website rebuild blocking launch** — time-box to 2 days or fall back to updating existing builder page

## Suggested Phase Structure

| Phase | Focus | Dependencies | Research needed? |
|-------|-------|-------------|-----------------|
| 1 | Repo + License Readiness | None — start immediately | No |
| 2 | Email Capture + Patreon | Phase 1 (repo public) | No |
| 3 | Personal Website | Phase 2 (CTAs need destinations) | Yes — MagicUI/Impeccable |
| 4 | Demo Content Creation | Parallel with Phase 3 | No |
| 5 | Launch Content + Announcement | Phases 3 + 4 complete | No |

## Confidence

| Area | Level |
|------|-------|
| Stack choices | HIGH |
| Feature priorities | MEDIUM-HIGH |
| Architecture / dependencies | HIGH |
| Legal (GPL transition) | HIGH |
| Platform-specific (social, Patreon) | MEDIUM |

---
*Synthesized from 4 parallel research agents on 2026-04-14*
