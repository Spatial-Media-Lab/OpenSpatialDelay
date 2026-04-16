---
phase: 03-personal-website
type: backlog
status: open
date_captured: 2026-04-16
captured_during: post-02-07-review
---

# Phase 3 — Personal Website Polish Backlog

User reviewed the Plan 02-07 dark-mode redesign at <https://silly-licorice-0ee82d.netlify.app/>
on 2026-04-16 and signed off on the overall direction ("This is the winner"). The following
items were captured for Phase 3 to take to completion. Not to be acted on during Phase 2.

## Items

### B-1 — Refresh out-of-date screenshots from v1.0.0 plugin state
Several `public/assets/screenshot_*.png` files in the andrewrahman-com repo were captured
against an older plugin state and no longer match v1.0.0 (release commit `768c248`). Re-capture
all screenshots used by the homepage from the current built v1.0.0 plugin so the marketing
imagery matches what the user actually downloads.

**Affected files (current homepage references):**
- `public/assets/screenshot_full.png` (hero — 1640×1160, also OG image)
- `public/assets/screenshot_spatial_map.png` (Feature 01)
- `public/assets/screenshot_wobble.png` (Feature 02)
- `public/assets/screenshot_orbit.png` (Feature 03)
- `public/assets/screenshot_elevation_map.png` (Feature 04)
- `public/assets/screenshot_header.png` (Screenshots row)
- `public/assets/screenshot_bottom_panel.png` (Screenshots row)
- `public/assets/screenshot_right_panel.png` (Screenshots row)
- `public/assets/screenshot_shimmer.png` (Screenshots row)
- `public/assets/screenshot.png` (Screenshots row)
- `public/assets/signal-flow.png` (Under the hood)

This overlaps with Phase 4 (Demo Content) success-criteria #3 — high-resolution screenshots
for KVR + social. Coordinate so the captures are done once, used in both places.

### B-2 — Re-pitch the feature highlights against actual v1.0.0 capabilities
The homepage currently leans on three specifics that don't represent the v1.0.0 plugin
accurately:

- **±12 semitones** — pitched as a Doppler-engine number; user says this isn't the right
  framing for v1.0.0.
- **2048-sample FFT** — pitched as a hero stat; user says this is internal plumbing, not a
  feature people care about.
- **GPL-3.0** — pitched as a hero stat; user wants this de-emphasised in the hero stats row
  (still keep it on the Download CTA, since it's a meaningful positioning point there).

User will conduct a dedicated interview during Phase 3 planning to nominate which features
should be foregrounded instead. Until then, treat the Features section as a placeholder.

**Affected sections in `app/page.tsx`:**
- `Hero()` — the four-stat `<dl>` (lines ~155–180)
- `Features()` — `FEATURES` array entries 1, 2 (Doppler + Phase vocoder copy)

### B-3 — Tighten taglines throughout the page
User flagged that the taglines/section copy can be improved across the page — the structure
is right, the words need work. Treat as a copy-editing pass once the v1.0.0 feature pitch is
locked (depends on B-2).

**Sections to revisit:**
- Hero headline (`OpenSpatialDelay — a 3D spatial delay for VST3 + AU on macOS & Windows.`)
- Features section subhead ("Five things, in order of how surprising they feel the first time
  you hear them.")
- Pipeline section copy
- Download CTA headline
- Patreon block headline ("Fund the pipeline. Free + open-source forever.")

### B-4 — Replace SML logo with the white SVG asset
User has a clean white SVG logo at `/Users/andrewrahman/Downloads/Web-Logo, white.svg` (8.6KB,
confirmed exists 2026-04-16). Currently the homepage uses `public/assets/sml-logo.png` in two
places:
- `Pipeline()` section right column (180×180 logomark in a panel)
- `FEATURES[4]` (the "Free & open-source" feature card thumbnail)

**Action:** copy the SVG into `andrewrahman-com/public/assets/sml-logo.svg`, swap the `<Image>`
`src` to the SVG, and remove the now-unused PNG. Also re-render the logomark in any other
asset that uses it (favicon, OG fallback) once Phase 3 finalises the asset inventory.

### B-5 — Recolour the Patreon CTA from rose to a light regal purple
Current Patreon block uses `--accent-rose #e467a6` (full-width). User wants it changed to a
light hue of a "regal and rich" purple instead.

**Suggested approach for Phase 3:**
- The plugin palette already has `--accent-violet #7457d1` (the tone-section accent). Try a
  *lighter* hue derived from this — e.g. `oklch(72% 0.15 290)` or similar — rather than
  darkening it. The block needs to read as warm and welcoming, not nightclub.
- Pair with `--bg-void #03060b` text (existing pattern) so the dark text on light-purple is
  readable.

**Affected sections:**
- `PatreonCTA()` in `app/page.tsx`
- The `Support on Patreon` ghost button colour in `Nav.tsx` and `Hero()` — currently uses
  `--accent-rose` for outline + label. Should change to match the new purple.

### B-6 — Switch contact email to `andrew@spatialmedialab.org`
User has a vanity address on the SML domain. Replace `andrewjrahman@gmail.com` everywhere on
the public website with `andrew@spatialmedialab.org`.

**Affected files:**
- `app/page.tsx` — `AboutAndrew()` "Get in touch" line + the `mailto:` `href`
- `app/privacy/page.tsx` — **CAUTION:** the privacy policy uses
  `andrewjrahman@gmail.com` in 6 places (rights-request contact, unsubscribe, data controller).
  Switching the controller-of-record contact has GDPR implications — confirm with user whether
  the legal contact stays Gmail (personal) and only the marketing/general contact moves to
  the SML domain, or whether both move.
- `app/get-osd/page.tsx` — does not currently reference the email, but verify.
- `tests/privacy-content.spec.ts` — asserts `andrewjrahman@gmail.com`. Update to whichever
  address survives.
- `tests/homepage-content.spec.ts` — asserts `andrewjrahman@gmail.com`. Update.

### B-7 — Make the Get-OSD CTA scroll instead of route — IF technically possible
User would prefer the "Download OSD" / "Get OSD" CTAs to scroll to a download section at the
bottom of the homepage rather than navigate to `/get-osd/`.

**Open question:** the gated download flow currently lives at `/get-osd/` because the Tally
form embed + the binary-handoff flow (Phase 4 → Plan 02-04 follow-up) needs a dedicated page
URL for the email-capture redirect target. Verify with the Tally implementation whether the
form can live inline on the homepage (anchor scroll) without breaking the "submit → redirect
to GitHub Releases" handoff documented in REQUIREMENTS DIST-01/02.

**If feasible:**
- Add an `<EmailCaptureSection id="get-osd">` to the bottom of `app/page.tsx` (above
  `PatreonCTA`).
- Change every `Link href="/get-osd/"` to `<a href="#get-osd">` with smooth-scroll behaviour.
- Either delete `app/get-osd/page.tsx` entirely or keep it as a dedicated landing surface for
  inbound campaign links and have it redirect to `/#get-osd`.

**If not feasible** (Tally redirect requires dedicated route, or analytics requires page
isolation): leave the route-navigation as-is and move on. Document the constraint in this
backlog item.

### B-8 — Replace `AR`-monogram headshot placeholder with the SML About headshot
Source: <https://spatialmedialab.org/about/> — pull the headshot used there and drop it into
`public/assets/andrew.jpg`. Then in `app/page.tsx > AboutAndrew()`:
- Remove the placeholder `<div>` (the gradient block with the `AR` monogram and the
  `TODO: replace with headshot` comment)
- Add `<Image src="/assets/andrew.jpg" alt="Andrew Rahman" width={560} height={560} className="aspect-square rounded-lg object-cover border" style={{ borderColor: 'var(--border-subtle)' }} />` in its place

**Image-rights note:** confirm the SML site headshot is a photo Andrew owns / has rights to
republish on andrewrahman.com (likely yes since SML is his org, but worth a 5-second check).

## Suggested execution order

When Phase 3 planning starts, sequence these as:

1. **B-2 interview** — Andrew nominates the v1.0.0 features to highlight. Until this is locked,
   B-1 (screenshot list) and B-3 (taglines) can't be finalised.
2. **B-1 + B-8** — capture all the new screenshots + the headshot in one session.
3. **B-3** — copy editing pass against the new feature list.
4. **B-4 + B-5 + B-6 + B-7** — design / contact / IA tweaks (independent of each other; can
   parallelise).
5. Final review against Phase 3 success criteria + WEB-01/02/03 requirements.
