---
phase: 02-email-capture-funding-infrastructure
plan: 05-RESEARCH
subsystem: email-capture-doi
status: complete
tags: [sender-net, doi, gdpr, mailerlite, brevo, provider-evaluation]

# Dependency graph
requires: []
provides:
  - "02-05-MAILING-LIST-ANALYSIS.md — full provider comparison matrix (Sender.net vs MailerLite vs Brevo vs ConvertKit vs EmailOctopus) evaluated against R1–R15"
  - "02-05-RESEARCH-LOG.md — F1–F11 confirmed Sender.net facts, X1–X6 failed approach record, two-automation chain architecture"
  - "Decision: stay on Sender.net with Path A (form-DOI Email #1 + automation Yes-branch Email #2)"
  - "Legal clarity: Path A confirm-click is valid DOI under German UWG §7 + DSGVO Art. 7"
affects:
  - "Plan 02-05 (implementation) — provider locked, architecture confirmed, resume from Sender dashboard design pass"
  - "Plan 02-08 — Sender Brand Settings scope defined"

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Provider evaluation matrix: EU data processing, DPA availability, free tier, DOI support, post-DOI redirect, embeddable form, custom sender domain, account verification time"

key-files:
  created:
    - ".planning/phases/02-email-capture-funding-infrastructure/02-05-MAILING-LIST-ANALYSIS.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/02-05-RESEARCH-LOG.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/02-05-RESEARCH-PLAN.md"
  modified: []

key-decisions:
  - "Stay with Sender.net. Zero migration risk (already 80% deployed), legally valid DOI, launches on time 2026-04-28."
  - "Path A (automation-based DOI) is legally valid. Confirm-click in Email #1 satisfies German UWG §7 + DSGVO Art. 7 as explicit affirmative consent. Earlier 'broken' framing conflated DOI validity with access control on a public page."
  - "No provider locks a public marketing page on free tier — only Netlify serverless middleware could, which is out-of-scope and pointless (downloads are also on public GitHub Release URLs per GPL-3.0)."
  - "Best alternative if Sender fails post-launch: Brevo Free (native post-DOI redirect, France-based, 300 emails/day). Migration cost is always ~3h — do it after launch."

requirements-completed: []

# Metrics
duration: ~2 hours research (across sessions 2026-04-24)
completed: 2026-04-24
---

# Plan 02-05-RESEARCH: Mailing List Provider Research Summary

**Provider evaluation and DOI architecture analysis complete — stay on Sender.net with Path A confirmed legally valid; full comparison matrix and decision rationale in 02-05-MAILING-LIST-ANALYSIS.md.**

## Performance

- **Duration:** ~2 hours
- **Completed:** 2026-04-24
- **Tasks:** Research Tracks 1–3 (Sender.net assessment, alternative providers, architecture alternatives)
- **Files created:** 3 (MAILING-LIST-ANALYSIS.md, RESEARCH-LOG.md, RESEARCH-PLAN.md)

## Accomplishments

- Resolved the "critical flaw" framing from prior sessions: Path A's confirm-click IS a legally valid DOI consent action under German UWG §7 + DSGVO Art. 7. The earlier concern about `/get-osd/` being publicly accessible is provider-agnostic and not a blocking issue.
- Evaluated 5 alternative providers (MailerLite Free, MailerLite Paid, Brevo Free, ConvertKit/Kit, EmailOctopus) against all 15 requirements (R1–R15). ConvertKit and EmailOctopus fail R2 (EU data processing). MailerLite Free offers no UX improvement over Sender on the free tier.
- Confirmed Sender.net's two isolated DOI systems (form-level vs. automation-based) and documented 6 failed approaches (X1–X6) to prevent re-investigation.
- Documented architecture alternatives (serverless middleware, token-based URLs, GitHub Release direct-links) and concluded GitHub Release direct-links (current setup) satisfies the funnel requirement without added complexity.

## Files Created/Modified

- `02-05-MAILING-LIST-ANALYSIS.md` — Provider comparison matrix, legal analysis, recommendation with concrete timeline
- `02-05-RESEARCH-LOG.md` — F1–F11 confirmed Sender.net facts, X1–X6 failed approaches, two-automation chain architecture spec
- `02-05-RESEARCH-PLAN.md` — Research methodology, hard requirements (R1–R15), evaluation matrix template

## Decisions Made

**Decision: Stay with Sender.net. Finish Path A. Launch 2026-04-28.**

Rationale:
- Zero migration risk — account verified, DNS authenticated, form embedded, automation 80% built
- Path A is legally valid DOI — confirm-click in Email #1 is distinct affirmative consent
- No free-tier alternative improves the UX (MailerLite Free custom redirect = paid-only; Brevo's 300 emails/day cap + 3h migration cost is not worth it inside a 4-day window)
- If post-DOI UX becomes important after launch, Brevo Free migration is always 3h

## Deviations from Plan

None — research tracks 1–3 fully executed. Provider matrix filled in for all 5 Tier 1+2 candidates. Architecture alternatives assessed. Output files created.

## Issues Encountered

The research revealed the earlier "critical flaw" diagnosis was wrong — this was itself the key finding. The DOI validity question (R1/R7) and the access-control question (R6) were conflated across prior sessions. Separating them unlocked the path forward.

## Next Phase Readiness

- Provider decision locked. No blocker on Plan 02-05 implementation resumption.
- Resume from: Sender form design pass (Sender dashboard Design tab complete + site-side DownloadForm.tsx CSS override — user sign-off 2026-04-24 ~18:20)
- Production flip still blocked on Plan 02-04 (Netlify account migration)

---
*Phase: 02-email-capture-funding-infrastructure*
*Completed: 2026-04-24*
