---
phase: 03
slug: personal-website
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-04-16
---

# Phase 03 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.
> Source of truth: `03-RESEARCH.md` §Validation Architecture (lines 414–459).
> All Playwright commands run from the **sibling** site repo `../andrewrahman-com/`.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Playwright 1.59.1 (site repo) |
| **Config file** | `../andrewrahman-com/playwright.config.ts` |
| **Quick run command** | `cd ../andrewrahman-com && npm test -- tests/homepage-content.spec.ts` |
| **Full suite command** | `cd ../andrewrahman-com && npm test` |
| **Estimated runtime** | ~30 seconds (full suite on local hardware) |

---

## Sampling Rate

- **After every task commit:** `cd ../andrewrahman-com && npm test -- tests/<changed-file>.spec.ts` (fast, scoped)
- **After every plan wave:** `cd ../andrewrahman-com && npm test` (full Playwright suite)
- **Before `/gsd-verify-work`:** Full suite green + axe violations = 0 + og-metadata assertions all pass
- **Max feedback latency:** ~30 seconds per full run

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 03-00-01 | 00 | 0 | — | — | N/A | infra | `cd ../andrewrahman-com && npm install --save-dev @axe-core/playwright` | ❌ W0 | ⬜ pending |
| 03-00-02 | 00 | 0 | — | — | N/A | scaffold | `test -f ../andrewrahman-com/tests/a11y.spec.ts` | ❌ W0 | ⬜ pending |
| 03-00-03 | 00 | 0 | WEB-03 | — | N/A | scaffold | `test -f ../andrewrahman-com/tests/og-metadata.spec.ts` | ❌ W0 | ⬜ pending |
| 03-00-04 | 00 | 0 | WEB-01 | — | N/A | scaffold | `test -f ../andrewrahman-com/tests/asset-existence.spec.ts` | ❌ W0 | ⬜ pending |
| 03-00-05 | 00 | 0 | — | — | N/A | audit | `ls ~/Library/Audio/Plug-Ins/Components/ \| grep 'v1.0'` | ❌ W0 | ⬜ pending |
| 03-00-06 | 00 | 0 | WEB-01 | — | N/A | triage | `grep -l 'tally' ../andrewrahman-com/app/ -r` (Phase 2 sync check) | ❌ W0 | ⬜ pending |
| 03-01-01 | 01 | 1 | WEB-01 | — | N/A | content | `cd ../andrewrahman-com && npm test -- tests/homepage-content.spec.ts -g "hero stats"` | ❌ W0 (new assertion) | ⬜ pending |
| 03-01-02 | 01 | 1 | WEB-01 | — | N/A | content | `cd ../andrewrahman-com && npm test -- tests/homepage-content.spec.ts -g "features"` | ❌ W0 (new assertion) | ⬜ pending |
| 03-02-01 | 02 | 1 | WEB-01 | — | N/A | content | `cd ../andrewrahman-com && npm test -- tests/homepage-content.spec.ts -g "contact email"` | ✅ update assertion | ⬜ pending |
| 03-02-02 | 02 | 1 | WEB-01 | — | PII-controller disclosure | content | `cd ../andrewrahman-com && npm test -- tests/privacy-content.spec.ts -g "rights-request"` | ✅ update assertion | ⬜ pending |
| 03-02-03 | 02 | 1 | WEB-01 | — | N/A | negative | `cd ../andrewrahman-com && npm test -- tests/privacy-content.spec.ts -g "no legacy email"` | ❌ W0 (new negative assertion) | ⬜ pending |
| 03-03-01 | 03 | 1 | WEB-01 | — | N/A | rendered-style | `cd ../andrewrahman-com && npm test -- tests/homepage-content.spec.ts -g "patreon.*regal"` | ❌ W0 (new assertion) | ⬜ pending |
| 03-04-01 | 04 | 2 | WEB-01 | — | N/A | filesystem | `cd ../andrewrahman-com && npm test -- tests/asset-existence.spec.ts` | ❌ W0 | ⬜ pending |
| 03-04-02 | 04 | 2 | WEB-01 | — | N/A | DOM attribute | `cd ../andrewrahman-com && npm test -- tests/homepage-content.spec.ts -g "SML logo.*svg"` | ❌ W0 (new assertion) | ⬜ pending |
| 03-04-03 | 04 | 2 | WEB-01 | — | N/A | DOM presence | `cd ../andrewrahman-com && npm test -- tests/homepage-content.spec.ts -g "headshot"` | ❌ W0 (new assertion) | ⬜ pending |
| 03-05-01 | 05 | 3 | WEB-01 | — | N/A | DOM link | existing `smoke.spec.ts` + D-14 update | ✅ maybe-update | ⬜ pending |
| 03-06-01 | 06 | 3 | — (D-08) | — | N/A | axe-core | `cd ../andrewrahman-com && npm test -- tests/a11y.spec.ts` | ❌ W0 | ⬜ pending |
| 03-06-02 | 06 | 3 | — (D-08) | — | N/A | contrast script | `cd ../andrewrahman-com && node scripts/contrast-check.mjs` (optional) | ❌ W0 (optional) | ⬜ pending |
| 03-07-01 | 07 | 3 | WEB-03 | — | N/A | metadata | `cd ../andrewrahman-com && npm test -- tests/og-metadata.spec.ts` | ❌ W0 | ⬜ pending |
| 03-07-02 | 07 | 3 | WEB-03 | — | N/A | metadata + fs | `cd ../andrewrahman-com && npm test -- tests/og-metadata.spec.ts -g "og:image"` | ❌ W0 (part of above) | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

> Plan IDs above are placeholders — the gsd-planner may restructure. The Req → Test mapping is authoritative.

---

## Wave 0 Requirements

- [ ] `cd ../andrewrahman-com && npm install --save-dev @axe-core/playwright` — install axe-core runner
- [ ] `tests/a11y.spec.ts` — NEW. axe-core scan of `/`, `/privacy`, `/get-osd` with WCAG 2.1 AA ruleset
- [ ] `tests/og-metadata.spec.ts` — NEW. Assertions on `og:title`, `og:description`, `og:image`, `twitter:card`; verify OG image ≥1200×630
- [ ] `tests/asset-existence.spec.ts` — NEW. Filesystem check: 11 screenshot PNGs + `sml-logo.svg` + `andrew.jpg` exist with non-zero size
- [ ] Extend `tests/homepage-content.spec.ts` with: hero 4-stat assertions (D-01), 6-feature-card assertion (D-03), patreon colour regal (D-12), SML logo is `.svg` (D-11), headshot image present (D-15), negative assertion for `andrewjrahman@gmail.com`
- [ ] Extend `tests/privacy-content.spec.ts` with: 6× updated email assertions (D-13), negative assertion for legacy email, privacy-policy version-bump date assertion
- [ ] Phase 2 sync check — verify Sender.net vs Tally state in site repo before Wave 3 inline-scroll work (D-14); if Tally refs remain, Phase 3 absorbs migration (Option B); otherwise consumes Phase 2's work (Option A)
- [ ] Verify `OpenSpatialDelay v1.0.0.component` AU plugin installed in `~/Library/Audio/Plug-Ins/Components/` (bare `v1.0.component` is a legacy pre-release — don't rely on it); if missing, run `bash scripts/build_version.sh 768c248 v1.0.0 O100` in plugin repo before Wave 2 screenshot capture

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Social preview card renders correctly on Twitter/X and LinkedIn after deploy | WEB-03 | Live-scrape of social platforms requires auth + rate limits + TTL caches; low-frequency post-deploy manual check is cheaper than automation | After Netlify deploy with updated OG image: paste deploy URL into `https://www.opengraph.xyz/` and `https://metatags.io/`; visually confirm title, description, and image render. Optional but encouraged: paste into a Twitter DM and LinkedIn post composer to verify actual platform rendering. |
| Headshot visual sharpness at 560×560 on Retina displays | WEB-01 (D-15) | Source is 819×1024 — below 2× retina target (1120×1120). Axe/Playwright cannot judge perceptual softness | After swap, load `/` on a Retina display at 100% zoom, inspect `<AboutAndrew>` headshot region, confirm no visible softness. If soft: request higher-res source from user or accept per research §Pitfall 6 |
| Plugin v1.0.0 UI capture fidelity | WEB-01 (D-10) | Requires live DAW session (REAPER) with correct plugin version loaded | Launch REAPER, open blank project, insert `OpenSpatialDelay v1.0.0` (AU or VST3 — NOT the legacy `v1.0` pre-release, NOT post-release `v1.0.1`/`v1.0.2`), confirm plugin window title reads exactly "OpenSpatialDelay v1.0.0", capture 11 screenshots per UI-SPEC asset list using `Cmd+Shift+4+Space`, save with naming convention `screenshot_<name>.png` to `../andrewrahman-com/public/assets/` |
| Patreon CTA colour renders as light regal lavender (not rose) | WEB-01 (D-12) | Playwright can assert computed style, but final visual verification requires human eyes on actual Netlify deploy | After Wave 3 deploy, visit `/#support-on-patreon` section on Netlify preview, confirm PatreonCTA background is light purple/lavender and NOT rose pink. Also verify Nav and Hero ghost-buttons match |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references (3 NEW Playwright spec files + axe install + existing-spec extensions)
- [ ] No watch-mode flags (Playwright runs one-shot by default)
- [ ] Feedback latency < 30s (full suite) / < 5s (scoped)
- [ ] `nyquist_compliant: true` set in frontmatter (after gsd-nyquist-auditor pass)

**Approval:** pending
