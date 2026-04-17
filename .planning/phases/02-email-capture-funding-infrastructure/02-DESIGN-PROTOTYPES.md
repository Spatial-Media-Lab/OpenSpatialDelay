# 02 — Landing Page Design Prototypes

Three distinct landing-page prototypes built on the `andrewrahman-com` sibling repo for side-by-side comparison at the live Netlify preview. All three share identical content (all 11 required sections); only the visual treatment differs.

**Picker:** https://silly-licorice-0ee82d.netlify.app/designs/

**Current minimal homepage:** https://silly-licorice-0ee82d.netlify.app/ (now also carries a temporary banner linking to `/designs/` — remove after selection).

---

## /design-1/ — Plugin-brand

**URL:** https://silly-licorice-0ee82d.netlify.app/design-1/

**Anchor:** FabFilter / Baby Audio / forever89.studio / electricsmudge.com.

**Fonts (next/font/google):**
- `DM Sans` (weights 400, 500, 600, 700, 800) — full page.

**Key decisions:**
- **Accent:** `#FF4D12` (warm orange from the "forever89 + electric smudge" reference pair). Dark ink `#0A0A0A`.
- **Background:** white with warm gradient hero (`#ffe8dd → #ffd2be → #ff4d12`) and checkered stat-band.
- **Shape language:** 2px black borders + hard `4px 4px 0 #0a0a0a` shadow offsets. Buttons translate on hover.
- **Hero:** headline + feature chips + pair of CTAs at left, full plugin screenshot in an offset-shadow frame at right.
- **Distinctive moves:** tilted "Price — Free" stamp, big-number band (5, ±12, 2048, ∞), feature cards with numbered dividers, dark download section with side-by-side system-requirements table.

---

## /design-2/ — Developer-editorial

**URL:** https://silly-licorice-0ee82d.netlify.app/design-2/

**Anchor:** raviklaassens.com / danielrudrich.de / Linear / Vercel.

**Fonts (next/font/google):**
- `Fraunces` (weights 300, 400, 500, 600; italic 400) — display headlines + italic emphasis.
- `Inter` (weights 400, 500, 600) — body.

**Key decisions:**
- **Background:** `#fafaf8` warm-paper off-white (committed to white route per brief; the mild warmth helps the serif display).
- **No saturated accent.** Grayscale hierarchy only: `#0a0a0a` ink, `#6b7280` secondary, `#9ca3af` tertiary, `#d4d4d4` hairlines.
- **Typography:** 96px Fraunces serif headlines with italic contrast phrases ("built in public", "inhabit", "coming soon"). Size contrast is the hierarchy.
- **Header:** live Berlin-time clock (`'use client'`, `Europe/Berlin`, ticks every 1s — raviklaassens' "9:00:24 CET" homage).
- **Layout:** asymmetric 12-col grid with section numbering (`01 / … 08 /`), arrow links that expand gap on hover, `Fig. 01 / Fig. 02 / Fig. 03` captioned images with carousel pagination (1/3, 2/3, 3/3).
- **No gradients, no effects.** Typography and whitespace do all the work.

---

## /design-3/ — Arts-driven

**URL:** https://silly-licorice-0ee82d.netlify.app/design-3/

**Anchor:** Teenage Engineering / Are.na / Berlin-gallery looseness.

**Fonts (next/font/google):**
- `Archivo Black` (weight 400) — giant single-word display blocks.
- `JetBrains Mono` (weights 400, 500, 700) — spec rows, captions, body prose.

**Key decisions:**
- **Background:** `#F4F1EB` warm paper.
- **Color palette:** electric blue `#0038FF`, mustard `#FFB800`, ink `#0A0A0A`, paper `#F4F1EB`. Used as full-bleed blocks, checkerboard-style.
- **Hero:** 4-tile grid breaking "SPATIAL / DELAY / FOR / MUSICIANS" across cream/blue/ink/yellow blocks. No hero screenshot — it comes later as a polaroid.
- **Polaroid screenshots:** white paper border, small rotations (`-2deg`, `-0.5deg`, `+1.5deg`), dropshadow, mono caption at bottom ("OpenSpatialDelay · UI / 001", "Orbit / 002", etc.).
- **Dividers:** ASCII-ish `◼ ◻ ◼ ◻ …` glyph rows between sections with a small uppercase label.
- **Tech specs:** dashed-border dl rows, mono font, blue keys.
- **About block:** two side-by-side full-bleed blocks (blue About vs mustard Pipeline).
- **Patreon block:** giant ink-on-cream 112px "Fund / the / pipeline" with yellow section label.

---

## Fonts summary (exact Google family names)

| Route | Fonts | Roles |
| --- | --- | --- |
| `/design-1/` | `DM Sans` | Display + body |
| `/design-2/` | `Fraunces` + `Inter` | Display (serif, with italic) + body (sans) |
| `/design-3/` | `Archivo Black` + `JetBrains Mono` | Display blocks + mono body/specs |

Each prototype loads its fonts only on its own route (scoped in that route's `layout.tsx` using `next/font/google`).

---

## Shared content (all 11 sections, per brief)

All three prototypes contain:
1. Hero (headline, subheadline, primary CTA to `/get-osd`, secondary to `https://patreon.com/AndrewRahman`)
2. Product showcase with screenshots + 5 feature callouts
3. Video demo placeholder (16:9, "Phase 4" label, no real URL)
4. 30-second audio demo placeholder (HTML5 `<audio>` with no `src`, "pending" note)
5. Tech / HRTF explainer using `signal-flow.png`
6. About Andrew (Berlin, GPL ethos, builds in public)
7. Spatial Media Library pipeline positioning paragraph
8. System requirements (macOS Apple Silicon+Intel 11+, Windows 10/11 x64, VST3+AU, 4GB RAM)
9. Download section with CTA to `/get-osd` + secondary GitHub link to `github.com/Spatial-Media-Lab/OpenSpatialDelay`
10. Patreon CTA with `https://patreon.com/AndrewRahman` (vanity URL locked to `AndrewRahman` mixed-case; Patreon URL resolver is case-insensitive)
11. Existing `<Footer />` from `components/Footer.tsx` (untouched)

All accuracy anchors match codebase:
- 5 HRTF profiles by name (KU100, CIPIC, HUTUBS, MIT KEMAR, SADIE)
- 2048-point phase vocoder with Laroche–Dolson phase locking + Röbel-style transient detection
- Partitioned convolver, lock-free processBlock
- ITD / ILD spatial model
- ±12 semitones Doppler range
- 2048-sample PDC-reported latency, dry path latency-compensated
- VST3 + AU only (no other formats invented)
- macOS 11+ Apple Silicon + Intel; Windows 10/11 x64
- 4 GB RAM recommended

---

## Deviations from brief

- **Design 2 background:** brief said "off-white `#FAFAF8` or pure white — I'd go WHITE". I committed to `#fafaf8` (the warmer off-white value from the brief) because the Fraunces serif renders slightly better on a warm page than stark white. Grayscale hierarchy is still the only decoration; no saturated accent. If stark white is preferred, a single variable swap in `design-2.module.css` covers it.
- **Design 1 accent:** chose `#FF4D12` (warm orange) over `#E91E63` (magenta) per brief's first suggestion.
- **Design 3 hero phrase:** brief offered "SPATIAL / DELAY / FOR / MUSICIANS" split across 4 tiles — used that exact phrase and split.
- **Vanity Patreon URL:** locked to `https://patreon.com/AndrewRahman` (mixed case, canonical per Patreon vanity registration 2026-04-17). Patreon URL resolver is case-insensitive so pre-existing lowercase inbound links continue to resolve.
- **Layout refactor (commit 1):** took Option 2 from the brief — restructured `app/layout.tsx` so `<main>` has no max-width and each route sets its own container locally. Smoke + privacy-content tests (10/10) still pass.
- **Existing `<Nav />` and `<Footer />`:** untouched and wrap every prototype automatically via the root layout, per brief.

---

## Commits pushed (origin/main)

```
0b2cfbe feat(design-3): arts-driven prototype (TE/Are.na vibe)
fd65705 feat(design-2): developer-editorial prototype (raviklaassens/danielrudrich vibe)
f575273 feat(design-1): plugin-brand prototype (FabFilter/Baby Audio vibe)
1ed53da feat(designs): add picker page and temporary homepage banner
2a514db refactor(layout): move max-width wrapper from layout.tsx into each route
```

Netlify will rebuild from `main`. Wait ~1–2 minutes after push for the preview URLs to reflect the new routes.

---

## Definition of done

- [x] `/design-1/`, `/design-2/`, `/design-3/` all build and render (`out/*/index.html` verified after `npm run build`)
- [x] Each visually distinct (different fonts, different palettes, different layouts, different hero strategies — not color swaps)
- [x] All 11 required content sections in each prototype
- [x] Existing routes (`/`, `/privacy`, `/get-osd`) still work; 10/10 Playwright tests pass
- [x] Picker at `/designs/` links to all 3
- [x] Homepage carries temporary banner to `/designs/` (marked for removal after selection)
- [x] Pushed to origin/main — Netlify rebuild triggered
- [x] This SUMMARY committed in OSD repo

---

## Removal checklist (after design selection)

1. Delete the two losing `app/design-N/` directories and their `components/design-N/` counterparts.
2. Rename the chosen `app/design-X/page.tsx` content into a new `app/page.tsx` (or redirect from `/` to `/design-X/`).
3. Delete `app/designs/page.tsx`.
4. Remove the temporary banner block in `app/page.tsx` (marked with `TEMPORARY banner` comment).
5. Optional: fold the chosen design's `layout.tsx` font imports into the root `app/layout.tsx` so fonts load on every page.
