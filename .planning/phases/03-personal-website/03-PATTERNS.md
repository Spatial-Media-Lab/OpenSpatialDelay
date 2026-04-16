---
phase: 03
slug: personal-website
type: patterns
status: ready_for_planning
date: 2026-04-16
author: gsd-pattern-mapper
consumer: gsd-planner
---

# Phase 03 — Personal Website — Pattern Map

**Mapped:** 2026-04-16
**Target repo:** `../andrewrahman-com/` (NOT the plugin repo — this phase is Next.js 16 / React 19 / Tailwind 4 / Playwright)
**Files analysed:** 19 new/modified targets
**Analogs found:** 17 / 19 (2 files require new patterns — axe-core + asset-existence — documented under "No Analog Found")

> Executor primer: every code excerpt below is from the actual site repo at its current state
> (commit prior to 2026-04-16T23:09+02:00). Paths are absolute so they copy/paste into
> `Read` / `Edit` tool calls directly. **Do not look inside `/Users/andrewrahman/conductor/repos/openspatialdelay/Source/`** — that's JUCE C++, irrelevant here.

---

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `../andrewrahman-com/app/page.tsx` (FEATURES rewrite, Hero stats, PatreonCTA recolour, AboutAndrew headshot+email, Pipeline logo swap) | page / section-composition | static render | **self** (same file — rewrite array entries + surgical element edits) | **exact** |
| `../andrewrahman-com/app/globals.css` (add `--accent-regal` token) | design-tokens / CSS | static build-time | **self** (`:root { … }` block at lines 7–46) | **exact** |
| `../andrewrahman-com/app/privacy/page.tsx` (6 email refs + version bump + changelog entry) | page / prose-content | static render | **self** (6 mailto patterns at lines 18–21, 31–34, 166–169, 178–181 + effective-date line 13) | **exact** |
| `../andrewrahman-com/app/get-osd/page.tsx` (D-14 disposition — likely reduce to Sender.net shim) | page / client-component | client JS + SaaS | **self** (existing Tally-embed `useEffect` pattern — replace entire body) | role-match |
| `../andrewrahman-com/components/Nav.tsx` (Patreon ghost-button colour) | component / nav | static render | **self** (lines 17–28: `<a>` with `style={{ borderColor: 'var(--accent-rose)' }}`) | **exact** |
| `../andrewrahman-com/tests/homepage-content.spec.ts` (extend: hero stats, feature count, patreon colour, headshot, SML logo SVG, email migration, negative assertion) | test / Playwright content-regression | request-response | **self** (existing 7 tests — add assertions in same `describe` block) | **exact** |
| `../andrewrahman-com/tests/privacy-content.spec.ts` (email migration + processor-name update) | test / Playwright content-regression | request-response | **self** (existing 6 tests — update string literals + add negative assertion) | **exact** |
| `../andrewrahman-com/tests/smoke.spec.ts` (conditional — update Tally placeholder assertion if Sender.net wired) | test / Playwright route-smoke | request-response | **self** (existing 4 tests) | **exact** |
| `../andrewrahman-com/app/layout.tsx` (verify OG block — no change expected) | config / metadata | build-time | **self** (lines 27–52 already correct) | **exact** (read-only verify) |
| `../andrewrahman-com/tests/a11y.spec.ts` (**NEW**) | test / Playwright a11y | request-response + third-party API (axe) | `tests/homepage-content.spec.ts` (for `test.describe` shape + `beforeEach`) | role-match |
| `../andrewrahman-com/tests/og-metadata.spec.ts` (**NEW**) | test / Playwright metadata-assertion | request-response | `tests/homepage-content.spec.ts` (for shape); `page.locator('head > meta[...]')` pattern is new | role-match |
| `../andrewrahman-com/tests/asset-existence.spec.ts` (**NEW**) | test / filesystem presence | file-I/O | No direct analog — see "No Analog Found" §1 | none |
| `../andrewrahman-com/public/assets/sml-logo.svg` (**NEW** — copy from Downloads) | asset / static | file-I/O | **self** (filesystem pattern — existing PNGs in same dir) | exact (pattern is `cp` + commit) |
| `../andrewrahman-com/public/assets/andrew.jpg` (**NEW** — download from SML) | asset / static | file-I/O | **self** (filesystem pattern — existing PNGs in same dir) | exact |
| 11 × `../andrewrahman-com/public/assets/screenshot_*.png` + `signal-flow.png` (**RECAPTURE**) | asset / static | file-I/O (from macOS `screencapture`) | **self** (existing files — overwrite in place) | exact |
| `../andrewrahman-com/scripts/contrast-check.mjs` (**NEW — optional**) | utility / one-off script | batch | No analog — new file, ad-hoc only | none |
| `<EmailCaptureSection>` component (**NEW — conditional on D-14/Phase 2**) | component / section | client JS + SaaS | `get-osd/page.tsx` (current Tally loader) + `PatreonCTA()` in `page.tsx` (for section shape) | role-match |
| `../andrewrahman-com/package.json` (add `@axe-core/playwright` devDep) | config / npm-manifest | build-time | **self** (add to `devDependencies` block) | exact |
| `../andrewrahman-com/_headers` (conditional: CSP swap `tally.so` → `cdn.sender.net`) | config / Netlify CSP | CDN edge | **self** (current CSP line) | exact (conditional on Phase 2) |

---

## Pattern Assignments

### 1. `../andrewrahman-com/app/page.tsx` (page, static render)

**Analog:** itself (the file IS the analog — rewrite arrays and surgically edit existing elements; do NOT rename or restructure components).

#### Imports pattern (lines 1–2, header of file)
```tsx
import Image from 'next/image';
import Link from 'next/link';
```
No path aliases, no barrel. When adding the headshot `<Image>`, the `Image` import is already present — no change.

#### Data-driven FEATURES array pattern (lines 10–51 — D-03 rewrites entries, NOT the renderer)
```tsx
const FEATURES = [
  {
    tapIndex: 0,
    title: 'Each echo, its own 3D position',
    body:
      'HRTF convolution renders 12 delay taps in full 3D space. Five SOFA profiles — KU100, CIPIC, HUTUBS, MIT KEMAR, SADIE — let you match the one that matches your head.',
    image: '/assets/screenshot_spatial_map.png',
    alt: 'OpenSpatialDelay spatial map — 12 taps distributed in 3D space',
  },
  // ... 4 more entries
] as const;
```
**Invariants the rewrite MUST preserve:**
- `as const` at end of array
- Every entry has exactly these 5 keys: `tapIndex`, `title`, `body`, `image`, `alt`
- `tapIndex` ∈ [0..11] — assigns the coloured dot via `TAPS[feature.tapIndex]` (page.tsx line 262)
- `image` is an absolute path under `/assets/`
- `alt` is a real description, not placeholder (accessibility lint)

**Phase-3 rewrite (6 entries, tap indices per UI-SPEC line ~190):** use `0, 3, 5, 7, 9, 11` OR `0, 3, 5, 7, 10, 11` (executor's call — UI-SPEC §Component Inventory suggests the latter to avoid tap-9/10 purple collision).

#### Renderer pattern (lines 260–291 — DO NOT CHANGE, the rewrite is data-only)
```tsx
<ul className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-5">
  {FEATURES.map((feature, i) => (
    <li
      key={feature.title}
      className="rounded-lg border p-6 flex flex-col"
      style={{
        background: 'var(--bg-panel)',
        borderColor: 'var(--border-subtle)',
      }}
    >
      <div className="flex items-center gap-3 mb-4">
        <span className="font-mono-osd text-[11px] leading-none" style={{ color: 'var(--text-dim)' }}>
          0{i + 1}
        </span>
        <span
          className="inline-block h-[10px] w-[10px] rounded-full"
          style={{ background: TAPS[feature.tapIndex] }}
          aria-hidden
        />
      </div>
      <h3 className="font-display text-[22px] leading-[1.15] mb-3" style={{ color: 'var(--text-primary)' }}>
        {feature.title}
      </h3>
      <p className="text-[15px] leading-[1.6] mb-5" style={{ color: 'var(--text-secondary)' }}>
        {feature.body}
      </p>
      <div className="mt-auto rounded-md overflow-hidden border" style={{ borderColor: 'var(--border-dim)', background: 'var(--bg-recessed)' }}>
        <Image src={feature.image} alt={feature.alt} width={800} height={500} className="w-full h-auto block opacity-95" />
      </div>
    </li>
  ))}
</ul>
```
Note the card-index `0{i + 1}` — with 6 cards this renders `01..06` (no change needed; 2-digit zero-pad works up to 09).

#### Hero stat `<dl>` pattern (lines 177–200 — D-01 rewrite)
```tsx
<dl
  className="mt-10 grid grid-cols-2 sm:grid-cols-4 gap-x-6 gap-y-4 font-mono-osd"
  aria-label="OpenSpatialDelay by the numbers"
>
  {[
    ['5', 'HRTF profiles'],
    ['±12', 'semitones'],
    ['2048', 'sample FFT'],
    ['GPL-3.0', 'forever free'],
  ].map(([num, label]) => (
    <div key={label}>
      <dt className="text-[28px] sm:text-[36px] leading-none font-display" style={{ color: 'var(--text-primary)' }}>
        {num}
      </dt>
      <dd className="mt-1 text-[11px] uppercase tracking-[0.15em]" style={{ color: 'var(--text-dim)' }}>
        {label}
      </dd>
    </div>
  ))}
</dl>
```
**Phase-3 rewrite — replace the inline tuple array ONLY.** D-01 locked values:
```tsx
[
  ['12', 'Delay Taps'],
  ['7', 'Spatialization Algorithms'],
  ['5', 'HRTF Profiles'],  // with footnote at executor's discretion
  ['70', 'Factory Presets'],
]
```
For the "5 HRTF Profiles" footnote (UI-SPEC Copywriting Contract), **follow the interaction-pattern contract**: `<abbr title="+1 Simple mode (low CPU)">` or `<sup className="font-mono-osd text-[10px]" style={{ color: 'var(--text-dim)' }}>+1</sup>`. Keep it inside the same `<div key={label}>`.

#### Hero Patreon ghost-button pattern (lines 149–158 — D-12 recolour)
```tsx
<a
  href={PATREON_URL}
  target="_blank"
  rel="noreferrer"
  className="inline-flex items-center gap-2 rounded-full px-6 py-3 text-[15px] font-semibold border transition-colors"
  style={{ borderColor: 'var(--accent-rose)', color: 'var(--accent-rose)' }}
>
  Support on Patreon
</a>
```
**Phase-3 edit:** replace `var(--accent-rose)` → `var(--accent-regal)` in BOTH `borderColor` and `color`. Single-literal-pair edit.

#### PatreonCTA pattern (lines 726–771 — D-12 recolour)
```tsx
function PatreonCTA() {
  return (
    <section
      className="py-20 sm:py-28"
      style={{ background: 'var(--accent-rose)', color: 'var(--bg-void)' }}  // ← background swap here
    >
      <Container>
        {/* ... */}
        <a
          href={PATREON_URL}
          className="inline-flex items-center gap-2 rounded-full px-7 py-3.5 text-[15px] font-semibold"
          style={{ background: 'var(--bg-void)', color: 'var(--accent-rose)' }}  // ← button text swap
        >
          Support on Patreon →
        </a>
      </Container>
    </section>
  );
}
```
**Phase-3 edit:** two `var(--accent-rose)` occurrences → `var(--accent-regal)`. Section `<SectionLabel index="09 · Fund the pipeline" accent="var(--bg-void)" />` stays as-is.

**What NOT to touch:** the `AboutAndrew` `<SectionLabel index="05 · Me" accent="var(--accent-rose)" />` at line 466 — `--accent-rose` is retained there per D-12 + UI-SPEC §Accent reserved-for list.

#### AboutAndrew headshot swap pattern (lines 463–487 — D-15)
Current placeholder:
```tsx
<div
  className="aspect-square rounded-lg border flex items-center justify-center relative overflow-hidden"
  style={{
    background:
      'radial-gradient(circle at 30% 20%, rgba(228,103,166,0.12), transparent 60%), var(--bg-panel)',
    borderColor: 'var(--border-subtle)',
  }}
  aria-label="Headshot placeholder"
>
  {/* rainbow grid + AR monogram */}
</div>
```
**Replace the whole `<div>` with** (from RESEARCH.md §D-15 + UI-SPEC Asset Contract):
```tsx
<Image
  src="/assets/andrew.jpg"
  alt="Andrew Rahman"
  width={560}
  height={560}
  className="aspect-square rounded-lg object-cover border"
  style={{ borderColor: 'var(--border-subtle)' }}
/>
```
The `aspect-square rounded-lg object-cover border` + `borderColor: 'var(--border-subtle)'` is the established house image treatment (also used for `sml-logo.png` container at lines 438–445, though logo uses an outer panel `<div>`). Do NOT add `<figure>`, `<picture>`, shadow, or a caption.

#### AboutAndrew email-migration pattern (lines 539–545 — D-13)
```tsx
<a
  href="mailto:andrewjrahman@gmail.com"
  className="underline underline-offset-4"
  style={{ color: 'var(--accent-stellar)' }}
>
  andrewjrahman@gmail.com
</a>
```
**Phase-3 edit:** replace both occurrences (`href`'s mailto string AND the link text) with `andrew@spatialmedialab.org`. Single-file change; keep `className` and `style` verbatim. **Inline-link style is house pattern** (also used in `Footer.tsx`, `Pipeline()`, and `privacy/page.tsx`) — do not use `className="text-stellar underline"` style (that's only in `get-osd/page.tsx` and is an older convention).

#### Pipeline SML logo swap (lines 438–445 — D-11)
```tsx
<Image
  src="/assets/sml-logo.png"
  alt="Spatial Media Lab logomark"
  width={180}
  height={180}
  className="w-[140px] sm:w-[180px] h-auto"
/>
```
**Phase-3 edit:** `src="/assets/sml-logo.svg"` (ONE char change). Keep all other props verbatim per RESEARCH.md §SVG Handling — `unoptimized` is inherited from `next.config.ts`, no prop change needed. Also update `FEATURES[4]` (or `FEATURES[5]` in new 6-card slate — the open-source card) if its `image:` references `sml-logo.png` — swap to `.svg` identically. After both swaps verify visually, delete `public/assets/sml-logo.png`.

#### Error handling
No try/catch on this page — it's pure SSG render. If `andrew.jpg` is missing at build time, Next.js will warn but not fail. The `tests/asset-existence.spec.ts` is the belt-and-braces check (see §10).

---

### 2. `../andrewrahman-com/app/globals.css` (design-tokens, static build-time)

**Analog:** itself — extend the existing `:root { … }` block at lines 7–46.

#### Insertion point pattern (after line 26, inline with other accent tokens)
Existing adjacent tokens (lines 23–30):
```css
  --accent-stellar: #80d8ff;
  --accent-violet: #7457d1;
  --accent-amber: #f0a646;
  --accent-green: #3bce6c;
  --accent-rose: #e467a6;
  --accent-sync: #e1c34b;
  --accent-channel-l: #4499ff;
  --accent-channel-r: #ff4444;
```
**Phase-3 add (per D-12 + UI-SPEC):** insert **immediately after `--accent-rose` line 27**:
```css
  --accent-regal: oklch(72% 0.15 290); /* ≈ #b49bd8 — light regal lavender; Patreon CTA only; Phase 3 B-5 */
```
The comment matches the existing `/* Phase 3 AA fix */` convention called out in UI-SPEC §Accessibility audit contract.

#### @theme inline bridge pattern (lines 49–65)
```css
@theme inline {
  --color-void: var(--bg-void);
  --color-stellar: var(--accent-stellar);
  --color-rose-osd: var(--accent-rose);
  /* ... */
}
```
**Phase-3 decision:** do NOT add `--color-regal-osd` to `@theme inline` by default. Per RESEARCH.md §CSS Token Architecture: current Patreon usages are inline `style={{ background: 'var(--accent-rose)' }}`, NOT Tailwind utility classes like `bg-rose-osd`. Verify with:
```bash
grep -rn "regal-osd\|rose-osd" /Users/andrewrahman/conductor/repos/andrewrahman-com/app /Users/andrewrahman/conductor/repos/andrewrahman-com/components
```
If nothing uses `bg-rose-osd` or similar utility class, skip the `@theme inline` entry.

#### AA-fix annotation pattern (new, per UI-SPEC)
If the axe-core audit forces a nudge:
```css
  --text-dim: #787d85; /* Phase 3 AA fix — was #6d7279, failed 4.5:1 on --bg-void */
```
Preserve the old value in the inline comment so reviewers can see the fix's intent.

---

### 3. `../andrewrahman-com/app/privacy/page.tsx` (page, static render)

**Analog:** itself — 6 email occurrences at known line numbers, same `mailto:` + `underline` pattern throughout.

#### Mailto pattern (lines 18–21, 31–34, 166–169, 178–181)
```tsx
<a className="underline" href="mailto:andrewjrahman@gmail.com">
  andrewjrahman@gmail.com
</a>
```
**Phase-3 find-replace:** `andrewjrahman@gmail.com` → `andrew@spatialmedialab.org` in ALL 6 spots. (There are 4 `<a>` instances + 2 body-text mentions. Use Grep to verify zero matches remain post-edit; negative assertion in `tests/privacy-content.spec.ts` provides runtime safety — see §5.)

**Important style note:** privacy page uses `className="underline"` without `underline-offset-4`. This is intentional legacy — do NOT harmonise with the AboutAndrew pattern during this phase (out of D-06 copy scope).

#### Effective-date pattern (line 13 — D-13 version bump)
```tsx
<p className="text-sm text-text-secondary">Effective: 2026-04-16</p>
```
**Phase-3 edit:** bump to the execution date (will be Wave-2 or Wave-3 commit date). **Also add a `<section>` with `<h2>Changes to this policy</h2>` update entry** (heading already exists at lines 202–212; append a paragraph inside).

#### Changelog-entry insertion pattern (inside `Changes to this policy` section, lines 202–212)
Existing section already contains generic "we will update the effective date" prose. Append a new `<p>` inside the same `<section>`:
```tsx
<p className="text-base leading-[1.6] text-text-primary">
  <strong>2026-04-XX:</strong> Data Controller contact email updated from{' '}
  <code>andrewjrahman@gmail.com</code> to{' '}
  <a className="underline" href="mailto:andrew@spatialmedialab.org">andrew@spatialmedialab.org</a>
  . Andrew Rahman remains the data controller; only the contact channel changed.
  {/* Phase 2 Sender.net migration log entry will be added separately if Phase 3 absorbs it — see D-14 Option B. */}
</p>
```

#### Processor-name pattern (lines 83–91 — conditional, Phase-2 coordination)
Current (Tally):
```tsx
We use Tally.so (Tally Technologies SRL, Belgium) as a data processor [...]
```
**Phase-3 rule:** do NOT change unless Phase 3 absorbs the Sender.net migration (D-14 Option B, flagged in RESEARCH.md §D-14 Disposition). If Phase 2 handles this, Phase 3 only flips the email. Coordinate via Wave-0 check.

---

### 4. `../andrewrahman-com/app/get-osd/page.tsx` (page / client-component, D-14 disposition)

**Analog:** itself — replace the current Tally `useEffect` loader.

#### Current Tally-loader pattern (lines 1–19)
```tsx
'use client';

import { useEffect } from 'react';

const TALLY_FORM_ID = process.env.NEXT_PUBLIC_TALLY_FORM_ID || 'REPLACE_ME';

export default function GetOSDPage() {
  useEffect(() => {
    const script = document.createElement('script');
    script.src = 'https://tally.so/widgets/embed.js';
    script.async = true;
    document.body.appendChild(script);
    return () => {
      document.body.removeChild(script);
    };
  }, []);
  /* ... */
}
```

#### D-14 disposition — two paths
**Option A (Phase 2 already wired Sender.net):** reduce this file to a thin shim that renders the shared `<EmailCaptureSection>` component (see §17):
```tsx
import { EmailCaptureSection } from '@/components/EmailCaptureSection';

export default function GetOSDPage() {
  return <EmailCaptureSection standalone />;
}
```

**Option B (Phase 3 absorbs Sender.net migration):** rewrite `useEffect` to load Sender.net's `universal.js`:
```tsx
'use client';
import { useEffect } from 'react';

const SENDER_FORM_ID = process.env.NEXT_PUBLIC_SENDER_FORM_ID || 'REPLACE_ME';

export default function GetOSDPage() {
  useEffect(() => {
    const script = document.createElement('script');
    script.src = 'https://cdn.sender.net/accounts_resources/universal.js';  // ASSUMED — verify at Wave 0
    script.async = true;
    document.body.appendChild(script);
    return () => { document.body.removeChild(script); };
  }, []);
  return (
    <article className="mx-auto max-w-[720px] px-4 py-16 space-y-8">
      {/* mirror current header/section structure */}
      <div className="sender-form-field" data-sender-form-id={SENDER_FORM_ID} />
    </article>
  );
}
```
**Planner decision point:** Wave 0 check — does the file already reference `NEXT_PUBLIC_TALLY_FORM_ID`? If yes (current state), branch to Option B. If no (Phase 2 shipped), branch to Option A.

**Preserved elements regardless of path:**
- `'use client'` directive
- The page's article-frame: `<article className="mx-auto max-w-[720px] px-4 py-16 space-y-8">`
- The `<h1>Get OpenSpatialDelay</h1>` header
- The GPL-3.0 note at the bottom (lines 44–55)
- The `<noscript>` fallback

---

### 5. `../andrewrahman-com/components/Nav.tsx` (component / nav, static render)

**Analog:** itself — lines 17–28 Patreon ghost-button.

#### Current pattern (lines 17–28)
```tsx
<a
  href="https://www.patreon.com/c/AndrewRahman"
  target="_blank"
  rel="noreferrer"
  className="hidden sm:inline-block text-sm px-3 py-1.5 border rounded-full transition-colors"
  style={{
    borderColor: 'var(--accent-rose)',
    color: 'var(--accent-rose)',
  }}
>
  Support on Patreon
</a>
```
**Phase-3 edit:** both `var(--accent-rose)` → `var(--accent-regal)`. Same as the Hero ghost button — reuse the pattern verbatim.

---

### 6. `../andrewrahman-com/tests/homepage-content.spec.ts` (test / Playwright, request-response)

**Analog:** itself — extend the existing `test.describe('homepage content', …)` block.

#### Imports + `test.describe` shell (lines 1–11)
```ts
import { test, expect } from '@playwright/test';

test.describe('homepage content', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto('/');
  });
  /* ... existing tests ... */
});
```

#### Content-assertion pattern (lines 32–38 — email test, CANONICAL for D-13 migration)
```ts
test('contact email is present and clickable as mailto', async ({ page }) => {
  await expect(page.locator('body')).toContainText('andrewjrahman@gmail.com');
  await expect(
    page.getByRole('link', { name: 'andrewjrahman@gmail.com' }),
  ).toHaveAttribute('href', 'mailto:andrewjrahman@gmail.com');
});
```
**Phase-3 edit:** three string swaps per test. Replace `andrewjrahman@gmail.com` → `andrew@spatialmedialab.org` in ALL three positions.

#### Negative-assertion pattern (lines 51–53 — CANONICAL for Pitfall 3 mitigation)
```ts
// Belt-and-braces: no occurrence of the old placeholder slug anywhere on the page.
await expect(page.locator('body')).not.toContainText('patreon.com/andrewrahman');
```
**Phase-3 add (NEW test):** mirror this pattern for the email migration:
```ts
test('old Gmail address is nowhere on the page (D-13 migration)', async ({ page }) => {
  await expect(page.locator('body')).not.toContainText('andrewjrahman@gmail.com');
});
```

#### Multi-link enumeration pattern (lines 41–54 — CANONICAL for hero stats assertion)
```ts
test('uses the real Patreon URL — no placeholder', async ({ page }) => {
  const patreonLinks = page.getByRole('link', { name: /Patreon/i });
  await expect(patreonLinks.first()).toBeVisible();
  const hrefs = await patreonLinks.evaluateAll((els) =>
    els.map((el) => (el as HTMLAnchorElement).href),
  );
  expect(hrefs.length).toBeGreaterThan(0);
  for (const href of hrefs) {
    expect(href).toBe('https://www.patreon.com/c/AndrewRahman');
  }
});
```
**Phase-3 add — hero stats test:**
```ts
test('hero shows the 4 locked v1.0.0 stats (D-01)', async ({ page }) => {
  const dl = page.getByLabel('OpenSpatialDelay by the numbers');
  await expect(dl).toContainText('12');
  await expect(dl).toContainText('Delay Taps');
  await expect(dl).toContainText('7');
  await expect(dl).toContainText('Spatialization Algorithms');
  await expect(dl).toContainText('5');
  await expect(dl).toContainText('HRTF Profiles');
  await expect(dl).toContainText('70');
  await expect(dl).toContainText('Factory Presets');
  // Negative: retired stats must NOT appear
  await expect(dl).not.toContainText('2048');
  await expect(dl).not.toContainText('semitones');
});
```

#### Role-count pattern (lines 64–68 — CANONICAL for "6 feature cards" assertion)
```ts
const downloadLinks = page.getByRole('link', { name: /Download OSD/i });
expect(await downloadLinks.count()).toBeGreaterThanOrEqual(2);
```
**Phase-3 add — feature-count test:**
```ts
test('features section renders exactly 6 cards (D-03)', async ({ page }) => {
  const cards = page.locator('ul li').filter({ has: page.locator('h3.font-display') });
  await expect(cards).toHaveCount(6);
});
```
(Selector may need adjustment once executor confirms the feature list's exact selector fingerprint — use `page.getByRole('heading', { level: 3 })` count as fallback.)

#### Rendered-style assertion pattern (NEW for Patreon colour — no direct analog, but follows Playwright idiom)
```ts
test('Patreon CTA uses the new regal-lavender token (D-12)', async ({ page }) => {
  const patreonSection = page.locator('section').filter({
    hasText: /Fund the pipeline/i,
  });
  // Runtime-computed background. Expected: oklch(72% 0.15 290) ≈ rgb(180, 155, 216) ≈ #b49bd8.
  // Allow any rgb rendering of that OKLCH value; do NOT hard-code the rgb string.
  const bg = await patreonSection.evaluate((el) => getComputedStyle(el).backgroundColor);
  expect(bg).toMatch(/rgb\(1[7-9]\d,\s*1[4-6]\d,\s*2[0-2]\d\)/);  // loose range
});
```

#### Image-src attribute pattern (NEW for headshot + SML logo SVG — no direct analog)
```ts
test('headshot is a real image (not the AR monogram placeholder)', async ({ page }) => {
  await expect(page.getByRole('img', { name: 'Andrew Rahman' })).toHaveAttribute(
    'src',
    /\/assets\/andrew\.(jpg|webp)/,
  );
});

test('SML logo is served as SVG (D-11)', async ({ page }) => {
  const logoImgs = page.getByRole('img', { name: /Spatial Media Lab/i });
  // At least one instance on the page; all must be SVG.
  const srcs = await logoImgs.evaluateAll((els) => els.map((el) => (el as HTMLImageElement).src));
  expect(srcs.length).toBeGreaterThan(0);
  for (const src of srcs) expect(src).toMatch(/\.svg(\?.*)?$/);
});
```

---

### 7. `../andrewrahman-com/tests/privacy-content.spec.ts` (test / Playwright, request-response)

**Analog:** itself — update string literals and mirror homepage's negative-assertion.

#### Current email assertion (lines 15–17)
```ts
test('lists rights-request contact email', async ({ page }) => {
  await expect(page.locator('body')).toContainText('andrewjrahman@gmail.com');
});
```
**Phase-3 edit:** `andrewjrahman@gmail.com` → `andrew@spatialmedialab.org`.

#### Current processor assertion (lines 19–24)
```ts
test('names Tally as a data processor (not a generic third-party reference)', async ({ page }) => {
  await expect(page.locator('body')).toContainText('Tally Technologies SRL');
});
```
**Phase-3 edit — conditional on Phase 2 outcome:**
- If Phase 2 migrates → `'Tally Technologies SRL'` → `'UAB Sender.lt'` (or whatever Phase 2 writes; coordinate via STATE.md per RESEARCH.md §Open Questions #2).
- Rename test to `'names the email-capture processor …'`.

#### New negative-assertion (mirror homepage pattern)
```ts
test('old Gmail address is nowhere in privacy policy (D-13 migration)', async ({ page }) => {
  await expect(page.locator('body')).not.toContainText('andrewjrahman@gmail.com');
});
```

---

### 8. `../andrewrahman-com/tests/smoke.spec.ts` (test / Playwright route-smoke, request-response)

**Analog:** itself — surgical edit to Tally-placeholder test (conditional).

#### Current get-osd assertion (lines 35–40)
```ts
test('get-osd page renders heading + Tally placeholder (FORM_ID not set)', async ({ page }) => {
  await page.goto('/get-osd');
  await expect(page.getByRole('heading', { level: 1, name: 'Get OpenSpatialDelay' })).toBeVisible();
  await expect(page.getByText('Tally form ID not yet configured')).toBeVisible();
});
```
**Phase-3 edit — depends on D-14 outcome:**
- Option A (Sender.net wired, inline adopted): replace the `Tally form ID not yet configured` assertion with `page.locator('.sender-form-field')` visibility OR the shim-rendered `<EmailCaptureSection>` marker.
- Option B: replace Tally string with Sender-specific placeholder; keep heading assertion.

Minimally, **Phase 3 MUST NOT leave this test asserting stale Tally text** — it will fail post-migration.

---

### 9. `../andrewrahman-com/app/layout.tsx` (config / metadata, build-time)

**Analog:** itself — read-only verification per WEB-03 (RESEARCH.md confirms no change needed).

#### Metadata pattern (lines 27–52 — ALREADY CORRECT, DO NOT CHANGE)
```tsx
export const metadata: Metadata = {
  title: 'Andrew Rahman — OpenSpatialDelay & the Spatial Media Library',
  description:
    '3D spatial delay plugin for VST3 + AU on macOS & Windows. Free, open-source. Built by Andrew Rahman in Berlin.',
  metadataBase: new URL('https://andrewrahman.com'),
  openGraph: {
    title: 'Andrew Rahman — OpenSpatialDelay & the Spatial Media Library',
    description: '...',
    images: [{
      url: '/assets/screenshot_full.png',
      width: 1640,
      height: 1160,
      alt: '...',
    }],
    type: 'website',
  },
  twitter: {
    card: 'summary_large_image',
    title: '...',
    description: '...',
    images: ['/assets/screenshot_full.png'],
  },
};
```
**Phase-3 rule:** verify these values survive the asset refresh (D-10). If new `screenshot_full.png` dimensions differ from `1640×1160`, update `width`/`height` in the `openGraph.images` object; otherwise **no change**. The `alt` text is editorial — executor may revise to match the new hero framing at their discretion (within UI-SPEC copywriting contract).

---

### 10. `../andrewrahman-com/tests/a11y.spec.ts` (**NEW** — test / Playwright a11y)

**Analog (for shape):** `tests/homepage-content.spec.ts` lines 1–11 (imports + `test.describe` + `beforeEach`).
**Analog (for axe-core integration):** no local analog — follow the canonical skeleton in RESEARCH.md §Axe test fixture (lines 614–630) verbatim, which is the official Playwright docs pattern.

#### Full-file skeleton (copy verbatim, adjust paths list to D-14 outcome)
```ts
import { test, expect } from '@playwright/test';
import AxeBuilder from '@axe-core/playwright';

const WCAG_AA_TAGS = ['wcag2a', 'wcag2aa', 'wcag21a', 'wcag21aa'] as const;

test.describe('WCAG 2.1 AA accessibility', () => {
  for (const path of ['/', '/privacy', '/get-osd']) {
    test(`${path} has zero axe violations`, async ({ page }) => {
      await page.goto(path);
      const { violations } = await new AxeBuilder({ page })
        .withTags([...WCAG_AA_TAGS])
        .analyze();
      expect(violations, JSON.stringify(violations, null, 2)).toEqual([]);
    });
  }
});
```

**Expected first-run borderline failures (per RESEARCH.md §Expected violations):**
- `--text-dim` (#6d7279) on `--bg-void` (#03060b) — borderline 4.5:1
- `--text-dim` (#6d7279) on `--bg-panel` (#0a0d12) — borderline 4.5:1
- `--bg-void` (#03060b) on `--accent-regal` (≈#b49bd8) — verify before shipping

**Fix procedure:** nudge OKLCH lightness ±2 points in `globals.css`, annotate `/* Phase 3 AA fix */`, re-run axe.

---

### 11. `../andrewrahman-com/tests/og-metadata.spec.ts` (**NEW** — test / Playwright metadata-assertion)

**Analog (for shape):** `tests/homepage-content.spec.ts` `test.describe` shell.
**Analog (for meta-tag assertion):** none local — follow RESEARCH.md §OG Metadata Verification (lines 260–291) skeleton, which uses the Playwright-official `locator('head > meta[...]') + toHaveAttribute` pattern.

#### Full-file skeleton (copy verbatim)
```ts
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
      .toHaveAttribute('content', /^(1[2-9]\d{2,}|[2-9]\d{3,})$/);
  });

  test('twitter:card is summary_large_image', async ({ page }) => {
    await expect(page.locator('head > meta[name="twitter:card"]'))
      .toHaveAttribute('content', 'summary_large_image');
  });
});
```

---

### 12. `../andrewrahman-com/tests/asset-existence.spec.ts` (**NEW** — test / filesystem, file-I/O)

**Analog:** no local analog (no filesystem test exists). See "No Analog Found" §1 for rationale.

#### Skeleton (new pattern — Node `fs` inside Playwright test)
```ts
import { test, expect } from '@playwright/test';
import { existsSync, statSync } from 'node:fs';
import { resolve } from 'node:path';

const ROOT = resolve(__dirname, '..', 'public', 'assets');

const REQUIRED_ASSETS = [
  'screenshot_full.png',
  'screenshot_spatial_map.png',
  'screenshot_wobble.png',
  'screenshot_orbit.png',
  'screenshot_elevation_map.png',
  'screenshot_header.png',
  'screenshot_bottom_panel.png',
  'screenshot_right_panel.png',
  'screenshot_shimmer.png',
  'screenshot.png',
  'signal-flow.png',
  'sml-logo.svg',
  'andrew.jpg',
] as const;

test.describe('required static assets (WEB-01 / D-10 / D-11 / D-15)', () => {
  for (const name of REQUIRED_ASSETS) {
    test(`public/assets/${name} exists and is non-empty`, () => {
      const path = resolve(ROOT, name);
      expect(existsSync(path), `missing asset: ${path}`).toBe(true);
      expect(statSync(path).size, `empty asset: ${path}`).toBeGreaterThan(0);
    });
  }

  test('public/assets/sml-logo.png is DELETED (D-11)', () => {
    const png = resolve(ROOT, 'sml-logo.png');
    expect(existsSync(png), `legacy PNG still present: ${png}`).toBe(false);
  });
});
```
**Note:** this test runs in Node context (no `page` fixture used), but Playwright's test runner accepts sync tests like this without issue. No webServer dependency.

---

### 13. `../andrewrahman-com/public/assets/sml-logo.svg` (**NEW asset**)

**Action pattern:** filesystem copy, not code.
```bash
cp "/Users/andrewrahman/Downloads/Web-Logo, white.svg" \
   /Users/andrewrahman/conductor/repos/andrewrahman-com/public/assets/sml-logo.svg
```
Quote the source path because of the space and comma. Verify with `ls -la` + `file` (confirms SVG MIME).

---

### 14. `../andrewrahman-com/public/assets/andrew.jpg` (**NEW asset**)

**Action pattern:** filesystem download from SML site (per RESEARCH.md §Headshot workflow).
```bash
curl -o /Users/andrewrahman/conductor/repos/andrewrahman-com/public/assets/andrew.jpg \
  https://spatialmedialab.org/wp-content/uploads/2026/04/Andrew-Rahman-819x1024.jpg
sips -g pixelWidth -g pixelHeight \
  /Users/andrewrahman/conductor/repos/andrewrahman-com/public/assets/andrew.jpg
```
Pitfall 6 — 819×1024 source is slightly under native Retina 2× for 560×560 render. Accept or ask user for higher-res original.

---

### 15. 11 × PNG screenshot recapture (**MODIFIED assets**)

**Analog:** existing PNGs in `public/assets/` — overwrite in place, same filenames.
**Action pattern:** macOS `screencapture` (per RESEARCH.md §Screenshot workflow):
- Verify v1.0.0 AU installed: `ls ~/Library/Audio/Plug-Ins/Components/ | grep "v1.0.0.component"` (Wave 0 gate — target the shipped binary, not the legacy `v1.0.component`).
- Open REAPER → OpenSpatialDelay v1.0.0 (NOT the legacy v1.0, NOT post-release v1.0.1/v1.0.2) → Cmd-Shift-4 (space+click) per window.
- Save to `public/assets/` with exact filenames from UI-SPEC §Asset Contract.
- Ensure `screenshot_full.png` ≥ 1200×630 (WEB-03 minimum; existing 1640×1160 baseline is the target).

**Coordination gate (D-10):** this capture session must also produce Phase 4's CONT-03 deliverables in one sitting. **Commit screenshots once, reference from both repos.**

---

### 16. `../andrewrahman-com/scripts/contrast-check.mjs` (**NEW — optional**)

**Analog:** none — this is an ad-hoc script, not CI. RESEARCH.md §Manual-pair verification describes it.

**Skeleton (if planner decides to include; optional):**
```js
// scripts/contrast-check.mjs — one-off manual verification of the 8 token pairs in UI-SPEC
// Usage: node scripts/contrast-check.mjs

function relLum(hex) {
  const rgb = hex.replace('#', '').match(/.{2}/g).map((h) => parseInt(h, 16) / 255);
  const [r, g, b] = rgb.map((c) => (c <= 0.03928 ? c / 12.92 : ((c + 0.055) / 1.055) ** 2.4));
  return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}
function ratio(fg, bg) {
  const L1 = Math.max(relLum(fg), relLum(bg));
  const L2 = Math.min(relLum(fg), relLum(bg));
  return (L1 + 0.05) / (L2 + 0.05);
}

const PAIRS = [
  ['#e1e5ea', '#03060b', 'text-primary / bg-void'],
  ['#9fa5ae', '#03060b', 'text-secondary / bg-void'],
  ['#9fa5ae', '#0a0d12', 'text-secondary / bg-panel'],
  ['#6d7279', '#03060b', 'text-dim / bg-void'],
  ['#6d7279', '#0a0d12', 'text-dim / bg-panel'],
  ['#80d8ff', '#03060b', 'accent-stellar / bg-void'],
  ['#03060b', '#b49bd8', 'bg-void on accent-regal'],
];
for (const [fg, bg, label] of PAIRS) {
  const r = ratio(fg, bg).toFixed(2);
  const pass = r >= 4.5 ? 'PASS' : 'FAIL';
  console.log(`${pass}  ${r}:1  ${label}`);
}
```
Not required in CI; one-off belt-and-braces.

---

### 17. `<EmailCaptureSection>` component (**NEW — conditional on D-14 + Phase 2**)

**Analogs (for shape):**
- `PatreonCTA()` in `app/page.tsx` lines 726–771 — for the outer section + Container + heading + body layout.
- `app/get-osd/page.tsx` lines 7–17 — for the Sender.net `useEffect` script loader.

**Suggested file location:** `../andrewrahman-com/components/EmailCaptureSection.tsx` (alongside `Nav.tsx` and `Footer.tsx`).

#### Skeleton (Option A — shared component consumed by both homepage inline and get-osd shim)
```tsx
'use client';
import { useEffect } from 'react';

const SENDER_FORM_ID = process.env.NEXT_PUBLIC_SENDER_FORM_ID || 'REPLACE_ME';

export function EmailCaptureSection({ standalone = false }: { standalone?: boolean }) {
  useEffect(() => {
    const script = document.createElement('script');
    script.src = 'https://cdn.sender.net/accounts_resources/universal.js';  // ASSUMED — Phase 2 confirms
    script.async = true;
    document.body.appendChild(script);
    return () => { document.body.removeChild(script); };
  }, []);

  const Wrapper = standalone ? 'article' : 'section';
  return (
    <Wrapper
      id="get-osd"
      className={standalone ? 'mx-auto max-w-[720px] px-4 py-16' : 'border-b py-16 sm:py-24'}
      style={standalone ? undefined : { borderColor: 'var(--border-dim)' }}
    >
      <div className={standalone ? '' : 'mx-auto max-w-[1200px] px-5 sm:px-8'}>
        {/* SectionLabel pattern from page.tsx lines 95–107 — inline here since it's not exported */}
        <h2
          className="font-display text-[32px] sm:text-[44px] leading-[1.05] mb-6"
          style={{ color: 'var(--text-primary)' }}
        >
          Get OpenSpatialDelay — free, forever.
        </h2>
        {SENDER_FORM_ID === 'REPLACE_ME' ? (
          <div
            className="rounded-md border p-6 text-sm"
            style={{ borderColor: 'var(--border-subtle)', background: 'var(--bg-panel)', color: 'var(--text-secondary)' }}
          >
            Sender.net form ID not yet configured. Set <code>NEXT_PUBLIC_SENDER_FORM_ID</code>.
          </div>
        ) : (
          <div className="sender-form-field" data-sender-form-id={SENDER_FORM_ID} />
        )}
      </div>
    </Wrapper>
  );
}
```

#### Insertion point in `app/page.tsx` (if adopted)
Between `DownloadCTA()` and `PatreonCTA()` in `HomePage()` (lines 66–79). Also swap `Link href="/get-osd/"` → `<a href="#get-osd">` in:
- Nav (`components/Nav.tsx` line 30)
- Hero CTA (`app/page.tsx` lines 141–147)
- DownloadCTA (`app/page.tsx` lines 693–699)

---

### 18. `../andrewrahman-com/package.json` (config / npm-manifest)

**Analog:** itself — add to `devDependencies` block.
```json
"devDependencies": {
  "@axe-core/playwright": "^4.11.1",
  "@playwright/test": "^1.59.1",
  "@tailwindcss/postcss": "^4",
  "@types/node": "^20",
  // ...
}
```
**Install command:** `npm install --save-dev @axe-core/playwright` (per RESEARCH.md §Installation). Run in `../andrewrahman-com/`, NOT the plugin repo.

---

### 19. `../andrewrahman-com/_headers` (config / Netlify CSP — **CONDITIONAL on D-14 Option B**)

**Analog:** itself — current line:
```
Content-Security-Policy: default-src 'self'; script-src 'self' https://tally.so https://widgets.tally.so; frame-src https://tally.so https://*.tally.so; style-src 'self' 'unsafe-inline'; img-src 'self' data: https:; connect-src 'self' https://tally.so; base-uri 'self'; form-action 'self' https://tally.so
```
**Phase-3 edit (only if Phase 3 absorbs Sender.net migration):**
- Replace every `https://tally.so` → `https://cdn.sender.net` (ASSUMED subdomain — Phase 2 / Sender.net snippet confirms actual)
- Replace `https://*.tally.so` → appropriate Sender.net wildcard
- Remove `https://widgets.tally.so` if not needed

If Phase 2 handles this, **do not touch `_headers` in Phase 3.**

---

## Shared Patterns

### A. CSS token consumption via inline `style={{ … }}`
**Source:** every colour in `app/page.tsx`, `components/Nav.tsx`, `components/Footer.tsx`.
**Apply to:** all Phase-3 edits. Never use Tailwind colour utility classes (`text-rose-500`, `bg-violet-400`) — always `style={{ color: 'var(--accent-...)' }}`.
**Excerpt (page.tsx line 150):**
```tsx
style={{ borderColor: 'var(--accent-rose)', color: 'var(--accent-rose)' }}
```
**Phase-3 surgical edit pattern:** change the token name, not the property key.

### B. Section-component architecture
**Source:** `app/page.tsx` `HomePage()` at lines 66–79 — 12 section components composed linearly.
**Apply to:** D-14 new `<EmailCaptureSection>` (if adopted) — slot into `HomePage()` between `DownloadCTA` and `PatreonCTA`. Do NOT add new sections elsewhere.
**Excerpt:**
```tsx
export default function HomePage() {
  return (
    <div>
      <Hero />
      <RainbowStrip />
      <Features />
      <SignalFlow />
      <Screenshots />
      <Pipeline />
      <AboutAndrew />
      <MediaSlots />
      <SysReq />
      <DownloadCTA />
      {/* <EmailCaptureSection /> — insert here if D-14 Option A */}
      <PatreonCTA />
    </div>
  );
}
```

### C. House image treatment
**Source:** AboutAndrew pattern (D-15 lock):
```tsx
className="aspect-square rounded-lg object-cover border"
style={{ borderColor: 'var(--border-subtle)' }}
```
**Apply to:** headshot (D-15), any future square image. Do NOT add `<figure>`, shadow, or outer wrapper. Rectangular images (`signal-flow.png`, feature-card thumbnails) keep `border-dim` instead — see page.tsx line 279.

### D. Inline hyperlink style
**Source:** `AboutAndrew()` page.tsx line 540; `Footer.tsx` line 16; `Pipeline()` page.tsx line 453.
```tsx
className="underline underline-offset-4"
style={{ color: 'var(--accent-stellar)' }}
```
**Apply to:** all new inline hyperlinks in Phase 3 sections. **Exception:** `privacy/page.tsx` uses `className="underline"` without offset — leave as-is (not in D-06 scope).

### E. Playwright test-describe shell
**Source:** `tests/homepage-content.spec.ts` lines 1–11.
```ts
import { test, expect } from '@playwright/test';

test.describe('<feature-area>', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto('<path>');
  });
  /* tests */
});
```
**Apply to:** `tests/a11y.spec.ts`, `tests/og-metadata.spec.ts`. Skip `beforeEach` for `asset-existence.spec.ts` (filesystem, no page navigation needed).

### F. Belt-and-braces negative assertion (Pitfall 3 mitigation)
**Source:** `tests/homepage-content.spec.ts` line 52.
```ts
await expect(page.locator('body')).not.toContainText('<old-value>');
```
**Apply to:** D-13 email migration in BOTH `homepage-content.spec.ts` and `privacy-content.spec.ts`. Also to Tally references once Sender.net is wired (`'Tally Technologies SRL'`, `'tally.so'`).

### G. `as const` for data-driven arrays
**Source:** `FEATURES` lines 10–51, `TAPS` lines 4–8, hero-stats inline array lines 181–186.
**Apply to:** any new array literal that drives rendered content (e.g. a new `REQUIRED_ASSETS` constant in `asset-existence.spec.ts`). Preserves type inference.

### H. Tailwind 4 `@theme inline` bridge
**Source:** `globals.css` lines 49–65.
**Apply to:** only if a new Tailwind utility class (`bg-regal-osd`) is needed. Current pattern uses raw `var(--accent-...)` in inline styles; no bridge required for D-12.

### I. Next.js 16 `<Image>` with `unoptimized` (global)
**Source:** `next.config.ts` line 5: `images: { unoptimized: true }`.
**Implication:** no `unoptimized` prop needed on individual `<Image>`. SVG auto-unoptimizes regardless (per RESEARCH.md §SVG handling). **Do NOT add `qualities: [75]` to any new `<Image>`** — N/A because global `unoptimized`.

---

## No Analog Found

Files with no close match in the site repo; planner should use RESEARCH.md patterns or external docs:

| File | Role | Data Flow | Reason / Source |
|------|------|-----------|-----------------|
| `tests/a11y.spec.ts` | test / axe-core | third-party lib | No prior axe-core usage in repo. Use RESEARCH.md §Axe test fixture (Playwright-official pattern from `playwright.dev/docs/accessibility-testing`). |
| `tests/asset-existence.spec.ts` | test / filesystem | file-I/O | No prior filesystem tests in `tests/` — all existing tests are content-regression. New pattern: sync Node `fs` calls inside Playwright `test()` blocks, no `page` fixture. |
| `scripts/contrast-check.mjs` | utility / one-off | batch | No prior `scripts/` in the site repo root. This is optional; follows Node-script convention (RESEARCH.md §Manual-pair verification provides full skeleton). |
| Sender.net universal.js integration | component / SaaS embed | client JS → SaaS | Only Tally integration exists. RESEARCH.md §D-14 Disposition provides the Sender.net code pattern (script-inject + `<div class="sender-form-field">`). **Phase 2 is the canonical source** if it ships before Phase 3 executes. |

---

## Metadata

**Analog search scope:** `/Users/andrewrahman/conductor/repos/andrewrahman-com/` — `app/`, `components/`, `tests/`, config files. Plugin repo at `/Users/andrewrahman/conductor/repos/openspatialdelay/` deliberately excluded (JUCE C++, not relevant to Next.js site).
**Files scanned:** 11 analog files read end-to-end (`page.tsx`, `globals.css`, `layout.tsx`, `privacy/page.tsx`, `get-osd/page.tsx`, `Nav.tsx`, `Footer.tsx`, 3 × test specs, `package.json`, `playwright.config.ts`, `next.config.ts`, `_headers`).
**Pattern extraction date:** 2026-04-16.
**Critical convention to reinforce for executor:** this is Next.js 16 — read `node_modules/next/dist/docs/` before trusting training-data Next.js conventions. See `../andrewrahman-com/AGENTS.md` for the binding warning.

---

## PATTERN MAPPING COMPLETE
