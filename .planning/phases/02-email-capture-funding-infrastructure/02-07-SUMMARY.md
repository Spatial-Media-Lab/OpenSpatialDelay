---
phase: 02-email-capture-funding-infrastructure
plan: 07
status: complete
date: 2026-04-16
---

# Plan 02-07 — Dark-Mode Homepage Redesign — SUMMARY

## What shipped

A single, polished, dark-mode homepage at `andrewrahman.com` that consolidates feedback on
the three round-1 prototypes. The site is now live at the Netlify preview URL — Netlify will
rebuild from the new `main` automatically:

**👉 Review here: <https://silly-licorice-0ee82d.netlify.app/>**

(Allow ~30–60s after the push for the build to land.)

## Sibling-repo commits (origin/main, pushed)

| SHA       | Message                                                                  |
|-----------|--------------------------------------------------------------------------|
| `094d2e1` | `chore(designs): remove round-1 prototypes and picker after user feedback` |
| `5b7b04d` | `feat(homepage): dark-mode redesign with plugin palette + 12-tap brand motif` |
| `88686b2` | `test(homepage): content regression suite for key copy`                  |

Repo: `https://github.com/AndrewRahman/andrewrahman-com` — branch `main`.

## What changed

**Removed (clean slate):**
- `app/design-1/`, `app/design-2/`, `app/design-3/`, `app/designs/`
- `components/design-1/`, `components/design-2/`, `components/design-3/`
- The temporary "Preview design prototypes" banner from `app/page.tsx`

**Added / rewritten:**
- `app/page.tsx` — 10 sections in the user-mandated order: Hero (plugin) → 12-tap rainbow strip
  → Features grid (5 features × tap colours) → Signal-flow figure → Screenshots polaroid row
  → Spatial Media Library pipeline → About Andrew (~250 words of prose, with skill pills and
  email contact) → Demo video + audio placeholders → System requirements (mono table) →
  Download CTA (`--accent-stellar`) → Patreon block (`--accent-rose`, full-width).
- `app/layout.tsx` — Fraunces (display, with `opsz` axis), Inter (body), JetBrains Mono (specs)
  via `next/font/google`. New OG/Twitter metadata using `screenshot_full.png` (1640×1160).
- `app/globals.css` — full plugin palette as CSS custom properties (sourced from
  `Source/PluginEditor.cpp`), Tailwind v4 `@theme inline` aliases, `.font-display` /
  `.font-mono-osd` / `.polaroid` helpers, `prefers-reduced-motion` handling.
- `components/Nav.tsx` — sticky dark nav with both top-of-page CTAs (Download OSD + Patreon).
- `components/Footer.tsx` — dark footer with stellar-blue accent links.
- `app/privacy/page.tsx` + `app/get-osd/page.tsx` — colours swapped to the dark palette so
  they no longer render `#111111` text on a `#03060b` background. **Prose untouched** — every
  asserted privacy string still reads exactly as the legal team wrote it.

**Tests:**
- `tests/homepage-content.spec.ts` — new content-regression suite (7 tests) asserting:
  hero names plugin + platforms; GPL-3.0 + KU100; Berlin + Spatial Media Library; mailto link;
  every Patreon link points at the **real** URL (and no placeholder slug remains anywhere on
  the page); 12-tap rainbow strip is rendered; CTAs appear at top + bottom.
- `tests/smoke.spec.ts` — first test rewritten for new hero copy, footer test scoped to the
  `<footer>` role (the Pipeline section also links to github.com/Spatial-Media-Lab now).

**Total: 17/17 tests passing.** `npm run build` clean (4 static routes prerender).

## Design direction — what was synthesized

| User feedback                                                  | How the redesign answers it                                     |
|----------------------------------------------------------------|------------------------------------------------------------------|
| "Dark mode."                                                   | `--bg-void #03060b` is the page background everywhere.          |
| "Colors should come from the plugin itself."                   | Every accent comes from `Source/PluginEditor.cpp`. Nothing invented. |
| "D1 layout good, but focuses too heavily on plugin."           | Plugin still leads (per content-order spec), but the About-Andrew section gets ~250 words of prose, a 280px portrait block, a 6-pill skills row, and direct email contact — genuine real-estate, not a footer blurb. |
| "D2: love the fonts but boring."                               | Fraunces + Inter + JetBrains Mono via `next/font/google`. Variable `opsz` axis on display headlines. Saturated accent blocks (Patreon section is full-width `--accent-rose`) keep it from feeling beige. |
| "D3: love colorful + kooky, but no SUPERBOLD CAPSLOCK."        | No Archivo Black. No 120px all-caps hero. No ASCII dividers. The "kooky" energy comes from: the 12-tap rainbow strip under the hero, per-feature tap-coloured dots, Andrew's skill pills cycling through tap colours, and the rose-coloured Patreon block. |
| Real Patreon URL — `https://www.patreon.com/c/AndrewRahman`    | Wired to nav, hero, and bottom block. A test asserts every Patreon `href` matches and that the old slug is absent. |
| About content from `andrewrahman.weebly.com`                   | Synthesized — bio mentions 20+ years, the breadth (composing, touring, scoring, sound design, producing, DJing, mixing, instrument design, spatial audio, research, education), Berlin base, plugin-development pivot, and the Spatial Media Library framing as the bigger project. Skill tags: Production · Sound Design · 3D Audio · Scoring · Mixing · Plugin Development. |

## Success-criteria checklist

- [x] Round-1 routes deleted (`app/design-1`, `app/design-2`, `app/design-3`, `app/designs`)
- [x] Round-1 component dirs deleted (`components/design-1..3`)
- [x] `app/page.tsx` is the new polished homepage with all 10 required sections in order
- [x] `--bg-void #03060b` is the global background
- [x] All accents from plugin palette — no invented colours
- [x] 12-tap rainbow band is prominent (under the hero, full viewport width, `role="img"`)
- [x] Fonts: Fraunces + Inter + JetBrains Mono via `next/font/google`
- [x] No all-caps hero, no Archivo Black, no ASCII dividers
- [x] Andrew section: ~250 words of prose (well over the 200-word floor)
- [x] Real Patreon URL everywhere — placeholder slug fails the regression test
- [x] `/`, `/privacy`, `/get-osd` all render and pass smoke
- [x] New `tests/homepage-content.spec.ts` passing (7/7)
- [x] `npm run build` clean
- [x] `npm run test` clean (17/17 passing)
- [x] Three commits in spec order, pushed to `origin/main`
- [x] SUMMARY at `.planning/phases/02-email-capture-funding-infrastructure/02-07-SUMMARY.md`

## What to review

1. **Open <https://silly-licorice-0ee82d.netlify.app/>** in a browser (give Netlify ~30–60s).
2. **Top of page:** sticky nav with Download (stellar) + Patreon (rose) CTAs always visible.
3. **Hero:** display-serif headline in three lines, large screenshot, big-numbers row.
4. **Below the hero:** a thin full-width strip of the 12 tap colours — the brand motif.
5. **Features grid:** 5 cards, each tagged with one tap colour and a small product screenshot.
6. **About Andrew:** the section the user asked for more of. Confirm the prose feels like *you*.
7. **Patreon block at the bottom:** full-width rose, real campaign URL, $3 floor mentioned.
8. **Privacy + Get-OSD pages:** check they read cleanly on the new dark palette.

## Known carry-overs (non-blocking)

- **Headshot** is a placeholder (`AR` monogram on a tap-coloured radial gradient). Tagged with
  a `TODO` comment in the source — drop a `/public/assets/andrew.jpg` and swap the `<div>` for
  an `<Image>` when you have one.
- **Demo video + audio** are deliberate placeholders captioned "coming in Phase 4".
- **Source on GitHub link** in the Download CTA points at `Spatial-Media-Lab/OpenSpatialDelay`
  — confirm this is the right slug once the repo goes public (currently private).

## Iteration loop

If anything wants changing — colours, sections in a different order, copy edits, the headshot —
just tell me what to tweak and I'll do another round. Plan 02-07 is closed for the autonomous
phase; iterations from here are conversational.
