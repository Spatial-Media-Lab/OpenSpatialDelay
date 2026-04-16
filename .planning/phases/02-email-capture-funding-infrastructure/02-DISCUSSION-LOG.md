# Phase 2: Email Capture & Funding Infrastructure - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-04-15
**Phase:** 02-email-capture-funding-infrastructure
**Areas discussed:** Tally form design, Privacy policy hosting & drafting, Patreon identity & tiers, Launch-post content strategy, Gate architecture (emergent), SML integration (emergent), Site shell & hosting platform (emergent)

---

## Gray Area Selection

| Option | Description | Selected |
|--------|-------------|----------|
| Tally form design | Fields beyond email, post-submit UX, consent structure | ✓ |
| Privacy policy hosting & drafting | Where it lives, who drafts it, jurisdictions, retention | ✓ |
| Patreon identity & tiers | Creator identity, pitch, tier count, pricing | ✓ |
| Launch-post content strategy | Seed-post content, visibility, drafting | ✓ |

**User selected all four areas.**

---

## Tally form design — initial round

**User interjection:** Rejected the "email is optional / download ungated" framing I had drawn from ROADMAP.md and REQUIREMENTS.md. Corrected:
1. **Email is the gate for the built installer** — required, not optional. Email IS the payment for the free plugin.
2. Source code stays public on GitHub; only the built installer is email-gated.
3. Patreon must be always-mentioned-never-primary — present in every post, email, DM, in-plugin copy (aspirational), but never the lead CTA.
4. Added scope: draft influencer-outreach email for spatial-audio reviewers.
5. Added scope: draft community-forum posts for "nerd" forums (KVR, Gearspace, Reddit, HN, etc.).

Claude flagged both new scope items as Phase 5 work, not Phase 2.

### Q: What fields should the Tally form have beyond the optional email? (pre-correction)

| Option | Description | Selected |
|--------|-------------|----------|
| Email + consent only (Recommended) | Single-field form | ✓ |
| Email + one optional question | e.g. "What DAW?" | |
| Email + 2-3 optional questions | Richer signal, more drop-off | |

**User's choice:** Email + consent only — this decision survived the correction. Post-correction, email becomes required rather than optional.

Remaining three questions in this batch were rejected by user to clarify gate-architecture framing first.

---

## Gate architecture & doc reconciliation (emergent)

### Q: How strict should the installer gate be?

| Option | Description | Selected |
|--------|-------------|----------|
| Gate-by-friction (Recommended) | Installer on GitHub Releases, Tally is the official path, direct URLs will leak | ✓ |
| Gate-by-hosting | Installer off GitHub (Cloudflare R2 etc.); GitHub has only source tarball | |
| Hybrid: obscured GitHub release | Private/draft GitHub release, Tally reveals public link | |

**User's choice:** Gate-by-friction.

### Q: ROADMAP + REQUIREMENTS contradict "email is the gate." How to handle?

| Option | Description | Selected |
|--------|-------------|----------|
| Add a plan to fix ROADMAP + REQUIREMENTS (Recommended) | Dedicated plan step; audit-traceable | ✓ |
| Fix docs inline during planning | No dedicated step, faster but less traceable | |
| Leave docs as-is, reinterpret | Risky for future readers | |

**User's choice:** Dedicated plan step to fix docs.

### Q: Where do influencer-outreach email + forum-post drafts belong?

| Option | Description | Selected |
|--------|-------------|----------|
| Fold into Phase 5 (Recommended) | Matches Phase 5 Launch Announcements scope | ✓ |
| New Phase 4.5 — Outreach Drafts | Clean separation; adds phase overhead | |
| Fold into Phase 2 | Violates phase boundary | |

**User's choice:** Fold into Phase 5.

### Q: How should Patreon appear in Phase 2 artifacts?

| Option | Description | Selected |
|--------|-------------|----------|
| Tally post-submit + privacy footer (Recommended) | Soft presence, not featured in form | ✓ |
| Only in follow-up communications | Phase 5 only | |
| Tally redirect page only | Single touchpoint | |

**User's choice:** Tally post-submit + privacy policy footer.

---

## Tally form design — continued

### Q: What happens after the user submits the Tally form?

| Option | Description | Selected |
|--------|-------------|----------|
| Thank-you page with manual download + Patreon CTA (Recommended) | Clean, Patreon-air-time, no jarring redirect | ✓ |
| Brief thank-you + auto-redirect to GitHub Releases | Some users miss Patreon CTA | |
| Thank-you + auto-redirect + manual button | Hybrid | |

**User's choice:** Thank-you page with manual download + Patreon CTA.

### Q: How should the GDPR consent checkbox be structured?

| Option | Description | Selected |
|--------|-------------|----------|
| Single required checkbox, plain-English (Recommended) | Covers storage + marketing together | ✓ |
| Two checkboxes: storage + marketing | More granular legally | |
| No checkbox — notice text only | Weakest GDPR posture | |

**User's choice:** Single required checkbox.

### Q: Where do collected emails end up?

| Option | Description | Selected |
|--------|-------------|----------|
| Keep in Tally only for now (Recommended) | Lowest infra; CSV export in Phase 5 | ✓ |
| Auto-sync to SML newsletter via webhook | Requires paid tier | |
| Dedicated OSD Buttondown/Mailchimp list | Separate list, more setup | |

**User's choice:** Tally dashboard only for Phase 2.

### Q: Who drafts Tally form copy?

| Option | Description | Selected |
|--------|-------------|----------|
| Claude drafts, user reviews (Recommended) | Planner proposes copy for approval | ✓ |
| User writes during execution | Placeholders until execution | |
| Match a specific reference | User provides URL during execution | |

**User's choice:** Claude drafts, user reviews.

---

## Privacy policy hosting & drafting

### Q: Who drafts the privacy policy?

| Option | Description | Selected |
|--------|-------------|----------|
| Claude drafts from scratch, user reviews (Recommended) | Tailored to actual data flow | ✓ |
| Template generator (iubenda / TermsFeed) | Bloated boilerplate | |
| Copy-adapt from similar project | Legally fuzzy | |

**User's choice:** Claude drafts from scratch.

### Q: What jurisdictions should the privacy policy cover?

| Option | Description | Selected |
|--------|-------------|----------|
| GDPR + generic "rights regardless of location" (Recommended) | Minimal text, 90% coverage | |
| GDPR + CCPA explicitly | Doubled length | |
| GDPR + CCPA + UK DPA | Most exhaustive | ✓ |

**User's choice:** GDPR + CCPA + UK DPA.

### Q: Email retention and unsubscribe policy?

| Option | Description | Selected |
|--------|-------------|----------|
| Retain until unsubscribe; simple opt-out (Recommended) | Simplest to explain | ✓ |
| Retain for N years of inactivity, then purge | Stronger data-minimization | |
| Retain indefinitely, no auto-purge | Simplest operationally | |

**User's choice:** Retain until unsubscribe.

### Q: Where should the privacy policy live? (initial ask)

**User asked clarifying question:** "What is the privacy policy for exactly?"

Claude explained: Tally's consent checkbox links to it; it's the legal backing for informed consent; it needs a stable URL that exists before Tally goes live and that user controls long-term.

### Q: Netlify vs Vercel — user reopened platform decision

Claude compared both. Key finding: Vercel Hobby tier prohibits commercial use; Patreon-linked site qualifies as commercial — a real risk.

### Q: Could we try both and pick one?

Claude confirmed: easy to bake off; same source deploys to both; only one can own the custom domain at a time; set a deadline before it drags.

### Q: Domain & hub architecture (emergent redesign)

**User redesign:** Personal site is `andrewrahman.com`; hosts the legal copy; OpenSpatialDelay is "one product we post about." Needs capacity to expand into multi-product hub, standard nav, bio page.

Claude confirmed Next.js 15 + React 19 + Tailwind 4 stack handles all of this. Flagged as Phase 3 scope expansion — should be captured but not decided in Phase 2.

### Q: Is domain `andrewrahman.app`?

| Option | Description | Selected |
|--------|-------------|----------|
| Yes, andrewrahman.app | .app TLD | |
| No — it's andrewrahman.com | .com | ✓ |
| Something else | User specifies | |

**User's choice:** andrewrahman.com.

### Q: Confirm personal site is multi-product hub (Phase 3 scope expansion)?

| Option | Description | Selected |
|--------|-------------|----------|
| Yes, Phase 3 expands to hub (Recommended) | Capture for Phase 3 discussion | |
| Yes, discuss hub details in Phase 2 | Risks turning into Phase 3 discussion | |
| No — hub is aspirational, Phase 3 stays as OSD landing | Phase 3 unchanged | ✓ |

**User's choice:** Hub is aspirational. Phase 3 stays as OSD landing page (but now lives at andrewrahman.com instead of raw Netlify subdomain).

### Q: Where does the privacy page live in Phase 2?

| Option | Description | Selected |
|--------|-------------|----------|
| Stand up hub shell on andrewrahman.com now (Recommended) | Stable URL day 1; no migration churn | ✓ |
| Temporary URL, migrate in Phase 3 | Lower Phase 2 scope, broken-link risk | |
| Tally-hosted until Phase 3 | Weakest long-term | |

**User's choice:** Stand up minimal shell on andrewrahman.com now.

### Q: Netlify vs Vercel — when to pick?

| Option | Description | Selected |
|--------|-------------|----------|
| Bake off, pick by end of Phase 2 (Recommended) | 15 min setup, real information | ✓ |
| Pick Netlify now | Research validated | |
| Pick Vercel now | Better Next.js DX, commercial-use gray | |

**User's choice:** Bake off.

### Q: Reconciliation — Phase 2 stands up andrewrahman.com shell, Phase 3 stays as OSD landing within it

| Option | Description | Selected |
|--------|-------------|----------|
| Yes, that's the plan (Recommended) | Minimal shell + privacy + bake-off | (effectively yes — user provided additional info) |
| Yes with different homepage placeholder | User specifies | |
| No — re-explain | User clarifies | |

**User's note:** "I already own andrewrahman.com, we just need to point the mxrecord to the new site platform. I own the url through netfirms.com, I believe, or namecheap.com."

Claude corrected: MX is for email routing; web traffic needs A or CNAME record. User verifies registrar (Netfirms or Namecheap) as first Phase 2 execution step.

### Q: Registrar for andrewrahman.com

| Option | Description | Selected |
|--------|-------------|----------|
| Cloudflare Registrar (Recommended) | At-cost pricing | |
| Porkbun | Indie alternative | |
| Namecheap | Classic | |
| User already has it / handles themselves | Domain owned | ✓ |

**User's choice:** Already owned.

### Q: Placeholder homepage content

| Option | Description | Selected |
|--------|-------------|----------|
| Minimal bio + "more coming" (Recommended) | Enough to not look broken | ✓ |
| Literal "Coming soon" page | Fastest, unfinished look | |
| Skip homepage, only /privacy | Broken-homepage risk | |

**User's choice:** Minimal bio + "more coming".

---

## Patreon identity & tiers

### Q: Creator identity for Patreon?

| Option | Description | Selected |
|--------|-------------|----------|
| Andrew Rahman (personal) (Recommended) | Matches personal-hub direction | ✓ |
| Spatial Media Lab (org) | Cleaner tax/org separation | |
| New brand — "Spatial Audio Tools" etc. | Fresh identity, zero audience | |

**User's choice:** Andrew Rahman (personal).

### Q: Core patron-value pitch?

| Option | Description | Selected |
|--------|-------------|----------|
| Funding the spatial audio tools pipeline (Recommended) | Matches PROJECT.md | |
| Behind-the-scenes indie plugin dev | Paying for the process | |
| Exclusive plugins + feature requests | Transactional framing | |

**User's choice (with edit):** "Funding the **SpatialMediaLibrary** pipeline" — user supplied the actual brand name of the toolset (clarified later as "Spatial Media Library," three words).

### Q: Tier structure and pricing?

| Option | Description | Selected |
|--------|-------------|----------|
| Three tiers: $3 / $10 / $25 (Recommended) | Standard indie shape | |
| Two tiers: $5 / $20 | Simpler | |
| Single "pay what you can" | One tier, suggested minimums | |
| Four tiers with annual option | $3/$10/$25/$100 + annual discount | ✓ |

**User's choice:** Four tiers with annual option.

### Q: Mention future Patreon-only plugins on launch page?

| Option | Description | Selected |
|--------|-------------|----------|
| Yes, as a future plan (Recommended) | Soft commitment | ✓ |
| Yes, with a timeframe | Concrete but deadline-binding | |
| No, keep OSD-focused | Lower pressure | |

**User's choice:** Yes, as a future plan (no timeframe).

---

## Brand clarification (emergent)

### Q: Is "SpatialMediaLibrary" the actual pipeline brand?

| Option | Description | Selected |
|--------|-------------|----------|
| Yes — "Spatial Media Library" (three words) | Distinct from SML org | ✓ |
| Yes — "SpatialMediaLibrary" (one word) | Single-word stylized | |
| Typo — meant Spatial Media Lab | Roll back | |
| Different brand entirely | User supplies | |

**User's choice:** "Spatial Media Library" (three words).

### Q: Tier prices & benefits detail?

| Option | Description | Selected |
|--------|-------------|----------|
| Claude drafts recommendation (Recommended) | $3/$10/$25/$100 + ~15% annual discount | ✓ |
| Same structure, different prices | User specifies | |
| Same prices, rethink benefits now | Discuss benefits now | |

**User's choice:** Claude drafts the recommendation; user reviews in plan.

---

## Launch-post content strategy

### Q: How many Patreon seed posts at launch?

| Option | Description | Selected |
|--------|-------------|----------|
| Exactly 2 — minimum for DIST-03 (Recommended) | Lower risk, faster | |
| 3 posts | Intro + technical + roadmap | ✓ |
| 4+ posts | Fullest launch page | |

**User's choice:** 3 posts.

### Q: Which topics should the seed posts cover? (multi-select)

| Option | Description | Selected |
|--------|-------------|----------|
| Welcome / why-Patreon / pipeline vision | Anchor post | ✓ |
| OpenSpatialDelay technical deep-dive | Audio-nerd audience | ✓ |
| Spatial Media Library roadmap | Pipeline themes | ✓ |
| Personal story / path to indie audio dev | Human-interest framing | |

**User's choice:** Welcome + Technical + Roadmap.

### Q: Public vs patron-only?

| Option | Description | Selected |
|--------|-------------|----------|
| All public at launch (Recommended) | Max aliveness signal | |
| Welcome public, deep-dive patron-only | Conversion hook | ✓ |
| All patron-only | Conversion killer | |

**User's choice:** Welcome public, deep-dive patron-only. Roadmap visibility confirmed later as public.

### Q: Who drafts the seed posts?

| Option | Description | Selected |
|--------|-------------|----------|
| Claude drafts all, user revises (Recommended) | Fast, consistent, editorial control | ✓ |
| Claude drafts technical/roadmap, user writes welcome/personal | Split by tone-sensitivity | |
| User writes all | Most authentic, highest time cost | |

**User's choice:** Claude drafts all, user revises.

---

## Final revision — SML integration (emergent)

**User addition:** "Everything also needs to point to Spatial Media Lab. The Spatial Media Lab will own the github and the source code will live there. This should also have a small call to action to folks that live in berlin to join the spatial media lab, but not urgently, just a one-liner somewhere on the patreon."

Captured as D-01 through D-03, D-25, D-26 in CONTEXT.md:
- Three-identity relationship (Spatial Media Lab / Spatial Media Library / OpenSpatialDelay / Andrew Rahman) documented
- Patreon page links to `github.com/Spatial-Media-Lab/OpenSpatialDelay`, `SpatialMediaLab.org`, `andrewrahman.com`, `andrewrahman.com/privacy`
- Berlin CTA one-liner in Patreon sidebar/footer, low-urgency tone
- Data-controller designation flagged for planner to resolve (likely Spatial Media Lab)

### Q: Confirm SML integration capture?

| Option | Description | Selected |
|--------|-------------|----------|
| Yes — write CONTEXT.md (Recommended) | Ready to finalize | ✓ |
| Small fixes | User specifies | |
| One more revision | Reopen an area | |

**User's choice:** Write CONTEXT.md.

---

## Claude's Discretion

Areas where Claude will make specific choices during execution, within the constraints above:

- Exact Tally form visual styling within template limits
- Exact consent-checkbox wording (within plain-English + GDPR-valid constraints)
- Specific bio-paragraph copy for andrewrahman.com placeholder (user revises)
- Specific tier benefit descriptions beyond the D-23 baseline (editorial)
- Specific prose in each of the 3 Patreon seed posts (user revises for voice)
- Privacy policy paragraph ordering and specific jurisdictional-clause phrasing (within GDPR + CCPA + UK DPA compliance)

---

## Deferred Ideas

Captured in CONTEXT.md `<deferred>` section. Summary:

- **Phase 3:** full personal-hub expansion (multi-product, blog, per-product pages)
- **Phase 5:** influencer-outreach email draft, community-forum post drafts (KVR, Gearspace, Reddit, HN), newsletter-tool integration
- **Future milestone:** actual Patreon-exclusive plugin release, personal-hub build-out, in-plugin Patreon CTA

---

## 2026-04-16 — Tool swap: Tally → Sender.net (Plan 02-05 supersession)

### Trigger
During Plan 02-05 execution, user flagged that "Tally because a friend recommended it" should be re-reviewed against the actual desired flow: email field → signup to newsletter → (optional DOI per GDPR) → download. User specifically asked about putting the download link inside the verification email if double opt-in is legally required.

### Finding
Tally is a form builder only — it captures emails and displays thank-you pages but does NOT send newsletters, does NOT support DOI confirmation emails, and does NOT deliver downloads via email. The original plan quietly treated the "list" as a static Tally dashboard export (D-10 explicitly deferred newsletter integration to Phase 5). This was inconsistent with the user's stated flow.

### Options evaluated
Kit (ConvertKit) US; MailerLite US (EU region available); EmailOctopus UK+IE; Brevo FR; Mailchimp US; CleverReach DE; Rapidmail DE; Sender.net LT. Mailchimp disqualified (2026 free tier cut to 250 contacts / 500 sends, no automation). Rapidmail disqualified (not a true free plan — pay-per-send). CleverReach free cap too small (250 subs). Kit had strong feature match but US-based + mandatory creator recommendations on free tier. MailerLite free cap dropped to 500 subs — tight for launch. Brevo's 300 emails/day throttle blocks burst campaigns.

### Decision
**Sender.net** (UAB Sender.lt, Vilnius, Lithuania). Reasons:
- 2,500 subs / 15,000 emails/month (generous relative to all other EU options)
- Unlimited automation on free plan (needed for DOI → download flow)
- Lithuanian company = EU jurisdiction, GDPR applies fully, no SCCs needed in privacy policy
- No mandatory cross-promotion (unlike Kit)
- No daily send throttle (unlike Brevo)
- Embeddable form + native DOI support via automation workflow

### DOI flow chosen: Option A (download buttons in the confirmation email)
Option A — Single DOI email contains confirm button + macOS/Windows download buttons + Patreon CTA all inline. User clicks confirm → subscriber moves from `osd-unconfirmed` to `osd-confirmed`. Downloads are directly accessible from the email whether or not the user clicks confirm. Rationale: friction-minimal, standard creator pattern. Legal position: pre-submit consent notice (hint text below email field) + DOI email itself are adequate consent evidence.

### Consequences
- Superseded decisions: Init "Tally for email capture", D-10 (defer newsletter to Phase 5). See STATE.md updated 2026-04-16.
- Plan 02-05 rewritten from scratch (Tally dashboard walkthrough → Sender.net dashboard walkthrough + DOI automation build).
- andrewrahman-com site repo changes scheduled in Plan 02-05 Task 2: rename env var, rewrite get-osd page.tsx, update privacy page processor section, swap CSP, replace Playwright test.
- Privacy policy effective date bumps per Section 9 commitment.
- Reverted unused tally-embed.spec.ts commit on andrewrahman-com (commit 61519c3).

### Accepted trade-offs
- Sender branding appears in free-tier email footers (removing costs ~$8–15/mo on Standard tier).
- Data center physical location not explicitly stated by Sender publicly; will confirm via support email during Task 1 Step D and document in the privacy policy.
