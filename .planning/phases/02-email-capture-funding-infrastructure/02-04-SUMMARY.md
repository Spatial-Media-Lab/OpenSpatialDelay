---
phase: 02-email-capture-funding-infrastructure
plan: 04
subsystem: hosting-deployment
status: partial
tags: [netlify, hosting, dns, security-headers, hsts, csp, deployment, partial]

# Dependency graph
requires:
  - phase: 02-email-capture-funding-infrastructure
    provides: "Plan 02 site scaffold (commit 20a81cf) + Plan 03 privacy prose (commit c84864c) — pre-flight confirmed 0 `Prose pending` placeholders in app/privacy/page.tsx before deploy-config authoring"
provides:
  - "[site repo] netlify.toml — Netlify build manifest (publish=out, NODE_VERSION=20, 404 fallback redirect)"
  - "[site repo] _headers — production security headers baseline (HSTS, X-Content-Type-Options, X-Frame-Options, Referrer-Policy, Permissions-Policy, CSP scoped to Tally)"
  - "[site repo] scripts/verify-production.sh — 13-assertion production smoke script (3 status + 6 content + 3 header + 1 DNS)"
affects:
  - "02-05-PLAN (Tally form) — CSP frame-src + script-src already whitelist tally.so / widgets.tally.so / *.tally.so, so the Tally embed on /get-osd will not be blocked once deployed"
  - "02-06-PLAN (Patreon footer) — no direct change; privacy URL will be live once DNS is cut over by user"
  - "Phase verifier — MUST flag this plan as partial until Tasks 2/3/4 resume-signal (\"dns cutover complete\" + curl output) is received from user"

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Netlify static-hosting contract: `netlify.toml` at repo root + `_headers` file injected per-route (processed at CDN edge, not at build time)"
    - "Tally-scoped CSP: explicit allowlist of tally.so + widgets.tally.so + *.tally.so for script/frame/form-action, NO 'unsafe-eval', NO wildcard script-src"

key-files:
  created:
    - "[site repo] netlify.toml"
    - "[site repo] _headers"
    - "[site repo] scripts/verify-production.sh (mode 0755)"
  modified: []

key-decisions:
  - "Task 5 verification deferred: Plan Task 5 Action calls for running `./scripts/verify-production.sh` immediately after creation, but the execution rules explicitly instruct NOT to run it because Tasks 2/3/4 (Netlify deploy + DNS cutover) are delegated to the user via a consolidated 02-04-DEPLOY-SETUP.md guide. Running against a not-yet-cut-over domain would return Netfirms-parking responses (75.2.x.x absent, HSTS header absent, privacy content absent) and the 13-assertion script would exit non-zero. Script is staged in repo; full run is a user-gated resume-signal checkpoint."
  - "Tasks 2, 3, 4 are NOT executed in this agent run. They are dashboard + DNS operations (Netlify bake-off, custom-domain attach, Netfirms DNS edit) requiring console access the agent does not have. Delegated to user via 02-04-DEPLOY-SETUP.md."

# Metrics
duration: ~3 min
completed: 2026-04-16
---

# Phase 2 Plan 04: Hosting Deploy (Partial — File Artifacts Only) Summary

**Netlify build manifest, production security-headers baseline, and 13-assertion production smoke script all authored and pushed to andrewrahman-com site repo; Tasks 2, 3, and 4 (Netlify dashboard bake-off, custom-domain attach, Netfirms DNS cutover) delegated to the user and will close this plan only when the user returns the Task 4 resume signal "dns cutover complete" with `./scripts/verify-production.sh` exit-0 output.**

## Plan Status

**Partial — do not mark 100% complete.**

| Task | Description | Status | Owner |
| ---- | --- | --- | --- |
| 1 | Add netlify.toml + _headers | **Complete** | Agent (this run) |
| 2 | Netlify-vs-Vercel bake-off (deploy both to preview URLs) | **Delegated to user** | User (via 02-04-DEPLOY-SETUP.md) |
| 3 | Attach andrewrahman.com as custom domain in Netlify dashboard | **Delegated to user** | User (via 02-04-DEPLOY-SETUP.md) |
| 4 | Edit Netfirms DNS — change apex A to 75.2.60.5, add www CNAME | **Delegated to user** | User (via 02-04-DEPLOY-SETUP.md) |
| 5 | Add production smoke verification script | **Complete (file staged; not yet run)** | Agent (this run) |

The plan closes only after the user completes Tasks 2–4 and posts the Task 4 resume signal with `bash scripts/verify-production.sh` exit-0 output. Phase verifier should flag this plan as incomplete until that signal arrives.

## Performance

- **Started:** 2026-04-16T07:57:52Z
- **Completed:** 2026-04-16T07:58:50Z (agent-side only — user-side DNS cutover remains)
- **Duration:** ~3 min (agent-side)
- **Tasks executed by agent:** 2 of 5 (Tasks 1 + 5)
- **Files created:** 3 in site repo
- **Commits pushed to origin main:** 2 in site repo

## Accomplishments

**Task 1 — Netlify deploy config + security headers**
- `netlify.toml` authored at site repo root: `publish = "out"`, `command = "npm run build"`, `NODE_VERSION = "20"`, and a `/*` → `/index.html` 404 fallback (`force = false` so real static files take precedence).
- `_headers` authored with: Strict-Transport-Security (max-age=31536000; includeSubDomains), X-Content-Type-Options: nosniff, X-Frame-Options: DENY, Referrer-Policy: strict-origin-when-cross-origin, Permissions-Policy: camera=(), microphone=(), geolocation=(), and a Tally-scoped Content-Security-Policy (script-src 'self' https://tally.so https://widgets.tally.so; frame-src https://tally.so https://*.tally.so; form-action 'self' https://tally.so; no `'unsafe-eval'`; no wildcard script-src).
- Automated verify from plan ran and passed: all required files, publish dir, HSTS header, and tally.so references confirmed present.

**Task 5 — Production smoke verification script**
- `scripts/verify-production.sh` authored exactly per plan spec (13 assertions across 3 HTTP status, 6 content, 3 header, 1 DNS check).
- `chmod +x` applied (mode 0755); `bash -n` syntax check passes.
- Script NOT executed in this run — see "Decisions Made" for why (DNS not yet cut over).

## Task Commits

| # | Task | Repo | Commit | Remote |
| --- | --- | --- | --- | --- |
| 1 | Task 1 (netlify.toml + _headers) | andrewrahman-com | `9015848` | pushed to origin/main |
| 2 | Task 5 (verify-production.sh) | andrewrahman-com | `7e8c907` | pushed to origin/main |

Site repo state on origin/main after push:
```
7e8c907 feat(scripts): production smoke verification script
9015848 feat(deploy): Netlify config + security headers (HSTS, CSP for Tally)
c84864c feat(privacy): author full GDPR+CCPA+UK DPA prose
20a81cf feat(site): scaffold andrewrahman.com shell + smoke tests
```

**OSD-repo metadata commit:** [added after this SUMMARY is written — will include SUMMARY.md only; STATE.md/ROADMAP.md/REQUIREMENTS.md updates are deferred until the plan is fully complete, because partial-plan status should not flip the STATE counter forward.]

## Files Created

- `/Users/andrewrahman/conductor/repos/andrewrahman-com/netlify.toml`
- `/Users/andrewrahman/conductor/repos/andrewrahman-com/_headers`
- `/Users/andrewrahman/conductor/repos/andrewrahman-com/scripts/verify-production.sh` (mode 0755)

No files modified in either repo during this run (beyond the site-repo commits above).

## Decisions Made

- **Do not run `verify-production.sh` in this run.** Plan Task 5 Action says "Run it now". The orchestrator's execution rules explicitly overrode this: "Do NOT run verify-production.sh — DNS isn't cut over yet, it would fail." Running against a not-yet-cut-over andrewrahman.com would return Netfirms-parking 66.96.149.1, no HSTS, no privacy content — all 13 assertions would fail. Script correctness is instead verified by `bash -n` syntax check; functional run is deferred to the user's Task 4 resume signal.
- **Partial-plan status declared in frontmatter (`status: partial`).** This is intentional and mandated by the orchestrator. Phase verifier must flag this plan as incomplete until the user posts Task 4 resume signal.
- **Do not advance STATE.md plan counter for this partial execution.** The `<state_updates>` protocol normally advances the Current Plan counter after SUMMARY.md is written. For a partial plan, advancing would give the phase a false green signal. STATE.md advancement is deferred to the orchestrator's second-pass resume handler.

## Deviations from Plan

### Scope deviations (orchestrator-mandated, not auto-fixed)

**1. Tasks 2, 3, 4 skipped — delegated to user via consolidated 02-04-DEPLOY-SETUP.md guide**
- **Directed by:** Orchestrator's `<objective>` block ("Skip Tasks 2, 3, 4 — those are dashboard/DNS checkpoints the user handles separately via a consolidated guide the orchestrator will write.")
- **Impact on plan:** Plan remains PARTIAL until user completes the three delegated tasks. File artifacts from Tasks 1 and 5 are pre-positioned so the user's work is pure dashboard + DNS + one script run.
- **User resume signal (per Task 4):** "dns cutover complete" + `bash scripts/verify-production.sh` exit-0 output.

**2. Task 5 verification run skipped**
- **Directed by:** Orchestrator execution rules.
- **Rationale:** DNS not yet cut over; all 13 assertions would fail, polluting the audit trail with a known-bad run.
- **Mitigation:** `bash -n` syntax check passes; script file exists with correct mode 0755. Functional run will be the user's resume signal for Task 4.

### Auto-fixed issues (Rules 1/2/3)

None. The two Tasks executed (1 and 5) were pure file-writing with no code dependencies, no test harness interaction, and no external service calls. Nothing broke, nothing needed inline fixing.

**Total deviations:** 2 scope deviations, 0 auto-fixes. Both scope deviations are orchestrator-mandated (not agent choice) and are fully accounted for by the partial-plan status.

## Issues Encountered

None.

## Acceptance Criteria Review

**Task 1 acceptance (from plan lines 166–171):**
- `<SITE_ROOT>/netlify.toml` exists with `publish = "out"` and `command = "npm run build"` and `NODE_VERSION = "20"` — **PASS**
- `<SITE_ROOT>/_headers` exists with Strict-Transport-Security, X-Content-Type-Options, X-Frame-Options, Referrer-Policy, Permissions-Policy, Content-Security-Policy — **PASS**
- CSP includes `script-src 'self' https://tally.so https://widgets.tally.so` — **PASS**
- CSP does NOT include `'unsafe-eval'` or broad wildcard script-src — **PASS** (`grep -c "'unsafe-eval'" _headers` returned 0)
- Files committed to site repo — **PASS** (`9015848`)

**Task 5 acceptance (from plan lines 417–422):**
- `<SITE_ROOT>/scripts/verify-production.sh` exists, executable bit set — **PASS** (mode 0755)
- Script covers: 3 HTTP status + 6 content + 3 header + 1 DNS = 13 assertions — **PASS** (verified by inspecting the script)
- Running the script exits 0 — **DEFERRED to user Task 4 resume signal** (cannot pass until DNS is cut over)
- `curl -sI https://andrewrahman.com/` returns `HTTP/2 200` + HSTS header — **DEFERRED to user**
- `dig andrewrahman.com +short A` returns Netlify IP — **DEFERRED to user**

**Plan-level success criteria (from plan lines 463–469):**
- [x] `netlify.toml` + `_headers` committed to site repo — PASS
- [ ] Netlify picks up config and deploys successfully — DEFERRED (user Task 2/3)
- [ ] andrewrahman.com apex A record = 75.2.60.5 — DEFERRED (user Task 4)
- [ ] HSTS header live on production response — DEFERRED (user cutover)
- [ ] `scripts/verify-production.sh` exits 0 — DEFERRED (user cutover)

## Threat Model Closure (Partial)

- **T-02-13 (TLS downgrade) — file-level mitigation complete.** `_headers` contains `Strict-Transport-Security: max-age=31536000; includeSubDomains`; mitigation will activate once Netlify serves the header on the production domain.
- **T-02-14 (CSP bypass) — file-level mitigation complete.** CSP is narrowly scoped to Tally; no `'unsafe-eval'`, no wildcard script-src.
- **T-02-16 (Netfirms credential compromise) — user responsibility.** 02-04-DEPLOY-SETUP.md guide will remind user to enable 2FA on Netfirms before DNS edit.

Neither mitigation is *effective* until Netlify actually serves these headers on the live andrewrahman.com domain, which requires Tasks 2/3/4 to complete.

## User Setup Required

Per orchestrator plan, the user will be provided a consolidated `02-04-DEPLOY-SETUP.md` guide that bundles:
- Task 2 (Netlify-vs-Vercel bake-off — recommended direct-to-Netlify path per RESEARCH.md)
- Task 3 (custom-domain attach in Netlify dashboard)
- Task 4 (Netfirms DNS edit: apex A → 75.2.60.5, www CNAME → {subdomain}.netlify.app, enable 2FA on Netfirms first)

Once cutover is confirmed, user runs:
```bash
cd /Users/andrewrahman/conductor/repos/andrewrahman-com
bash scripts/verify-production.sh
```
Expected output: `ALL CHECKS PASSED` with exit code 0. Post that output back as the Task 4 resume signal.

## Next Phase Readiness

**Not yet ready.** Plan 05 (Tally form) depends on the live custom domain for the consent-checkbox link and the deployed CSP to permit the Tally embed. Plan 05 is blocked until this plan closes.

**What IS ready:**
- The CSP is pre-configured for Tally (Plan 05), so once Netlify serves `_headers`, the form embed will not be blocked at first deploy.
- The privacy URL structure (andrewrahman.com/privacy) is what Plans 05 and 06 will link to; prose is already live in the dev scaffold and will ride to production on first Netlify deploy.

## Self-Check

**Files verified to exist (site repo):**
```
[ -f /Users/andrewrahman/conductor/repos/andrewrahman-com/netlify.toml ] → FOUND
[ -f /Users/andrewrahman/conductor/repos/andrewrahman-com/_headers ] → FOUND
[ -f /Users/andrewrahman/conductor/repos/andrewrahman-com/scripts/verify-production.sh ] → FOUND (mode 0755)
```

**Commits verified to exist (site repo, pushed to origin/main):**
```
git log --oneline origin/main → contains 9015848 → FOUND
git log --oneline origin/main → contains 7e8c907 → FOUND
```

**Content spot-checks:**
- `grep -q 'publish = "out"' netlify.toml` → match
- `grep -q "NODE_VERSION" netlify.toml` → match
- `grep -q "Strict-Transport-Security" _headers` → match
- `grep -q "tally.so" _headers` → match
- `grep -c "'unsafe-eval'" _headers` → 0 (correctly absent)
- `bash -n scripts/verify-production.sh` → exit 0

**Acceptance criteria summary:**
- All 5 agent-scoped Task 1 acceptance criteria: **PASS**
- 1 of 5 agent-scoped Task 5 acceptance criteria: **PASS** (remaining 4 require live domain; marked DEFERRED)
- 1 of 5 plan-level success criteria: **PASS** (remaining 4 require user-side Tasks 2/3/4)
- Partial-plan frontmatter declared (`status: partial`): **PASS**

## Self-Check: PASSED

Self-check passes for the agent-scoped work. The 4 DEFERRED criteria are explicitly expected to remain unverified at this point and are owned by the user resume signal for Task 4. Phase verifier should read `status: partial` in the frontmatter and hold this plan open until the orchestrator signals closure.

---
*Phase: 02-email-capture-funding-infrastructure*
*Plan 04 status at time of SUMMARY: PARTIAL (Tasks 1 + 5 complete; Tasks 2, 3, 4 delegated to user)*
*Completed (agent-side): 2026-04-16*
