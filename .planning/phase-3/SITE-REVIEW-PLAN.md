# Phase 3 — Site Review & A/B Plan

Target: `andrewrahman-com` (Next.js 16 / React 19 / Tailwind v4, static export, sibling repo).
Scope: homepage `/` and download flow `/get-osd/`. **Privacy page `/privacy/` is explicitly out of scope — it is a legal doc, not a designed surface.**

## Five-step review order (per conversation 2026-04-17)

1. **Anti-pattern scan** — `npx impeccable detect` on in-scope URLs. Severity-ranked findings.
2. **Design critique** — `/impeccable critique homepage` + `/impeccable critique download flow`. UX only, no edits.
3. **Technical audit** — `/impeccable audit homepage` + `npx playwright test --grep-invert "@external"` + `npx tsc --noEmit` + `npm run build`.
4. **Visual review** — webapp-testing screenshots at 1440×900 / 1024×768 / 390×844. Compare to Valhalla, Soundtoys, FabFilter, Goodhertz.
5. **GSD UI audit** — `/gsd-ui-review 3` → 6-pillar scored `UI-REVIEW.md`.

After the 5 steps: consolidated verdict as P0 / P1 / P2 punch list.

## Hard constraints (do not relax)

- No invented color tokens — palette must come from `Source/PluginEditor.cpp` in the OSD plugin repo.
- Marketing copy stays at capability level, not DSP jargon (see memory: `feedback_marketing_copy_depth`).
- VST3 + AU only.
- `tests/homepage-content.spec.ts` is a locked content-regression guard — changing it needs explicit justification.

## Known open items (do not re-discover)

- `#hear-it` CTA anchors to `#features` but needs a real binaural A/B audio player (deferred).
- No social proof yet — no GitHub stars widget, no practitioner quotes, no assets. Do not fabricate.
- No analytics instrumentation — CTR unmeasurable until that lands.
- MediaSlots section has "Phase 4 placeholder" copy for video + audio (intentional).

## Step 1 findings summary (completed 2026-04-17)

**`/`: 39 anti-patterns. `/get-osd/`: 5.**

Real issues:
- 4× WCAG AA contrast failures on `/`: muted-grey text on cyan surfaces + muted-grey body on near-black.
- 14× cyan usage (brand color, but density excessive).
- 10× dark-glow with 8 distinct hues (rainbow glow = slop tell).
- 1× purple/violet gradient (isolated — genuine slop).
- Inter fallback at 31% on `/` (Impeccable reflex-reject list).
- Pure `#000` body background.
- `all-caps-body` on 82 chars of homepage copy.
- 3× line-length overflow (~175, ~189, ~142 chars).

False-positive-ish:
- `#80d8ff on #80d8ff` (hero wordmark, intentional).
- `#03060b on #03060b` cluster (likely SVG decorative fills).

## A/B page build — first attempt REJECTED and removed 2026-04-17

Built `/v2/` mirroring homepage structure but with Bricolage + Geist typography, no BorderBeam/Meteors/tap-glows, dark-card Patreon CTA. **User rejected the entire direction** — "I hate everything about this, I don't want to keep v2 at all." The mistake was interpreting "A/B" as a redesign direction instead of a fork for targeted edits. `/v2/` deleted.

## Revised approach (next session)

Keep the A/B format — drop the redesign. Next session:
1. Duplicates `app/page.tsx` byte-for-byte to `app/v2/page.tsx` as the starting B copy (same fonts, same motion, same everything).
2. Runs a section-by-section interview of the live `/` — per-section keep/change/kill with one clarifying question.
3. Applies each approved change **only to `app/v2/page.tsx`**, one commit per decision.
4. At the end, `/` is untouched and `/v2/` holds the accumulated targeted fixes for side-by-side comparison; the user decides what to promote to `/`.

See `NEXT-SESSION-PROMPT.md` in this directory for the handoff prompt.

## Section decisions log

Format: one entry per section in source order, most recent at the top. Commits land on branch `AndrewRahman/site-review-ab` in the andrewrahman-com repo.

### §2 — `<Hero />` — in progress 2026-04-17 (3 open items carry to next session)

**Decisions landed:**

- **Flipped the hierarchy.** Product name is now the H1; tagline is a subhead. Convention on plugin product pages (Valhalla Supermassive, FabFilter Timeless 3, Soundtoys EchoBoy) is product-name-as-H1 — we'd been inverting it, which is why every earlier placement attempt felt wrong.
- **Wordmark.** `OpenSpatialDelay` renders as mono-caps `OPENSPATIALDELAY` using `font-mono-osd uppercase` at 32/48/58px with `tracking-[-0.04em]` (condensed so it fits the copy column on one line). Mono-caps voice intentionally differs from the display-font tagline below it.
- **Eyebrow killed.** The old `v1.0 · Free release` chip above the H1 was removed — the H1 now carries the product identity on its own.
- **Tagline.** "Every echo, in its place." (was "Every echo, somewhere in the room."). Currently rendered in `font-display` (non-italic) at 30/40/48px in `--accent-stellar`, across two lines with `Every` and `place` underlined via `decoration-from-font underline-offset-[6px]`. Reviewer picked #8 from a researched variant list anchored on preserving the emotional pull of "belongs" without implying autonomous (AI-ish) agency from the plugin.
- **Body copy.** Dropped the redundant "The OpenSpatialDelay is a" now that the H1 carries the name. `free` is bold but in the same `--text-secondary` as the rest of the body (not `--text-primary`).
- **"Hear it in 3D" CTA.** Anchor changed from `#features` → `#demo`. Added `id="demo"` + `scroll-mt-24` to `MediaSlots` in `app/v2/page.tsx`. Smooth scroll handled via CSS on `html, body`.
- **Smooth scroll fix (shared side-effect).** `app/globals.css` — Tailwind v4 / turbopack was silently stripping the standalone `html { scroll-behavior: smooth }` rule. Merged `scroll-behavior: smooth` into the existing `html, body { }` rule; reduced-motion guard now also targets `html, body`. This applies to both `/` and `/v2/` because it's in the shared globals.
- **Trust badges.** Order retained at `VST3 + AU · macOS + Windows · GPL-3.0 · Free` after the reviewer reverted an experimental reorder.
- **Ambient background.** Replaced the falling `<Meteors />` with a new `<StarField />` canvas — 160 all-white stars at palette near-white `#e1e5ea` with a gentle forward-drift (stars approach the viewer, grow from ~0.35px → ~2.15px, fade in/out at endpoints). Respects `prefers-reduced-motion`. Reference: the gently-drifting star field in the OSD plugin's observatory prototype (`.context/attachments/concept-A-observatory-v6.html` in pangyo workspace).

**Research consulted this round:**

- Hero wordmark placement — `WebFetch` of Valhalla Supermassive, FabFilter Timeless 3, Soundtoys EchoBoy, Goodhertz homepage. Convention on single-product plugin pages: product name is the biggest element in the hero.
- Tagline persuasion patterns — same fetches. FabFilter "Don't waste your time, modulate it!", Valhalla "Best for massive reverbs, harmonic echoes, space sounds." / "Make some space.", Soundtoys "The Ultimate Echo Plug-In." All three shift from the poetic-only register toward benefit / capability / imperative voice.
- Contemporary web hero keyword-highlight treatments — Linear, Stripe, Anthropic, Vercel, Apple product pages: accent-color on keywords is the dominant pattern; underline is the quieter alternative we ended up landing on.

**What we tried and threw away:**

- First wordmark attempt: enlarged stellar-cyan display-font eyebrow above the H1 tagline. Reviewer: "Terrible." Diagnosis in retrospect: inverted hierarchy — tagline was H1, product was subordinate, violates the plugin-site convention.
- Second attempt: massive mono-caps wordmark with stellar-cyan glow dot + `v1.0` pill sitting above the H1 tagline. Still wrong placement — same inverted-hierarchy root cause. This is the attempt that finally pushed the research that uncovered the convention.
- Tagline candidates before landing on #8:
  - A. "Place every echo / anywhere / in the room." (capability/imperative)
  - B. "Twelve echoes. / Twelve places. / One delay." (count-driven — used as working draft before D-variations)
  - C. "Not panned. / Placed. / In space." (pro-audio contrast)
  - D. "Every echo / knows / where it belongs." (highest emotional score; killed because "knows" implied autonomous AI positioning)
  - D-variations researched: #1 "Every echo, / somewhere / it belongs." · #2 "Place every echo / exactly / where it belongs." · #3 "A home / for / every echo." · #4 "somewhere real." · #5 "Every echo / finds / a home." · #6 "exactly where you want it." · #7 "somewhere deliberate." · **#8 "Every echo / in / its place."** (landed).
- Tagline highlight treatment iterations: stellar-cyan accent on keywords → stellar-cyan on whole line with display italic → all stellar-cyan with underline on `Every` and `place` (landed).
- Smooth-scroll attempt 1: standalone `html { scroll-behavior: smooth }` rule in `globals.css`. Silently dropped by Tailwind v4 / turbopack. Diagnosed by fetching the compiled CSS chunk directly and confirming other raw rules survived.
- Body copy typo: reviewer's "can be independently positions" corrected to "positioned" without asking.

**Open items — carry into next session:**

1. **Bold `SPATIAL` inside the wordmark.** Currently `OPENSPATIALDELAY` is a single-weight brick. Reviewer wants the middle "SPATIAL" substring in a heavier weight so it reads `OPEN **SPATIAL** DELAY`.
2. **Tagline to one line.** Collapse "Every echo, in its place." from two lines to one horizontal line. Likely needs a font-size check so it doesn't push the copy column wider than the screenshot can recover.
3. **Screenshot size.** The plugin screenshot on the right column is still visibly smaller than on `/` (V1). Reviewer reference: `https://www.eventideaudio.com/plug-ins/blackhole-immersive/`. Options to explore in order of increasing scope:
   - a) Widen the site's shared `Container` max width (currently `max-w-[1200px]`). Cheapest.
   - b) Move the H1 + tagline above the two-column grid (top-centered header), letting the full row below be a single wider screenshot cell. Reviewer preference: "I could see the title being top-centered above the text and plugin screenshot" — this is the leaning option.
   - c) Advanced: shrink-into-header on scroll — wordmark animates and docks into the sticky Nav as the user scrolls past the Hero. Reviewer flagged this as "quite a bit advanced" and acceptable only if (a)/(b) don't land.

**Commits (on `AndrewRahman/site-review-ab`):**

- `a966950` feat(hero): wordmark, tagline, starfield, smooth-scroll to demo (A/B §2)

**Files changed this section:**

- `app/v2/page.tsx` — Hero copy column rewritten; `id="demo"` + `scroll-mt-24` added to MediaSlots; `<Meteors>` swapped for `<StarField>`; Meteors import removed.
- `app/globals.css` — `scroll-behavior: smooth` folded into `html, body { }`; reduced-motion guard updated; stray standalone `html { }` rule removed.
- `components/magicui/starfield.tsx` — new canvas component.

### §1 — `<Nav />` — decided 2026-04-17

**Scope note:** Nav is shared between `/` (A) and `/v2/` (B). Both surfaces get the change — explicitly user-approved.

**Decisions:**
- Remove the 8px stellar-cyan dot + glow next to "Andrew Rahman" wordmark. Cleaner mark.
- Remove the "Features" anchor link entirely.

**What we tried and threw away (feel tuning):**
- Hand-rolled `requestAnimationFrame` smooth-scroll with ease-in-out cubic at 1200ms → reviewer wanted to start at 2000ms → 2000ms felt "insanely slow and choppy" → dropped to 1200ms (better) → dropped to 800ms (close, still not right).
- Swapped to Lenis (`lenis/react`, `<ReactLenis root>` + `useLenis()`) since that's the standard for modern smooth-scroll. Reviewer rejected the whole direction after testing: "remove the Features button from the UI in the top and roll back to pre-Lenis scroll functioning."
- **Takeaway:** when smooth-scroll feel can't be tuned to the reviewer's taste, the right fix is to remove the anchor that needs it, not to escalate the library. Don't pile on motion weight to solve a problem — delete the problem.

**Commits (on `AndrewRahman/site-review-ab`):**
- `9a4cf88` feat(nav): remove brand dot glow + remove Features link (A/B §1)
- (Superseded and reset: `f2070fc` feat(nav): remove brand dot glow + smooth-scroll Features link — replaced by `9a4cf88` via `git reset --mixed` before any push.)

**Files changed:** `components/Nav.tsx` only.
