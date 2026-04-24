# Plan 02-05: Mailing List Provider Analysis

Date: 2026-04-24

## Problem Statement

~8 hours across 4 sessions (Apr 19-23) attempting to build Sender.net DOI automation. All attempts failed due to a fundamental architectural misunderstanding: form-level DOI and automation-based DOI are two **isolated systems** in Sender.net that do not interoperate.

### Failure Chain

1. `{$double-optin-link}` mergetag used in automation email → 404 (only works in form-DOI context)
2. Expected "Subscriber status changed" trigger to fire on form-DOI confirm → it doesn't (unreliable/non-functional per Sender docs)
3. Form-DOI intercepted flow before automation trigger could fire
4. Path B (soft-gate, download in DOI email) rejected — DOI gate is non-negotiable (email subscription = payment)

### Root Cause

The two Sender DOI mechanisms operate independently:
- **Form-level DOI**: Sender's internal engine sends confirm email using `{$double-optin-link}`, flips subscriber status on confirm. Fires NO automation triggers.
- **Automation-based DOI**: Form is single opt-in. Automation handles entire confirm flow using plain URLs + "Link is clicked" condition.

All failed attempts mixed these two systems. The documented automation-based DOI pattern was never tried.

## Sender's Documented Automation-Based DOI Pattern

Source: [Enable double opt-in | Sender Automation](https://www.sender.net/help/automation/double-opt-in/)

1. Form DOI toggle = **OFF**
2. Form → subscriber added to `osd-unconfirmed`
3. Automation trigger: "Subscriber added to a group"
4. Send email with button → plain URL
5. Delay (hours)
6. Condition: "Link is clicked" on the plain URL
7. Yes: Move to `osd-confirmed`

**CRITICAL FLAW (identified 2026-04-24):** This pattern does NOT gate the download behind DOI confirmation. The "Link is clicked" condition is passive tracking, not access control. Any user who knows the download URL can access it without confirming. Clicking a download link is NOT formal DOI consent under German law — the confirmation and download delivery must be logically separate actions. This pattern fails requirements R6 and R7 (see 02-05-RESEARCH-PLAN.md).

**Status:** Further research needed — see 02-05-RESEARCH-PLAN.md for structured investigation.

## Sunk Cost Inventory (favors staying with Sender)

- DNS domain auth: SPF + DKIM + DMARC green for andrewrahman.com
- ImprovMX forwarding: hey@andrewrahman.com → Gmail
- Sender account: verified, DOI toggle available
- Groups: osd-unconfirmed + osd-confirmed exist
- Form bkRxov: published
- Privacy policy: names Sender.net (UAB Sender.lt, Vilnius, Lithuania)
- Site code: EmailCaptureSection.tsx + CSP + tests wired for Sender

## Fallback: MailerLite

If automation-based DOI also fails in Sender:
- EU-based (Lithuania, same jurisdiction)
- Native post-DOI redirect URL (the feature Sender lacks)
- Free tier: 1,000 subs / 12,000 emails/month
- Good REST API v2
- Migration cost: ~3 hours (DNS records, site code, privacy policy, tests)

## API/MCP Assessment

No mailing list providers have known MCP servers. All require UI for automation building. API differences:
- Sender: moderate REST API, webhooks paid-only
- MailerLite: good REST API v2, well-documented
- Brevo: comprehensive REST API
- ConvertKit: good REST API, best free tier (10k subs) but US-based

## Decision

**SUPERSEDED** — The automation-based DOI pattern does not meet requirements R6/R7 (download gated behind confirmed DOI). Structured research required before any further implementation. See `02-05-RESEARCH-PLAN.md` for the full research plan covering Sender.net viability and alternative providers.
