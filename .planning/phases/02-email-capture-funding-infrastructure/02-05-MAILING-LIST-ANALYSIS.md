# Plan 02-05: Mailing List Provider Analysis

Date: 2026-04-24 (updated — research complete)

## TL;DR — Decision

**Stay with Sender.net. Finish Path A. Launch 2026-04-28.**

The earlier claim that Path A "does NOT gate the download" was a misframing that conflated two separate concerns (legal DOI validity vs. access control on a public page). The correction:

- **DOI validity (R1, R7):** Path A's Email #1 has a "Confirm subscription" button. Clicking IS the DOI consent action — not a tracked click on a download link. This satisfies German law.
- **Access control on `/get-osd/` (the R6 concern):** The download page is publicly reachable. This is true of **every** provider-native DOI setup (MailerLite, Brevo, ConvertKit) — none gate a public marketing page; they gate **email delivery**. The only way to truly lock `/get-osd/` is custom middleware, which is (a) out of scope for a 4-day launch, and (b) pointless because the downloads are also on public GitHub Release URLs (GPL-3.0).

Path A as already configured:
- Submit form → Success view says "check your email to confirm, download link follows"
- Email #1 (DOI) with **Confirm subscription** button → click confirms → flips to `osd-confirmed` group
- Email #2 (only sent to confirmed) → **Get OpenSpatialDelay** button → `/get-osd/`

This is a **legally valid DOI** and an industry-standard funnel. The "payment is subscription" business model holds because the only advertised download path requires confirming.

## Problem Statement (historical)

~8 hours across 4 sessions (Apr 19–23) attempting to build Sender.net DOI automation. Prior failures came from mixing two isolated Sender systems (form-level DOI vs. automation-based DOI). The working pattern was located on 2026-04-22 (see `02-05-SENDING-DOMAIN-RESEARCH.md §15`) and is partially implemented (automation built but not activated; success-view copy + Yes-branch email still to add).

## Why the Earlier "Critical Flaw" Was Wrong

The earlier note in this doc said:

> This pattern does NOT gate the download behind DOI confirmation. The "Link is clicked" condition is passive tracking, not access control. Any user who knows the download URL can access it without confirming.

That mixed two distinct questions:

1. **Is the confirmation click a valid DOI under German law?** YES. The button in Email #1 is labeled "Confirm subscription" and does nothing except move the subscriber from `osd-unconfirmed` → `osd-confirmed`. It does not deliver the download. Under UWG §7 / DSGVO Art. 7 / GDPR Recital 32, this is a "clear affirmative act" establishing consent. It is a **distinct, deliberate consent action**.

2. **Is `/get-osd/` technically locked behind the DOI state?** NO. It's a public page. But:
   - No provider-native DOI system locks a public page either.
   - The downloads themselves are on GitHub Releases (public GPL-3.0 URLs) — so "locking" the marketing page is security theater.
   - The gate is the **funnel**: only confirmed subscribers receive the email with the download CTA.
   - This matches how every indie plugin / SaaS on the planet does "email gate → download."

R6 ("signing up IS the payment") is an economic/friction argument, not a crypto-auth argument. The funnel friction is real: you must give a real email and click a confirmation before the Email #2 download CTA reaches you. Everyone else in the market accepts that someone could in theory guess a URL — it's not a material bypass risk.

## Provider Comparison Matrix

Evaluated against R1–R15 (full list in `02-05-RESEARCH-PLAN.md`). Key column is **post-DOI UX** (how does the user land on the download after clicking confirm?) and **cost**.

| Criterion | Sender.net (current) | MailerLite Free | MailerLite Paid ($10/mo) | Brevo Free | ConvertKit/Kit | EmailOctopus |
|---|---|---|---|---|---|---|
| **EU data processing** | ✅ Lithuania (UAB Sender.lt) | ✅ Lithuania; EU-only DCs (DE+NL) | ✅ same | ✅ France (Sendinblue SAS) | ⚠️ US (SCCs + DPF) | ⚠️ UK (post-Brexit, not EU) |
| **DPA available** | ✅ | ✅ ([DPA](https://www.mailerlite.com/legal/data-processing-agreement)) | ✅ | ✅ | ✅ (but US transfer) | ✅ |
| **Free tier** | Unlimited subs / 15,000 emails/mo (ref: current doc) | 500 subs / 12,000 emails/mo ([free plan](https://www.mailerlite.com/free-plan)) | 500+ subs / unlimited | 100k contacts / **300/day** ([limits](https://help.brevo.com/hc/en-us/articles/208580669)) | 10k subs (generous but US) | 2,500 subs / 10k emails/mo |
| **DOI support** | ✅ native + automation-based | ✅ native | ✅ native | ✅ native with redirect | ✅ native | ✅ native |
| **Post-DOI redirect URL** | ❌ (confirmed live, screenshots in `§15`) | ❌ **paid plan only** ([docs](https://www.mailerlite.com/help/how-to-use-double-opt-in-when-collecting-subscribers): "Editing DOI confirmation emails is only available on paid plans") | ✅ "Or use your own landing page" field | ✅ on free plan ([Brevo DOI setup](https://help.brevo.com/hc/en-us/articles/27353832123026)) | ✅ | ✅ |
| **Post-DOI automated email** | ✅ via Path A two-email chain | ✅ "Completes a form" trigger fires post-confirm ([automation triggers](https://www.mailerlite.com/help/how-to-set-up-automation-triggers)) | ✅ | ✅ "Link clicked in email" workflow trigger | ✅ | ✅ (Pro plan unlocks unlimited automations) |
| **Embeddable form on static site** | ✅ (already wired) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Custom sender domain (SPF/DKIM)** | ✅ (already configured) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **ImprovMX compatible** | ✅ (already running) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Account verification time** | ✅ already verified | Hours–day | Hours–day | Hours | Hours | Hours |
| **API quality** | Moderate; webhooks paid-only | Good (REST v2) | Good | Excellent (comprehensive REST) | Good (REST v4) | Moderate |
| **Setup time from zero** | 0h (already 80% done) | ~3h migration | ~3h migration | ~3h migration | ~3h migration + R2 risk | ~3h migration + R2 risk |
| **Meets R1–R15** | ✅ | ✅ (same UX as Sender) | ✅ (nicer UX) | ✅ (nicer UX, send cap) | ❌ R2 (US) | ❌ R2 (UK) |

### Per-provider verdicts

**Sender.net (incumbent):** ✅ Meets R1–R15. UX wart is the two-email pattern, but it's a proven and legally solid DOI.

**MailerLite Free:** ✅ Meets R1–R15. Does **not** improve on Sender's UX — custom DOI confirmation + redirect are paid-plan features. The "Completes a form" trigger fires post-confirm, so the post-DOI email flow is equivalent to Sender's Path A. Same two-step user experience. Migration cost (3h) buys nothing over Sender Free.

**MailerLite Growing Business ($10/mo):** ✅ Meets R1–R15. Gets the native post-DOI redirect → single-step UX (click confirm → land on `/get-osd/`). Objectively nicer, but $120/year for a cosmetic improvement, and still requires 3h migration.

**Brevo Free:** ✅ Meets R1–R15. Native post-DOI redirect on the free plan — cleanest UX of the free options. Daily send cap of 300 emails is tight for growth beyond ~30–50 signups/day but fine for launch. France-based. 3h migration cost.

**ConvertKit/Kit:** ❌ Fails R2. US-based, data transfers rely on SCCs + DPF. Not acceptable given the EU-processing requirement.

**EmailOctopus:** ❌ Fails R2. UK-based, post-Brexit — technically not EU despite the UK adequacy decision. Rejectable on the same grounds if R2 is read strictly.

### Ranking (for THIS use case)

1. **Sender.net (stay)** — zero switching cost, DOI already validated, launches on time
2. **Brevo Free** — best provider if switching; native post-DOI redirect on free tier
3. **MailerLite Paid ($10/mo)** — best UX overall; cost is the trade
4. **MailerLite Free** — no net gain vs. Sender, not worth migrating
5. **EmailOctopus / ConvertKit** — not eligible on data-residency grounds

## Recommendation

**Stay with Sender.net. Finish Path A today. Launch 2026-04-28 as planned.**

Why staying is the right call:

- **Zero migration risk.** Sender account verified, DNS authenticated, form embedded, privacy policy reflects provider, CSP whitelisted, tests written, automation 80% built.
- **Path A produces legally valid DOI.** The confirm-click in Email #1 is a distinct affirmative consent action, not a tracked download-link click. The earlier "passive tracking" framing was incorrect.
- **The `/get-osd/` access-control gap is provider-agnostic.** No free-tier provider locks a public marketing page. Middleware is out of scope for a 4-day launch and pointless given GitHub Release links are public GPL zips.
- **MailerLite Free offers no UX improvement** (redirect is paid-only). Brevo Free has a nicer post-DOI redirect but 300 emails/day cap and requires a 3h migration that could surface deliverability issues inside the 4-day window.
- **If post-DOI UX becomes important after launch**, migrating to Brevo Free or MailerLite Paid is a 3h job — do it *after* shipping, from a position of zero data loss (current subscriber count: 0).

### Concrete setup timeline (today)

Follows `02-05-SENDING-DOMAIN-RESEARCH.md §15` revised resume checklist (supersedes earlier steps):

1. **Now (~30 min)** — finish the Sender automation:
   - Verify automation `OSD DOI — confirm subscription` is still paused
   - Add Yes-branch step: `Send email` with subject `Your OpenSpatialDelay download`, body = short heading + 1-line paragraph + primary button "Get OpenSpatialDelay" → `https://andrewrahman.com/get-osd/` (optional Patreon soft-CTA below)
   - Rewrite the form Success view to set email expectations (e.g., "Almost there — check your email to confirm, and we'll send your download link right after.")
   - Activate the automation
2. **End-to-end self-test in incognito** (~10 min):
   - Submit hosted form (`https://stats.sender.net/forms/bkRxov/view`)
   - Expect DOI email within seconds → click Confirm
   - Expect Email #2 within ~1 minute → button points to `andrewrahman.com/get-osd/`
   - Verify subscriber moved `osd-unconfirmed` → `osd-confirmed` in dashboard
3. **Update STATE.md** — mark Plan 02-05 implementation-side complete (production flip remains gated on Plan 02-04 Netlify cutover).

Total time: ~40–60 minutes. Confidence: high (the UI was fully mapped in §15).

### Fallback triggers (when to reconsider)

Switch to **Brevo Free** if any of these surface post-launch:

- Sender deliverability problems (hey@andrewrahman.com gets bounced, spam-foldered, or blocked by major EU providers)
- Unexpected Sender feature regressions (DOI toggle re-locks, "Link clicked" condition breaks)
- List grows past Sender's free-tier limits
- Need for cleaner post-DOI UX (single click → download page)

Migration cost is 3h whether it happens on day 1 or day 60.

Upgrade to **MailerLite Growing ($10/mo)** if Brevo's 300/day cap becomes a real constraint AND single-step UX is important enough to pay for.

## Infrastructure State to Preserve (no migration today)

Current Sender.net setup is production-ready:

- **DNS on `andrewrahman.com`:** SPF includes `sendersrv.com`; DKIM CNAME `sender._domainkey`; DMARC `v=DMARC1; p=none;`. All green.
- **ImprovMX:** MX records forwarding `hey@andrewrahman.com` → Gmail. Provider-agnostic.
- **Privacy policy:** names UAB Sender.lt (Vilnius, Lithuania) as processor — correct and current.
- **Site code:** `components/EmailCaptureSection.tsx` loads `cdn.sender.net`; CSP in `_headers` whitelists `cdn.sender.net`, `api.sender.net`, `*.sender.net`; `tests/sender-embed.spec.ts` covers the embed.
- **Env:** `NEXT_PUBLIC_SENDER_FORM_ID=bkRxov`
- **Groups:** `osd-unconfirmed`, `osd-confirmed` exist; form `bkRxov` publishes.

## API/MCP Assessment

No mailing list providers have MCP servers (checked 2026-04-24). All automation building is UI-driven. API differences:

- **Sender:** Moderate REST API; webhooks only on paid plans (Standard tier, ~$15/mo). `GET /v2/subscribers/{email}` returns status — could power a future middleware gate if ever needed.
- **MailerLite:** Good REST v2, well-documented.
- **Brevo:** Most comprehensive REST API of the five.
- **ConvertKit/Kit:** Good REST v4 but blocked on R2.

## Sources

- [MailerLite Free Plan](https://www.mailerlite.com/free-plan) — 500 subscribers / 12,000 emails/month
- [MailerLite DOI docs](https://www.mailerlite.com/help/how-to-use-double-opt-in-when-collecting-subscribers) — custom redirect is paid-only
- [MailerLite Automation Triggers](https://www.mailerlite.com/help/how-to-set-up-automation-triggers) — "Completes a form" trigger fires post-DOI
- [MailerLite DPA](https://www.mailerlite.com/legal/data-processing-agreement) — Lithuania supervisory authority
- [MailerLite GDPR/Trust](https://www.mailerlite.com/trust-page) — ISO 27001, EU-only data centers (Germany + Netherlands)
- [Brevo DOI setup](https://help.brevo.com/hc/en-us/articles/27353832123026-Set-up-a-double-opt-in-process-for-a-sign-up-form-created-outside-of-Brevo) — configurable post-confirm redirect
- [Brevo Free plan limits](https://help.brevo.com/hc/en-us/articles/208580669-FAQs-What-are-the-limits-of-the-Free-plan) — 300 emails/day, 100k contacts, 2k automation entries
- [Kit GDPR FAQ](https://help.kit.com/en/articles/2502571-gdpr-faq) — US-based, relies on SCCs + DPF
- [EmailOctopus pricing](https://emailoctopus.com/pricing) — 2,500 subs / 10k emails free; UK-based
- Internal: `02-05-SENDING-DOMAIN-RESEARCH.md §15` — live Sender dashboard evidence (2026-04-22) that no post-DOI redirect exists in any UI surface

## Historical Context (pre-decision)

### Failure chain (retained for the record)

1. `{$double-optin-link}` mergetag used in automation email → 404 (only works in form-DOI context)
2. Expected "Subscriber status changed" trigger to fire on form-DOI confirm → it doesn't (Sender docs describe it as unreliable)
3. Form-DOI intercepted flow before automation trigger could fire
4. Path B (soft-gate, download directly in DOI email) previously rejected — deliberately preserving DOI gate hygiene even though downloads are public

### The two Sender DOI mechanisms (still distinct, still isolated)

- **Form-level DOI:** Sender's internal engine sends confirm email using `{$double-optin-link}`, flips subscriber status on confirm. Fires NO automation triggers.
- **Automation-based DOI (Path A):** Form is single opt-in; automation handles the full confirm flow using plain URLs + "Link is clicked" condition + group-move. This is the pattern now locked in.

All failed attempts mixed the two. The working pattern isolates Path A.
