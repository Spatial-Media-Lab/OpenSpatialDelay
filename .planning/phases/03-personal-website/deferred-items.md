# Phase 3 Deferred Items

Out-of-scope findings logged during plan execution. Each item lists the plan that discovered
it and the wave/plan expected to resolve it.

## 2026-04-17 — Plan 03-03 (Patreon recolour)

During the full `npx playwright test` run at end of plan execution, 6 tests in the
Phase 3 Wave 0 validation scaffolding (committed `5e14b30` by Plan 03-00) failed. These
are intentional RED tests waiting on future waves to land their work — NOT regressions
caused by Plan 03-03's recolour.

- `tests/a11y.spec.ts` — 3 failures (`/`, `/privacy`, `/get-osd` zero axe violations).
  Waits on **Plan 03-06** (a11y audit) to identify and fix contrast/role issues.
- `tests/asset-existence.spec.ts:25 sml-logo.svg exists and is non-empty` — Waits on
  **Plan 03-04** (REAPER hero screenshot + SVG logo deliverable) or the asset-delivery
  plan that materialises `public/assets/sml-logo.svg`.
- `tests/asset-existence.spec.ts:25 andrew.jpg exists and is non-empty` — Waits on
  **Plan 03-05** (headshot delivery per D-15).
- `tests/asset-existence.spec.ts:32 public/assets/sml-logo.png is DELETED (D-11)` —
  Waits on the plan that supersedes the legacy PNG with the SVG (same owner as SVG test).

**Scope decision:** Not fixed in Plan 03-03. All 6 belong to later waves per the
Phase 3 plan graph. Plan 03-03 scope is exactly: add `--accent-regal` token + recolour
3 Patreon call-sites. All in-scope tests (content regression suites) pass.
