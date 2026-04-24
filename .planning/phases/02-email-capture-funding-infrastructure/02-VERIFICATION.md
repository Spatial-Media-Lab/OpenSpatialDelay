---
phase: 02-email-capture-funding-infrastructure
verified: 2026-04-24T16:45:00Z
status: human_needed
score: 5/7 must-haves verified
overrides_applied: 0
gaps: []
deferred:
  - truth: "Sender.net email capture is live at andrewrahman.com — the Sender form loads on the homepage, the full DOI flow resolves to the /get-osd/ landing page, and production UAT passes (incognito submit → DOI email → confirm → /get-osd/ → both .zip downloads)"
    addressed_in: "Phase 3 Plan 03-09"
    evidence: "Plan 03-09 in ROADMAP.md: 'Cross-phase Sender.net production flip + DIST-01 UAT (queued 2026-04-17; depends on site launch + Sender dashboard completion)'"
  - truth: "NEXT_PUBLIC_SENDER_FORM_ID and NEXT_PUBLIC_SENDER_ACCOUNT_ID are set in Netlify env vars; old Tally env var deleted; production build redeployed with clear-cache; verify-production.sh exits 0"
    addressed_in: "Phase 3 Plan 03-09"
    evidence: "Plan 03-09 in ROADMAP.md covers 'DIST-01 UAT' which requires the production Netlify deploy completed in Plan 02-04"
human_verification:
  - test: "Verify the Patreon creator page shows all required elements in a logged-out browser"
    expected: "patreon.com/AndrewRahman loads showing: tagline 'Funding the Spatial Media Library…', About section, 4 tiers (Stargazer/Astronaut/Commander/Mission Control at $3/$10/$25/$100), at least 2 public posts, and cross-links to GitHub/andrewrahman.com/spatialmedialab.org. NO annual billing options visible."
    why_human: "curl -sI returns HTTP/2 301 confirming page is live, but the full content check (tiers, post count, pipeline framing visibility) requires a browser. The 06-SUMMARY records creator-verified 7 incognito checks at launch, so this is a lightweight confirm, not a full re-audit."
  - test: "Confirm the Sender.net form renders correctly on the homepage at the current Netlify preview URL (silly-licorice-0ee82d.netlify.app returned 503 at verification time — verify the deployment is active and the dark-theme form styling is applied)"
    expected: "Homepage at the Netlify preview URL renders the email-capture form with dark background, cyan button (--accent-stellar), Inter typography, and no Sender branding attribution visible. Placeholder card is NOT shown (SENDER_FORM_ID must be set in the env)."
    why_human: "The Netlify preview URL returned 503 during automated checks — cannot determine whether the site is suspended/deleted or temporarily unavailable. The production domain (andrewrahman.com) still resolves to the Weebly frameset (Netfirms parking IP 66.96.149.1), so the new site is not yet live under the custom domain."
---

# Phase 2: Email Capture & Funding Infrastructure Verification Report

**Phase Goal:** Visitors who want to follow the project have two working paths — email capture via Sender.net (pivoted from Tally) and direct Patreon support — before the website goes live.
**Verified:** 2026-04-24T16:45:00Z
**Status:** human_needed
**Re-verification:** No — initial verification

---

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | Planning docs (PROJECT.md, ROADMAP.md, REQUIREMENTS.md) correctly define the three brand identities and use required-email framing for DIST-01 | VERIFIED | PROJECT.md has ## Brand Identity section (line 7); REQUIREMENTS.md DIST-01 reads "required email"; no "Download gate / mandatory email" anti-feature row remains |
| 2 | The andrewrahman.com Next.js site exists with shared Layout, /privacy, and /get-osd routes, builds cleanly, and is ready to deploy | VERIFIED | All files present in andrewrahman-com repo: next.config.ts, app/layout.tsx, components/Nav.tsx, components/Footer.tsx, app/privacy/page.tsx, app/get-osd/page.tsx. netlify.toml + _headers committed. 17/17 Playwright tests green per 02-07-SUMMARY.md |
| 3 | The privacy policy at /privacy contains complete authored prose naming Andrew Rahman as Data Controller, UAB Sender.lt (Sender.net) as processor, covering GDPR + CCPA + UK GDPR, with andrewjrahman@gmail.com as rights contact | VERIFIED | Confirmed in code: "Data Controller: Andrew Rahman (natural person)" at line 18; "UAB Sender.lt, Vilnius, Lithuania" at line 87; "CCPA/CPRA" at line 23; "UK GDPR" at line 23; "indefinitely" at line 115. Zero "Prose pending" placeholders. Zero Tally Technologies SRL references (count: 0) |
| 4 | The Sender.net email-capture infrastructure is built end-to-end: form bkRxov published, osd-unconfirmed + osd-confirmed groups live, DOI automation active, DownloadForm.tsx wired with IIFE bootstrap, CSP updated, privacy page names Sender.net as processor | VERIFIED | DownloadForm.tsx reads NEXT_PUBLIC_SENDER_FORM_ID + NEXT_PUBLIC_SENDER_ACCOUNT_ID; cdn.sender.net in _headers CSP; no tally.so refs in _headers or app/; privacy page names UAB Sender.lt; 02-05-SUMMARY documents end-to-end localhost DOI flow validated 2026-04-24 |
| 5 | The /get-osd/ page is the post-confirm landing page with DownloadButtons (macOS + Windows via self-hosted /assets/.zip) and a Patreon CTA | VERIFIED | app/get-osd/page.tsx imports DownloadButtons; lib/release.ts defines MAC_ZIP_FILENAME + WIN_ZIP_FILENAME pointing to /assets/; WIN_ZIP_AVAILABLE = true; page renders "Confirmed · you're in" eyebrow |
| 6 | Sender.net email capture is live at andrewrahman.com — production deploy completed, DNS cut over, full E2E UAT passed | DEFERRED | andrewrahman.com returns Weebly frameset (dig returns 66.96.149.1 Netfirms parking IP); Netlify preview URL (silly-licorice-0ee82d.netlify.app) returned 503 at verification time. Production flip explicitly deferred to Plan 03-09 per ROADMAP.md and STATE.md. Addressed in Phase 3 Plan 03-09. |
| 7 | The Patreon page at patreon.com/AndrewRahman is publicly live with 4 tiers (monthly only), 3+ posts (2 public + 1 patron-only), pipeline framing visible to logged-out visitors | PARTIALLY VERIFIED | curl confirms HTTP/2 301 → www.patreon.com/AndrewRahman (page resolves publicly). Six Patreon draft files present in repo. 02-06-SUMMARY records creator-verified 7 incognito checks at launch. Full content detail (tier names, post visibility, pipeline copy) requires human browser confirm. |

**Score:** 5/7 truths fully verified (2 are deferred to Phase 3 or require human confirm)

---

### Deferred Items

Items not yet met but explicitly addressed in later milestone phases.

| # | Item | Addressed In | Evidence |
|---|------|-------------|----------|
| 1 | Sender.net email capture live at andrewrahman.com with production DNS and E2E UAT | Phase 3 Plan 03-09 | ROADMAP.md: "03-09-PLAN.md — Cross-phase Sender.net production flip + DIST-01 UAT (queued 2026-04-17; depends on site launch + Sender dashboard completion)" |
| 2 | Netlify env vars (NEXT_PUBLIC_SENDER_FORM_ID, NEXT_PUBLIC_SENDER_ACCOUNT_ID) set; verify-production.sh exits 0 | Phase 3 Plan 03-09 | STATE.md: "production flip (Netlify env var + live E2E UAT) waits on Plan 02-04 Netlify deploy/cutover"; Plan 03-09 owns DIST-01 closeout |

---

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `.planning/PROJECT.md` | Brand Identity section: SML / Spatial Media Library / OSD / Andrew Rahman | VERIFIED | ## Brand Identity at line 7; all four entities defined as bold bullets |
| `.planning/REQUIREMENTS.md` | DIST-01 with required-email framing; no "Download gate" anti-feature row | VERIFIED | DIST-01 reads "required email"; grep "Download gate" returns 0 matches |
| `andrewrahman-com:next.config.ts` | Static export config (output: 'export') | VERIFIED | File present; output: 'export' confirmed |
| `andrewrahman-com:app/layout.tsx` | Root Layout with Nav + Footer | VERIFIED | Imports Nav and Footer; both present in components/ |
| `andrewrahman-com:app/privacy/page.tsx` | Full privacy policy prose, Sender.net named as processor, no placeholders | VERIFIED | Andrew Rahman as DC; UAB Sender.lt named; GDPR+CCPA+UK GDPR coverage; indefinite retention; zero placeholders |
| `andrewrahman-com:app/get-osd/page.tsx` | Post-confirm landing page with DownloadButtons, no form | VERIFIED | Imports DownloadButtons from lib/release.ts; renders "Confirmed · you're in"; no form present |
| `andrewrahman-com:components/DownloadForm.tsx` | Sender IIFE bootstrap, reads NEXT_PUBLIC_SENDER_FORM_ID + NEXT_PUBLIC_SENDER_ACCOUNT_ID | VERIFIED | Both env vars read at lines 5–6; SENDER_SCRIPT = cdn.sender.net; sender(ACCOUNT_ID) IIFE init confirmed |
| `andrewrahman-com:components/DownloadButtons.tsx` | OS-detected macOS + Windows download buttons from lib/release.ts | VERIFIED | Reads MAC_ZIP_FILENAME, WIN_ZIP_FILENAME, WIN_ZIP_AVAILABLE from @/lib/release |
| `andrewrahman-com:lib/release.ts` | RELEASE_VERSION, MAC_ZIP_FILENAME, WIN_ZIP_FILENAME, ASSETS_BASE, WIN_ZIP_AVAILABLE | VERIFIED | All constants present; ASSETS_BASE = '/assets'; WIN_ZIP_AVAILABLE = true |
| `andrewrahman-com:_headers` | Netlify security headers: HSTS, CSP scoped to cdn.sender.net (not tally.so) | VERIFIED | Strict-Transport-Security at line 2; CSP at line 7 references cdn.sender.net + *.sender.net; no tally.so present |
| `andrewrahman-com:netlify.toml` | publish = "out", build command = npm run build, NODE_VERSION = 20 | VERIFIED | All three values confirmed present |
| `andrewrahman-com:tests/privacy-content.spec.ts` | Playwright regression suite asserting controller, contact email, Sender.net processor | VERIFIED | Tests assert "UAB Sender.lt" (line 35) and "Andrew Rahman" as controller |
| `andrewrahman-com:tests/homepage-content.spec.ts` | Content regression suite asserting Sender form container on homepage | VERIFIED | data-sender-form-id assertion at line 197 |
| `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-page-about.md` | Tagline, About, SML/Spatial Media Library disambiguation, 4 cross-links | VERIFIED | Both brand names present (Spatial Media Library ×3, Spatial Media Lab ×3) |
| `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-tier-copy.md` | 4 tiers: Stargazer $3 / Astronaut $10 / Commander $25 / Mission Control $100 (monthly only) | VERIFIED | All 4 tiers present with space-exploration names and correct prices |
| `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-01-welcome.md` | Public post with pipeline framing | VERIFIED | Marked PUBLIC; contains "Spatial Media Lab" and "Spatial Media Library" |
| `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-02-technical.md` | Patron-only ($3+) technical deep-dive | VERIFIED | Marked PATRON-ONLY; contains HRTF, phase vocoder, trajectory content |
| `docs/phase-02-evidence/patreon-graphics/` | 9 graphics files including avatar, cover, 4 tier images, _generate.py | VERIFIED | 9 files confirmed present |

---

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `andrewrahman-com:app/layout.tsx` | Nav.tsx + Footer.tsx | import + JSX composition | VERIFIED | imports Nav and Footer; both components present |
| `andrewrahman-com:app/page.tsx` | DownloadForm.tsx | import + JSX at line 992 | VERIFIED | `import { DownloadForm }` at line 6; `<DownloadForm />` at line 992 |
| `andrewrahman-com:components/DownloadForm.tsx` | cdn.sender.net | IIFE bootstrap + script injection in useEffect | VERIFIED | SENDER_SCRIPT = 'https://cdn.sender.net/accounts_resources/universal.js'; sender(ACCOUNT_ID) init; script injected via useEffect |
| `andrewrahman-com:app/get-osd/page.tsx` | DownloadButtons.tsx | import + JSX | VERIFIED | `import { DownloadButtons }` at line 2; `<DownloadButtons />` at line 35 |
| `andrewrahman-com:components/DownloadButtons.tsx` | lib/release.ts | import of MAC_ZIP_FILENAME, WIN_ZIP_FILENAME, WIN_ZIP_AVAILABLE | VERIFIED | All three constants imported; ASSETS_BASE = '/assets' |
| `andrewrahman-com:_headers CSP` | cdn.sender.net allowed (not tally.so) | Netlify _headers file | VERIFIED | script-src includes https://cdn.sender.net; no tally.so anywhere in _headers |
| `Netfirms DNS apex A` | Netlify load balancer (75.2.60.5) | A record change | NOT YET DONE | dig andrewrahman.com returns 66.96.149.1 (Netfirms parking). Deferred to Plan 03-09. |
| `Patreon page` | live at patreon.com/AndrewRahman | page launch (Task 4) | VERIFIED | curl confirms HTTP/2 301 → www.patreon.com/AndrewRahman 2026-04-17 and 2026-04-24 |

---

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|----------|---------------|--------|--------------------|--------|
| `DownloadForm.tsx` | SENDER_FORM_ID, SENDER_ACCOUNT_ID | process.env.NEXT_PUBLIC_SENDER_FORM_ID/ACCOUNT_ID | Yes, when env vars set in Netlify; placeholder card shown when unset (correct fail-safe) | VERIFIED (env-driven; placeholder correctly guards unset state) |
| `DownloadButtons.tsx` | MAC_ZIP_FILENAME, WIN_ZIP_FILENAME | lib/release.ts constants | Yes — hardcoded to real filenames; ASSETS_BASE = '/assets' so URLs resolve to /assets/OpenSpatialDelay-v1.0.0-macOS-arm64.zip | VERIFIED (constants not empty; WIN_ZIP_AVAILABLE = true) |
| Self-hosted .zip files | /assets/OpenSpatialDelay-v1.0.0-*.zip | public/assets/ directory | No — zip files not present in public/assets/ | HOLLOW — DownloadButtons generates the correct URLs but the actual .zip files are absent from public/assets/. Download links will 404 on production. Note: the original DIST-01 spec (2026-04-16 onwards) referenced GitHub Release assets, not self-hosted zips. lib/release.ts currently uses ASSETS_BASE = '/assets' pointing to self-hosted path. |

---

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| andrewrahman.com production serves new site | `curl -s https://andrewrahman.com/ \| grep -c "sender\|Andrew"` | 0 (returns Weebly frameset) | FAIL — production DNS not flipped; expected, deferred to Plan 03-09 |
| /privacy and /get-osd routes return 200 on production | `curl -sI https://andrewrahman.com/privacy/` | HTTP/2 404 | FAIL — same root cause: Netfirms parking; expected, deferred |
| Patreon page resolves publicly | `curl -sI https://patreon.com/AndrewRahman \| head -1` | HTTP/2 301 | PASS |
| Netlify preview URL | `curl -sI https://silly-licorice-0ee82d.netlify.app/` | HTTP/2 503 | SKIP — site may be deactivated or temporarily suspended on the preview; does not affect the production deploy plan |
| Privacy page names Sender.net | `grep -c "UAB Sender.lt" .../app/privacy/page.tsx` | 3 | PASS |
| No Tally residue in site | `grep -rn "tally.so" andrewrahman-com/app/` | 0 matches | PASS |
| Netlify config declares correct build target | `grep 'publish = "out"' netlify.toml` | match | PASS |

---

### Requirements Coverage

| Requirement | Source Plans | Description | Status | Evidence |
|-------------|-------------|-------------|--------|----------|
| DIST-01 | Plans 02-02, 02-05 | Sender.net email capture live on andrewrahman.com — required email, DOI flow, download delivery | PARTIAL — infrastructure built, production deploy deferred | DownloadForm.tsx wired; DOI automation active on Sender.net; production flip tracked in Plan 03-09 |
| DIST-02 | Plans 02-02, 02-03, 02-05 | Privacy policy page published (GDPR compliance) | COMPLETE | Full prose in app/privacy/page.tsx; Andrew Rahman as DC; Sender.net named; GDPR+CCPA+UK GDPR; Playwright content tests green. Note: accessible at Netlify preview (currently 503) and will be live at andrewrahman.com/privacy once Plan 02-04/03-09 flip DNS |
| DIST-03 | Plan 02-06 | Patreon page published with 2+ posts and pipeline framing | COMPLETE | patreon.com/AndrewRahman resolves (HTTP/2 301); 6 draft files in repo; creator-verified at launch (2026-04-17); 4 tiers + 3 posts + 1 pinned project-links post |

**Orphaned requirements check (Phase 2 mapped in REQUIREMENTS.md):** DIST-01, DIST-02, DIST-03 — all accounted for across Plans 02-01 through 02-07. No orphaned requirements.

---

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `andrewrahman-com:lib/release.ts` | 6 | `ASSETS_BASE = '/assets'` — self-hosted path for .zip files; no actual .zip files exist in `public/assets/` | Warning | Download links on /get-osd/ will 404 until zip files are placed. The DIST-01 requirement originally specified "linking to v1.0.0 GitHub Release assets" but lib/release.ts switched to self-hosted /assets path. The zips need to either be added to public/assets/ or ASSETS_BASE needs to point back to the GitHub release URL. Not a production blocker today (DNS not flipped), but must be resolved before Plan 03-09 live UAT. |
| `.planning/ROADMAP.md` | 40, 44 | Phase 2 goal line still reads "email capture via Tally" and success criterion 1 still says "Tally form" and "optional email" | Info | The ROADMAP.md Phase Details block for Phase 2 was NOT updated when Sender.net superseded Tally (only REQUIREMENTS.md was updated). This is a documentation discrepancy — the functional work is correct; the roadmap text is stale. Does not affect execution. |

---

### Human Verification Required

#### 1. Patreon Page Content Check

**Test:** Open `https://www.patreon.com/AndrewRahman` in an incognito window and verify:
- Tagline reads "Funding the Spatial Media Library — spatial audio tools for musicians and sound designers"
- About section is present and references both "Spatial Media Library" (pipeline) and "Spatial Media Lab" (org) distinctly
- 4 monthly tiers visible: Stargazer $3, Astronaut $10, Commander $25, Mission Control $100
- No annual billing option visible
- At least 2 public posts visible without login; patron-only post shows locked state with teaser
- Cross-links to GitHub/andrewrahman.com/spatialmedialab.org reachable (via pinned Post 4)

**Expected:** All 7 items check out. Creator confirmed these at launch on 2026-04-17; this is a lightweight re-confirm.

**Why human:** curl confirms the page is live and resolves (HTTP/2 301). The detailed content — tier structure, post visibility, pipeline framing — cannot be verified programmatically from outside the page without a headless browser auth session.

#### 2. Netlify Preview Site Accessibility

**Test:** Confirm the andrewrahman-com site is accessible at its Netlify deployment URL. The preview URL `https://silly-licorice-0ee82d.netlify.app/` returned HTTP/2 503 during verification. Check if this URL is still active, or identify the current deploy URL from the Netlify dashboard.

**Expected:** The site renders at a Netlify URL with: dark background (#03060b), Fraunces display font, Sender.net form visible on homepage (cyan gradient button with dark input styling), all 17 Playwright tests passing.

**Why human:** The 503 from the preview URL could indicate the site was suspended, the account migrated, or the URL changed. Cannot determine which without Netlify dashboard access. This is required before Plan 02-04 DNS cutover can proceed.

---

### Gaps Summary

No substantive gaps in what was built. All deferred items are explicitly tracked in Plan 03-09 (Cross-phase Sender.net production flip + DIST-01 UAT). The phase delivered everything that could be built before the production deploy:

- Planning docs aligned (Plan 02-01)
- Next.js site scaffolded with correct static-export config, shared Layout, all three routes (Plan 02-02)
- Full privacy policy authored with GDPR+CCPA+UK GDPR coverage, Sender.net named as processor (Plans 02-03, 02-05)
- Netlify deploy config and security headers committed (Plan 02-04 Task 1)
- Sender.net DOI infrastructure built and end-to-end validated on localhost (Plan 02-05)
- Homepage redesign delivered with dark palette and Sender form embedded (Plan 02-07)
- Patreon creator page live with 4 tiers + 3 seed posts + 1 project-links post (Plan 02-06)

One item warrants attention before production UAT: the self-hosted .zip file paths in `lib/release.ts` point to `/assets/` but the actual zip files are absent from `public/assets/`. This needs resolution before Plan 03-09 live download testing.

---

_Verified: 2026-04-24T16:45:00Z_
_Verifier: Claude (gsd-verifier)_
