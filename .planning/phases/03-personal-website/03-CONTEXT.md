# Phase 3: Personal Website — Context

**Gathered:** 2026-04-16
**Status:** Ready for planning
**Launch deadline:** 2026-05-01 (15 days from context capture)

<domain>
## Phase Boundary

Phase 3 delivers the public-facing landing page for OpenSpatialDelay on `andrewrahman.com` — the hub that connects the GitHub v1.0.0 release, the Sender.net email-capture flow, the Patreon page, and the SML site. The base dark-mode design (signed off on 2026-04-16 as "the winner") is already live on Netlify preview at `silly-licorice-0ee82d.netlify.app`. This phase **finalises** that page: re-pitches the hero stats and feature cards against the actual v1.0.0 capability set, refreshes stale screenshots, swaps placeholders for real assets (headshot, SML logo), migrates contact addresses to the SML domain, and runs a narrowly-scoped accessibility audit on colour contrast.

**Scope anchor:** landing-page finalisation, not a new page or a design overhaul.

**Out of scope (carried into a post-launch phase):** formal DESIGN-SYSTEM.md authoring, Patreon-generator token retrofit, plugin token alignment (v1.1 territory), SpatialCore-repo style-guide migration.

**Struck from original scope:** WEB-02 (SML About Us page). Verified complete off-site during this discussion — `spatialmedialab.org/about` already lists Andrew Rahman (co-founder 2017), Basel Naouri (joined 2025), and Timo Bittner (co-founder 2017, current president). No placeholder sections remain. REQUIREMENTS.md WEB-02 row should flip to Complete; Phase 3 does not need to deliver anything for it.

</domain>

<decisions>
## Implementation Decisions

### Feature pitch & copy (B-2, B-3)

- **D-01:** Hero grid keeps the 4-stat structure. Four replacement stats are locked:
  1. **12 Delay Taps** (replaces "12 echoes in 3D space" — terminology change)
  2. **7 spatialization algorithms** (new)
  3. **5 HRTF profiles** (new; carries a "+1 CPU-lite mode" footnote/tooltip for precision — see `memory/reference_spatialization.md` and `Source/PluginProcessor.cpp`)
  4. **70 factory presets** (new; source: `Source/PresetData.cpp:73`)
- **D-02:** The old stats are explicitly retired from the page: `±12 semitones` (not the right framing for v1.0.0), `2048-sample FFT` (internal plumbing, not a user-facing number), and `GPL-3.0` (kept on the Download CTA and the 6th feature card, removed from the hero stats row).
- **D-03:** Feature-card count goes from 5 → **6 cards** (even number, pro-user-forward slate):
  1. **12 taps in 3D space** — core concept, each echo has its own 3D position.
  2. **Trajectory engine** — 12 echoes, 12 configurable paths; in sync with tempo or free-running.
  3. **Measured HRTFs** — 5 real profiles (KU100, CIPIC, HUTUBS, MIT KEMAR, SADIE); elevation + azimuth from measurements, not generic pan laws.
  4. **7 spatialization algorithms** — Ambisonics (HOA), ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP. Signals depth to spatial-audio pros; gives an editorial reason why a film/VR/live practitioner picks this over a generic panner.
  5. **OSC in/out (ADM-OSC)** — drives or is driven by Spat Revolution, Ircam Panoramix, Iannix, TouchDesigner. Power-user differentiator. **Verify mapping details in `Source/OSCController.*` during planning.**
  6. **Free & open-source** — GPL-3.0, source on GitHub under Spatial Media Lab; the DSP kernel is the foundation for every future Spatial Media Library tool.
- **D-04:** The current phase-vocoder Doppler card (card 2 in the live page) **is dropped from the hero slate**. The feature still exists in the plugin; it just doesn't earn a card on the public landing. Moves to the "Deferred Ideas" list for later marketing surfaces (blog, demo video).
- **D-05:** Hero audience framing is unchanged (music producers first, sound designers second). No tone rewrite.
- **D-06:** B-3 (tagline/copy polish) is **narrowly scoped for Phase 3**: only the sections touched by the new feature pitch (hero headline, 6 new feature-card headlines + bodies, Download CTA headline) get rewritten this phase. Full-page copy review (pipeline copy, Patreon headline, About Andrew copy) is deferred to a post-launch pass.

### Design system scope (Area 2 — deliberately narrowed)

- **D-07:** **Formal style-guide authoring is deferred to a post-launch phase** (candidate "Phase 6 — design-system consolidation"). Rationale: May-1st launch is 15 days out; WEB-01/02/03 do not require a formal design system; the site's colour scheme already looks right; drift is ~20% in supporting tokens only (not in the 12-tap rainbow, which is already canonical everywhere).
- **D-08:** In Phase 3, the design-system work is **a single task: WCAG-AA accessibility/contrast audit of current site tokens**. Scope: run axe-core + manual contrast check on every colour pairing in `andrewrahman-com/app/globals.css` and `app/page.tsx` (text-primary/secondary/dim on bg-void/bg-panel; every accent on its background; 12-tap rainbow text legibility). Fix any pair that fails AA 4.5:1 (normal text) or 3:1 (large text / UI).
- **D-09:** When the style-guide phase happens post-launch:
  - It will live **first in `andrewrahman-com/`** (site repo) because that's the most recent design and the audit finalises values there.
  - **After launch-and-stabilisation**, it migrates to a shared **SpatialCore** repo (plugin + future SML tools + site all pull from it). This is captured as a deferred idea so it doesn't get lost.
  - Format: Markdown + JSON (human reference + machine-consumable tokens for build systems and the Patreon generator).
  - Canonical values: plugin wins by default (longest-lived), **unless** the accessibility audit in Phase 3 forces a recolour for AA compliance — then the audit-fixed value becomes canonical and the plugin aligns in v1.1.
  - Retrofit order when that phase happens: site → Patreon generator → plugin (v1.1 cycle).

### Assets, identity & IA (B-1, B-4…B-8)

- **D-10:** **B-1 — Screenshot refresh.** All homepage PNGs currently in `andrewrahman-com/public/assets/screenshot_*.png` are recaptured from a running v1.0.0 build (release commit `768c248`). Coordinate capture with Phase 4's CONT-03 (high-res screenshots for KVR + social) so captures are done once and reused. **Specific PNGs** (cross-ref backlog B-1): `screenshot_full.png` (hero + OG), `screenshot_spatial_map.png`, `screenshot_wobble.png`, `screenshot_orbit.png`, `screenshot_elevation_map.png`, `screenshot_header.png`, `screenshot_bottom_panel.png`, `screenshot_right_panel.png`, `screenshot_shimmer.png`, `screenshot.png`, `signal-flow.png`. B-1 execution is blocked until D-03 card titles are locked (done) so the planner knows which features each screenshot must illustrate.
- **D-11:** **B-4 — SML logo swap.** Copy `/Users/andrewrahman/Downloads/Web-Logo, white.svg` (8.6 KB, confirmed exists 2026-04-16) to `andrewrahman-com/public/assets/sml-logo.svg`. Update `Pipeline()` section and the feature-card-6 thumbnail `<Image src=…>` from `.png` to `.svg`. Delete the PNG afterward. Also re-render the logomark in any downstream asset that uses it (favicon fallback, OG fallback) once Phase 3 assets are locked.
- **D-12:** **B-5 — Patreon CTA colour.** Change from `--accent-rose #e467a6` to **`oklch(72% 0.15 290)`** — a light regal lavender, hex-approx `#b49bd8`. Pair with `--bg-void #03060b` dark text for readability. Update: `PatreonCTA()` block background + ghost-button outline+label colour in `Nav.tsx` and `Hero()`'s "Support on Patreon" button. The new token should be named `--accent-regal` (or similar) in `globals.css`.
- **D-13:** **B-6 — Email migration.** All references to `andrewjrahman@gmail.com` on the public site migrate to **`andrew@spatialmedialab.org`**, including the privacy-policy data-controller field. This is a full migration, not a split. Triggers:
  - `app/page.tsx` `AboutAndrew()` — visible text + `mailto:` href.
  - `app/privacy/page.tsx` — all 6 occurrences (rights request, unsubscribe, data controller).
  - `app/get-osd/page.tsx` — verify no references; swap if found.
  - `tests/privacy-content.spec.ts` + `tests/homepage-content.spec.ts` — update assertions.
  - **Privacy-policy version bump required** — add a changelog entry and date-stamp the update (GDPR-best-practice: controller-of-record changes warrant notification).
- **D-14:** **B-7 — Get-OSD scroll vs route.** The gsd-phase-researcher investigates whether **Sender.net**'s post-submission redirect flow (Phase 2 decision — Sender.net replaced Tally; see S135 in claude-mem) works from an inline embed on `app/page.tsx#get-osd`. If yes: add an `<EmailCaptureSection id="get-osd">` above `PatreonCTA`, swap every `Link href="/get-osd/"` to `<a href="#get-osd">` with smooth-scroll, and either delete `app/get-osd/page.tsx` or keep it as a campaign-landing redirect to `/#get-osd`. If no (redirect requires a dedicated route): leave as-is, document the constraint in the plan, strike B-7 from the phase.
- **D-15:** **B-8 — Headshot.** Image rights are clean (user owns the SML site; the headshot is his own). Download the headshot from `https://spatialmedialab.org/about/`, save to `andrewrahman-com/public/assets/andrew.jpg`, and replace the AR-monogram placeholder `<div>` in `AboutAndrew()` with an `<Image src="/assets/andrew.jpg" alt="Andrew Rahman" width={560} height={560} className="aspect-square rounded-lg object-cover border" style={{ borderColor: 'var(--border-subtle)' }} />`.

### Claude's Discretion

- Exact **headline copy** for each of the 6 feature cards and the Hero headline — B-3 execution. Planner/executor decides phrasing within the locked feature-pitch direction (D-03).
- Whether to use the exact `oklch(72% 0.15 290)` value or nudge by ≤2 lightness points during the accessibility audit (D-08) if contrast demands it.
- Micro-copy on the "+1 CPU-lite mode" footnote/tooltip attached to the "5 HRTF profiles" hero stat (D-01).

### Folded Todos

No todos folded from `.planning/todos/` during this discussion — the pre-captured 03-BACKLOG.md items (B-1…B-8) already covered the inventory.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project-level planning
- `.planning/ROADMAP.md` §Phase 3 — goal statement, WEB-01/02/03 success criteria, dependency on Phase 2
- `.planning/REQUIREMENTS.md` §WEB-01 / §WEB-02 / §WEB-03 — acceptance criteria. **Note:** WEB-02 should flip to Complete per D-02/scope-strike.
- `.planning/STATE.md` — current progress snapshot
- `.planning/phases/02-email-capture-funding-infrastructure/02-CONTEXT.md` — Phase 2 decisions (Sender.net replacing Tally is the critical one for B-7)
- `.planning/phases/03-personal-website/03-BACKLOG.md` — B-1…B-8 full detail (affected files, rationale, dependencies)
- `.planning/phases/03-personal-website/03-PALETTE-AUDIT.md` — palette drift quantification (informs D-07/D-08)

### Plugin source (stat verification + feature-card copy)
- `Source/PluginProcessor.h:616` — `MAX_OBJECTS = 12` (D-01 stat 1)
- `Source/PluginProcessor.cpp:1801-1807` — 7 spatialization algorithms enumeration (D-01 stat 2, D-03 card 4)
- `HRTF/*.sofa` (5 files: bernschuetz_ku100, cipic_subject_003, hutubs_pp2, mit_kemar_large_pinna, sadie_d2_ku100) — D-01 stat 3, D-03 card 3
- `Source/PresetData.cpp:73` — `NUM_FACTORY_PRESETS = 70` (D-01 stat 4)
- `Source/OSCController.*` — verify ADM-OSC feature claim for D-03 card 5 before writing copy
- `memory/reference_spatialization.md` — corrected algorithm + binauralization reference (fallout from same-day stale-doc cleanup; Woodworth is *not* an HRTF, it's the "Simple (Low CPU)" mode)

### Site source (target files for every decision)
- `../andrewrahman-com/app/page.tsx` — `FEATURES` array at lines 10–51 (D-03 rewrite target), `Hero()` 4-stat `<dl>` at ~155–180 (D-01), `AboutAndrew()` (D-13, D-15), `PatreonCTA()` (D-12), `Pipeline()` (D-11)
- `../andrewrahman-com/app/globals.css` — plugin-palette CSS tokens (D-08 accessibility-audit target, D-12 `--accent-regal` addition)
- `../andrewrahman-com/app/layout.tsx` — OG metadata baseline (WEB-03 verification)
- `../andrewrahman-com/app/privacy/page.tsx` — D-13 migration target (6 email occurrences + version bump)
- `../andrewrahman-com/app/get-osd/page.tsx` — D-14 disposition target
- `../andrewrahman-com/tests/privacy-content.spec.ts` — D-13 assertion update
- `../andrewrahman-com/tests/homepage-content.spec.ts` — D-13 assertion update
- `../andrewrahman-com/public/assets/` — target for B-1 screenshots, B-4 SVG logo, B-8 headshot

### External references
- `https://spatialmedialab.org/about/` — WEB-02 verification source + B-8 headshot source
- `https://silly-licorice-0ee82d.netlify.app/` — current live Phase 2 preview (the "winner" baseline this phase finalises)
- `/Users/andrewrahman/Downloads/Web-Logo, white.svg` — D-11 source asset (8.6 KB, confirmed 2026-04-16)

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `FEATURES` array (`app/page.tsx:10-51`) — data-driven feature cards; D-03 rewrites the 5 entries into 6 entries without touching the `Features()` renderer.
- `SYSREQ` array (`app/page.tsx:53-60`) — already correct for v1.0.0 platforms; no change.
- `plugin-palette` CSS token block in `globals.css` — all 12 tap colours plus `--bg-void`, `--bg-panel`, `--border-subtle`, `--accent-rose/violet/stellar/sync/channel-L/channel-R`. Audit target for D-08.
- Next.js `Image` component imported at top of `page.tsx` — used for all asset swaps in D-11, D-15.

### Established Patterns
- **Section-component architecture:** 13 top-level section components composed in `HomePage()` (`Hero`, `RainbowStrip`, `Features`, `SignalFlow`, `Screenshots`, `Pipeline`, `AboutAndrew`, `MediaSlots`, `SysReq`, `DownloadCTA`, `PatreonCTA`, plus `Nav` from a sibling file). D-03 / D-12 / D-13 / D-15 all stay within existing components — no new section needed except possibly `EmailCaptureSection` (D-14, conditional on research).
- **CSS tokens via `var(--…)`:** every colour is tokenised; D-12 adds a new `--accent-regal` token rather than hard-coding the lavender.
- **`aspect-square rounded-lg object-cover border`** is the established image treatment — D-15 reuses it verbatim.
- **`tapIndex` on each `FEATURES` entry** — assigns a rainbow colour to each card. Need to pick tap indices for the 6 new cards that stay visually distinct (existing: 0, 5, 7, 3, 10 → pick one more around 1, 4, 8, or 11).

### Integration Points
- **Sender.net form embed** (Phase 2 deliverable) — the contract for D-14 (inline vs route). Planner must read the Sender.net integration code in `andrewrahman-com/` before recommending scroll-vs-route.
- **OG image at `/assets/screenshot_full.png`** (`layout.tsx`) — recaptured in D-10; WEB-03 acceptance criteria unaffected as long as the new image is ≥1200×630.
- **Netlify deploy preview** — continues to be the validation surface; no CI changes needed.
- **Plugin repo ↔ site repo** — site is a sibling directory, not a submodule. No cross-repo tooling; changes to the site are standalone PRs against its own main.

</code_context>

<specifics>
## Specific Ideas

- User quote on feature depth: `"13 trajectory shapes I think is not that impressive"` → drove the D-01 swap from trajectories to 70 factory presets in the hero stats.
- User quote on tone: `"Current framing is fine (music producers first, sound designers second)"` → D-05 no tone rewrite.
- User quote on design-system urgency: `"launch deadline is May 1st"` paired with `"I wonder how important this actually is to be able to launch?"` → drove D-07 deferral of formal style-guide work. Strategic call: accessibility audit yes (launch-risk), formal tokens no (post-launch).
- User quote on style-guide home: `"For now it will live in the Site repo, but later, which it is solidified and verified, will move to the SpatialCore repo"` → explicit migration intent captured in D-09.
- User quote on "Simple" confusion: `"on the Spatialization algorithms there is one listed as 'Simple' but what is that?"` → clarified during discussion: there is NO "Simple" in the 7 spatialization algorithms (those are Ambisonics/ConstantPower/DBAP/KNN/MDAP/VBAP/VBIP). "Simple" is the CPU-lite Woodworth mode in the *binauralization* dropdown (5 HRTFs + 1 Simple). Two separate UI choices in the plugin. Same-day stale-doc cleanup (see claude-mem 3335-3374) corrected this across README, SPECIFICATION, PROJECT.md, and memory-reference files — so this confusion should not recur.
- User correction caught during B-7: `"remember that we have switched to Sender.net and no longer are using Tally"` → threaded through D-14 and D-13 (privacy policy references).

</specifics>

<deferred>
## Deferred Ideas

### Post-launch phase candidate — "Design-system consolidation"
- Author formal `DESIGN-SYSTEM.md` + `design-tokens.json` in `andrewrahman-com/` (then migrate to `SpatialCore` repo after site stabilisation).
- Refactor Patreon-generator (`docs/phase-02-evidence/patreon-graphics/_generate.py`) to import shared tokens instead of inlined RGB tuples.
- Plugin token alignment — if the Phase 3 accessibility audit (D-08) forces recolours, those changes land in the plugin in v1.1.

### Post-launch copy pass
- Full-page copy/tagline review of every section (Pipeline copy, Patreon headline, About Andrew copy, SysReq framing). D-06 scoped Phase 3 to only the sections touched by the new feature pitch.

### Hero-page content ideas not fitting the 6-card slate
- **Pitch-shifted Doppler** as a dedicated feature card. Still shipping in the plugin, but lost to the Pro-user-forward slate in D-03. Revisit for blog posts, demo video, KVR listing.

### Reviewed Todos (not folded)
None — pre-captured backlog (03-BACKLOG.md) covered the inventory.

### Scope-creep guardrails (items raised but properly out-of-scope)
None this session — discussion stayed inside the phase boundary after the design-system narrowing.

</deferred>

---

*Phase: 03-personal-website*
*Context gathered: 2026-04-16*
