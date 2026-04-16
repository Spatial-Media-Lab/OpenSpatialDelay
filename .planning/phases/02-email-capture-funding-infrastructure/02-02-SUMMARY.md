---
phase: 02-email-capture-funding-infrastructure
plan: 02
subsystem: site-scaffold
tags: [nextjs, static-export, tailwind, playwright, andrewrahman-com, wave-1]
requires:
  - github-personal-account-andrewrahman-com-repo-created
provides:
  - andrewrahman.com-nextjs-15-static-export-shell
  - shared-layout-nav-footer-component
  - privacy-page-skeleton-9-gdpr-headings
  - get-osd-page-with-tally-iframe-placeholder
  - playwright-smoke-test-harness
affects:
  - phase-02-plan-03-hosting-bake-off
  - phase-02-plan-04-tally-form-wiring
tech-stack:
  added:
    - next@16.2.4
    - react@19.2.4
    - react-dom@19.2.4
    - tailwindcss@4
    - typescript@5
    - "@playwright/test@1.59.1"
    - serve@14.2.6
  patterns:
    - static-export-next-config-output-export
    - tailwind-4-utility-first-no-shadcn
    - system-ui-font-stack-no-web-fonts
    - client-side-tally-embed-via-script-injection
key-files:
  created:
    - path: andrewrahman-com:next.config.ts
      purpose: "Static export config (output: 'export', trailingSlash: true, images.unoptimized: true)"
    - path: andrewrahman-com:app/layout.tsx
      purpose: "Root layout wrapping Nav + max-w-[720px] main + Footer on every route"
    - path: andrewrahman-com:app/page.tsx
      purpose: "Homepage with UI-SPEC bio copy + 'Get OpenSpatialDelay' CTA"
    - path: andrewrahman-com:app/privacy/page.tsx
      purpose: "Privacy policy skeleton with 9 GDPR-required <h2> headings (prose deferred to Plan 03)"
    - path: andrewrahman-com:app/get-osd/page.tsx
      purpose: "Client component: Tally iframe placeholder reading NEXT_PUBLIC_TALLY_FORM_ID at build time"
    - path: andrewrahman-com:app/globals.css
      purpose: "Minimal Tailwind import + system-ui font stack + @media print rules"
    - path: andrewrahman-com:components/Nav.tsx
      purpose: "Shared nav with 'Andrew Rahman' home link, 56px border-bottom bar"
    - path: andrewrahman-com:components/Footer.tsx
      purpose: "Shared footer with github.com/Spatial-Media-Lab attribution + /privacy link + © 2026"
    - path: andrewrahman-com:playwright.config.ts
      purpose: "Playwright config — webServer runs `npx serve out -l 3000` (static export, no -s flag)"
    - path: andrewrahman-com:tests/smoke.spec.ts
      purpose: "4 smoke tests: homepage, privacy 9 headings, get-osd placeholder, footer on every page"
    - path: andrewrahman-com:.env.example
      purpose: "Declares NEXT_PUBLIC_TALLY_FORM_ID, NEXT_PUBLIC_PATREON_URL, NEXT_PUBLIC_RELEASES_URL"
    - path: andrewrahman-com:README.md
      purpose: "5-line scaffold readme describing npm scripts and phase linkage"
  modified:
    - path: andrewrahman-com:.gitignore
      change: "Added exception for .env.example + ignore /test-results/ and /playwright-report/"
    - path: andrewrahman-com:package.json
      change: "Replaced `start` with `serve out -l 3000`; added `test` and `test:ci` scripts; added serve + @playwright/test devDeps"
decisions:
  - "Site repo lives at https://github.com/AndrewRahman/andrewrahman-com (separate private repo on personal account, not Spatial-Media-Lab org). Cloned locally at /Users/andrewrahman/conductor/repos/andrewrahman-com. Resolves Task 1 (option-a)."
  - "Use `npx serve out` as webServer for Playwright instead of plan-specified `next start` because Next.js 16 rejects `next start` when `output: 'export'` is set. No `-s` flag — SPA fallback would wrongly reroute `/privacy` to `index.html` instead of serving `privacy/index.html`."
  - "`start` npm script now runs `serve out -l 3000` (previously `next start` from create-next-app default)."
metrics:
  completed: 2026-04-16T09:38:00+02:00
  duration: ~8 minutes
  tasks: 4
  files-created: 13
  files-modified: 2
  tests-added: 4
---

# Phase 2 Plan 02: andrewrahman.com Shell Scaffold Summary

andrewrahman.com Next.js 16 static-export shell scaffolded in sibling repo `AndrewRahman/andrewrahman-com` with shared Layout, three routes (/, /privacy, /get-osd), and a Playwright smoke-test harness that all 4 tests pass against the real `out/` build served by `serve`.

## Task 1 Resolution (Checkpoint — Pre-Resolved)

**User chose option-a: separate repo under https://github.com/AndrewRahman/andrewrahman-com (personal account, not Spatial-Media-Lab org).**

The orchestrator confirmed this decision before this agent ran, and provided the already-cloned empty target directory `/Users/andrewrahman/conductor/repos/andrewrahman-com`. The agent proceeded directly from Task 2.

## Exact Versions Locked (Task 2 Step 1)

`npm view` output at 2026-04-16 09:30 GMT+2:

| Package | Registry latest | Installed in site |
|---------|-----------------|-------------------|
| next | 16.2.4 | 16.2.4 |
| react | 19.2.5 | 19.2.4 (pinned by create-next-app template) |
| react-dom | 19.2.5 | 19.2.4 (pinned by create-next-app template) |
| tailwindcss | 4.2.2 | ^4 (resolves to 4.2.x at install) |
| typescript | 6.0.2 | ^5 (pinned by create-next-app template; plan acceptance criterion says major 5) |

Note: The plan title said "Next.js 15" but `npm view next version` returned 16.2.4, and the plan Step 1 instruction explicitly said "do NOT lock blindly — use those exact major.minor values." Next.js 16 was used as the latest. No behavioral difference for this plan's scope (static export API is unchanged).

## Scaffold Approach (Task 2 Step 2 — Deviation)

Because `/Users/andrewrahman/conductor/repos/andrewrahman-com/` already existed (empty, with `.git/` on branch `main` tracking GitHub origin) before scaffolding, the standard `npx create-next-app@latest <path>` form would have refused the non-empty directory. Approach used:

```bash
cd /Users/andrewrahman/conductor/repos/andrewrahman-com && \
  npx --yes create-next-app@latest . \
    --typescript --tailwind --app --no-src-dir --no-eslint \
    --import-alias "@/*" --use-npm --skip-install --yes
```

Key flags:
- `.` (current dir) — scaffolds into the existing empty git repo without clobbering `.git/`
- `--skip-install` — faster bootstrap; `npm install` ran as a separate step
- `--yes` — non-interactive confirmation

After scaffolding, all plan-specified files (`next.config.ts`, `app/layout.tsx`, `app/page.tsx`, `app/globals.css`, `components/Nav.tsx`, `components/Footer.tsx`, `app/privacy/page.tsx`, `app/get-osd/page.tsx`, `.env.example`, `README.md`) were authored with verbatim plan content.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 — Blocking] Replaced `next start` with `npx serve out -l 3000` for Playwright webServer**

- **Found during:** Task 5 first test run
- **Issue:** The plan's playwright.config.ts spawned `npm run start` (= `next start`). Next.js 16 refuses to run `next start` when `output: 'export'` is configured, erroring: *"next start" does not work with "output: export" configuration. Use "npx serve@latest out" instead."* The identical guidance appears in Next.js 14+ too — the plan's assumption was incorrect.
- **Fix:** Installed `serve@14.2.6` as a devDep. Changed `package.json` `start` script to `serve out -l 3000`. Playwright `webServer.command` runs `npx serve out -l 3000`.
- **Subsequent fix (same deviation):** Initially used `npx serve out -l 3000 -s` for SPA fallback. That made `/privacy` (no trailing slash) 200 with **homepage content** because SPA-fallback routes all non-matching paths to `index.html`. Removed `-s`. Without it, `serve` correctly resolves `/privacy` → `out/privacy/index.html`.
- **Files modified:** `playwright.config.ts`, `package.json`
- **Commit:** Part of scaffold commit `20a81cf`

**2. [Rule 1 — Bug] Added `exact: true` to Playwright heading matcher**

- **Found during:** Task 5 second test run
- **Issue:** `page.getByRole('heading', { level: 2, name: 'Your rights' })` matched **both** the "Your rights" h2 and the "How to exercise your rights" h2 (substring match), causing a strict-mode violation.
- **Fix:** Added `exact: true` to the matcher options in the heading loop.
- **Files modified:** `tests/smoke.spec.ts`
- **Commit:** Part of scaffold commit `20a81cf`

**3. [Rule 2 — Missing critical functionality] .gitignore did not protect .env.example and left test-results/ tracked**

- **Found during:** `git status` before first commit
- **Issue:** create-next-app's default `.gitignore` has `.env*` which also ignores the plan-required `.env.example`. Additionally, Playwright's `test-results/` and `playwright-report/` directories would be tracked by default, polluting subsequent commits.
- **Fix:** Added `!.env.example` exception and explicit ignores for `/test-results/`, `/playwright-report/`, `/blob-report/`, `/playwright/.cache/`.
- **Files modified:** `.gitignore`
- **Commit:** Part of scaffold commit `20a81cf`

### None Required (Plan Matched Reality)

Tasks 3 (privacy) and 4 (get-osd) executed exactly as written — copy is verbatim from the UI-SPEC Copywriting Contract. No deviation.

## Build + Test Results

**Build output (`npm run build`):**
```
Route (app)
┌ ○ /
├ ○ /_not-found
├ ○ /get-osd
└ ○ /privacy
○  (Static)  prerendered as static content
```

`out/` contains `index.html`, `privacy/index.html`, `get-osd/index.html`. Build time: ~1.2s compile + 0.7s TypeScript + 0.2s static page generation.

**Playwright smoke tests (`npm run test`):**
```
Running 4 tests using 1 worker
  ✓  1 [chromium] › tests/smoke.spec.ts:3:5 › homepage renders bio + OSD CTA (96ms)
  ✓  2 [chromium] › tests/smoke.spec.ts:10:5 › privacy page renders all 9 GDPR section headings (84ms)
  ✓  3 [chromium] › tests/smoke.spec.ts:28:5 › get-osd page renders heading + Tally placeholder (FORM_ID not set) (69ms)
  ✓  4 [chromium] › tests/smoke.spec.ts:35:5 › footer present on every page with github + privacy links (105ms)
4 passed (1.1s)
```

## Handoff Confirmations

### Plan 03 (Hosting Bake-off + DNS Cutover) — Ready

- `out/` is a fully static export. Plan 03 can deploy it to Netlify / Vercel / Cloudflare Pages as-is.
- `npm run build` produces deterministic output on any Linux/macOS runner with Node 22+ (create-next-app template compat; local dev used Node 25.9.0).
- Plan 03 Task 1 (privacy prose authoring) will replace the 9 `[Prose pending Plan 03.]` placeholders in `app/privacy/page.tsx` plus the `[Data-controller designation pending user confirmation. Default per D-16: Spatial Media Lab.]` line, without touching layout or section structure.

### Plan 04 (Tally SaaS Config) — Ready

- `/get-osd` reads `process.env.NEXT_PUBLIC_TALLY_FORM_ID` at build time. When unset (current state), the placeholder card renders: *"Tally form ID not yet configured. Set `NEXT_PUBLIC_TALLY_FORM_ID` in the deploy environment once Plan 04 is complete."*
- Plan 04 only needs to set the env var (in whatever hosting platform Plan 03 picks) and trigger a rebuild. No source changes required.

### Plan 05 (Patreon SaaS Config) — No Coupling

- `NEXT_PUBLIC_PATREON_URL` is declared in `.env.example` for Phase 3 consumption. Phase 2 does not surface it on any current route.

## Known Stubs

| File | Line | Reason |
|------|------|--------|
| andrewrahman-com:app/privacy/page.tsx | Effective date line + 9 section bodies | Privacy prose deferred to Plan 03 Task 1 (after data-controller designation confirmed). Skeleton is intentional, documented in plan's Task 3. |
| andrewrahman-com:app/get-osd/page.tsx | Tally iframe placeholder card | FORM_ID unset by design; Plan 04 Task 3 provides it. Placeholder is intentional, documented in plan's Task 4. |

Both stubs are **not** violations — each is explicitly called out in the plan as "deferred to a subsequent plan" and is protected by the env-driven / placeholder-prose contract.

## Threat Flags

No new threat surface introduced beyond what is already in the plan's `<threat_model>`. All mitigations from T-02-03 through T-02-07 apply as documented:
- Tally embed uses official verbatim snippet (T-02-03)
- `NEXT_PUBLIC_` prefix + placeholder fail-safe for form ID (T-02-04)
- `[Prose pending Plan 03.]` placeholders make draft state obvious (T-02-05)
- Tally script URL hardcoded to TLS/verified domain (T-02-06)
- Iframe failure degrades gracefully via `<noscript>` fallback (T-02-07)

CSP headers remain Plan 03's responsibility (Netlify `_headers` file) per plan threat model note.

## Sibling-Repo Commit

| Repo | Commit | Branch | Remote Push |
|------|--------|--------|-------------|
| AndrewRahman/andrewrahman-com | `20a81cf` | `main` | pushed, tracks `origin/main` |

Commit message: `feat(site): scaffold andrewrahman.com shell + smoke tests`

## Self-Check: PASSED

- [x] `/Users/andrewrahman/conductor/repos/andrewrahman-com/next.config.ts` exists and contains `output: 'export'`
- [x] `/Users/andrewrahman/conductor/repos/andrewrahman-com/components/Footer.tsx` exists and contains `github.com/Spatial-Media-Lab`
- [x] `/Users/andrewrahman/conductor/repos/andrewrahman-com/app/privacy/page.tsx` contains 9 `<h2` elements
- [x] `/Users/andrewrahman/conductor/repos/andrewrahman-com/app/get-osd/page.tsx` contains `tally.so/widgets/embed.js` and `dynamicHeight=1`
- [x] `/Users/andrewrahman/conductor/repos/andrewrahman-com/tests/smoke.spec.ts` contains 4 tests
- [x] `/Users/andrewrahman/conductor/repos/andrewrahman-com/.env.example` contains `NEXT_PUBLIC_TALLY_FORM_ID`
- [x] `npm run build` exits 0 and produces `out/index.html`, `out/privacy/index.html`, `out/get-osd/index.html`
- [x] `npm run test -- --reporter=list` exits 0 with 4 passed
- [x] Commit `20a81cf` exists on branch `main` in sibling repo and has been pushed to `https://github.com/AndrewRahman/andrewrahman-com.git`
