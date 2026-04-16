# Phase 2: Email Capture & Funding Infrastructure - Research

**Researched:** 2026-04-15
**Domain:** External-service configuration + minimal Next.js static site shell
**Confidence:** HIGH for verifiable platform mechanics (Tally, Netlify, Vercel, Patreon, DNS); MEDIUM for legal-template choices (privacy policy jurisdictional drafting); LOW where decisions depend on user-side knowledge (which registrar account holds the credentials)

## Summary

Phase 2 stands up four publicly-live destinations: a Tally email-gate form, a privacy policy at `andrewrahman.com/privacy`, a minimal Next.js shell on andrewrahman.com (Netlify-vs-Vercel bake-off), and a seeded Patreon page under "Andrew Rahman." Almost all of the technical work is **configuration on hosted SaaS** — none of it touches the JUCE plugin codebase. The repo will gain at most one new sibling project (the andrewrahman.com Next.js source). This is a low-code, high-coordination phase.

Three findings change the planning landscape vs. CONTEXT.md assumptions:

1. **DNS authority verified:** `andrewrahman.com` is registered through **Tucows** (registrar) but its nameservers are `NS1.NETFIRMS.COM` / `NS2.NETFIRMS.COM` — DNS records are managed at **Netfirms**. The current A record points to `66.96.149.1` (Netfirms parking/hosting). The first execution step is logging into Netfirms (not Namecheap, not Tucows) to change A/CNAME records. CONTEXT.md D-19 listed Netfirms-or-Namecheap; this resolves the ambiguity.
2. **Patreon annual memberships are NOT available at launch.** The annual-billing option (D-23) requires the page to be live 3+ months AND earning $200+/month. A brand-new creator page cannot enable annual billing on day 1. This needs to be either (a) deferred until eligibility hits, or (b) explicitly noted on the Patreon page as "annual coming soon."
3. **Tally custom domains require the $29/mo Pro plan.** The free plan keeps forms on `tally.so/r/...` URLs. To put the form on `andrewrahman.com/get-osd` (or similar), the choice is: (a) **embed** the Tally form in a Next.js page on andrewrahman.com (free, recommended), (b) link out to a tally.so URL (free, ugly), or (c) pay $29/mo for white-label. Embedding is free + branded.

**Primary recommendation:**
- **Tally:** free plan + embed in `andrewrahman.com/get-osd` page (Next.js component); thank-you screen with manual download buttons + soft Patreon CTA; export emails via Tally's built-in CSV (Phase 5 will add webhook → newsletter tool).
- **Hosting:** Bake off Netlify and Vercel as planned (D-18). **Strongly lean Netlify** because Vercel's Hobby tier explicitly prohibits commercial use, and although Vercel says "donations don't count as commercial," the policy is at Vercel's discretion to interpret — running a Patreon-linked site on Hobby is a real account-suspension risk. Netlify's free plan explicitly permits commercial use.
- **DNS:** Use an A record on the Netfirms control panel pointing the apex `andrewrahman.com` at the chosen platform's load balancer (Netlify: `75.2.60.5`; Vercel: provided in Vercel dashboard at deploy time). Add a CNAME for `www`.
- **Privacy policy:** Hand-author from a GDPR + CCPA + UK DPA template (Termly/Termsfeed boilerplate as starting structure, not as final text). Data-controller designation: **Spatial Media Lab** (matches CONTEXT.md D-16 default; flag for user confirmation in plan).
- **Patreon:** Launch with 4 monthly tiers as drafted in D-23, **omit annual option at launch** (with a "coming later" line if desired); use the standard 10% platform fee plan; vanity URL `patreon.com/andrewrahman` (claimed first-come-first-served — verify availability as plan task).

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Identity & Branding**
- **D-01:** Three distinct identities preserved consistently:
  - **Spatial Media Lab** (SML) — org owning `github.com/Spatial-Media-Lab/OpenSpatialDelay` and `SpatialMediaLab.org`
  - **Spatial Media Library** — pipeline/toolset brand funded via Patreon (three words)
  - **OpenSpatialDelay** — first product in the Spatial Media Library pipeline
  - **Andrew Rahman** — personal Patreon creator identity
- **D-02:** All Phase 2 artifacts reference SML as the source-code org and link to `github.com/Spatial-Media-Lab/OpenSpatialDelay`
- **D-03:** PROJECT.md must be updated to introduce "Spatial Media Library" pipeline brand alongside "Spatial Media Lab" org

**Gate Architecture**
- **D-04:** Email is the **gate** for the built installer (required, not optional). Source stays public on GitHub. Gate-by-friction accepted; community sharing of direct installer URLs is an accepted cost.
- **D-05:** Update ROADMAP.md Phase 2 success criterion 1 — "optional email" → "required email"
- **D-06:** Update REQUIREMENTS.md — remove "Download gate / mandatory email" from anti-features (contradicts D-04); revise DIST-01 wording

**Tally Form**
- **D-07:** Required email + single required GDPR-consent checkbox. No additional fields.
- **D-08:** Single required checkbox in plain English covering both storage and marketing use. Claude drafts wording.
- **D-09:** Post-submit = **thank-you page with manual download buttons** (macOS, Windows) + soft Patreon CTA. No auto-redirect.
- **D-10:** Email storage = Tally dashboard only for Phase 2. No newsletter integration. CSV export deferred to Phase 5.
- **D-11:** Claude drafts headline, microcopy, consent wording, thank-you page text in plan; user revises.

**Privacy Policy**
- **D-12:** Hosted at `andrewrahman.com/privacy` (stable permanent URL).
- **D-13:** Authored by Claude from scratch, tailored to actual Phase 2 data flow.
- **D-14:** Covers GDPR (EU) + CCPA (California) + UK DPA jurisdictions.
- **D-15:** Email retention = indefinite until user-initiated unsubscribe.
- **D-16:** Data-controller designation = **flag for planner**. Default recommendation: Spatial Media Lab.

**Site Shell & Hosting**
- **D-17:** Minimal Next.js static shell at `andrewrahman.com`: placeholder homepage + `/privacy` + shared `<Layout>` with nav/footer slots; footer attribution: "code lives at github.com/Spatial-Media-Lab"
- **D-18:** **Netlify vs Vercel bake-off** during Phase 2. Decide before pointing DNS. Vercel Hobby tier commercial-use prohibition is a known risk.
- **D-19:** User owns `andrewrahman.com` via Netfirms or Namecheap. Verify registrar first. Use **A or CNAME** record (not MX).
- **D-20:** Homepage placeholder = short bio + OSD mention (link to GitHub) + "more coming." Footer with `/privacy` link + attribution. Claude drafts.

**Patreon**
- **D-21:** Creator identity = **Andrew Rahman (personal)**.
- **D-22:** Pitch = "Funding the Spatial Media Library pipeline — OSD is the first; more are coming. Source lives at SML. Your support keeps these tools free and open-source."
- **D-23:** **4 tiers + annual-payment option**, baseline:
  - $3 Stargazer — newsletter, public thanks, funding ack
  - $10 Astronaut — Stargazer + feature-request voting + early builds
  - $25 Commander — Astronaut + 1:1 Discord/email + early access to future patron-exclusive plugins
  - $100 Mission Control — Commander + public thank-you credit in plugin About dialog
  - Annual ~15% discount
- **D-24:** Future Patreon-only plugins = soft-mentioned, **no specific timeframe**.
- **D-25:** Patreon page includes **soft Berlin one-liner** in sidebar/footer, low-urgency: "In Berlin? Come by Spatial Media Lab — we run events and collaborate with local artists. [link]"
- **D-26:** Patreon links: (a) `github.com/Spatial-Media-Lab/OpenSpatialDelay`, (b) `SpatialMediaLab.org`, (c) `andrewrahman.com`, (d) `andrewrahman.com/privacy`

**Patreon Seed Posts**
- **D-27:** **3 seed posts at launch** (exceeds DIST-03 minimum of 2). Required live before any public Patreon link is shared.
- **D-28:**
  - Post 1 — Welcome / why-Patreon / pipeline vision → **public**
  - Post 2 — OpenSpatialDelay technical deep-dive → **patron-only ($3+)**
  - Post 3 — Spatial Media Library roadmap → **public**
- **D-29:** Claude drafts all three; user revises for voice.

**Cross-Phase**
- **D-30:** Phase 5 expansion — influencer-outreach + community-forum drafts belong in Phase 5.

### Claude's Discretion
- Tally form visual design (within template constraints)
- Specific consent-checkbox wording (within plain-English + GDPR-valid constraints)
- Specific bio copy for placeholder homepage (Andrew edits for voice)
- Specific tier benefit descriptions (D-23 baseline + editorial)
- Specific seed-post prose (Andrew revises for voice)
- Privacy policy paragraph ordering and jurisdictional-clause phrasing (within GDPR + CCPA + UK DPA compliance)

### Deferred Ideas (OUT OF SCOPE)
- **Phase 3:** full personal-hub expansion (multi-product, blog, per-product pages)
- **Phase 5:** influencer-outreach email draft, community-forum post drafts (KVR, Gearspace, Reddit, HN), newsletter-tool integration (Tally → Mailchimp/Buttondown)
- **Future milestone:** actual Patreon-exclusive plugin release, personal-hub build-out, in-plugin Patreon CTA / About-dialog enhancements
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| **DIST-01** | Tally email capture form live — required email, GDPR consent checkbox, redirect/thank-you flow leading to OSD installer | Tally free plan supports unlimited submissions, redirect-on-completion (free), custom thank-you screens (free), GDPR-compliant consent checkbox (Tally documents required pattern: empty checkbox + explicit opt-in language). REQUIREMENTS.md DIST-01 wording must be updated per D-06 (remove "optional email"). |
| **DIST-02** | Privacy policy page published (GDPR-required) | Hosted at `andrewrahman.com/privacy` per D-12. Drafted from scratch per D-13. Covers GDPR+CCPA+UK DPA per D-14. Multiple authoritative templates available (Termly, GDPR.eu, ICO UK guidance) for structural reference. Site shell exists in Phase 2 to host this URL. |
| **DIST-03** | Patreon page published with 2+ posts and pipeline-funding framing | 3 posts planned (exceeds minimum) per D-27. Patreon free to launch (no minimum earnings required). Pipeline pitch and SML attribution per D-22, D-26. Annual billing flagged: NOT available until 3 months + $200/mo earnings reached — must be omitted at launch. |
</phase_requirements>

## Project Constraints (from CLAUDE.md)

CLAUDE.md is plugin-build focused (build_version.sh, no sudo, no AU cache refresh, version registry discipline). **None of it applies to Phase 2** — this phase touches no plugin source, no builds, no DAW caches. The relevant constraint is implicit: don't break Phase 1's released v1.0.0 artifact (Phase 2 only links to it; the GitHub Release page URL is `github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/latest`).

PROJECT.md does carry one relevant constraint: **Personal website hosting must be free** (under `## Constraints`). Both Netlify and Vercel offer free tiers; the bake-off must verify that the chosen platform's free tier is sufficient for the andrewrahman.com shell. Netlify's free tier (100 GB bandwidth, 300 build minutes) is far beyond what a static placeholder + privacy page will consume.

## Standard Stack

### Core (External Services)
| Service | Plan | Purpose | Why Standard |
|---------|------|---------|--------------|
| Tally.so | Free | Email-capture form, consent checkbox, thank-you screen | Free unlimited submissions; native GDPR-compliant consent pattern; built-in redirect; CSV export available [VERIFIED: tally.so/help/redirect-on-completion, tally.so/pricing] |
| Patreon | Standard 10% plan (default for new creators post-2025-08-04) | Funding pipeline, patron tiers, posts | Industry standard for indie creators funding ongoing work; aligns with D-21/D-22 pitch [VERIFIED: support.patreon.com/hc/en-us/articles/36426991446797] |
| Netlify | Free (Starter) | Hosting `andrewrahman.com` static site | Free tier explicitly permits commercial use; 100 GB bandwidth/mo; instant Git deploys [VERIFIED: netlify.com/pricing, answers.netlify.com] |

### Supporting (Code Stack — for the andrewrahman.com shell)
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| Next.js | 15.x | Static site shell (App Router + static export) | Per CONTEXT.md D-17; Phase 3 already plans Next.js — Phase 2 establishes the same stack [CITED: nextjs.org/docs/app/getting-started/deploying] |
| React | 19.x | UI primitives | Pinned to Next.js 15 default [VERIFIED via Next.js 15 release docs] |
| Tailwind CSS | 4.x | Styling for placeholder + privacy page | Lightweight, no design-system overhead for a 2-page shell [ASSUMED — Phase 3 may select something else; Phase 2 should pick a stack that doesn't lock Phase 3 in] |
| TypeScript | 5.x | Type safety | Standard for new Next.js apps; default in `create-next-app` [CITED: nextjs.org docs] |

**Version verification:** Before locking versions, run `npm view next version`, `npm view react version`, `npm view tailwindcss version`. Plan task should include this verification step rather than locking versions in research (Next.js ships frequently).

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Netlify | **Vercel** | Better Next.js DX (built by same company), but **Hobby tier prohibits commercial use** — Patreon-linked site is gray-area-to-clearly-commercial. Real account-suspension risk. Donations explicitly *don't* count as commercial per Vercel docs, BUT enforcement is at Vercel's discretion. CONTEXT.md D-18 mandates a bake-off — research recommends finishing the bake-off then picking Netlify unless Vercel adds clearer Patreon-friendly language. [VERIFIED: vercel.com/docs/limits/fair-use-guidelines] |
| Netlify | **Cloudflare Pages** | Also has a generous free tier with commercial use allowed; not in CONTEXT.md scope. Skip unless Netlify+Vercel both fail. |
| Tally | **ConvertKit / Buttondown / Mailchimp form** | Heavier, often paid for branded forms, and CONTEXT.md D-10 explicitly defers newsletter-tool integration to Phase 5. Stay on Tally. |
| Tally Pro custom domain ($29/mo) | **Embed Tally form in Next.js page** | Free, branded URL (`andrewrahman.com/get-osd`), maintains form rendering in Tally's iframe. Recommended approach. |
| Hand-authored privacy policy | **Termly / iubenda generator** | Generators are bloated, generic, and often inject vendor branding. CONTEXT.md D-13 explicitly chose hand-authored. Use generators only as structural reference. |

**Installation (for the andrewrahman.com Next.js shell):**
```bash
# In a NEW repo (recommended — see Architecture Patterns below)
npx create-next-app@latest andrewrahman-com --typescript --tailwind --app --no-src-dir
cd andrewrahman-com
# Verify versions:
npm view next version
npm view react version
npm view tailwindcss version
```

## Architecture Patterns

### Recommended Project Structure (for andrewrahman.com Next.js shell)

**This site lives in a NEW repository, not in the OpenSpatialDelay repo.** Rationale: the plugin repo is GPL-3.0 + JUCE-heavy + C++/CMake-focused; mixing a Next.js personal-site project into it would muddy build tooling, CI, and license clarity (the personal site doesn't need to be GPL). The new repo can be MIT or unlicensed (private).

```
andrewrahman-com/                    # NEW repo
├── app/
│   ├── layout.tsx                   # Shared <Layout>: nav slot + footer (D-17)
│   ├── page.tsx                     # Placeholder homepage (D-20)
│   ├── privacy/
│   │   └── page.tsx                 # Privacy policy (D-12, D-14)
│   └── get-osd/                     # Optional: embed Tally form here
│       └── page.tsx
├── components/
│   ├── Footer.tsx                   # Includes "code lives at github.com/Spatial-Media-Lab"
│   └── Nav.tsx                      # Empty/minimal slot Phase 3 fills
├── public/
├── next.config.ts                   # output: 'export' for static export
├── package.json
└── README.md
```

### Pattern 1: Static Export for Maximum Portability
**What:** Configure `next.config.ts` with `output: 'export'` to produce a fully static `out/` directory deployable to any static host.
**When to use:** When deploy target choice is still open (matches D-18 bake-off — same `out/` directory deploys identically to Netlify or Vercel).
**Example:**
```typescript
// next.config.ts
// Source: https://nextjs.org/docs/app/getting-started/deploying
import type { NextConfig } from 'next';
const nextConfig: NextConfig = {
  output: 'export',           // Static HTML/CSS/JS in out/
  trailingSlash: true,        // Better cross-host compatibility
  images: { unoptimized: true } // Static export disables Next.js image optimization
};
export default nextConfig;
```

### Pattern 2: Embed Tally Form (Free Custom-Domain Workaround)
**What:** Render the Tally form inside a Next.js page on andrewrahman.com. The form lives at `andrewrahman.com/get-osd` (branded URL), but the actual form HTML is loaded from `tally.so` via an embed script.
**When to use:** When custom-domain branding matters but the $29/mo Tally Pro plan is overkill (CONTEXT.md keeps Phase 2 free per PROJECT.md constraints).
**Example:**
```html
<!-- Tally embed code from form's Share → Embed → "Embed on website" tab -->
<!-- Source: https://tally.so/help/embed-your-form -->
<iframe
  data-tally-src="https://tally.so/embed/{FORM_ID}?alignLeft=1&hideTitle=1&transparentBackground=1&dynamicHeight=1"
  loading="lazy"
  width="100%"
  height="500"
  frameborder="0"
  marginheight="0"
  marginwidth="0"
  title="Get OpenSpatialDelay">
</iframe>
<script src="https://tally.so/widgets/embed.js"></script>
```

### Pattern 3: Tally Thank-You Page with Manual Buttons (D-09)
**What:** Configure Tally's "Thank you page" feature (free) with the macOS + Windows download buttons inline, plus a Patreon CTA at the bottom.
**When to use:** Always — D-09 mandates this over auto-redirect.
**How:** In Tally form editor → Settings → "Thank you page" → enable, then add buttons via the rich-text editor. Tally allows multiple buttons with custom URLs. Direct download buttons should link to the v1.0.0 release asset URLs (e.g. `github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/download/v1.0.0/OpenSpatialDelay-macOS.pkg`). [VERIFIED: tally.so/help/how-to-create-a-thank-you-page]

### Pattern 4: Apex Domain DNS for Static Hosts
**What:** `andrewrahman.com` (apex) cannot use a CNAME; must be A or ALIAS/ANAME. Netfirms control panel may not support ALIAS — verify, fall back to A record.
**When to use:** Always for the apex; CNAME for `www` subdomain.
**Example records (for Netlify):**
```
Type   Host   Value
A      @      75.2.60.5                      # Netlify load balancer
CNAME  www    [your-site].netlify.app        # Netlify-assigned subdomain
```
[VERIFIED: docs.netlify.com/manage/domains/configure-domains/configure-external-dns/]

### Anti-Patterns to Avoid

- **Don't put the Next.js shell in the OSD plugin repo.** Mixing Node/TypeScript build tooling into the JUCE/CMake repo creates CI complexity and license-clarity problems. Use a separate repo.
- **Don't use Vercel Hobby for a Patreon-linked site.** Even though Vercel says donations don't count as commercial, the Hobby ToS explicitly carves out "personal/non-commercial use" and reserves enforcement discretion. A Patreon link prominently displayed is legible enough as commercial that an automated review could flag it. [VERIFIED: vercel.com/legal/terms, vercel.com/docs/plans/hobby]
- **Don't use a generator (Termly/iubenda) and publish unreviewed.** D-13 explicitly chose hand-authored. Generators produce policies that describe data flows the project doesn't have (cookies, analytics, third-party trackers) and omit the actual flow (Tally → Tally dashboard → manual newsletter use later).
- **Don't enable Patreon's annual-billing option in the launch UI.** It's not eligible until 3 months + $200/mo. Trying to enable it returns an error and signals a misconfigured page.
- **Don't promise specific Patreon-exclusive plugin timelines.** D-24 is explicit: soft-mention only.
- **Don't ship the privacy policy without confirming the data-controller line.** D-16 flagged this for explicit user confirmation; planner must surface in PLAN.md.
- **Don't link to `releases/latest` if a specific version (v1.0.0) is the intended target.** `releases/latest` is GitHub's auto-resolver, fine for a "current version" link, but if the Tally thank-you page should always point to v1.0.0 specifically (immutable), use the versioned URL. CONTEXT.md says current target is `releases/latest` — confirm in plan.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Email capture form | Custom HTML form + backend endpoint | Tally (free, unlimited) | Spam protection, GDPR audit trail, CSV export, thank-you page logic all built in |
| GDPR consent storage | Database column for consent timestamps | Tally's submission record (timestamp + consent value stored automatically) | Tally records every submission with timestamp + each field including the consent checkbox value [VERIFIED: tally.so/help/how-to-create-a-gdpr-compliant-form] |
| Privacy policy from blank page | Pure-creative draft with no reference | Start from gdpr.eu/privacy-notice/ structural template + ICO UK guidance, hand-write the actual content | Templates ensure no required clause is missed (lawful basis, retention, rights enumeration); D-13 still requires the actual prose to be hand-written for the actual data flow |
| Patreon membership/payment infrastructure | Custom Stripe integration + tier logic | Patreon (10% fee, fully managed) | Tax handling, dispute handling, trial periods, member dashboard, post-visibility logic all managed |
| Static site host | Custom server / S3 bucket / nginx | Netlify (free, Git-integrated) | Free SSL, atomic deploys, instant rollback, CDN, deploy previews on PRs |
| HTTPS / SSL cert | Manual certbot | Netlify (auto-provisioned via Let's Encrypt on custom domain attach) | Auto-renewal, no maintenance |
| DNS records UI | New DNS provider | Stay on Netfirms (already authoritative) | Existing nameservers; just edit A/CNAME records. Migrating nameservers introduces propagation risk for no benefit. |
| Custom domain on Tally | Pay $29/mo for Tally Pro custom domains | Embed Tally form in Next.js page on andrewrahman.com | Same branded UX, $0 cost, keeps Tally on free tier |

**Key insight:** This phase is almost entirely "configure standard SaaS." The temptation to over-engineer (custom backend, self-hosted form, custom payment) should be resisted — every external service named here exists precisely because the work is non-trivial when self-built. The only thing being authored from scratch is *content* (form copy, privacy policy text, Patreon page text, seed posts) and the minimal Next.js shell.

## Runtime State Inventory

> Phase 2 is **not** a rename/refactor/migration phase — it adds new external infrastructure. However, the Spatial Media Library brand introduction (D-03) creates light state-update concerns. Most categories are N/A.

| Category | Items Found | Action Required |
|----------|-------------|------------------|
| Stored data | None — Phase 2 does not modify any datastore inside the OSD repo | None |
| Live service config | (a) New Patreon page setup (creator URL, tiers, posts) — fresh creation, no existing config to migrate. (b) Tally form — fresh creation. (c) Netfirms DNS — A record currently `66.96.149.1` (Netfirms parking); will be changed to point at chosen platform. Old value should be noted in plan task in case rollback needed. | DNS change is the sensitive one — capture original record values before edit |
| OS-registered state | None — no OS-level scheduled tasks, services, or daemons involved | None |
| Secrets / env vars | (a) Patreon creator account credentials (user-held, not in repo). (b) Netfirms DNS account credentials (user-held). (c) Tally account credentials (user-held). (d) Netlify and/or Vercel account credentials (user-held). **None of these belong in the OSD repo** — Phase 2 introduces zero new secrets to the codebase. | Plan task: confirm user has access to all four account credentials before execution |
| Build artifacts / installed packages | The new andrewrahman-com repo (separate from OSD repo) will produce a Next.js `out/` static export at deploy time — managed by Netlify build, not local artifact. | None for OSD repo |

**Plus three repo-doc updates** (per D-03, D-05, D-06):
- `PROJECT.md` — introduce Spatial Media Library brand alongside SML
- `ROADMAP.md` Phase 2 success criterion 1 — "optional email" → "required email"
- `REQUIREMENTS.md` — remove "Download gate / mandatory email" from anti-features; revise DIST-01 wording

These are pure markdown edits — no runtime state to update.

## Common Pitfalls

### Pitfall 1: Vercel Hobby Suspension on Patreon-Linked Site
**What goes wrong:** Site deployed to Vercel Hobby; Vercel's automated review (or manual review triggered by report) flags the Patreon link as commercial use; account is restricted; site goes down.
**Why it happens:** Vercel ToS: "Hobby teams are restricted to non-commercial personal use only." Vercel docs say donations don't count, but the policy phrasing leaves enforcement discretion to Vercel. A site whose primary call-to-action is "support me on Patreon" reads as monetized.
**How to avoid:** Use Netlify (free tier explicitly permits commercial use). If running the bake-off, deploy to Vercel only as a *throwaway test* (no DNS pointed); never make Vercel the production host for andrewrahman.com.
**Warning signs:** Vercel email re: "Fair Use Guidelines"; sudden site 503/account-restricted page.
[VERIFIED: vercel.com/legal/terms; community.vercel.com/t/fair-use-of-the-hobby-plan/2725]

### Pitfall 2: Apex Domain CNAME Failure on Netfirms
**What goes wrong:** Trying to point `andrewrahman.com` (apex/root) at a CNAME record. DNS spec doesn't allow CNAME at the apex when other records exist (esp. SOA/NS). Netfirms may silently reject or partially apply.
**Why it happens:** CNAME at apex is a perpetual confusion point. Many cloud hosts (Netlify, Vercel) document apex setup with ALIAS/ANAME — a non-standard extension Netfirms may not support.
**How to avoid:** Use an **A record** at the apex pointing at the host's load-balancer IP (Netlify: `75.2.60.5`). Use a CNAME only for `www`. Verify with `dig andrewrahman.com A` after change and wait up to 24h for global propagation.
**Warning signs:** `dig andrewrahman.com A` returns the old `66.96.149.1` after 1+ hours; browser hits Netfirms parking page.
[VERIFIED: docs.netlify.com/manage/domains/configure-domains/dns-records/]

### Pitfall 3: Patreon Page Not Launched, So Posts Aren't Visible
**What goes wrong:** Posts created but page never explicitly clicks "Launch." Posts exist but `patreon.com/{slug}` returns "page not yet launched" to the public. DIST-03 success criterion fails despite all the work being done.
**Why it happens:** Patreon's two-step flow: build the page (private editing), then click Launch (publishes publicly). Easy to skip.
**How to avoid:** Plan task explicitly named "Click Launch button on Patreon page after seed posts are uploaded and reviewed." Verify by opening `patreon.com/{slug}` in a logged-out browser.
**Warning signs:** Page renders for the creator but a private/incognito tab shows a "not found" or "coming soon" message.
[VERIFIED: support.patreon.com/hc/en-us/articles/115002958403-Launch-Checklist]

### Pitfall 4: Annual Membership Toggle Won't Enable
**What goes wrong:** Following D-23 verbatim, planner adds a task to enable annual billing; Patreon UI rejects with error: "Your page must be live 3 months and earning $200+/mo."
**Why it happens:** Patreon's annual-billing eligibility is a brand-new-creator restriction.
**How to avoid:** Skip annual at launch. Add a deferred-task to revisit annual billing in ~3-6 months. Optionally: add a sentence to Patreon page text saying "annual billing coming later."
**Warning signs:** Patreon settings page grays out the annual option and shows the eligibility text.
[VERIFIED: support.patreon.com/hc/en-us/articles/360041721372-Annual-memberships-creator-overview]

### Pitfall 5: GDPR Consent Checkbox Pre-Checked
**What goes wrong:** The Tally consent checkbox is configured as pre-checked (default value "true"). Under GDPR Recital 32, consent must be "freely given, specific, informed and unambiguous" — pre-ticked boxes are explicitly not valid consent. Form passes Tally's UI but fails any GDPR audit.
**Why it happens:** Tally's consent-checkbox documentation explicitly warns about this; easy to miss when configuring.
**How to avoid:** Verify checkbox default is **empty/unchecked** before publishing form. Plan task: add manual UAT step "Open form in incognito tab; verify consent checkbox is unchecked by default."
**Warning signs:** Form submits without user clicking the checkbox.
[VERIFIED: tally.so/help/how-to-create-a-gdpr-compliant-form]

### Pitfall 6: Privacy Policy Cites Tools Not Actually In Use
**What goes wrong:** Using a generator template, the policy mentions cookies, analytics, third-party advertising trackers, etc. that the actual andrewrahman.com site doesn't use. This both exposes the project legally (claims about data flows that don't exist) and looks sloppy.
**Why it happens:** Generators output kitchen-sink policies covering hypothetical use.
**How to avoid:** Hand-author per D-13. Reference template structure (sections, required clauses) but write each clause about the actual data flow: "We collect emails via Tally → stored in Tally dashboard → used to email occasional updates about OSD and other Spatial Media Library tools → retained until you unsubscribe."
**Warning signs:** Policy mentions Google Analytics, cookies, advertising IDs, or third-party social-media trackers.

### Pitfall 7: Tally Form Embed Breaks Layout on Mobile
**What goes wrong:** Iframe embed of the Tally form on andrewrahman.com renders fine on desktop but is cut off / scrolls awkwardly on mobile.
**Why it happens:** Iframes have fixed height; the form's height varies as conditional logic shows/hides fields.
**How to avoid:** Use Tally's `dynamicHeight=1` URL parameter (in the embed snippet) — this triggers the embed script to resize the iframe based on form content height. Test on actual mobile devices, not just devtools.
**Warning signs:** Submit button cut off on mobile; horizontal scroll inside iframe.
[VERIFIED: tally.so/help/embed-your-form, conversiontracking.io/blog/tally-forms-conversion-tracking-7-methods-google-tag-manager]

### Pitfall 8: Brand Confusion — Spatial Media Lab vs Spatial Media Library
**What goes wrong:** Patreon page text, privacy policy, and seed posts use "Spatial Media Library" and "Spatial Media Lab" interchangeably. Readers are confused about whether they're the same entity.
**Why it happens:** Names are nearly identical (one word difference). D-01 introduces this as a deliberate brand choice but the burden is on every artifact to disambiguate.
**How to avoid:** First reference in any document explicitly defines: "Spatial Media Lab (the open-source organization that owns the source code)" and "Spatial Media Library (the pipeline of spatial-audio plugins, of which OpenSpatialDelay is the first)." Use abbreviation "SML" only for Spatial Media Lab. Plan task: cross-reference review of all Phase 2 artifacts for consistent usage.
**Warning signs:** A reader has to re-read a paragraph to figure out which one is which.

## Code Examples

Verified patterns from official sources and standard practice.

### Tally Embed Snippet (free plan, custom URL via Next.js page)
```tsx
// app/get-osd/page.tsx
// Source: https://tally.so/help/embed-your-form
'use client';
import { useEffect } from 'react';

export default function GetOSDPage() {
  useEffect(() => {
    const script = document.createElement('script');
    script.src = 'https://tally.so/widgets/embed.js';
    script.async = true;
    document.body.appendChild(script);
    return () => { document.body.removeChild(script); };
  }, []);

  return (
    <main>
      <h1>Get OpenSpatialDelay</h1>
      <p>Drop your email to get the v1.0.0 installer for macOS and Windows.</p>
      <iframe
        data-tally-src="https://tally.so/embed/{FORM_ID}?alignLeft=1&hideTitle=1&transparentBackground=1&dynamicHeight=1"
        loading="lazy"
        width="100%"
        height="500"
        title="Get OpenSpatialDelay"
        style={{ border: 'none' }}
      />
    </main>
  );
}
```

### Next.js 15 Static Export Config
```typescript
// next.config.ts
// Source: https://nextjs.org/docs/app/getting-started/deploying
import type { NextConfig } from 'next';
const nextConfig: NextConfig = {
  output: 'export',
  trailingSlash: true,
  images: { unoptimized: true }
};
export default nextConfig;
```

### Privacy Policy Page Skeleton (App Router)
```tsx
// app/privacy/page.tsx
export const metadata = {
  title: 'Privacy Policy — andrewrahman.com',
  description: 'How we collect, store, and use email addresses.'
};
export default function PrivacyPage() {
  return (
    <article className="prose mx-auto max-w-3xl p-6">
      <h1>Privacy Policy</h1>
      <p><em>Last updated: 2026-04-XX</em></p>

      <h2>Who we are</h2>
      <p>This site is operated by <strong>Spatial Media Lab</strong>...</p>

      <h2>What data we collect</h2>
      <p>When you submit the email-capture form...</p>

      {/* Sections required by GDPR/CCPA/UK DPA — see Privacy Policy Required Sections below */}
    </article>
  );
}
```

### Netlify DNS — Apex A Record (set in Netfirms control panel)
```
# Netfirms DNS UI:
Type:  A
Host:  @           (or leave blank — both mean apex)
Value: 75.2.60.5   (Netlify load balancer IPv4)
TTL:   3600

Type:  CNAME
Host:  www
Value: {your-site-name}.netlify.app
TTL:   3600
```
[VERIFIED: docs.netlify.com/manage/domains/configure-domains/configure-external-dns/]

### Tally Consent Checkbox Configuration
```
# In Tally form editor:
1. Add a "Checkboxes" field type
2. Single option labeled (D-08 baseline):
   "I agree that my email will be stored and used to send occasional updates
    about OpenSpatialDelay and other Spatial Media Library tools.
    [Privacy policy](https://andrewrahman.com/privacy)"
3. Mark field as REQUIRED
4. Verify default state is UNCHECKED (no preselection)
```

## Privacy Policy Required Sections (GDPR + CCPA + UK DPA)

Hand-author each section. Template-references at the end.

| Section | Required by | Phase 2 actual content |
|---------|-------------|------------------------|
| Identity of data controller | GDPR Art. 13(1)(a), UK DPA | "Spatial Media Lab, an open-source organization. Contact: [email]." Per D-16, planner to confirm. |
| Contact details for data protection inquiries | GDPR Art. 13(1)(b), UK DPA | Email address (org email or Andrew's). |
| Categories of personal data collected | GDPR Art. 13(1)(c-d), CCPA §1798.100 | "Email address only." |
| Purpose and legal basis for processing | GDPR Art. 13(1)(c), Art. 6 | Legal basis: **Consent** (Art. 6(1)(a)). Purpose: occasional product updates about OpenSpatialDelay and other Spatial Media Library tools. |
| Recipients (third parties) | GDPR Art. 13(1)(e), CCPA "sale/share" | "We use Tally.so as our form/email-storage processor. We do not sell, share, or transfer your email to any other third party." |
| International data transfers | GDPR Art. 13(1)(f) | Tally is EU-based (Belgium); if hosted elsewhere, note SCCs. [Verify Tally hosting region — `tally.so/help/gdpr` documents this] |
| Retention period | GDPR Art. 13(2)(a) | "Indefinite until you unsubscribe by replying to any update email or contacting us." (D-15) |
| Data subject rights | GDPR Art. 13(2)(b), Art. 15-22 | Access, rectification, erasure, restriction, objection, portability. Provide email contact for requests. |
| Right to withdraw consent | GDPR Art. 13(2)(c) | Statement that consent can be withdrawn at any time. |
| Right to lodge a complaint with supervisory authority | GDPR Art. 13(2)(d) | EU: data protection authority of user's country. UK: ICO. |
| CCPA-specific: "Do Not Sell" | CCPA §1798.135 | "We do not sell or share personal information." (Likely accurate for this project.) |
| CCPA-specific: California consumer rights | CCPA §1798.100-130 | Right to know, right to delete, right to non-discrimination. |
| UK DPA additions | UK DPA 2018 | UK DPA largely mirrors UK-GDPR (post-Brexit retained EU law); a single GDPR-compliant policy with ICO referenced as supervisory authority is sufficient. |
| Cookies / analytics | Various | "This site does not use cookies or analytics." (Verify — Phase 2 shell should genuinely have no analytics; if Vercel Web Analytics or Netlify Analytics is enabled later, this section needs update.) |
| Last-updated date | Best practice | Plain ISO date. |

**Templates to reference (structure, NOT prose):**
- gdpr.eu/privacy-notice/ — official EU template
- ICO UK guidance: ico.org.uk/for-organisations/uk-gdpr-guidance-and-resources/
- termly.io/resources/templates/privacy-policy-template/ — multi-jurisdiction example structure

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Patreon Pro/Premium two-tier creator plans (8% / 12%) | Single Standard plan (10%) for new creators | 2025-08-04 | Simpler — no plan-selection decision needed at signup. New creators automatically pay 10%. |
| Vercel Hobby tier ambiguous on commercial use | Explicit: "Hobby is non-commercial only; donations don't count as commercial; advertising and payment processing do" | Ongoing (current ToS) | Patreon-linked sites are gray area; Netlify is the unambiguous choice. |
| Tally custom domains via Pro plan | Same — still $29/mo for Pro | Stable | Embed in your own Next.js page on free Tally plan to get branded URLs without paying. |
| Next.js Pages Router | App Router (Next.js 13+, fully default in 14+, mature in 15) | 2023-2024 | Use App Router for new projects (matches `create-next-app` default). Static export still supported via `output: 'export'`. |
| Manual SSL via certbot | Auto-provisioned by host (Netlify, Vercel, Cloudflare) on custom-domain attach | Standard since 2020 | No SSL tooling needed in Phase 2. |

**Deprecated / outdated in this domain:**
- **Tally hosted-form-only workflow** (using only `tally.so/r/{form-id}` URLs) — works but ugly; embedding is the default modern pattern.
- **Patreon's "per-creation" billing** — still supported but most new creators use monthly-subscription billing. CONTEXT.md D-23 implies monthly subscription (the right choice).
- **MX-only DNS for web hosting** — confusing the user originally; explicitly corrected in CONTEXT.md D-19.

## Validation Architecture

Phase 2 is **predominantly external-service configuration plus a 2-page static site**. Standard automated test frameworks don't apply to most of this work. Validation leans heavily on manual UAT (User Acceptance Testing) with explicit click-through scripts.

### Test Framework
| Property | Value |
|----------|-------|
| Framework | **Manual UAT scripts + Playwright** for the Next.js shell smoke tests (optional) |
| Config file | `playwright.config.ts` in andrewrahman-com repo (if Playwright adopted) |
| Quick run command | `npx playwright test --grep @smoke` (if adopted) |
| Full suite command | Manual UAT checklist execution |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|--------------|
| DIST-01 | Tally form is publicly reachable | manual UAT | Open `andrewrahman.com/get-osd` (or Tally direct URL) in incognito | ❌ N/A — manual |
| DIST-01 | GDPR consent checkbox is required and unchecked by default | manual UAT | Open form, attempt submit without checking → expect blocked | ❌ N/A — manual |
| DIST-01 | Submitting form with email + consent reaches thank-you page with macOS + Windows download buttons | manual UAT | Submit form with throwaway email; observe thank-you page renders both buttons; click each → confirms it links to a v1.0.0 release asset URL | ❌ N/A — manual |
| DIST-01 | Email lands in Tally dashboard | manual UAT | After submit, open Tally → form → Submissions; confirm new row exists with email and consent=true | ❌ N/A — manual |
| DIST-02 | Privacy policy is publicly accessible at `andrewrahman.com/privacy` | smoke (Playwright) or manual | `curl -sf https://andrewrahman.com/privacy \| grep -i "privacy policy"` | ❌ Wave 0 (if Playwright adopted) |
| DIST-02 | Privacy policy is linked from Tally form's consent checkbox | manual UAT | In form, locate checkbox label; click "Privacy policy" link; expect navigation to `andrewrahman.com/privacy` | ❌ N/A — manual |
| DIST-03 | Patreon page is publicly accessible (launched, not just drafted) | manual UAT | Open `patreon.com/andrewrahman` (or chosen slug) in incognito; expect creator page renders | ❌ N/A — manual |
| DIST-03 | Patreon page has 3 published posts (D-27) | manual UAT | On public page, count visible posts (1 public welcome + 1 patron-only deep-dive + 1 public roadmap = 3) | ❌ N/A — manual |
| DIST-03 | Patreon page text mentions "Spatial Media Library" pipeline framing | manual UAT | Read About section; verify pipeline pitch language is present | ❌ N/A — manual |
| DIST-03 | Patreon page links to `github.com/Spatial-Media-Lab/OpenSpatialDelay`, `SpatialMediaLab.org`, `andrewrahman.com`, `andrewrahman.com/privacy` | manual UAT | Click each link; confirm correct destination | ❌ N/A — manual |
| DIST-03 | Berlin one-liner present in sidebar/footer (D-25) | manual UAT | Visual inspection | ❌ N/A — manual |
| Site shell | andrewrahman.com loads on apex domain over HTTPS | smoke | `curl -sIf https://andrewrahman.com \| grep "200 OK"` and verify cert via `openssl s_client -connect andrewrahman.com:443 -servername andrewrahman.com </dev/null 2>/dev/null \| grep "Verify return"` | N/A — manual or trivial shell script |
| Site shell | `www.andrewrahman.com` redirects to apex (or vice versa) | smoke | `curl -sI https://www.andrewrahman.com \| grep -i location` | N/A |
| Site shell | Footer attribution "code lives at github.com/Spatial-Media-Lab" present | smoke | `curl -sf https://andrewrahman.com \| grep "Spatial-Media-Lab"` | N/A |
| Doc updates | ROADMAP.md success criterion 1 says "required email" | grep | `grep "required email" .planning/ROADMAP.md` | ❌ Wave 0 |
| Doc updates | REQUIREMENTS.md anti-features no longer mentions "mandatory email" | grep | `! grep -i "mandatory email" .planning/REQUIREMENTS.md` | ❌ Wave 0 |
| Doc updates | PROJECT.md introduces "Spatial Media Library" | grep | `grep "Spatial Media Library" .planning/PROJECT.md` | ❌ Wave 0 |

### Sampling Rate
- **Per task commit:** Lightweight grep / curl checks for the affected artifact (e.g., editing the privacy page → `curl` it after deploy)
- **Per wave merge:** Run the full manual UAT checklist for any external-service changes in that wave
- **Phase gate:** All 12+ manual UAT items above pass; doc-update grep checks pass

### Wave 0 Gaps
- [ ] Manual UAT checklist document — `manual-uat-phase2.md` enumerating each click-through with pass/fail boxes
- [ ] Optional: Playwright smoke test for andrewrahman.com (1-2 tests covering homepage + /privacy load); set up `playwright.config.ts` and `tests/smoke.spec.ts` in the new andrewrahman-com repo
- [ ] grep check scripts for the 3 doc updates (could be a single `verify-doc-updates.sh` shell script)
- [ ] Throwaway email account designated for form-submission UAT (so test submissions don't pollute production list — or: clear test rows from Tally after UAT)

## Security Domain

Phase 2 collects user emails — V5 (Input Validation) and minor V8 (Data Protection) apply. Most security concerns are off-loaded to Tally/Patreon/Netlify (their responsibility). Phase 2's scope is: don't introduce new attack surface; verify the third parties handle data correctly.

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | No user accounts on andrewrahman.com; auth is on Tally, Patreon, Netlify dashboards (their problem) |
| V3 Session Management | no | No sessions on andrewrahman.com (static site) |
| V4 Access Control | no | All Phase 2 content is publicly readable by design |
| V5 Input Validation | yes | Tally validates email format and required-field constraints (delegated). Privacy page has no inputs. |
| V6 Cryptography | partial | TLS via Netlify auto-provisioned Let's Encrypt cert (delegated). No application-level crypto. |
| V8 Data Protection | yes | GDPR consent record kept in Tally; retention per D-15; user can request deletion (privacy policy enumerates this) |
| V14 Configuration | yes | DNS apex → only Netlify (no dangling CNAME); HTTPS-only (HSTS via Netlify defaults); no exposed secrets in Next.js client bundle (none used) |

### Known Threat Patterns for {static site + hosted form}

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Form spam (bots filling Tally) | Tampering / DoS | Tally has built-in spam protection; optional CAPTCHA can be enabled if abuse becomes problem [VERIFIED: tally.so/features] |
| Phishing via lookalike domain | Spoofing | Out of scope for Phase 2 (no defensive register); flag as future-monitoring |
| Plaintext email exposure on public site | Information Disclosure | No mailto: links to personal email on public page; route inquiries through privacy@... or a contact form (Phase 3) |
| Subdomain takeover | Spoofing | Don't create CNAME records pointing to abandoned external services; `www` CNAME points to a Netlify subdomain that's owned by the deployment |
| Lack of HTTPS | Information Disclosure | Netlify enforces HTTPS by default with auto-renewing cert; HSTS on by default on `https://...`. |
| GDPR consent non-compliance | Repudiation / regulatory | Single empty checkbox + privacy-policy link in label + record in Tally submission row [VERIFIED] |
| Stale privacy policy contradicting actual data flow | Repudiation / regulatory | Keep policy in sync with what site actually does; "Last updated" date visible |
| Patreon account compromise | Spoofing / Tampering | Enable Patreon 2FA (creator-side responsibility, not Phase 2 code task — flag in plan) |
| Netfirms DNS account compromise | Tampering | Enable Netfirms 2FA if available; note in plan as recommended user-side action |

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|-------------|-----------|---------|----------|
| node | Next.js shell build | ✓ | v25.9.0 | — |
| npm | Package install | ✓ | 11.12.1 | — |
| git | Repo creation/commits | ✓ | 2.50.1 | — |
| dig | DNS verification | ✓ | (system) | — |
| whois | Registrar verification | ✓ | (system) | — |
| Tally account | Form creation | UNKNOWN | — | User signs up (free) — first-execution task |
| Patreon account | Patreon page | UNKNOWN | — | User signs up (free) — first-execution task |
| Netlify account | Site hosting | UNKNOWN | — | User signs up (free); GitHub auth |
| Vercel account | Bake-off comparison | UNKNOWN | — | User signs up (free); GitHub auth (only used for bake-off, not production) |
| Netfirms account access | DNS edits on andrewrahman.com | UNKNOWN | — | User must locate credentials before DNS-change task can execute |

**Missing dependencies with no fallback:**
- **Netfirms account access** — DNS cannot be changed without it. The very first plan task should be "user verifies Netfirms login works"; if not, recover credentials before any other DNS work.

**Missing dependencies with fallback:**
- All four SaaS accounts (Tally, Patreon, Netlify, Vercel) have free signup; just plan a "create account" task at the start of each relevant work stream.

**Verified facts (this audit):**
- `whois andrewrahman.com` → registrar = **Tucows Domains Inc.** (IANA ID 69)
- `dig andrewrahman.com NS +short` → `ns1.netfirms.com.` and `ns2.netfirms.com.` (DNS authority is Netfirms despite Tucows being the registrar)
- `dig andrewrahman.com A +short` → `66.96.149.1` (Netfirms parking/hosting IP)

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Tailwind CSS 4.x is the right styling choice for the andrewrahman.com shell | Standard Stack — Supporting | Low — Phase 3 may select a different design system; the choice in Phase 2 is just for the placeholder. Tailwind is replaceable. |
| A2 | The Patreon vanity slug `patreon.com/andrewrahman` is available | Primary recommendation | Medium — if taken, user picks alternate slug (e.g., `patreon.com/andrew-rahman` or `patreon.com/spatialmedialibrary`). Add a "check vanity availability" plan task. |
| A3 | The current Tally form will use `releases/latest` rather than a versioned `v1.0.0` URL on the thank-you buttons | Pattern 3 | Low — versioned URL is more stable but requires updating Tally on every release. CONTEXT.md doesn't prescribe; planner should confirm with user. |
| A4 | Tally is hosted in the EU (Belgium) so no SCC clauses needed in privacy policy | Privacy Policy Required Sections | Medium — if Tally's actual hosting region differs, "international data transfer" section needs SCC language. **Plan task: verify via tally.so/help/gdpr** before publishing privacy policy. |
| A5 | Netfirms DNS control panel supports A and CNAME records (standard) | DNS section | Low — universally supported; if their UI is unusual, a 5-min support ticket clarifies. |
| A6 | The OSD repo's GitHub org name is exactly `Spatial-Media-Lab` (case + hyphenation) | All link references | Medium — must verify exact slug at first plan task. CONTEXT.md uses `github.com/Spatial-Media-Lab/OpenSpatialDelay`; STATE.md/PROJECT.md confirm SML branding but don't show the exact slug. **Verify by visiting the URL or `gh repo view spatialmedialab/openspatialdelay`.** |
| A7 | Spatial Media Lab as data controller is acceptable (vs. Andrew personally) | Privacy Policy / D-16 | High — legal designation. Per D-16 must be flagged for explicit user confirmation in PLAN.md. Not a Phase 2 default — it's a question awaiting user answer. |
| A8 | The Vercel-vs-Netlify decision will resolve to Netlify | Primary recommendation | Low impact even if wrong — both work technically; the policy risk is what tilts it. If user accepts the Vercel-Hobby risk knowingly, Vercel works. |
| A9 | A "Get OSD" page on andrewrahman.com is wanted (vs. linking directly to a tally.so URL from elsewhere) | Pattern 2 | Low — the embed-on-own-site pattern is just one option; a direct `tally.so/r/...` URL also works and is simpler. CONTEXT.md doesn't prescribe; planner picks based on URL aesthetics priority. |
| A10 | The privacy policy "contact email" can be a simple address like `privacy@spatialmedialab.org` or `andrew@andrewrahman.com` | Privacy Policy section | Medium — needs an email that exists and is monitored for GDPR data-subject requests. Plan task: confirm which email and whether it exists. |

**If user confirms or corrects each above, all become locked decisions for execution.**

## Open Questions (RESOLVED)

1. **Which email address handles GDPR data-subject requests?**
   - What we know: Privacy policy must list a contact for data subject requests (access, deletion, etc.) per GDPR Art. 13(1)(b).
   - What's unclear: Does `privacy@spatialmedialab.org` exist? Does Andrew want this routed to a personal email or an org email?
   - Recommendation: Plan task — user names the email and confirms it's monitored.
   - **RESOLVED:** Plan 03 Task 2 hardcodes `andrewjrahman@gmail.com` (PROJECT.md Brand Identity) as the GDPR rights-contact in the privacy policy.

2. **Data controller designation: Spatial Media Lab vs Andrew Rahman personally?**
   - What we know: D-16 explicitly flags this; default suggestion is SML.
   - What's unclear: SML's legal status — is it an incorporated entity in some jurisdiction? Or an unincorporated open-source organization? This affects whether SML can legally serve as "controller."
   - Recommendation: Surface explicitly in PLAN.md; user confirms before privacy page goes live.
   - **RESOLVED:** Plan 03 Task 1 is an explicit user decision checkpoint (SML / Andrew / Joint) before privacy prose is written; Task 2 then hardcodes the chosen entity, replacing the `<CONTROLLER>` placeholder.

3. **Tally form URL on andrewrahman.com vs. raw tally.so URL?**
   - What we know: D-12 and D-17 establish andrewrahman.com as the hub. CONTEXT.md doesn't explicitly prescribe whether the Tally form lives at `andrewrahman.com/get-osd` (embed) or just at `tally.so/r/{form-id}` linked from elsewhere.
   - What's unclear: User aesthetic preference + how form is shared in Phase 5 launch communications.
   - Recommendation: Plan an embed page (free, clean URL); the raw Tally URL still works as a fallback.
   - **RESOLVED:** Plan 02 Task 4 + Plan 05 embed the form at `andrewrahman.com/get-osd` via the Tally React embed; users never see the raw `tally.so` URL.

4. **Patreon vanity slug — `patreon.com/andrewrahman` available?**
   - What we know: First-come-first-served per Patreon vanity policy.
   - What's unclear: Whether someone else already claimed it.
   - Recommendation: First Patreon plan task = check availability and claim it (or pick alternate).
   - **RESOLVED:** Plan 06 Task 3 Step B verifies `patreon.com/andrewrahman` availability with documented fallback slugs (`andrew-rahman`, `andrewjrahman`).

5. **Tally hosting region for SCC clauses in privacy policy?**
   - What we know: GDPR requires disclosure of international transfers and SCCs if data leaves the EEA.
   - What's unclear: Which region Tally's data actually sits in.
   - Recommendation: Plan task — read `tally.so/help/gdpr` and `tally.so/help/data-processing-agreement`; copy the relevant region/SCC language into the privacy policy if needed.
   - **RESOLVED:** Plan 05 Task 1 Step F sets the Tally hosting region to EU; Plan 03 Task 2 Section 4 names Tally Technologies SRL (Belgium) as the processor, eliminating the SCC requirement.

6. **Annual billing on Patreon — defer or mention?**
   - What we know: Not eligible at launch (3 months + $200/mo minimum).
   - What's unclear: User preference — silently omit, or proactively say "annual coming later"?
   - Recommendation: Silently omit; revisit when eligible.
   - **RESOLVED:** Plan 06 Draft 1 soft-mention ("Annual membership at ~15% discount is planned — coming once Patreon eligibility is met") + Plan 06 SUMMARY output block will note D-23 annual option as deferred per RESEARCH.md Pitfall 4 (Patreon 3-month/$200+ eligibility rule — external constraint, not planner discretion).

7. **Will Phase 3 use the same hosting platform as Phase 2 picks in the bake-off?**
   - What we know: D-17 establishes the shell; Phase 3 extends it.
   - What's unclear: Phase 3 may reach for different platform features.
   - Recommendation: Bake-off result documents *why* the platform was chosen so Phase 3 can re-evaluate consciously if needed.
   - **RESOLVED (for Phase 2):** The outcome of Plan 04's bake-off becomes Phase 3's starting platform choice; no Phase 2 action required.

## Sources

### Primary (HIGH confidence)
- **Tally.so docs:**
  - `tally.so/help/redirect-on-completion` — redirect-on-submit feature in free plan
  - `tally.so/help/how-to-create-a-thank-you-page` — custom thank-you page (free)
  - `tally.so/help/how-to-create-a-gdpr-compliant-form` — consent checkbox required pattern
  - `tally.so/help/embed-your-form` — embed snippet
  - `tally.so/help/custom-domains` — Pro-plan custom domain feature
  - `tally.so/pricing` — plan limits and pricing
  - `tally.so/help/webhooks` — CSV/webhook export
- **Patreon docs:**
  - `support.patreon.com/hc/en-us/articles/115002958403-Launch-Checklist` — pre-launch checklist
  - `support.patreon.com/hc/en-us/articles/360041721372-Annual-memberships-creator-overview` — 3-month + $200 eligibility
  - `support.patreon.com/hc/en-us/articles/36426991446797` — standard 10% fee for new creators post-Aug 2025
  - `support.patreon.com/hc/en-us/articles/38341256940557-Patreon-Page-Vanity-Policy` — vanity claim/dispute
  - `support.patreon.com/hc/en-us/articles/115004048046-Posting-to-your-Patreon` — post visibility (public/patron-only)
- **Vercel docs:**
  - `vercel.com/legal/terms` — Hobby commercial-use restriction
  - `vercel.com/docs/limits/fair-use-guidelines` — definition of commercial usage; donations explicitly excluded
  - `vercel.com/docs/plans/hobby` — Hobby plan terms
- **Netlify docs:**
  - `netlify.com/pricing` — free tier limits and commercial-use permitted
  - `docs.netlify.com/manage/domains/configure-domains/configure-external-dns/` — apex A record `75.2.60.5`
  - `docs.netlify.com/manage/domains/configure-domains/dns-records/` — record types
- **Next.js docs:**
  - `nextjs.org/docs/app/getting-started/deploying` — static export configuration
- **GDPR primary references:**
  - `gdpr.eu/privacy-notice/` — official EU template structure
  - GDPR Recital 32 — pre-ticked boxes invalid
  - ICO UK guidance — ico.org.uk

### Verified by tool execution (HIGH confidence — this session)
- WHOIS for `andrewrahman.com` → registrar = Tucows Domains Inc.
- `dig andrewrahman.com NS +short` → `ns1.netfirms.com`, `ns2.netfirms.com`
- `dig andrewrahman.com A +short` → `66.96.149.1`

### Secondary (MEDIUM confidence)
- `community.vercel.com/t/fair-use-of-the-hobby-plan/2725` — community discussion of edge cases
- `answers.netlify.com/t/can-we-use-netlify-free-plan-for-commercial-purposes/41545` — confirms commercial use OK
- `conversiontracking.io/blog/tally-forms-conversion-tracking-7-methods` — embed details
- `events.patreon.com/prelaunchmusicians` — Patreon's musician launch playbook (relevant by analogy for indie audio creator)

### Tertiary (LOW confidence)
- General "Vercel vs Netlify 2026" comparison blog posts — useful for vibes, not authoritative

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — Tally, Patreon, Netlify, Next.js all directly verified against current docs
- Architecture: HIGH — patterns match official deploy guides; embed pattern is documented
- Pitfalls: HIGH for ToS-driven (Vercel commercial use, Patreon annual billing, Tally consent UI); MEDIUM for the brand-confusion pitfall (judgment call)
- Privacy policy required sections: MEDIUM — clauses are correct per GDPR/CCPA texts but exact wording is jurisdiction- and circumstance-sensitive; recommend cross-check during execution
- DNS for `andrewrahman.com`: HIGH — verified by live WHOIS + dig
- Patreon eligibility for annual billing: HIGH — directly cited from Patreon support docs

**Research date:** 2026-04-15
**Valid until:** 2026-05-15 (Tally pricing, Patreon plans, hosting tier ToS all change occasionally — re-verify before any dispute or migration)

---
*Research complete. Planner can now create plans for: (1) external-service account setup, (2) Tally form configuration, (3) Patreon page setup + 3 seed posts, (4) andrewrahman.com Next.js shell + privacy policy, (5) Netlify-vs-Vercel bake-off + DNS migration, (6) repo doc updates per D-03/D-05/D-06.*
