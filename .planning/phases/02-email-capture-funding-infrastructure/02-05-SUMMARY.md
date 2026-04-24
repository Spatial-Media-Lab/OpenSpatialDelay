---
phase: 02-email-capture-funding-infrastructure
plan: 05
subsystem: email-capture-doi
status: complete
tags: [sender-net, doi, email-capture, embed, deferred-deploy]

# Dependency graph
requires:
  - phase: 02-email-capture-funding-infrastructure
    provides: "Plan 02-02 privacy policy with Sender.net (UAB Sender.lt) named as processor"
  - phase: 02-email-capture-funding-infrastructure
    provides: "Plan 02-04 (partial) — _headers CSP whitelisting cdn.sender.net + *.sender.net"
provides:
  - "[andrewrahman-com] components/DownloadForm.tsx — Sender embed with IIFE bootstrap (account ID 9b01e3bbeb8393 + sender() init); placeholder card when env vars unset"
  - "[andrewrahman-com] NEXT_PUBLIC_SENDER_ACCOUNT_ID env var contract (sibling to existing NEXT_PUBLIC_SENDER_FORM_ID)"
  - "[Sender.net dashboard] groups osd-unconfirmed + osd-confirmed; form bkRxov; DOI automation OSD DOI — confirm subscription (active); Yes-branch download email; SPF+DKIM+DMARC green on andrewrahman.com"
  - "[.planning] 02-05-MAILING-LIST-ANALYSIS.md (provider matrix + decision rationale); 02-05-RESEARCH-LOG.md (F1–F11 confirmed facts, X1–X6 failed approaches, two-automation chain alternative); 02-05-RESEARCH-PLAN.md (research methodology)"
affects:
  - "Plan 03-09 (DIST-01 cross-phase closeout) — precondition gate now passes for the SUMMARY check; production-URL precondition still blocked by Plan 02-04 Netlify cutover"
  - "Plan 02-08 (Sender Brand Settings, follow-on) — scope now includes the dashboard Design-tab form styling deferred from this plan"

# Tech tracking
tech-stack:
  added:
    - "Sender.net IIFE bootstrap pattern (window.Sender + sender('ACCOUNT_ID') + universal.js loader). Loading universal.js bare crashes with 'Cannot read properties of undefined (reading q)'."
  patterns:
    - "Two-step DOI funnel — form-DOI handles confirm email (#1); automation-based 'A link is clicked' chain delivers download email (#2). Legally valid DOI under German UWG §7 + DSGVO Art. 7 (the click on Email #1's Confirm button is the explicit consent action)."
    - "Self-hosted downloads at /assets/OpenSpatialDelay-v1.0.0-{macOS-arm64|Windows-x64}.zip — DOI email is confirm-button-only; downloads served from /get-osd/ post-confirm landing page (NOT from the email body)."

key-files:
  created:
    - ".planning/phases/02-email-capture-funding-infrastructure/02-05-MAILING-LIST-ANALYSIS.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/02-05-RESEARCH-LOG.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/02-05-RESEARCH-PLAN.md"
    - ".planning/phases/02-email-capture-funding-infrastructure/02-05-SENDER-SETUP.md (HITL setup walkthrough)"
    - ".planning/phases/02-email-capture-funding-infrastructure/02-05-SENDING-DOMAIN-RESEARCH.md (sending-domain investigation; §15 = working pattern)"
  modified:
    - "[andrewrahman-com] components/DownloadForm.tsx (commit e9c6b27 — IIFE bootstrap fix)"
    - "[andrewrahman-com] app/privacy/page.tsx (Tally → Sender.net processor swap, prior plans)"
    - "[andrewrahman-com] _headers (CSP migrated tally.so → cdn.sender.net + *.sender.net, prior plans)"

key-decisions:
  - "Stay on Sender.net (do NOT migrate to Brevo Free or MailerLite). Switching cost ~3h with no UX gain on free tier — Sender's two-email pattern is matched by every free competitor; only Brevo Free / MailerLite Paid offer single-step post-DOI redirect, and the daily-cap / cost trade-offs make migration not worth doing inside the launch window. Full matrix in 02-05-MAILING-LIST-ANALYSIS.md."
  - "Path A is legally valid DOI. The earlier 'Path A broken because public download URL bypasses DOI' framing (a46071a, since reset) conflated two distinct concerns: (a) is the confirm-click a valid consent action? — YES, the labelled Confirm button in Email #1 satisfies UWG §7 + DSGVO Art. 7; (b) is /get-osd/ access-controlled? — NO, but no provider locks public marketing pages and downloads are also on public GitHub Releases (GPL-3.0). Funnel-as-gate is industry standard for indie plugins."
  - "Sender's IIFE bootstrap is REQUIRED — loading https://cdn.sender.net/accounts_resources/universal.js directly crashes (Cannot read properties of undefined reading 'q'). Account ID 9b01e3bbeb8393 must be initialised via the sender('9b01e3bbeb8393') call from Sender's official Publishing Settings snippet."
  - "Two-email UX wart accepted, deferred for post-launch optimisation. Future simplification path captured in 02-05-RESEARCH-LOG.md: change Email #1 confirm-button URL from current target to /get-osd/, eliminating Email #2 entirely (downloads delivered immediately on confirm-click landing). Not done now to preserve the documented post-confirm funnel and to keep DOI consent action visually distinct from download CTA."
  - "Design-pass deferred. User decision 2026-04-24: ship the entire andrewrahman-com site (including any Sender form styling) BEFORE creating the second Netlify account, migrating the site to it, and cutting DNS for andrewrahman.com to the new site. Rationale: minimise change-surface during the deploy migration. Sender form styling pass tracked as the next active task in this session; falls under Plan 02-08 (Sender Brand Settings) scope if it isn't closed before the Netlify migration begins."

# Metrics
duration: ~12 hours total across sessions 2026-04-19 → 2026-04-24
completed: partial — see "Plan Status" below
---

> **2026-04-24 late-afternoon UPDATE:** Sender form design pass started this session (site-side A2 CSS injection into the iframe's same-origin contentDocument) but user pivoted mid-session to A1 (Sender account-level Brand Settings) for maintainability. Chrome is open+logged-in at `https://app.sender.net/forms/builder/bkRxov`; Chrome drive helpers verified working at `/tmp/osdchrome/`. Sender ToS has zero free-tier branding clauses — attribution-hide via site-side CSS is ToS-safe. **Full resume walkthrough with paste-ready design tokens → `02-05-SENDER-DESIGN-HANDOFF.md` (read this first next session).**

> **2026-04-24 evening UPDATE (supersedes morning pivot):** Chrome-drive proved unreliable for Sender's Redactor rich-text WYSIWYG editing — pivoted to HITL for the form Design tab. User hand-drove the per-form styling (title copy `Drop your email. Get the file.`, cyan `#80d8ff` button, dark input bg `#010205` + border `#252930`, radius 8, labels `#9fa5ae`, legal text `#9fa5ae`, fonts OpenSans as Sender fallback). Sender preview screenshot approved. Account Brand Settings partial — only Dark bg `#000000` → `#0a0d12` saved. Site-side `DownloadForm.tsx` rewritten with `html body .sender-subs-embed-form-bkRxov` prefix on every selector (v2 specificity 23+ vs Sender forms.css 21, longhand props, new `stripZwnbsp()` helper for Redactor's zero-width word separators).

> **2026-04-24 evening-late UPDATE (plan closed):** Site rendering verified at localhost:3000. First pass (sizes copied from Sender dashboard) rendered weak — title 18px, inputs 40px, cramped. User rejected. **Design iteration:** (1) bumped CSS sizes to match Sender preview proportions — title 36px/700 Inter with -0.025em tracking, inputs 56px with `height:auto` override (Sender was pinning to 40px), button 63px with gradient `#9ce0ff→#80d8ff` + cyan glow shadow + tight tracking, labels 14px/600, legal text 14px centered. (2) Fixed right-padding gap — Sender's `#sender-form-content` was capped at 424px inside a 474px iframe; forced `width: 100% !important` on the full descendant chain (`#sender-form-content, .sender-form, .sender-form-flex, .sender-form-els, .sender-form-inputs, .sender-form-field`). (3) Swapped font stack from DM Sans → Inter to match site body font; injected `@import url('https://fonts.googleapis.com/css2?family=Inter...')` into iframe `<style>` since `next/font` can't cascade into `about:blank`. (4) Widened outer card max-w 460 → 540 px, reduced padding `p-8/sm:p-10` → `p-6/sm:p-8`. (5) Attribution switched from color-hide to `display: none` (empty ghost row was inflating card height). (6) Removed iframe `min-height` so card hugs content. User sign-off at ~18:20 2026-04-24.


# Phase 2 Plan 05: Sender.net Email-Capture Summary (Partial)

**Sender.net DOI workflow built end-to-end and validated working on localhost:3000 (form → osd-unconfirmed → confirm email → click → osd-confirmed + download email → /get-osd/). Form embed fixed on andrewrahman-com via Sender's required IIFE bootstrap. Two follow-on items remain: (1) Sender form design pass to match the site's dark theme — active task this session, started in andrewrahman-com (CSS override attempt) before falling back to Sender dashboard Design tab if needed; (2) production flip — blocked on Plan 02-04 Netlify account migration (free-tier credits exhausted; user must create a fresh Netlify account before andrewrahman.com DNS can be cut over).**

## Plan Status

**Partial — do not mark 100% complete.**

| Sub-task | Description | Status |
| --- | --- | --- |
| Sender account + groups + form + DOI automation | Dashboard work — `osd-unconfirmed`, `osd-confirmed`, form `bkRxov`, automation `OSD DOI — confirm subscription` (active) | **Complete** |
| Domain authentication | SPF + DKIM + DMARC on andrewrahman.com via Netfirms; ImprovMX forwarding hey@andrewrahman.com → Gmail | **Complete** |
| Form embed on site | DownloadForm.tsx with IIFE bootstrap + NEXT_PUBLIC_SENDER_ACCOUNT_ID env var (commit e9c6b27 on andrewrahman-com) | **Complete** |
| Privacy policy update | UAB Sender.lt named as processor; Section 9 effective-date discipline maintained | **Complete (prior plans)** |
| CSP migration | _headers whitelists cdn.sender.net + api.sender.net + *.sender.net; tally.so removed | **Complete (prior plans)** |
| End-to-end DOI flow | Validated on localhost:3000 with real test subscriber (2026-04-24) | **Complete** |
| Sender form design pass (match site dark theme) | HITL pass on Sender dashboard Design tab + site-side CSS override (A1 + A2 combined). Form now renders full-width inside dark panel, Inter typography matching site body, 36px bold title, tall inputs (56px) with #252930 borders, cyan gradient button with glow, centered legal text, attribution hidden. User sign-off 2026-04-24 ~18:20. | **Complete** |
| Production flip (env vars in Netlify, live UAT) | Blocked on Plan 02-04 (new Netlify account due to free-tier credit exhaustion) | **Blocked — deferred** |

## What Works Today

- Form `bkRxov` is published and embedded on `localhost:3000` of andrewrahman-com.
- Submitting the form adds the subscriber to group `osd-unconfirmed` immediately (Sender's standard form behaviour).
- Sender's form-DOI engine sends Email #1 with a Confirm button that resolves `{$double-optin-link}` correctly (this mergetag works in form-DOI context only — see Failed Approach X2 in research log).
- Clicking Confirm in Email #1 flips the subscriber to `osd-confirmed` and triggers automation `OSD DOI — confirm subscription`.
- The automation's Yes branch sends Email #2 (`Your OpenSpatialDelay download`) with a "Get OpenSpatialDelay" button → `https://andrewrahman.com/get-osd/`.
- On localhost the button currently 404s (DNS still on Netfirms parking; production flip blocked on 02-04). Self-hosted .zip files at /assets/OpenSpatialDelay-v1.0.0-{macOS-arm64|Windows-x64}.zip already deploy with the static export; the only missing piece is the live domain.

## Known UX Wart (Accepted)

The flow currently asks the user to receive **two emails and click two buttons**:
1. Email #1 → click Confirm subscription
2. Email #2 → click Get OpenSpatialDelay → land on /get-osd/

**Reduction-to-one-link path documented but deferred.** Email #1's Confirm button can be retargeted to `/get-osd/` directly, which would (a) collapse the funnel to one email + one click, (b) keep DOI legally valid (the labelled Confirm button still satisfies UWG §7 / DSGVO Art. 7), and (c) eliminate Email #2 entirely. Not implemented now because:
- Current flow is working and shipping-ready
- Single-link variant trades funnel-vs-consent visual separation for convenience
- Better attempted post-launch with traffic data

See `02-05-RESEARCH-LOG.md` "Recommended Architecture: Two-Automation Chain" for the full alternative pattern.

## What's Deferred and Why

**Sender form design pass (active task this session):**
The hosted Sender form, when rendered inside `DownloadForm.tsx`, does not match the andrewrahman-com dark theme — Sender's default light styling clashes with the surrounding `var(--bg-panel)` card. Two approaches:

- **A2 (preferred, current attempt):** Custom CSS override in andrewrahman-com targeting `.sender-form-field` descendants. Lowest risk to deploy migration; lives in the site repo.
- **A1 (fallback):** Drive Sender dashboard Design tab to restyle the form server-side. Requires Chrome drive against `app.sender.net`; user must be logged in. Falls under Plan 02-08 (Sender Brand Settings) scope.

If A2 fails (Sender renders unsteyleable shadow-DOM or critical visual properties via inline styles), session falls back to A1 with the user.

**Production flip (env vars + live UAT):**
Blocked on Plan 02-04. Netlify free-tier build credits were exhausted by pre-launch build churn (see STATE.md "Session 2026-04-22 14:17 → 14:25" entry). User must create a fresh Netlify account before andrewrahman.com DNS can be cut over from Netfirms parking to Netlify. Plan 02-04 owns this work; this plan picks back up once 02-04 closes (env vars: `NEXT_PUBLIC_SENDER_FORM_ID=bkRxov`, `NEXT_PUBLIC_SENDER_ACCOUNT_ID=9b01e3bbeb8393`).

**End-to-end production UAT:**
Cannot run until production flip is live. Test plan documented in PLAN.md `must_haves.truths` (incognito submit → DOI email → click → confirm → /get-osd/ → both .zip downloads).

## Failure Chain (For The Record)

Approximately 12 hours across 5 sessions (2026-04-19 → 2026-04-24) producing the working DOI configuration. Key failures (each rules out a path):

- **X1** Form-DOI ON + automation triggered by "Subscriber added to group" — fires on form submit, not on confirm click (group-add is immediate in Sender; F2)
- **X2** `{$double-optin-link}` mergetag in automation email body — 404 (mergetag only resolves in form-DOI context; F4)
- **X3** Form-DOI ON + automation listening for group membership — sends two confirmation emails (Sender's native one + automation's), confusing
- **X4** Sender post-DOI redirect URL — does not exist anywhere in Sender's UI (F5; confirmed by sweeping every settings panel)
- **X5** Form's "Redirect after submit" field — fires pre-DOI on submit, not post-confirm (F6; tested with `?sender-test=1` tracer)
- **X6** Single-automation DOI with delay + condition — condition evaluates ONCE after delay (F7); short delay misses slow clickers, long delay delays delivery

Full facts in `02-05-RESEARCH-LOG.md`. Path A (form-DOI for Email #1 + automation Yes-branch for Email #2) is the surviving pattern.

## Next Steps

**This session:**
1. Inspect rendered Sender form on localhost:3000 (DOM, computed styles, screenshot vs surrounding site)
2. Attempt A2 (custom CSS override in andrewrahman-com)
3. If A2 fails → fall back to A1 (Sender dashboard Design tab) with user assistance

**Subsequent sessions (in order):**
4. Plan 02-04 Netlify account migration → cutover andrewrahman.com DNS to Netlify
5. Live production UAT (PLAN.md `must_haves.truths` end-to-end test)
6. Re-evaluate single-link DOI optimisation (point Email #1 confirm button at /get-osd/) post-launch
7. Plan 03-09 (DIST-01 cross-phase closeout) — precondition gate now passes the SUMMARY check; production-URL check still blocked on (4)

## Key Links

- Sender dashboard form: https://app.sender.net (form `bkRxov`, account `9b01e3bbeb8393`)
- Site repo: ../andrewrahman-com (commit e9c6b27 — IIFE bootstrap fix)
- Provider analysis + decision: `02-05-MAILING-LIST-ANALYSIS.md`
- Confirmed facts + failed approaches: `02-05-RESEARCH-LOG.md`
- Setup walkthrough (HITL): `02-05-SENDER-SETUP.md`
- Sending-domain investigation (legacy): `02-05-SENDING-DOMAIN-RESEARCH.md`
