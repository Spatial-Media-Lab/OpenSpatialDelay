---
title: "Plan 02-05 (Sender Design Pass) — Next-Session Handoff (v2)"
phase: 02-email-capture-funding-infrastructure
plan: 02-05
created: 2026-04-24
updated: 2026-04-24
supersedes: "v1 (same filename, earlier in 2026-04-24) — v1 outlined a Chrome-drive autonomous walkthrough; that proved unreliable for WYSIWYG form-builder mechanics. v2 captures the actual state after a human-driven Sender dashboard pass."
reason: "Session hit a hard wall: Sender's form-scoped CSS (served from cdn.sender.net/accounts_resources/forms.css, cross-origin) beats the site-side CSS injection on specificity. Pivoted to HITL — user drove the Sender dashboard Design tab directly (button bg, input border, input bg, radius, labels, fonts, title colour). Site-side CSS rewritten with much higher-specificity selectors as a defensive floor. Sender preview screenshot approved by user. Site-side rendering at localhost:3000 NOT YET re-verified after CSS rewrite."
---

# Plan 02-05 Sender Design Pass — Handoff v2

## Session goal for next session

**Verify the form renders correctly on the actual site (localhost:3000) after this session's changes, fix any remaining cases where Sender's CSS still beats mine, then resume Sender account-level Brand Settings (currently only the Dark background colour is updated; accent, logo, fonts still at defaults / to be set globally)**.

## What was done this session (2026-04-24 evening)

### Sender account Brand Settings (partial)
- Dark background colour `#000000` → `#0a0d12` (saved)
- Accent colour verified already `#80d8ff` (no change needed)
- Fonts untouched at account level (DM Sans + Inter unavailable in Sender; fallback = OpenSans, site-side CSS overrides regardless)
- Logo, "from" name/email, social links: not set this session

### Sender form `bkRxov` Design tab (per-form styling, done HITL by user)
- **Title copy**: `Get OpenSpatialDelay` → `Drop your email. Get the file.` (short enough to not wrap orphan)
- **Title**: colour `#e1e5ea`, font OpenSans (site CSS overrides to DM Sans), weight 600, size 18
- **Form/card background**: `#0a0d12`, border width 0
- **Input fields**: background `#010205`, border `#252930` (1px solid), radius 8, font 15, placeholder `#5a5f66`, typed text `#e1e5ea`
- **Input labels (Email/Name)**: colour `#9fa5ae`, size 13, weight 500
- **Submit button**: background `#80d8ff`, text `#03060b`, radius 8, weight 600, size 15
- **Bottom legal text**: colour `#9fa5ae`, size 12, line-height 1.5 (darker `#787d84` was too dim)
- Saved. Preview screenshot in Sender dashboard approved by user (Desktop file: `Screenshot 2026-04-24 at 17.45.58.png`).

### Site-side `components/DownloadForm.tsx` (andrewrahman-com repo) — UNCOMMITTED
- FORM_THEME_CSS rewritten with `html body .sender-subs-embed-form-bkRxov` prefix on every selector (specificity 23+ beats Sender's scoped ~21)
- Longhand properties throughout (`background-color`, `border-color`, `border-width`, `border-style`) to avoid shorthand/longhand cascade issues
- Added `stripZwnbsp()` function — walks iframe title + bottom-text nodes, removes U+FEFF / U+200B / U+200C / U+200D chars that Redactor inserts between words (these cause periods to orphan on narrow viewports)
- Attribution-hide still in place — `color: #0a0d12` + `img { opacity: 0 }` (ToS-safe per §Sender ToS finding in prior handoff)

### Dev server state
- `andrewrahman-com` `next dev` running on :3000 (PID backgrounded)

## The specific problem that blocks shipping

When the site-side CSS was tested in the user's real Chrome (not Playwright headless):

| Element | Site-side CSS said | Real Chrome rendered | Who won |
|---|---|---|---|
| Button bg | `#80d8ff` (`!important`) | BLACK | Sender's forms.css |
| Input border | `#252930` (`!important`) | `rgb(204,204,204)` (light gray) | Sender's forms.css |
| Title font | DM Sans (`!important`) | Roboto | Sender's forms.css |

Playwright headless DID render correctly (cyan button, dark border). So the CSS is technically correct — but in the user's live Chrome session, Sender's forms.css arrives either with `!important` at higher specificity than my rules OR at a cascade position that beats mine.

**The v2 CSS rewrite** (Chrome test not yet run) bumps every selector to specificity 23+ with longhand props. Expectation: matches Sender's specificity AND wins cascade because my `!important` beats their non-`!important` (if the earlier dump is authoritative). If it still loses, the fallback is to JS-apply inline styles via `element.style.setProperty('...', '...', 'important')` — inline specificity is 1000 and beats everything.

**But** — now that Sender dashboard has set the right colours per-form, the site-side CSS may be mostly redundant. The ideal outcome: with Sender dashboard set correctly, the form renders right natively inside the iframe and site-side CSS is just the attribution-hide trick plus ZWNBSP stripping.

## Next-session task (in order)

### 1. Verify site rendering (primary goal)
- `cd ../andrewrahman-com && npm run dev > /tmp/osdsite-dev.log 2>&1 &` (if not already running)
- Reload `http://localhost:3000` in Chrome
- Inspect the embedded form — does it now render with:
  - Cyan button (`#80d8ff`)?
  - Dark input fields with `#252930` borders?
  - Title in DM Sans or close-enough-to-DM-Sans?
  - Single line title or nicely-wrapping ("Drop your email. / Get the file." is 2 lines, both full)?
  - No red attribution envelope?
- Playwright re-shoot for reference: `cd /tmp/sender-form-inspection && node states.mjs`

### 2. Fix remaining CSS wins (if any)
- If the v2 CSS rewrite still loses to Sender's forms.css for any element: use JS inline-style application in `injectThemeIntoIframe()`. Specifically:
  ```ts
  const btn = doc.querySelector('.sender-form-button');
  if (btn) btn.style.setProperty('background-color', '#80d8ff', 'important');
  ```
  Inline styles have specificity 1000 and beat every stylesheet rule.
- Cover: button bg + hover, input border, input bg, title colour, title font (if Roboto still leaks through).

### 3. Strip the now-redundant CSS (opportunistic)
- Now that Sender dashboard handles most styling, some CSS rules may be doing nothing. Skip to Part 4 below only AFTER visual verification passes in step 1.
- Keep: IIFE bootstrap, min-height parent CSS on `.sender-form-field iframe`, attribution-hide trick, ZWNBSP stripper, REPLACE_ME placeholder branch.
- Consider removing: rules that Sender dashboard now delivers natively (button bg/color, input bg, title weight/size, label colour) — test removal one rule at a time with reload.
- **Caveat**: user's real Chrome may have cached old versions of Sender's forms.css. Hard-reload (Cmd+Shift+R) before declaring a rule redundant.

### 4. Commit + close plan
- Two commits (one per repo, local only):
  - `andrewrahman-com`: `fix(download-form): raise CSS specificity above Sender forms.css + strip zero-width spaces in Redactor-sourced copy` (or similar)
  - `openspatialdelay`: `docs(02-05): Sender design pass complete — handoff v2 + SUMMARY flipped partial→complete`
- Flip `02-05-SUMMARY.md` `status: partial` → `status: complete`
- Do NOT push without user approval (per memory feedback_no_destructive_ops_without_approval)

### 5. Resume Sender account-level Brand Settings (if time)
Still pending from this session:
- Upload OSD wordmark logo
- Set social links if any
- Confirm "from" name / email are correct (`Andrew Rahman` / `hey@andrewrahman.com`)
- Consider setting the Light-theme colours for email rendering (Light bg, Headline, Paragraph, Accent) — currently defaults; matters for email clients that render in light mode

This is scoped as **Plan 02-08 (Sender Brand Settings)** per the current SUMMARY. Can be done in the same next-session flow or left for Plan 02-08 when it runs.

## Uncommitted state inventory (as of end-of-session 2026-04-24 ~17:55)

**`openspatialdelay`** (branch `main`, 0 commits ahead):
- `M  .planning/STATE.md` (updates from 2026-04-24)
- `??  .planning/phases/02-email-capture-funding-infrastructure/02-05-SUMMARY.md` (partial, not yet flipped complete)
- `??  .planning/phases/02-email-capture-funding-infrastructure/02-05-SENDER-DESIGN-HANDOFF.md` (this file, v2)

**`andrewrahman-com`** (branch `main`, 0 commits ahead):
- `M  components/DownloadForm.tsx` — FORM_THEME_CSS rewritten with high-specificity selectors + stripZwnbsp() added + attribution-hide trick. UNVERIFIED on the live site.

**Backup branch (openspatialdelay, unchanged):** `backup/2026-04-23-path-a-investigation` — keep until Plan 02-05 fully closes.

## Chrome drive helpers (still useful, reference only)

Pre-written at `/tmp/osdchrome/{run.sh, click.sh, js.js, probe.js}`. Click.sh regex patched this session to handle negative screenY (regex `[0-9]+` → `-?[0-9]+`). Chrome on user's setup can be on a monitor with `screenY: -993` or `screenY: 126` — both now work. Helpers are tmp-wiped on reboot; recreate from `~/.claude/projects/.../memory/reference_chrome_drive_mechanism.md`.

Chrome-drive is reliable for DOM probes, navigation, and input-value setting. **It is NOT reliable for WYSIWYG form-builder element selection + text typing** — Sender's Redactor needs real click events with isTrusted=true and the cross-origin iframe chain makes text typing unreliable. HITL (user driving Sender dashboard in their own Chrome) was the correct pivot.

## Constraints (must-honor, inherited)

- No destructive ops without per-item approval (git push, reset, etc.)
- Next-session prompts inline in chat
- Screenshot/mock HITL escalation after 2–3 failed attempts (applied this session)
- Andrew = tinkerer shipping first software via AI agentic workflows; not "software developer"
- Marketing copy stays capability-level; no DSP jargon

## Success signal for next session

1. Live site at localhost:3000 shows: cyan button, dark-bordered inputs, DM-Sans title, no red attribution, clean 2-line title
2. Playwright re-screenshot matches visual expectation
3. User signs off
4. Two local commits land (one per repo); `02-05-SUMMARY.md` flipped `partial` → `complete`
5. Next-session prompt emitted inline for: (a) Sender Brand Settings finalisation (Plan 02-08), OR (b) Plan 02-04 Netlify migration if Brand Settings is deferred
