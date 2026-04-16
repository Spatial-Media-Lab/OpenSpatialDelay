---
phase: 03
slug: personal-website
type: research
status: ready_for_planning
date: 2026-04-16
researcher: gsd-phase-researcher
confidence: HIGH
---

# Phase 03 — Personal Website — Research

**Researched:** 2026-04-16
**Domain:** Next.js 16 static-export marketing page finalisation (sibling repo `andrewrahman-com/`)
**Confidence:** HIGH — nearly all decisions pre-locked in CONTEXT.md + UI-SPEC.md; research scope was narrow and surgical.

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

- **D-01** Hero grid keeps 4 stats; locked values: `12 Delay Taps`, `7 Spatialization Algorithms`, `5 HRTF Profiles` (+1 Simple mode footnote), `70 Factory Presets`.
- **D-02** Retire `±12 semitones`, `2048-sample FFT`, `GPL-3.0` from hero stats (GPL-3.0 stays on Download CTA + card 6).
- **D-03** Features 5 → 6 cards. Slate: (1) 12 taps in 3D space, (2) Trajectory engine, (3) Measured HRTFs (5 profiles), (4) 7 spatialization algorithms, (5) OSC in/out (ADM-OSC), (6) Free & open-source.
- **D-04** Drop the phase-vocoder Doppler card from the hero slate (feature still ships; moved to blog/demo marketing).
- **D-05** Hero framing stays producers-first, sound-designers-second — no tone rewrite.
- **D-06** Copy polish scoped to sections touched by new feature pitch only: hero headline, 6 new feature-card headlines + bodies, Download CTA headline. Full-page pass deferred.
- **D-07** Formal style-guide authoring deferred post-launch (candidate Phase 6).
- **D-08** In Phase 3, design-system work = **single task: WCAG-AA accessibility/contrast audit** on every colour pairing in `globals.css` and `page.tsx`.
- **D-09** Post-launch: style-guide lives first in `andrewrahman-com/`, migrates to `SpatialCore` later; Markdown + JSON; canonical values = plugin-wins-by-default unless AA audit forces recolour.
- **D-10** B-1 — all homepage screenshots re-captured from v1.0.0 build (commit `768c248`); coordinate with Phase 4 CONT-03 (capture once, reuse).
- **D-11** B-4 — copy `/Users/andrewrahman/Downloads/Web-Logo, white.svg` (8.6 KB, confirmed 2026-04-16) to `public/assets/sml-logo.svg`, swap `<Image>` src to SVG, delete PNG afterward.
- **D-12** B-5 — new token `--accent-regal: oklch(72% 0.15 290)` (≈ `#b49bd8`). Added *after* `--accent-rose` in `globals.css`. Paired with `--bg-void #03060b` dark text. Replaces `--accent-rose` in: `PatreonCTA()` background + Hero Patreon ghost button + Nav Patreon ghost button. `--accent-rose` stays in globals.css and in `AboutAndrew` section label + radial gradient.
- **D-13** B-6 — **full migration** of `andrewjrahman@gmail.com` → `andrew@spatialmedialab.org` across `app/page.tsx` (AboutAndrew), `app/privacy/page.tsx` (6 occurrences including Data Controller line), `tests/privacy-content.spec.ts`, `tests/homepage-content.spec.ts`. Privacy-policy version bump + changelog entry required.
- **D-14** B-7 — researcher investigates Sender.net inline embed feasibility (see §D-14 Disposition Recommendation below).
- **D-15** B-8 — download headshot from `https://spatialmedialab.org/about/`, save to `public/assets/andrew.jpg`, replace AR-monogram `<div>` with locked `<Image>` pattern (560×560, `aspect-square rounded-lg object-cover border`).

### Claude's Discretion

- Exact headline copy for the 6 feature cards and the Hero headline (within D-03 direction + D-05 tone).
- Micro-copy on the "+1 Simple mode (low CPU)" footnote/tooltip on the HRTF hero stat.
- Whether to nudge `oklch(72% 0.15 290)` by ≤2 lightness points during the AA audit (D-08) if contrast fails.

### Deferred Ideas (OUT OF SCOPE)

- Formal `DESIGN-SYSTEM.md` + `design-tokens.json` authoring.
- Patreon-generator (`docs/phase-02-evidence/patreon-graphics/_generate.py`) shared-token refactor.
- Plugin-palette alignment (lands in v1.1 if AA audit forces recolours).
- Full-page copy review (pipeline copy, Patreon headline, About Andrew copy, SysReq framing).
- Pitch-shifted Doppler as a dedicated feature card (keep in plugin; revisit for blog/demo video/KVR listing only).
- Visual regression / screenshot diff testing.
- WEB-02 — SML About page already complete at `spatialmedialab.org/about`; flip REQUIREMENTS.md WEB-02 to Complete.

</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| WEB-01 | Personal landing page live on Netlify — hero, 30s audio demo, download CTA, screenshots, system requirements, license statement. | FEATURES rewrite (§9 Existing patterns); asset refresh workflow (§4 Screenshot capture); email migration + accessibility audit (§3 AA mechanism). Demo-audio embed is Phase 4 deliverable; Phase 3 verifies the slot exists in `MediaSlots()`. |
| WEB-02 | SML About Us page updated with Andrew + board members. | **Struck from phase scope per CONTEXT.md §domain.** Verified complete at `spatialmedialab.org/about/` during discussion. Planner must flip REQUIREMENTS.md WEB-02 row to Complete; no andrewrahman.com work needed. |
| WEB-03 | Open Graph meta tags configured (og:title, og:image, og:description) for social link previews. | Current `app/layout.tsx` already has `openGraph` + `twitter` blocks (§5 OG verification); Phase 3 verifies image ≥1200×630 after screenshot refresh and adds Playwright assertions. |

</phase_requirements>

## Summary

- **Nearly every Phase 3 decision is already locked** (CONTEXT.md D-01…D-15, UI-SPEC.md 6/6 dimension-PASS). Research scope was narrow: resolve 7 technical unknowns, confirm patterns the planner must preserve, and propose Validation Architecture.
- **D-14 Sender.net disposition — RECOMMEND inline scroll** (`<EmailCaptureSection id="get-osd">` + anchor links). Sender.net embedded forms use a JS snippet (`universal.js`) that renders into a `<div class="sender-form-field">` and shows a configurable inline thank-you message in-place by default; redirect is opt-in, not required; double-opt-in is an email-side flow that works identically regardless of inline-vs-route. The blocker on B-7 (Tally-required dedicated route for redirect) no longer applies after the Phase 2 switch to Sender.net. **Important scheduling note**: the site repo still contains Tally references (`get-osd/page.tsx` uses `NEXT_PUBLIC_TALLY_FORM_ID`, CSP in `_headers` whitelists `tally.so`, privacy policy names Tally Technologies SRL as processor). Phase 2's plans 02-05/02-06 are incomplete (STATE.md: "5 of 7"). Phase 3 must coordinate with Phase 2 OR absorb the Sender.net migration; the cleanest read is **Phase 2 delivers Sender.net embed wiring; Phase 3 builds the inline `<EmailCaptureSection>` above `PatreonCTA` that consumes it**.
- **AA audit mechanism — RECOMMEND `@axe-core/playwright`** (v4.11.1) piggy-backed onto the existing Playwright suite at `andrewrahman-com/tests/`. One test file (`tests/a11y.spec.ts`), WCAG2A+2AA tags, no new CI infra. Manual-pair verification against the 8-pair matrix in UI-SPEC §Accessibility audit contract provides belt-and-braces for the pairings axe can only evaluate when actually rendered on-screen.
- **Screenshot workflow — v1.0.0 plugin already builds cleanly** at commit `768c248` (confirmed via `git log`, verified in `agent_docs/version_registry.md`). macOS Cmd-Shift-4 captures at 2× retina by default; for the OG image (≥1200×630 required), the existing `screenshot_full.png` is 1640×1160 — already passes, but must be re-shot against v1.0.0 UI. Coordination with Phase 4 CONT-03 is explicit in D-10: capture session happens once, PNGs land in both `public/assets/` (site) and Phase 4 deliverables.
- **OG metadata is mostly in place** — `layout.tsx` already has complete `openGraph` + `twitter` metadata blocks with correct dimensions (`1640×1160`). Phase 3 work is (a) new Playwright assertion that the `<meta property="og:*">` tags exist and point at `/assets/screenshot_full.png`, (b) manual verification via `opengraph.xyz` or `metatags.io` post-deploy, (c) add `qualities: [75]` to any new `<Image>` component that needs optimization (Next.js 16 requirement, per upstream docs — flagged for executor).

**Primary recommendation:** Execute Phase 3 as a 4-wave plan — (Wave 0) test-infra setup + Sender.net/Tally coordination check, (Wave 1) copy + stats + email migration (highest-safety edits), (Wave 2) assets (screenshots, logo SVG, headshot) + colour token + Patreon recolour, (Wave 3) AA audit + OG verification. The inline `<EmailCaptureSection>` conversion (D-14) slots into Wave 2 or early Wave 3 depending on Phase 2 progress.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Content / marketing copy | Static page (build-time) | — | Next.js `output: 'export'` — page rendered to flat HTML at build; no runtime server. |
| Image delivery | CDN / static (`public/assets/*`) | — | `images: { unoptimized: true }` set in `next.config.ts` — served as-is by Netlify, no runtime optimization. |
| OG metadata | Static page (build-time) | Netlify edge | Metadata injected by `layout.tsx` at build; Netlify edge serves with correct `Content-Type` + cache headers. |
| Email capture form | Browser (client component) | Sender.net (SaaS) | `get-osd/page.tsx` already `'use client'`; Sender.net `universal.js` renders into a client-side `<div>`. All submission + DOI + delivery happens on Sender.net's side. |
| Accessibility validation | Playwright (CI/local) | — | Dev/CI tooling only; no runtime impact. |
| Analytics / tracking | None | — | Privacy policy explicitly: "andrewrahman.com sets no cookies of its own, runs no analytics". Do not introduce tracking for Phase 3. |

## D-14 Disposition Recommendation

### The question
CONTEXT.md D-14: does Sender.net's post-submission redirect flow work from an inline embed on `app/page.tsx#get-osd`? If yes → add `<EmailCaptureSection id="get-osd">` above `PatreonCTA`, swap `/get-osd/` links to `#get-osd` smooth-scroll, decide disposition of `app/get-osd/page.tsx`. If no → leave as-is, strike B-7.

### Evidence

1. **Sender.net embed is a JS-driven inline widget.** The integration code is a `universal.js` snippet that injects a `<script>` tag and renders into a `<div class="sender-form-field" data-sender-form-id="…">`. Form submission is handled in-browser via Sender's JS; no full-page POST. [CITED: sender.net/help/lead-capture/create-an-embedded-form/, help.sender.net/knowledgebase/creating-your-first-embedded-form/]
2. **Post-submission behaviour is configurable and defaults to inline.** Sender's documentation: "If your form is embedded on a website, the redirect URL will open on the same page regardless of settings to open it in a new tab." Inline success message is the default; redirect is an opt-in field. [CITED: help.sender.net/knowledgebase/embedded-form/]
3. **Double-opt-in is an email-side flow independent of form location.** The DOI confirmation email is sent by Sender.net after the form submission — it does not matter whether the form lives inline on `/` or on a dedicated `/get-osd/` page. [CITED: help.sender.net/knowledgebase/double-opt-in/]
4. **The original B-7 blocker is specific to Tally.** Phase 2 already decided Tally → Sender.net (STATE.md "Phase 02: Email capture + newsletter tool = Sender.net"; site repo commit `61519c3` reverted Tally smoke test "tool changed to Sender.net"). The binary-handoff flow Phase 1 required a dedicated `/get-osd/` URL for was the Tally redirect target; **with Sender.net, download buttons land in the DOI confirmation email itself** (Phase 2 D-09 Option A — "download buttons embedded directly in the DOI confirmation email alongside the confirm button"). No dedicated landing URL is needed for handoff.

### Recommendation: **Inline `<EmailCaptureSection id="get-osd">`**

Add to `app/page.tsx` between `MediaSlots()` and `DownloadCTA()` (or between `DownloadCTA()` and `PatreonCTA()` — UI-SPEC allows either). Behaviour:

- Swap every `Link href="/get-osd/"` in `page.tsx`, `Nav.tsx`, and any `DownloadCTA` → `<a href="#get-osd">` (no `scroll-behavior: smooth` hack needed — Tailwind's default `scroll-smooth` on `<html>` handles it).
- **Keep `app/get-osd/page.tsx` as a campaign-landing shim** that renders the same `<EmailCaptureSection>` standalone. Reason: inbound Patreon/social posts may already link to `/get-osd/`, and keeping the route prevents broken backlinks. The shim is 15 lines — not a maintenance burden.
- `app/get-osd/page.tsx` changes: (a) drop the `'use client'` + `useEffect` Tally script loader, (b) import + render the shared `<EmailCaptureSection>` component, (c) no longer reference `NEXT_PUBLIC_TALLY_FORM_ID`.

### Coordination flag (CRITICAL for the planner)

Phase 2 plans 02-05 (Sender.net wiring) and 02-06 (Patreon) are **still incomplete** per STATE.md (2026-04-16T20:51:29 — "5 of 7 complete"). The site repo today has:
- `get-osd/page.tsx` still references `NEXT_PUBLIC_TALLY_FORM_ID` and loads `tally.so/widgets/embed.js`
- `_headers` CSP whitelist includes `https://tally.so https://widgets.tally.so`
- `privacy/page.tsx` names Tally Technologies SRL as processor (2 mentions + 1 tally.so/privacy link)

**The Sender.net integration code does not exist yet.** Two planner options:

- **Option A (preferred)** — Phase 2 delivers `<EmailCaptureSection>` component wired to Sender.net (form ID + DOI + CSP update). Phase 3 consumes it: imports into `page.tsx`, anchors `/get-osd/` links to it, reduces `app/get-osd/page.tsx` to a shim. Privacy-policy processor swap (Tally → Sender.net) belongs to Phase 2's plan.
- **Option B (fallback)** — Phase 3 absorbs the Sender.net migration if Phase 2 doesn't ship in time for the May-1st deadline. This expands Phase 3 scope (~1 day of work: Sender.net account setup, form creation, DOI template, CSP update, privacy-policy processor swap). Flag at Wave 0; decide based on Phase 2 progress.

Planner should add a Wave-0 "Phase 2 Sender.net status check" task that triages which path applies before Wave 1 starts.

## Accessibility Audit Mechanism

### Chosen approach
**`@axe-core/playwright` v4.11.1** + one new test file in the existing Playwright suite, WCAG2A+2AA tag scope, with manual-pair verification against the 8 token-pair matrix in UI-SPEC §Accessibility audit contract.

### Rationale
- Site repo already has `@playwright/test ^1.59.1` (`package.json`) and an active test suite at `tests/` with `playwright.config.ts` running against the static-export `out/` dir via `serve`. Zero new CI infrastructure.
- `@axe-core/playwright` is the de-facto standard for WCAG coverage in Playwright — chainable API, `AxeBuilder` + `.withTags(['wcag2a','wcag2aa'])`. [CITED: playwright.dev/docs/accessibility-testing; npmjs.com/package/@axe-core/playwright]
- Axe's `color-contrast` rule can only evaluate text that's actually rendered and visible — it won't catch *potential* mismatches (e.g. tokens that exist in globals.css but aren't used on the current route). The 8-pair table in UI-SPEC is the complementary manual audit; running both gives coverage.
- `jest-axe` is NOT recommended — no Jest setup exists in the repo; adding one just for a11y doubles test surface.
- CLI `axe-core` + screenshot diffing was considered and rejected: adds a new tool, doesn't meaningfully improve coverage over the Playwright runner.

### Installation
```bash
npm install --save-dev @axe-core/playwright
# Verified 2026-04-16: @axe-core/playwright 4.11.1 (latest), axe-core 4.11.3
```

### Code skeleton

`/Users/andrewrahman/conductor/repos/andrewrahman-com/tests/a11y.spec.ts`:

```typescript
import { test, expect } from '@playwright/test';
import AxeBuilder from '@axe-core/playwright';

test.describe('WCAG 2.1 AA accessibility', () => {
  for (const path of ['/', '/privacy', '/get-osd']) {
    test(`${path} has no axe violations`, async ({ page }) => {
      await page.goto(path);
      const results = await new AxeBuilder({ page })
        .withTags(['wcag2a', 'wcag2aa'])
        .analyze();
      expect(results.violations).toEqual([]);
    });
  }

  test('homepage color-contrast on all token pairs', async ({ page }) => {
    await page.goto('/');
    // Axe scans only rendered elements — this test enforces the subset actually on-page.
    const results = await new AxeBuilder({ page })
      .withRules(['color-contrast'])
      .analyze();
    expect(results.violations).toEqual([]);
  });
});
```

### Manual-pair verification
For each of the 8 pairings in UI-SPEC.md §Accessibility audit contract, a simple Node-script (run once, not in CI) can compute WCAG contrast ratios via the formula `(L1 + 0.05) / (L2 + 0.05)` where `L = relative luminance`. If a pair fails, adjust the token, record the new hex in UI-SPEC.md, and annotate `globals.css` with `/* Phase 3 AA fix */`. This is a one-time sanity check, not an ongoing test.

### Expected violations
Based on the hex values in UI-SPEC.md, the likely-borderline pairs (from that spec's own annotations) are:
- `--text-dim` (#6d7279) on `--bg-void` (#03060b) — likely borderline 4.5:1
- `--text-dim` (#6d7279) on `--bg-panel` (#0a0d12) — likely borderline 4.5:1
- `--bg-void` (#03060b) on `--accent-regal` (≈ #b49bd8) — verify before shipping

## Screenshot Capture Workflow

### Plugin state — v1.0.0 already built
- Confirmed via `agent_docs/version_registry.md` + `git log 768c248`: release commit `768c248` exists; v1.0.0 / O100 is the canonical build.
- Release binaries already installed in `~/Library/Audio/Plug-Ins/Components/` (v0.1–v0.5 present; v1.0 needs verification via `bash scripts/build_version.sh 768c248 v1.0.0 O100` if not already installed). Planner should add a Wave-0 verification task.

### macOS capture commands
```bash
# Full-window (Retina 2× default) — use for hero + OG image
# Cmd-Shift-4 then SPACE, click the plugin window. Saves to ~/Desktop as PNG @ 2x DPI.

# CLI alternatives (deterministic for scripting):
screencapture -R <x,y,w,h> ~/Desktop/osd_capture.png       # region
screencapture -l <window-id> ~/Desktop/osd_capture.png     # specific window (use `osascript` to get window ID)
screencapture -T 3 ~/Desktop/osd_capture.png               # 3s delay
```

### Retina handling
- macOS screenshots on Retina displays are saved at 2× the logical pixel count with `dpiWidth=72` metadata. [CITED: mybyways.com/blog/fixing-high-dpi-retina-screenshot-metadata]
- For the OG image (`screenshot_full.png`), the existing file is 1640×1160 @ 72dpi — passes the WEB-03 minimum (≥1200×630) comfortably. New capture should match or exceed this.
- Browsers upscale PNGs automatically; there's no need to tag @2x in the filename for web delivery. The site's `next.config.ts` sets `images: { unoptimized: true }` so PNGs ship as-authored — capture at native 2×, deliver as-is.

### Asset naming (per UI-SPEC §Asset Contract)
```
screenshot_full.png           ← hero + OG (≥1200×630, recommended ≥1640×1160 to keep existing ratio)
screenshot_spatial_map.png    ← feature card 1 (12 taps in 3D space)
screenshot_orbit.png          ← feature card 2 (trajectory engine)
screenshot_elevation_map.png  ← feature card 3 (measured HRTFs)
# card 4 (algorithms) — no dedicated screenshot per UI-SPEC; uses colour dot + body copy
# card 5 (OSC) — no dedicated screenshot; tie to DAW screenshot if one exists
# card 6 (open-source) — uses sml-logo.svg, not a screenshot
screenshot_wobble.png         ← Screenshots row (Doppler card is DROPPED from features, still valid here)
screenshot_header.png         ← Screenshots row
screenshot_bottom_panel.png   ← Screenshots row
screenshot_right_panel.png    ← Screenshots row
screenshot_shimmer.png        ← Screenshots row
screenshot.png                ← Screenshots row (generic)
signal-flow.png               ← SignalFlow section
```

### Phase 4 coordination (CONT-03)
D-10 explicitly: "captures must be done once and reused across both phases". Planner sequences:
1. Wave 2 opens capture session in REAPER, plugin loaded, v1.0.0 UI rendered.
2. Capture all 11 PNGs in one sitting.
3. Deposit to `andrewrahman-com/public/assets/` (overwrite existing) AND to `docs/phase-04-evidence/screenshots/` (or wherever Phase 4 CONT-03 lands — verify when Phase 4 plans write).
4. Commit to both repos.

Avoid capturing twice — Phase 4 must inherit Phase 3's files, not re-capture.

## OG Metadata Verification

### Current state of `app/layout.tsx`
Already complete and correct. Lines relevant:
```typescript
export const metadata: Metadata = {
  title: 'Andrew Rahman — OpenSpatialDelay & the Spatial Media Library',
  description: '3D spatial delay plugin for VST3 + AU on macOS & Windows. ...',
  metadataBase: new URL('https://andrewrahman.com'),
  openGraph: {
    title: '...',
    description: '...',
    images: [{ url: '/assets/screenshot_full.png', width: 1640, height: 1160, alt: '...' }],
    type: 'website',
  },
  twitter: {
    card: 'summary_large_image',
    title: '...', description: '...',
    images: ['/assets/screenshot_full.png'],
  },
};
```

### Phase 3 work
Nothing *in* the metadata block changes — it's already correct and WEB-03-compliant. Phase 3 additions:
1. **Playwright assertion** that `<meta property="og:title">`, `<meta property="og:description">`, `<meta property="og:image">` exist and contain expected values.
2. **Screenshot refresh** (§4) — regenerates `/assets/screenshot_full.png`. Since the `<meta property="og:image">` references the file by path, nothing else needs updating.
3. **Manual verification post-deploy** via `https://opengraph.xyz/url/https%3A%2F%2Fandrewrahman.com` or `https://www.opengraph.xyz/` or `https://metatags.io/`. Do NOT require a live Twitter/X/LinkedIn share test (too fragile, not required by WEB-03).

### Test code
`tests/og-metadata.spec.ts`:
```typescript
import { test, expect } from '@playwright/test';

test.describe('Open Graph metadata (WEB-03)', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto('/');
  });

  test('og:title is set', async ({ page }) => {
    await expect(page.locator('head > meta[property="og:title"]'))
      .toHaveAttribute('content', /OpenSpatialDelay/);
  });

  test('og:description names the plugin and platforms', async ({ page }) => {
    await expect(page.locator('head > meta[property="og:description"]'))
      .toHaveAttribute('content', /VST3 \+ AU/);
  });

  test('og:image references a full-size screenshot (≥1200×630)', async ({ page }) => {
    await expect(page.locator('head > meta[property="og:image"]'))
      .toHaveAttribute('content', /\/assets\/screenshot_full\.png$/);
    await expect(page.locator('head > meta[property="og:image:width"]'))
      .toHaveAttribute('content', /^(1[2-9]\d\d|[2-9]\d{3,})$/); // ≥1200
  });

  test('twitter:card is summary_large_image', async ({ page }) => {
    await expect(page.locator('head > meta[name="twitter:card"]'))
      .toHaveAttribute('content', 'summary_large_image');
  });
});
```

Pattern: use `locator('head > meta[property="og:*"]')` + `toHaveAttribute('content', …)` — recommended over `getAttribute` for auto-waiting + retry. [CITED: sergiodxa.com/tutorials/test-meta-tags-using-playwright; playwright.dev/docs/api/class-locator]

## Playwright Test Update Strategy

### Current state (tests/)
Three spec files, each content-specific (not fixture-driven):
- `tests/smoke.spec.ts` — route-reachability + structural assertions (hero headline, Download CTA link, privacy headings, get-osd placeholder). Already contains `await expect(page.getByText('Tally form ID not yet configured')).toBeVisible();` — **breaks after Sender.net migration**.
- `tests/homepage-content.spec.ts` — content regressions; asserts `andrewjrahman@gmail.com` visible + as mailto link.
- `tests/privacy-content.spec.ts` — privacy content; asserts `andrewjrahman@gmail.com` and `Tally Technologies SRL`.

### Minimum-surface-area changes

**`tests/privacy-content.spec.ts`:**
- `'andrewjrahman@gmail.com'` → `'andrew@spatialmedialab.org'` (1 match, in the "lists rights-request contact email" test).
- `'Tally Technologies SRL'` → TBD (likely `'UAB Sender.lt'` based on Phase 2 decision; verify in Phase 2's privacy-policy rewrite plan).

**`tests/homepage-content.spec.ts`:**
- All occurrences of `andrewjrahman@gmail.com` → `andrew@spatialmedialab.org` (3 matches: body text, role=link name, href attribute). Mailto href: `'mailto:andrew@spatialmedialab.org'`.

**`tests/smoke.spec.ts`:**
- After Sender.net migration: replace `await expect(page.getByText('Tally form ID not yet configured')).toBeVisible();` with a new assertion on the Sender.net form container (likely `page.locator('.sender-form-field')` or a form-present heading). Details TBD once Phase 2 wires Sender.net.
- If Phase 3 ends up running inline (D-14 recommendation), the get-osd Playwright assertion may become `page.goto('/')` + `page.locator('#get-osd')` visibility instead.

**New test files:**
- `tests/a11y.spec.ts` — axe-core WCAG AA (§3).
- `tests/og-metadata.spec.ts` — WEB-03 verification (§5).

### Privacy-policy version bump (D-13 — REQUIRED)
`app/privacy/page.tsx` header: `<p className="text-sm text-text-secondary">Effective: 2026-04-16</p>` — bump to Phase 3 execution date AND add a §Changes paragraph noting: "2026-04-XX — controller-of-record contact email updated from `andrewjrahman@gmail.com` to `andrew@spatialmedialab.org` (Andrew Rahman remains the data controller)." This is GDPR best-practice for controller-contact changes. UI-SPEC D-13 calls this out explicitly.

## SVG Handling in Next.js 16

### Verified behaviour
Next.js 16's `<Image>` component:
1. **Automatically sets `unoptimized` when `src` ends with `.svg`** — confirmed in official docs. [CITED: nextjs.org/docs/app/api-reference/components/image]
2. No additional CSP or `dangerouslyAllowSVG` flag needed when SVG is served from the same origin as the page (our case — `/assets/sml-logo.svg` under `public/`).
3. **`images: { unoptimized: true }` in `next.config.ts`** (already set) means *all* images bypass the optimizer regardless of extension — SVG behaviour is therefore a no-op special case. Everything already renders as-authored.
4. **Next.js 16 `qualities` requirement** does NOT apply to SVGs (they're unoptimized) or to any image with `unoptimized` prop. Phase 3 can ignore this. [CITED: nextjs.org/docs/messages/invalid-images-config]

### Action
D-11 swap is trivial:
```tsx
// Before
<Image src="/assets/sml-logo.png" alt="..." width={180} height={180} />
// After
<Image src="/assets/sml-logo.svg" alt="..." width={180} height={180} />
```
No `unoptimized` prop needed (set globally), no code-level changes beyond the `src` string. After SVG is in `public/assets/` and swap verified visually in dev, delete `public/assets/sml-logo.png`.

**Security note:** the logo SVG is a locally-authored file from the user (verified 8.6KB, confirmed 2026-04-16) — no third-party SVG ingest, no XSS risk. Standard `img-src 'self' data: https:` CSP in `_headers` already covers it.

## Headshot Asset Provenance

### Source verified
- URL: `https://spatialmedialab.org/wp-content/uploads/2026/04/Andrew-Rahman-819x1024.jpg`
- Format: JPG (WordPress-hosted, likely progressive JPEG)
- Natural dimensions: **819 × 1024** (portrait)
- Image-rights: clean per D-15 ("user owns the SML site; the headshot is his own")

### Dimension analysis
- UI-SPEC D-15 locked target: `<Image … width={560} height={560} />` (square aspect with `aspect-square` CSS).
- Source is portrait 819×1024 — 0.80 aspect ratio, not square.
- With `className="aspect-square ... object-cover"`, the browser will centre-crop vertically. Acceptable if the face is well-centred in the source — **verify visually after swap**; if subject is too high/low in frame, either re-crop the source with `sips` or adjust `object-position`.

### Recommended workflow
```bash
# Download
curl -o /Users/andrewrahman/conductor/repos/andrewrahman-com/public/assets/andrew.jpg \
  https://spatialmedialab.org/wp-content/uploads/2026/04/Andrew-Rahman-819x1024.jpg

# Verify dimensions
sips -g pixelWidth -g pixelHeight /Users/andrewrahman/conductor/repos/andrewrahman-com/public/assets/andrew.jpg

# Optional: if crop is off, crop to square centred on face
# sips -c 819 819 public/assets/andrew.jpg --out public/assets/andrew.jpg
```

The 819×1024 source is smaller than the target 560×560 *display* size but render at 2× retina (1120×1120) — the JPEG won't have enough resolution for 2× and may look soft. **Consider asking user if a higher-res original exists**, or accept the softness (the Image element down/up-samples). Flag for executor: check visual quality after swap; do not block the phase on it.

## Existing Patterns to Preserve

The planner MUST maintain these conventions. Violating them signals "foreign code" to the executor and breaks silhouette.

### Data-driven arrays (CRITICAL)
```typescript
// app/page.tsx lines 10–51
const FEATURES = [
  { tapIndex: 0, title: '...', body: '...', image: '/assets/...', alt: '...' },
  // 5 → 6 entries (D-03)
] as const;
```
- Rewrite the *entries*, not the `Features()` component. The renderer consumes the array.
- Keep `tapIndex` values for visual rainbow spread (UI-SPEC: 0, 3, 5, 7, 9, 11 for the 6 new cards).

### Section-component architecture
13 top-level components in `HomePage()` (Hero, RainbowStrip, Features, SignalFlow, Screenshots, Pipeline, AboutAndrew, MediaSlots, SysReq, DownloadCTA, PatreonCTA + Nav from sibling). Phase 3 adds at most ONE new section: `<EmailCaptureSection id="get-osd">` (conditional on D-14/Phase 2).

### Token consumption via `style={{ … : 'var(--…)' }}`
Every colour on every element is expressed as `style={{ borderColor: 'var(--accent-rose)' }}`, not `className="border-rose-500"`. D-12 replaces `var(--accent-rose)` with `var(--accent-regal)` in exactly 3 places (PatreonCTA bg, Hero ghost button, Nav ghost button) — surgical find-replace, NOT className rewrites.

### Image treatment
```tsx
className="aspect-square rounded-lg object-cover border"
style={{ borderColor: 'var(--border-subtle)' }}
```
This is the house style. D-15 reuses it verbatim. Do NOT add shadows, inner glows, or wrapper `<figure>` elements — those appear nowhere else in the codebase.

### Mailto + inline link style
```tsx
<a href="mailto:..." className="underline underline-offset-4" style={{ color: 'var(--accent-stellar)' }}>...</a>
```
D-13 email swap preserves this pattern — only the string changes.

### Next.js 16 breaking-change awareness
`../andrewrahman-com/AGENTS.md` contains: *"This is NOT the Next.js you know. This version has breaking changes — APIs, conventions, and file structure may all differ from your training data. Read the relevant guide in `node_modules/next/dist/docs/` before writing any code."* The planner must carry this warning forward. Most relevant for Phase 3: `next.config.ts` types (`NextConfig` already imported correctly), `<Image>` `qualities` requirement (N/A — `unoptimized` globally), `Metadata` type from `next` (already used correctly).

### CSS token architecture
- `globals.css` `:root { … }` block = plugin-palette source of truth.
- Tailwind 4 `@theme inline { --color-void: var(--bg-void); … }` block = bridges tokens to Tailwind's utility generation (enables `bg-void`, `text-text-primary`, etc.).
- D-12: add `--accent-regal: oklch(...)` to `:root` AND expose via `@theme inline` as `--color-regal-osd: var(--accent-regal);` if any Tailwind utility class `text-regal-osd` / `bg-regal-osd` needs it. Current Patreon uses raw `style={{ color: 'var(--accent-rose)' }}` not a utility class, so the `@theme inline` entry may be unnecessary — verify by grepping for `rose-osd` utility usage (none expected).

## Validation Architecture

> `workflow.nyquist_validation` is absent from `.planning/config.json` — treated as enabled per protocol.

### Test Framework
| Property | Value |
|----------|-------|
| Framework | Playwright 1.59.1 (site repo) |
| Config file | `/Users/andrewrahman/conductor/repos/andrewrahman-com/playwright.config.ts` |
| Quick run command | `npm test -- tests/homepage-content.spec.ts` (from site repo) |
| Full suite command | `npm test` (from site repo) |
| Phase gate | Full Playwright suite + axe-core + og-metadata green before `/gsd-verify-work` |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| WEB-01 | Hero shows 4 updated stats (12 Delay Taps, 7 Algorithms, 5 HRTF, 70 Presets) | content regression | `npm test -- tests/homepage-content.spec.ts -g "hero stats"` | ❌ Wave 0 — extend existing file |
| WEB-01 | 6 feature cards rendered (up from 5) | unit/content | `npm test -- tests/homepage-content.spec.ts -g "features"` | ❌ Wave 0 — new assertions |
| WEB-01 | All 11 screenshot PNGs exist in `public/assets/` with non-zero size | filesystem | `npm test -- tests/asset-existence.spec.ts` | ❌ Wave 0 — NEW file |
| WEB-01 | Contact email is `andrew@spatialmedialab.org` (D-13) | content regression | `npm test -- tests/homepage-content.spec.ts -g "contact email"` | ✅ exists, UPDATE assertion |
| WEB-01 | Privacy policy email = `andrew@spatialmedialab.org` | content regression | `npm test -- tests/privacy-content.spec.ts -g "rights-request"` | ✅ exists, UPDATE assertion |
| WEB-01 | Patreon CTA uses `var(--accent-regal)`, NOT `var(--accent-rose)` | rendered style | `npm test -- tests/homepage-content.spec.ts -g "patreon.*regal"` | ❌ Wave 0 — new assertion |
| WEB-01 | Headshot image loads and is not placeholder | DOM presence | `npm test -- tests/homepage-content.spec.ts -g "headshot"` | ❌ Wave 0 — new assertion |
| WEB-01 | SML logo is the SVG, not the PNG | DOM attribute | `npm test -- tests/homepage-content.spec.ts -g "SML logo.*svg"` | ❌ Wave 0 — new assertion |
| WEB-01 | Download CTA links to `/get-osd/` OR `#get-osd` (D-14 outcome) | DOM | existing `smoke.spec.ts` assertion, UPDATE if inline | ✅ exists, maybe-update |
| WEB-02 | **Struck from phase** — already complete at `spatialmedialab.org/about` | — | REQUIREMENTS.md row flip only | — |
| WEB-03 | `og:title`, `og:description`, `og:image`, `twitter:card` correctly set | metadata assertion | `npm test -- tests/og-metadata.spec.ts` | ❌ Wave 0 — NEW file |
| WEB-03 | OG image is `/assets/screenshot_full.png` and ≥1200×630 | metadata + filesystem | `npm test -- tests/og-metadata.spec.ts -g "og:image"` | ❌ Wave 0 — part of above |
| D-08 | WCAG 2.1 AA violations = 0 on `/`, `/privacy`, `/get-osd` | axe-core | `npm test -- tests/a11y.spec.ts` | ❌ Wave 0 — NEW file |
| D-08 | 8 token-pair contrast matrix (UI-SPEC) all pass WCAG | one-off script | `node scripts/contrast-check.mjs` (ad-hoc) | ❌ Wave 0 — optional script |

### Sampling Rate
- **Per task commit:** `npm test -- tests/<changed-file>.spec.ts` (fast, scoped).
- **Per wave merge:** `npm test` (full Playwright suite, ~30s on local hardware).
- **Phase gate:** full suite green + axe violations = 0 + og-metadata all assertions pass before `/gsd-verify-work`.

### Wave 0 Gaps

- [ ] **Install `@axe-core/playwright`** — `npm install --save-dev @axe-core/playwright` in site repo.
- [ ] `tests/a11y.spec.ts` — axe-core WCAG 2 AA (§3 skeleton).
- [ ] `tests/og-metadata.spec.ts` — WEB-03 verification (§5 skeleton).
- [ ] `tests/asset-existence.spec.ts` — filesystem check for all 11 PNGs + sml-logo.svg + andrew.jpg non-zero size.
- [ ] Extend `tests/homepage-content.spec.ts` with: hero stats, feature count, patreon colour, headshot, SML logo SVG.
- [ ] **Phase 2 progress check** — verify whether Sender.net is wired before Phase 3 starts touching `get-osd/page.tsx`. Informational task, no file artefact.
- [ ] Determine Phase 2 Sender.net processor name (`UAB Sender.lt, Vilnius, Lithuania` per STATE.md) for privacy-policy processor-swap assertion update.

## Project Constraints (from CLAUDE.md)

### Plugin-side directives (mostly N/A for Phase 3, but relevant for screenshot capture)

- **Never use `sudo`.** Screenshot capture + file moves in `public/assets/` don't require sudo.
- **Never run `killall AudioComponentRegistrar`** — destroys AU cache. AU plugin does not need to be reloaded for screenshot captures.
- **Build versioned plugins** — `bash scripts/build_version.sh <commit> vX.Y.Z OXYZ`. For screenshot capture of v1.0.0, verify `OpenSpatialDelay v1.0.0.component` is installed in `~/Library/Audio/Plug-Ins/Components/` before capturing (the bare `v1.0.component` on disk is a legacy pre-release iteration, NOT the shipped binary). If missing: `bash scripts/build_version.sh 768c248 v1.0.0 O100`.
- **Never reuse a version number.** Not applicable to Phase 3 (no plugin code changes).
- **VST3 + AU only.** Phase 3 screenshots should show the plugin in REAPER/AU form (user's standard A/B test environment).

### Site-repo directives (from `../andrewrahman-com/AGENTS.md`)

- **"This is NOT the Next.js you know. … Read the relevant guide in `node_modules/next/dist/docs/` before writing any code. Heed deprecation notices."** — carry forward to all executor agents. Particular care on `<Image>`, `Metadata` types, and `next.config.ts`.

## Runtime State Inventory

Phase 3 is *not* a rename/refactor/migration phase in the datastore sense — but the email-address migration (D-13) and Tally → Sender.net coordination touch runtime state worth itemising explicitly.

| Category | Items Found | Action Required |
|----------|-------------|------------------|
| Stored data | **None in Phase 3 scope.** No databases touched. Sender.net subscriber list (if already populated by Phase 2) does not care about this phase. | None. |
| Live service config | (1) **Tally form** — to-be-deleted/superseded by Phase 2's Sender.net migration. Phase 3 assumes Phase 2 has handled this. (2) **Netlify deploy** — env var `NEXT_PUBLIC_TALLY_FORM_ID` currently referenced in `app/get-osd/page.tsx` — will be removed when Sender.net inline migration lands. (3) **Netlify `_headers` CSP** — whitelists `https://tally.so` + `https://widgets.tally.so`; must be updated to whitelist `https://cdn.sender.net` (or whichever domain Sender.net uses for `universal.js`) and remove Tally. | Phase 2 (Option A) or Phase 3 (Option B fallback) — see §D-14. |
| OS-registered state | `OpenSpatialDelay v1.0.0.component` AU plugin — needs to be installed for screenshot capture. Verify via `ls ~/Library/Audio/Plug-Ins/Components/ \| grep 'v1.0.0.component'`. (The bare `v1.0.component` is a distinct legacy pre-release binary — don't conflate.) | Wave 0 verification task; if missing, run `bash scripts/build_version.sh 768c248 v1.0.0 O100`. |
| Secrets/env vars | `NEXT_PUBLIC_TALLY_FORM_ID` (referenced in `get-osd/page.tsx`, `.env.example`) — obsolete. Likely replaced by `NEXT_PUBLIC_SENDER_FORM_ID` (Phase 2 naming TBD). | Coordinate with Phase 2 on final env-var name; update `.env.example` and Netlify env config. |
| Build artifacts | `andrewrahman-com/out/` (static-export dir), `andrewrahman-com/.next/` (cache) — regenerated per build, no Phase 3 action required. | None. |

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| Node.js | Next.js build + Playwright | ✓ (assumed; verify at Wave 0) | project uses `^20` per `@types/node` | — |
| npm | Install `@axe-core/playwright` | ✓ | bundled with Node | — |
| `cmake` | v1.0.0 plugin build (for screenshots) | ✓ (per CLAUDE.md build instructions) | assumed current | — |
| REAPER | Host plugin for screenshot capture | ✓ (user's A/B testing DAW) | assumed current | any VST3/AU host (Logic, AU Lab) |
| `screencapture` (macOS) | OS screenshot tool | ✓ (built-in) | system | Shottr, RetinaCapture (overkill) |
| `curl` | Download headshot + SML logo (already local) | ✓ | system | `wget`, browser download |
| `sips` | Verify/resize image dimensions | ✓ (macOS built-in) | system | ImageMagick `convert` |
| Sender.net account + form ID | D-14 inline embed | **Depends on Phase 2 status** | — | **Phase 3 absorbs Sender.net setup if Phase 2 incomplete by Wave 2** |
| `@axe-core/playwright` | AA audit | ✗ (not yet installed) | target: 4.11.1 | jest-axe (rejected — no Jest setup); manual contrast-only script |

**Missing dependencies with no fallback:** None blocking. `@axe-core/playwright` installed in Wave 0 via `npm install --save-dev`.

**Missing dependencies with fallback:** Sender.net account (if Phase 2 incomplete) → Phase 3 falls back to absorbing Sender.net migration in Option B.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Contrast ratio calculation | Custom WCAG-formula JS | `@axe-core/playwright` `color-contrast` rule | Axe covers relative-luminance formula, text-size gates, font-weight exemptions, pseudo-element contrast — all subtle and historically wrong in hand-rolled checkers. |
| Accessibility audit in general | Custom DOM traversal + ARIA checks | `@axe-core/playwright` full-rule scan | 90+ WCAG rules maintained by Deque (industry-standard); hand-rolling is weeks of work. |
| Email capture form + DOI + download delivery | Custom React form + SMTP | Sender.net embed (Phase 2) | Already the Phase 2 locked decision. DOI flow, compliance, deliverability are solved domains. |
| OG tag parsing for validation | Custom HTML parser | Playwright locator (`head > meta[property="og:*"]`) | Browser handles the parsing correctly. |
| Social-preview live-test | Automated Twitter/LinkedIn scrape | Manual visit to `opengraph.xyz` or `metatags.io` | Low-frequency manual check. Live social-platform tests require auth + rate limits + rendering latency (TTL caches on FB/LI). |
| SVG optimization | Custom build step | Next.js `<Image>` + `unoptimized` prop (auto for `.svg`) | Already set globally via `next.config.ts`. |
| Custom contrast-fix colour picker | Eyeballing OKLCH values | `oklch(L% C H)` + axe re-run loop | UI-SPEC locked starting value; audit either passes or nudges lightness ±2 points. |

## Common Pitfalls

### Pitfall 1: Axe violations on pages that look fine
**What goes wrong:** Axe flags `--text-dim` on `--bg-panel` (or similar borderline pair) as failing 4.5:1 ratio.
**Why it happens:** These tokens were never audited; the plugin-palette heritage assumes chroma readings that weren't designed against WCAG.
**How to avoid:** Expect 1–3 real violations in first run (UI-SPEC flags the borderline pairs). Don't catastrophise — fix by nudging lightness ±2 OKLCH points until the rule passes, then record the new hex in UI-SPEC.md and annotate `globals.css` with `/* Phase 3 AA fix */`.
**Warning signs:** "First axe run, 40 violations" — means something systemic is wrong (e.g., test running against an un-styled page, static export cache serving stale CSS). Don't fix 40 symptoms; fix the one cause.

### Pitfall 2: Screenshots from wrong plugin version
**What goes wrong:** REAPER has multiple OpenSpatialDelay versions installed (v0.1 through v1.0.2). Accidentally capturing UI from a legacy or patch build breaks the v1.0.0 marketing commitment.
**Why it happens:** `OpenSpatialDelay v0.5.component`, `v0.4.component`, etc. are all present in `~/Library/Audio/Plug-Ins/Components/` per CLAUDE.md user-preference (A/B testing). The bare `v1.0.component` is a legacy pre-release iteration from before the 2026-04-12 squash and is distinct from the shipped `v1.0.0.component`. Post-release patches `v1.0.1.component` + `v1.0.2.component` also coexist.
**How to avoid:** Before capture session, verify the plugin window title reads exactly "OpenSpatialDelay v1.0.0" (matches `PLUGIN_NAME` set by `build_version.sh 768c248 v1.0.0 O100`). Capture only from that specific window — NOT from `v1.0`, NOT from `v1.0.1`/`v1.0.2`.
**Warning signs:** UI elements look familiar but parameter names/counts are slightly off from v1.0.0 spec; title bar reads `v1.0` or `v1.0.1`/`v1.0.2`.

### Pitfall 3: Email-migration assertion miss
**What goes wrong:** One of the six privacy-policy email occurrences (or one of the three homepage occurrences) is missed in find-replace; Playwright tests pass because they only assert *some* occurrences.
**Why it happens:** Manual find-replace on a long file; tests assert `toContainText` (substring) rather than `toHaveCount(0)` for the old address.
**How to avoid:** Add a **negative-assertion test** for each file: `await expect(page.locator('body')).not.toContainText('andrewjrahman@gmail.com')`. Belt-and-braces. Mirror the pattern already in `homepage-content.spec.ts` for the old patreon URL.
**Warning signs:** Test suite green, but `grep -rn andrewjrahman /Users/andrewrahman/conductor/repos/andrewrahman-com/app/` returns matches.

### Pitfall 4: SVG renders but is huge (no width constraint)
**What goes wrong:** SVG lacks a `viewBox` or the `<Image>` width/height props don't clamp it; logo renders at 100vw.
**Why it happens:** Next.js `<Image>` with `unoptimized` + SVG + missing intrinsic dimensions can fall back to browser default scaling.
**How to avoid:** Keep the existing `width={180} height={180}` props verbatim from the PNG swap. Next.js honours these as CSS pixel dimensions even for unoptimized SVG.
**Warning signs:** Browser-dev-tools shows `<img>` at natural SVG-internal viewport dimensions (likely 1024×1024 for a Web-Logo asset).

### Pitfall 5: Phase 2 Sender.net migration collides with Phase 3 inline work
**What goes wrong:** Phase 3 executor starts inline `<EmailCaptureSection>` work assuming Phase 2 has wired Sender.net, but Phase 2 is still in progress and the env var / CSP / privacy-policy processor haven't been updated.
**Why it happens:** Parallel phase execution; STATE.md shows Phase 2 at 5/7 complete; no hard dependency block.
**How to avoid:** Wave-0 task in Phase 3 plan reads the site repo's `get-osd/page.tsx` + `_headers` + `.env.example` + `privacy/page.tsx` and branches: if Tally references remain → Option B (absorb migration); if Sender.net references present → Option A (consume).
**Warning signs:** Commit `61519c3 revert: remove tally-embed test` is the *last* Sender.net-related commit in the site repo; nothing after it. Indicates migration not yet done.

### Pitfall 6: Headshot looks soft
**What goes wrong:** The 819×1024 source, displayed at 560×560 on a Retina screen (effectively 1120×1120 rendered), is slightly undersampled.
**Why it happens:** Retina density doubles required pixels; source doesn't meet it.
**How to avoid:** Check visual quality after swap; if noticeably soft, ask user for original resolution; otherwise accept — background-level asset, face still clearly readable.
**Warning signs:** User reviews preview and flags "the headshot looks blurry".

## Code Examples

### D-12 — PatreonCTA colour swap (after)
```tsx
// app/page.tsx — PatreonCTA()
<section
  className="py-16 sm:py-24 px-5 sm:px-8"
  style={{ background: 'var(--accent-regal)' }}  // was: var(--accent-rose)
>
  <Container>
    <h2
      className="font-display text-[40px] sm:text-[60px] leading-[1.02] mb-6"
      style={{ color: 'var(--bg-void)' }}  // dark text on light lavender
    >
      Fund the pipeline. Free + open-source forever.
    </h2>
    {/* … body + button … */}
    <a
      href={PATREON_URL}
      className="inline-flex items-center gap-2 rounded-full px-7 py-3.5 text-[15px] font-semibold"
      style={{ background: 'var(--bg-void)', color: 'var(--accent-regal)' }}  // button inverts
    >
      Support on Patreon →
    </a>
  </Container>
</section>
```

### D-13 — Contact email migration
```tsx
// app/page.tsx — AboutAndrew()
<p className="text-[15px] leading-[1.6]" style={{ color: 'var(--text-secondary)' }}>
  Get in touch —{' '}
  <a
    href="mailto:andrew@spatialmedialab.org"  // was: mailto:andrewjrahman@gmail.com
    className="underline underline-offset-4"
    style={{ color: 'var(--accent-stellar)' }}
  >
    andrew@spatialmedialab.org
  </a>
</p>
```

### D-15 — Headshot swap
```tsx
// app/page.tsx — AboutAndrew() (replace TODO div)
import Image from 'next/image';
// …
<Image
  src="/assets/andrew.jpg"
  alt="Andrew Rahman"
  width={560}
  height={560}
  className="aspect-square rounded-lg object-cover border"
  style={{ borderColor: 'var(--border-subtle)' }}
/>
```

### Axe test fixture
```typescript
// tests/a11y.spec.ts
import { test, expect } from '@playwright/test';
import AxeBuilder from '@axe-core/playwright';

const WCAG_AA_TAGS = ['wcag2a', 'wcag2aa', 'wcag21a', 'wcag21aa'] as const;

for (const path of ['/', '/privacy', '/get-osd']) {
  test(`${path} WCAG 2.1 AA`, async ({ page }) => {
    await page.goto(path);
    const { violations } = await new AxeBuilder({ page })
      .withTags([...WCAG_AA_TAGS])
      .analyze();
    expect(violations, JSON.stringify(violations, null, 2)).toEqual([]);
  });
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Tally.so form + redirect to GitHub Releases | Sender.net inline embed + DOI + download-in-email | 2026-04-16 (Phase 2 decision) | D-14 becomes "inline possible"; privacy policy processor changes; Phase 3 URL strategy shifts from route-navigation to anchor-scroll |
| `width`/`height` props optional on `<Image>` | Required in Next.js 16 (strict); `qualities` also required for optimized images | Next.js 16 (2025+) | Already handled in existing site code; Phase 3 follows the pattern |
| Hand-rolled WCAG contrast checkers | `@axe-core/playwright` | industry-standard 2020+ | Use the library; don't reinvent |
| `getAttribute` in Playwright assertions | `await expect(locator).toHaveAttribute(...)` | Playwright 1.30+ | Auto-waiting + retry; recommended in `tests/og-metadata.spec.ts` |

**Deprecated / outdated:**
- `andrewjrahman@gmail.com` as public contact — migrates to `andrew@spatialmedialab.org` (D-13).
- `--accent-rose` on Patreon CTA — becomes `--accent-regal` (D-12); token retained for `AboutAndrew` SectionLabel + radial gradient.
- Tally Technologies SRL as privacy-policy processor — migrates to UAB Sender.lt (Phase 2 responsibility; Phase 3 verifies).

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | `cdn.sender.net` is the domain the Sender.net `universal.js` loads from (referenced in CSP update recommendation). | Runtime State Inventory | Wrong subdomain → CSP blocks Sender.net script → inline form fails silently in prod. Mitigation: executor verifies actual URL from Sender.net integration snippet in Phase 2 before updating `_headers`. [ASSUMED — not verified in this research session; Phase 2 will have the exact snippet.] |
| A2 | Phase 4 CONT-03 screenshot workflow has not been designed yet; Phase 3 captures land in `public/assets/` only, with Phase 4 inheriting those files directly. | Screenshot workflow / Phase 4 coordination | If Phase 4 needs different framing/resolution (e.g., KVR wants 4K), Phase 3 captures may not be reusable → capture session happens twice. Mitigation: planner flags this in the plan as "confirm with Phase 4 plan when it exists". [ASSUMED — Phase 4 not yet planned.] |
| A3 | Test selector `page.locator('.sender-form-field')` will match the Sender.net form container on the homepage (naming convention from Sender.net docs). | Playwright test update strategy | Wrong selector → test assertion flakes. Mitigation: update selector once Phase 2 ships; the research skeleton is a starting point, not a final. [ASSUMED.] |
| A4 | Netlify custom-domain `andrewrahman.com` is DNS-pointed to the Netlify deploy currently at `silly-licorice-0ee82d.netlify.app`. | OG metadata verification | If not, `metadataBase: https://andrewrahman.com` is wrong and social previews will fail. Mitigation: Wave 0 or Phase 2 DNS verification. [ASSUMED — not confirmed in this session.] |
| A5 | Sender.net's subdomain for the form-script is served over HTTPS and doesn't impose a second CSP requirement beyond `script-src` whitelist. | D-14 / Runtime State | Could require `connect-src` or `frame-src` updates depending on Sender.net's implementation. [ASSUMED — Phase 2 will expose exact CSP needs.] |

**If this table is populated:** The planner should surface A1, A3, A5 as coordination points with Phase 2; A2 as coordination with Phase 4; A4 as a Wave-0 verification task.

## Open Questions (RESOLVED)

All four open questions from this research session have been resolved by the Phase 3 plan set. Each is annotated inline with the resolving plan and task.

1. **Is the v1.0.0 AU component already installed under `~/Library/Audio/Plug-Ins/Components/`?**
   - What we know: v0.1–v0.5 are there; v1.0.0 release commit exists (768c248); `build_version.sh` is the canonical installer.
   - What's unclear: whether anyone has run it for 768c248 yet.
   - Recommendation: Wave 0 task in Phase 3 plan — `ls ~/Library/Audio/Plug-Ins/Components/ | grep "v1.0"`; if absent, run the build command.
   - **RESOLVED:** Plan 00 Task 1 (step 2) runs `ls ~/Library/Audio/Plug-Ins/Components/ | grep -E 'OpenSpatialDelay v1\.0(\.component|$)'` and records "installed" or "BLOCKED pending build" in 03-00-SUMMARY.md before Wave 2 screenshot capture (Plan 04 Task 1 checkpoint) consumes the result.

2. **What is Phase 2's final privacy-policy processor name?**
   - What we know: Phase 2 CONTEXT.md D-XX (captured in STATE.md) says "UAB Sender.lt, Vilnius, Lithuania".
   - What's unclear: exact string Phase 2's plan will write into `privacy/page.tsx`.
   - Recommendation: Phase 3 test assertion defers to whatever Phase 2 writes. Coordinate via STATE.md.
   - **RESOLVED:** Plan 00 Task 1 (step 3) triages the current Tally/Sender.net state and records D-14 disposition (Option A vs B) in 03-00-SUMMARY.md. Plan 05 Task 1 fetches the exact `SENDER_SCRIPT` URL at runtime from Phase 2's SUMMARY (or from the live Sender.net dashboard embed code per the Task 1 Option B branch) and pins it in `components/EmailCaptureSection.tsx`. Plan 05 Task 3 (Option B branch) writes the final processor name "UAB Sender.lt, Vilnius, Lithuania" into `app/privacy/page.tsx`.

3. **Does the existing `screenshot_full.png` file need a dimension change?**
   - What we know: file is currently 1640×1160 @ 72dpi. OG minimum is 1200×630.
   - What's unclear: whether a wider aspect (e.g., 1920×1080 for better Twitter preview fidelity) would improve the hero card in-page.
   - Recommendation: executor's discretion during capture session; don't pre-optimize.
   - **RESOLVED:** Plan 04 Task 1 checkpoint instructs the user to verify `screenshot_full.png` >= 1200×630 via `sips -g pixelWidth -g pixelHeight` before signalling "captured". The checkpoint acceptance gate blocks Wave 2 completion until the OG floor is satisfied; any wider aspect remains the executor's discretion within the same step.

4. **Should `app/get-osd/page.tsx` be deleted or retained as a shim after D-14 inline conversion?**
   - What we know: D-14 says "either delete or keep as campaign-landing redirect to `/#get-osd`".
   - What's unclear: whether any inbound Patreon/social post already links to `/get-osd/`.
   - Recommendation: KEEP as a thin shim that renders the same component standalone. Zero-cost maintenance; prevents broken backlinks. (Encoded in §D-14 recommendation.)
   - **RESOLVED:** KEEP as a thin shim. Plan 05 Task 2 (step D) replaces the route body with `<EmailCaptureSection standalone headingLevel="h1" />` so backlinks to `/get-osd/` continue to 200 and the standalone page retains its required `<h1>`.

## Sources

### Primary (HIGH confidence)
- Playwright official docs — `playwright.dev/docs/accessibility-testing` (axe integration pattern)
- Next.js 16 official docs — `nextjs.org/docs/app/api-reference/components/image` (SVG auto-unoptimized; `qualities` requirement)
- Next.js 16 migration — `nextjs.org/docs/messages/invalid-images-config`
- Sender.net help — `help.sender.net/knowledgebase/embedded-form/`, `help.sender.net/knowledgebase/creating-your-first-embedded-form/`, `help.sender.net/knowledgebase/double-opt-in/`
- npm registry `npm view @axe-core/playwright version` → 4.11.1 (verified 2026-04-16)
- npm registry `npm view axe-core version` → 4.11.3 (verified 2026-04-16)
- npm registry `npm view @playwright/test version` → 1.59.1 (verified 2026-04-16 — matches `package.json`)
- Site repo `package.json`, `playwright.config.ts`, `next.config.ts`, `app/layout.tsx`, `app/page.tsx`, `app/privacy/page.tsx`, `app/get-osd/page.tsx`, `tests/*.spec.ts`, `components/Nav.tsx`, `_headers`, `AGENTS.md`
- Plugin repo `agent_docs/version_registry.md`, `scripts/build_version.sh`, `CLAUDE.md`
- `.planning/phases/03-personal-website/03-CONTEXT.md` (D-01…D-15 verbatim)
- `.planning/phases/03-personal-website/03-UI-SPEC.md` (component inventory, asset contract, accessibility audit matrix)
- `.planning/phases/02-email-capture-funding-infrastructure/02-CONTEXT.md` (Sender.net decision D-28)
- `.planning/STATE.md` (Phase 02 progress + Sender.net tool decision)
- `.planning/REQUIREMENTS.md` §WEB-01/02/03
- Live HTTP fetch of `https://spatialmedialab.org/about/` (headshot URL verified 2026-04-16)

### Secondary (MEDIUM confidence)
- sergiodxa.com/tutorials/test-meta-tags-using-playwright (og-metadata pattern — verified against Playwright official Locator docs)
- dev.to/subito + dev.to/corinamurg (axe-core Playwright patterns — reinforce npm + Playwright docs)
- mybyways.com/blog/fixing-high-dpi-retina-screenshot-metadata (macOS Retina screenshot DPI metadata behaviour)

### Tertiary (LOW confidence — flagged as ASSUMED in §Assumptions Log)
- Sender.net CSP subdomain (`cdn.sender.net`) — inferred from published `universal.js` snippet; Phase 2 will provide exact URL.
- Sender.net form-container DOM selector (`.sender-form-field`) — from Sender.net help-centre search result excerpt, not confirmed by direct docs fetch (WebFetch ECONNREFUSED'd twice).

## Metadata

**Confidence breakdown:**
- Standard stack (Next.js 16 / Tailwind 4 / Playwright 1.59 / axe 4.11): **HIGH** — versions verified via npm registry; patterns from official docs.
- Architecture (static-export, client-embedded form, token-driven styles): **HIGH** — read directly from site repo.
- Pitfalls: **HIGH** for axe-false-positives + screenshot-wrong-version (observed patterns); **MEDIUM** for Phase 2 coordination (depends on Phase 2 progress).
- Sender.net inline behaviour: **HIGH** for "inline is default" and "DOI is email-side"; **MEDIUM** for specific CSP/subdomain requirements (two of the direct docs fetches failed with ECONNREFUSED; cross-verified via multiple search-result excerpts).
- Pattern inventory: **HIGH** — read verbatim from `app/page.tsx`, `globals.css`, and AGENTS.md.

**Research date:** 2026-04-16
**Valid until:** 2026-05-16 (stable framework versions, locked decisions — 30 days).
