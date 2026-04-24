# Plan 02-05: Mailing List Provider — Research Plan

Date: 2026-04-24
Status: Ready for next session to execute
Deadline: 2026-04-28 (launch day)

## Why This Research Exists

~8 hours spent on Sender.net DOI across 4 sessions. All prior approaches failed. A proposed "untried pattern" (automation-based DOI with plain URLs) was identified but has a **critical flaw**: it does not actually gate the download behind DOI confirmation. The confirm button links to a public download page — clicking it is tracked but not validated as formal DOI consent. A user who never confirms can still access the download URL directly. Under German law this is insufficient DOI and does not satisfy the project's business requirement that email subscription IS the payment.

## Hard Requirements (non-negotiable)

These are extracted from REQUIREMENTS.md (DIST-01), PROJECT.md, and the privacy policy:

### Legal / GDPR
- R1: **Double opt-in (DOI) mandatory** — German law requires explicit email confirmation before adding to mailing list
- R2: **EU-based data processing** — processor must store/process data within EEA (Andrew is Germany-based)
- R3: **GDPR Art. 28 data processing agreement** — processor must offer DPA
- R4: **Data controller**: Andrew Rahman (natural person)
- R5: **Unsubscribe mechanism** in every email

### Business Model
- R6: **Download is gated behind confirmed email subscription** — signing up to the mailing list IS the payment for the product. A user who has not confirmed their subscription must NOT receive the download
- R7: **DOI confirmation must be a distinct, explicit consent action** — not a tracked click on a download link. The confirmation and the download delivery can be sequential but must be logically separate
- R8: **Post-confirmation download delivery** — after confirmed DOI, user must receive access to macOS + Windows download files

### Technical
- R9: **Embeddable form** on static Next.js site (output: 'export', no server-side)
- R10: **Email forwarding compatible** — must work with ImprovMX forwarding (hey@andrewrahman.com → Gmail) or similar
- R11: **Custom sender domain** — emails from hey@andrewrahman.com (or similar branded address)
- R12: **Free tier** preferred (launch scale ~100-500 subscribers)
- R13: **SPF + DKIM + DMARC** authentication support

### Timeline
- R14: **Must be fully operational by 2026-04-28** (4 days from research date)
- R15: Setup time must be realistic — account verification delays, DNS propagation, etc.

## Research Track 1: Can Sender.net Meet These Requirements?

### What to investigate

The core question: **Is there ANY Sender.net architecture where the download is gated behind a validated DOI confirmation?**

Check these specific angles:

1. **Form-level DOI with automation chaining**
   - Form DOI toggle ON → Sender handles DOI natively
   - After DOI confirm, does subscriber status change to `Active` in a way that triggers "Subscriber status changed"?
   - Memory file says this trigger is "unreliable" — but has it been tested empirically with form-DOI ON + automation listening for status change?
   - If this trigger works: automation sends download email ONLY to confirmed subscribers

2. **Form-level DOI + "Subscriber joins a group" timing**
   - When form-DOI is ON, at what point does the subscriber get added to the form's target group?
   - If group-add happens AFTER DOI confirmation (not on form submit), then "Subscriber joins a group" trigger could fire post-confirm
   - This was identified as needing empirical testing in the 02-05-NEXT-SESSION.md but was never tested

3. **Form-level DOI redirect as confirmation signal**
   - The form's "Redirect after submit" fires pre-DOI (empirically confirmed)
   - But what about the DOI confirmation page itself? Can it be customized?
   - Sender's DOI settings panel — is there a "confirmation page URL" or "post-confirmation redirect" field that was missed?

4. **Two-automation chain**
   - Automation 1: form-DOI OFF, subscriber added to `holding` group, sends confirm email with button to a **non-download URL** (e.g., `https://andrewrahman.com/confirmed/` — a simple "thanks, you're confirmed" page)
   - "Link is clicked" condition → move to `osd-confirmed` group
   - Automation 2: trigger = "Subscriber joins group: osd-confirmed" → send download email with links to `/get-osd/`
   - Problem: the "confirm" URL is still publicly accessible and clicking it ≠ formal DOI consent
   - Does Sender's "Link is clicked" tracking constitute sufficient DOI proof for German law?

5. **Sender API polling fallback**
   - API endpoint `GET /v2/subscribers/{email}` returns status
   - Could a Netlify serverless function check subscriber status before showing downloads?
   - This adds server-side logic to what is currently a static site — is it worth it?

6. **Paid tier consideration**
   - Sender Standard plan has webhooks (8 event types)
   - Does any webhook fire on DOI confirmation?
   - Cost vs. switching to a free alternative?

### Decision criteria for Sender

Sender stays IF and ONLY IF:
- There is a confirmed mechanism where download delivery happens ONLY after validated DOI
- The DOI confirmation is a distinct consent action (not just clicking a download link)
- It can be set up within 2 days

## Research Track 2: Alternative Providers

### Candidates to evaluate

Each candidate must be checked against ALL requirements R1-R15.

#### Tier 1 (most promising — check first)

**MailerLite**
- EU-based: Yes (UAB MailerLite, Vilnius, Lithuania)
- Free tier: 1,000 subscribers / 12,000 emails/month
- DOI: Native with customizable confirmation page
- Key question: Does the DOI confirmation page support a **redirect URL** to a post-confirm page? (This is the feature Sender lacks)
- Key question: Can the post-DOI redirect go to `/get-osd/` while the DOI itself is a separate "confirm subscription" action?
- Key question: Can you send a post-confirmation automated email with download links?
- API: REST v2, well-documented
- Migration cost: DNS records (swap SPF/DKIM), site code (EmailCaptureSection.tsx, CSP, tests), privacy policy update
- Account verification time?

**Brevo (Sendinblue)**
- EU-based: Yes (Sendinblue SAS, Paris, France)
- Free tier: Unlimited contacts / 300 emails per day
- DOI: Native support
- Key question: Post-DOI redirect URL configurable?
- Key question: Post-DOI automated email support?
- API: Comprehensive REST API
- Migration cost: similar to MailerLite

#### Tier 2 (check if Tier 1 fails)

**ConvertKit / Kit**
- EU-based: No (US) — check if EU data processing available
- Free tier: 10,000 subscribers (very generous)
- DOI: Native with customizable confirmation
- Key question: Does it offer EU data residency?
- API: REST v4

**EmailOctopus**
- EU-based: Check (UK-based, post-Brexit)
- Free tier: 2,500 subscribers
- DOI: Check
- API: REST

**Mailchimp**
- EU-based: No (US) — but has EU data processing addendum
- Free tier: 500 contacts / 1,000 sends per month (very limited)
- DOI: Native, mature
- Key question: Already used for SML newsletter — conflict with OSD list?

#### Tier 3 (only if desperate)

**Buttondown** — 100 subscriber free tier (too low)
**Resend** — transactional only, no forms/automations

### Evaluation matrix per candidate

For each candidate, the research agent must fill in:

| Criterion | Answer | Evidence URL |
|-----------|--------|-------------|
| EU data processing | Yes/No | |
| DPA available | Yes/No | |
| Free tier limits | X subs / Y emails | |
| DOI support | Native/Manual/None | |
| Post-DOI redirect URL | Yes/No | |
| Post-DOI automated email | Yes/No | |
| Embeddable form (static site) | Yes/No | |
| Custom sender domain (SPF/DKIM) | Yes/No | |
| ImprovMX compatible | Yes/No/Unknown | |
| Account verification time | Instant/Hours/Days | |
| API quality | Excellent/Good/Moderate/Poor | |
| Setup time estimate | Hours | |
| Meets ALL R1-R15 | Yes/No + which fail | |

## Research Track 3: Architecture Alternatives

If no provider natively solves the "gate download behind DOI" requirement:

1. **Netlify serverless function as middleware**
   - User confirms DOI → provider sends webhook → Netlify function records confirmation
   - `/get-osd/` page calls function to check status before showing downloads
   - Adds server-side complexity to static site
   - Estimate setup time

2. **Token-based download URL**
   - DOI confirmation email contains a unique download URL with a token
   - Token validates against a simple KV store or serverless function
   - More complex but fully gates the download

3. **GitHub Release assets as gate**
   - Downloads stay on GitHub Releases (already there)
   - DOI confirmation email contains direct GitHub Release links
   - No need to host downloads on the site at all
   - `/get-osd/` page becomes a "thanks" page, not a download page
   - Simplest architecture — does this satisfy R6?

## Existing Infrastructure to Preserve or Migrate

Current Sender.net setup that would need equivalent in any new provider:
- DNS: andrewrahman.com SPF includes `sendersrv.com`, DKIM CNAME `sender._domainkey`
- ImprovMX: MX records for hey@andrewrahman.com forwarding (provider-agnostic)
- DMARC: `v=DMARC1; p=none;` (provider-agnostic)
- Privacy policy: names UAB Sender.lt — policy Section 9 requires update + subscriber notification before processor change (but no subscribers yet, so notification is vacuous)
- Site code: `components/EmailCaptureSection.tsx` loads `cdn.sender.net` script
- CSP headers: `_headers` whitelists `cdn.sender.net`, `api.sender.net`, `*.sender.net`
- Tests: `tests/sender-embed.spec.ts` (3 tests)
- Env: `NEXT_PUBLIC_SENDER_FORM_ID=bkRxov`

## Output Expected

The research session should produce:
1. A clear YES/NO on whether Sender.net can meet requirements R1-R15
2. A ranked list of alternatives with the evaluation matrix filled in
3. A recommended provider with a concrete setup timeline
4. If recommending a switch: a step-by-step migration checklist with time estimates
5. Update to `.planning/phases/02-email-capture-funding-infrastructure/02-05-MAILING-LIST-ANALYSIS.md` with findings

## How to Execute This Research

Use GSD research agents (`/gsd-research-phase` or manual research) to:
1. Web search each provider's DOI documentation
2. Check their GDPR/DPA pages
3. Verify free tier limits on current pricing pages
4. Test signup flows where possible (create throwaway accounts)
5. Check API docs for webhook/automation capabilities

Time budget: ~2 hours for research, ~1 hour for decision + migration plan.
