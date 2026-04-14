---
phase: 1
slug: repo-license-readiness
status: draft
nyquist_compliant: false
wave_0_complete: true
created: 2026-04-14
---

# Phase 1 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Shell commands (grep, gh CLI) — no unit tests applicable |
| **Config file** | N/A — documentation-only phase |
| **Quick run command** | `git grep -i "commercial licen" -- . ':!.planning/'` |
| **Full suite command** | See verification commands per requirement below |
| **Estimated runtime** | ~2 seconds |

---

## Sampling Rate

- **After every task commit:** Run `git grep -i "commercial licen" -- . ':!.planning/'`
- **After all tasks complete:** Run full verification suite below

---

## Phase Requirements — Verification Map

| Req ID | Behavior | Verification Command | Expected |
|--------|----------|---------------------|----------|
| REPO-01 | Zero "commercial licen" matches in tracked files | `git grep -i "commercial licen" -- . ':!.planning/'` | Zero results |
| REPO-02 | Release metadata updated, macOS arm64 binary present | `gh release view v1.0.0 --json body -q '.body' \| grep -ic "commercial"` | 0 |
| REPO-03 | System requirements section in README | `grep -c "## System [Rr]equirements" README.md` | 1 |
| REPO-04 | xattr command in README | `grep -c "xattr" README.md` | >= 1 |

---

## Wave 0 Gaps

None — all verification is shell-based. No test framework setup required.
