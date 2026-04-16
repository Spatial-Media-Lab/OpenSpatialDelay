---
phase: 2
slug: email-capture-funding-infrastructure
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-04-15
---

# Phase 2 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.
> **Phase character:** This phase is dominated by external-service configuration (Tally, Patreon, Netfirms DNS, Netlify/Vercel) and a static Next.js site. Most verification is **manual UAT** against live services, not automated tests. The planner MUST favor `<manual>` verifications and concrete, observable acceptance criteria over unit/integration tests where the system under test is a SaaS dashboard or a DNS zone.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Playwright or Vitest + Testing Library (Wave 0 installs, site repo only) |
| **Config file** | `site/playwright.config.ts` (new, Wave 0) |
| **Quick run command** | `cd site && npm run test` |
| **Full suite command** | `cd site && npm run test && npm run build` (static export validates routes) |
| **Estimated runtime** | ~30 seconds (quick) / ~90 seconds (full with build) |

---

## Sampling Rate

- **After every task commit:** Run quick command for any task that touches `site/**`
- **After every plan wave:** Run full suite
- **Before `/gsd-verify-work`:** Full suite green + all manual UAT items signed off
- **Max feedback latency:** 90 seconds

**External-service tasks (Tally, Patreon, DNS, Netlify/Vercel dashboard work) have no automated feedback loop — executors MUST pause for manual UAT confirmation before marking complete.**

---

## Per-Task Verification Map

> Populated by the planner once PLAN.md files exist. Each task must map to one of:
> - `automated` — unit/integration/e2e command with exit-code check
> - `manual` — UAT step with observable condition (screenshot, URL fetch, form submission)
> - `file_exists` — Wave 0 infrastructure bootstrapping

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command / Manual Step | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|--------------------------------|-------------|--------|
| *(populated post-planning)* | | | | | | | | | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] Create new site repo (decision per RESEARCH.md — separate from OSD plugin repo)
- [ ] `site/package.json` — Next.js 15 + static export config
- [ ] `site/playwright.config.ts` — routes smoke test
- [ ] `site/tests/smoke.spec.ts` — stubs for privacy page render + Tally embed presence
- [ ] `site/.env.example` — Tally form ID, Patreon URL, GitHub Releases URL placeholders

*If site repo already exists from a prior phase, Wave 0 reduces to test scaffolding only.*

---

## Manual-Only Verifications

> High-confidence "this can only be verified by humans against live SaaS" list. Planner MUST keep these as `<manual>` tasks with explicit observable steps.

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Tally form redirect to GitHub Releases | DIST-01 | Tally dashboard config; redirect only observable on live form submit | 1. Visit live form URL. 2. Check optional email + GDPR consent. 3. Submit. 4. Confirm browser lands on `github.com/spatial-media-lab/OpenSpatialDelay/releases` |
| Privacy policy page publicly accessible | DIST-02 | Page must be live on production domain, not localhost | 1. `curl -I https://<domain>/privacy` returns 200. 2. Page renders GDPR-required sections (controller, rights, retention, contact). 3. Linked from Tally form footer. |
| Patreon page published with ≥2 posts | DIST-03 | Patreon dashboard state — no API to assert on free tier | 1. Visit `patreon.com/<slug>`. 2. Count public posts ≥ 2. 3. Read tier/about text — confirm "pipeline, not just one plugin" framing present. |
| andrewrahman.com resolves to new host | DIST-02 implied | DNS propagation is live-service state | 1. `dig andrewrahman.com +short` returns Netlify/Vercel IP. 2. HTTPS cert valid. 3. Root URL loads site shell. |
| GDPR consent checkbox is REQUIRED (not pre-checked) | DIST-02 | Tally UI-only state; verified via form inspection | 1. Open Tally form in incognito. 2. Confirm consent checkbox is unchecked by default. 3. Try submitting without ticking — confirm submission blocked. |
| Tally form is embedded at branded URL (not raw tally.so) | D-09 from CONTEXT | Must verify in live browser that URL bar shows andrewrahman.com path | 1. Navigate to `andrewrahman.com/get-osd` (or final branded path). 2. Confirm form renders inline. 3. URL bar does NOT show `tally.so`. |
| Privacy policy data-controller designation matches final decision | DIST-02 | User-level decision (Spatial Media Lab vs Andrew personally) — recorded at UAT | 1. Read live privacy policy. 2. Confirm "Data Controller:" line matches the decision captured during Phase 2 execution. |

---

## Automated Verification (where applicable)

Limited scope — only where we control code/config:

- `site/build` must exit 0 (static export renders every route)
- Playwright smoke test hits `/privacy`, `/get-osd`, and root — expects HTTP 200 and expected HTML markers (e.g., page contains "Privacy Policy" heading, Tally iframe/script present)
- Link-check: every internal and external link in the site must return 2xx/3xx (CI step, non-blocking warn on 3xx)
- Lint: `eslint` + `tsc --noEmit` on `site/**`

---

## Validation Sign-Off

- [ ] Every task has `<automated>` or `<manual>` verify; manual tasks have explicit observable steps
- [ ] Sampling continuity: at most 3 consecutive external-service tasks without a site/build automated checkpoint
- [ ] Wave 0 covers all MISSING references (site repo scaffold, test framework install)
- [ ] No watch-mode flags in any command
- [ ] Feedback latency < 90s for site/** tasks
- [ ] Manual UAT checklist (above) explicitly referenced from `/gsd-verify-work` at phase end
- [ ] `nyquist_compliant: true` set in frontmatter after planner fills Per-Task map

**Approval:** pending
