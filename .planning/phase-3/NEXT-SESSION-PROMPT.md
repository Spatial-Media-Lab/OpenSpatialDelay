# Next session prompt — andrewrahman.com site-review-ab, mid-§2 (Hero) open items

Paste the block below as your first message in the next session.

---

## PROMPT TO PASTE

Resume the section-by-section A/B site review on the andrewrahman-com repo, branch `AndrewRahman/site-review-ab`. We're mid-§2 (`<Hero />`) — three specific open items from last session carry over.

**Read before doing anything:**

1. `/Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phase-3/SITE-REVIEW-PLAN.md` — the full plan + **Section decisions log**. §2's current state + the three open items are captured at the top of the log.
2. `andrewrahman-com/CLAUDE.md` + `AGENTS.md` — Next.js 16 has breaking changes; read `node_modules/next/dist/docs/` before writing Next code.
3. `andrewrahman-com/app/v2/page.tsx` — currently diverges from `app/page.tsx` in Hero (§2 copy column) and MediaSlots (only `id="demo"` anchor target).
4. `andrewrahman-com/app/globals.css` — note the `scroll-behavior: smooth` lives inside the `html, body { }` rule (do NOT move it back out — Tailwind v4 silently strips standalone `html { }` rules).
5. `andrewrahman-com/components/magicui/starfield.tsx` — new canvas component used by Hero.
6. **Live compare** once the dev server is up: `http://localhost:3000/` (A) vs `http://localhost:3000/v2/` (B).

**Dev server guidance:** Start `npm run dev` in the background at the start of the session and keep it alive. If it drops, restart it immediately — the reviewer can't verify changes without a live surface.

**Current state:**

- Branch `AndrewRahman/site-review-ab` is on origin. §2 code changes are already committed — see the plan's §2 entry for the commit hash.
- §1 is done. §2 has landed the wordmark (mono-caps H1), tagline "Every echo, in its place." (2 lines, stellar cyan, `Every` and `place` underlined), body copy rewrite, `#demo` smooth-scroll anchor, and the `<StarField />` ambient background.

**Open items from the previous session (priority order):**

1. **Bold `SPATIAL` inside the wordmark.**
   - The H1 currently renders `OPENSPATIALDELAY` as a single-weight mono-caps brick.
   - Reviewer wants the middle `SPATIAL` substring in a heavier weight so the wordmark visually reads as `OPEN **SPATIAL** DELAY` — the "spatial" positioning claim becomes the optical anchor of the name.
   - Implementation sketch: wrap `SPATIAL` in a `<span>` with `font-semibold` or `font-bold`; verify the font-mono-osd stack renders a real heavier weight (not a faux-bolded `ui-monospace` fallback). If the mono stack doesn't have a bold, propose a fix before shipping.

2. **Collapse the tagline to one line.**
   - Currently "Every echo," / "in its place." across two lines. Reviewer wants it as a single horizontal line: `Every echo, in its place.` with `Every` and `place` still underlined in stellar cyan.
   - Check that the one-line tagline doesn't overflow the copy column at `sm`/`lg` breakpoints — likely a font-size adjustment (48px → ~32–36px at lg). Keep the stellar-cyan color and the `decoration-from-font underline-offset-[6px]` treatment.

3. **Plugin screenshot is still too small.**
   - Reference the reviewer pointed at: [Eventide Blackhole Immersive](https://www.eventideaudio.com/plug-ins/blackhole-immersive/). Compare hero screenshot scale vs current `/v2/`.
   - Options in increasing scope — propose the simplest first, escalate only if the reviewer rejects:
     - a) **Widen the shared `Container` max width** (currently `max-w-[1200px]` in `app/v2/page.tsx`). Cheapest.
     - b) **Move H1 + tagline to a top-centered header** above the two-column grid, so the row below can host a wider screenshot cell (more of the 1200px container, not just the ~511px `1fr` share). Reviewer's stated preference: *"I could see the title being top-centered above the text and plugin screenshot"* — this is the leaning option.
     - c) **Advanced: shrink-into-header on scroll** — the wordmark animates and docks into the sticky Nav as the user scrolls past the Hero. Reviewer's note: *"quite a bit advanced and I have no context for how easy or not that is for you to make."* Only pursue if (a)/(b) don't land.

**Review format (same as prior sessions):**

- One clarifying question max per item. Show the proposed diff before writing.
- Apply only to `app/v2/page.tsx` unless the change is in a shared component or `globals.css`; shared changes affect `/` too — call that out explicitly and wait for approval before editing.
- One commit per decision. Commit message format: `feat(<scope>): <decision> (A/B §<n>)`.
- After each approved change, append to the **Section decisions log** in `SITE-REVIEW-PLAN.md` (including any "what we tried and threw away" notes).

**Non-negotiable rules (unchanged):**

- Never edit `app/page.tsx` during this review.
- Never invent color tokens — palette is locked to `Source/PluginEditor.cpp` via `globals.css`.
- Never change the motion system (BorderBeam, DotPattern, StarField, tap-glows), the Patreon CTA's lavender treatment, fonts, or typography stack without explicit approval.
- `tests/homepage-content.spec.ts` is a locked content-regression guard on `/`. It does NOT cover `/v2/`.
- Marketing copy at capability level, not DSP jargon.
- `/privacy/` is off-limits.
- If smooth-scroll feel tuning doesn't land fast, **delete the trigger** — don't escalate to a library (see §1 takeaway).
- Don't commit to main locally or remotely unless explicitly told.
- Never `git push --force` or `git reset --hard` without explicit ask.

**Start with item 1.** Propose the exact diff for bolding `SPATIAL` (including confirming the mono stack actually has a bold weight available), wait for approval, commit, log the decision, then move to item 2.

After all three open items close, mark §2 decided in the plan and move to §3 (`<RainbowStrip />`) at `app/v2/page.tsx:353+`.
