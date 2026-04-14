# Stack Research

**Domain:** Audio plugin launch infrastructure (personal website, forms, funding, content)
**Researched:** 2026-04-14
**Confidence:** MEDIUM — core facts verified via official docs and current web sources; some version specifics (MagicUI package.json) not directly accessible but cross-referenced via multiple sources

---

## Recommended Stack

### Core Technologies

| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| Next.js | 15.x | Personal website framework | Static export (`output: 'export'`) works with GitHub Pages/Netlify; MagicUI is built for Next.js; App Router enables clean page structure |
| React | 19.x | UI rendering | Required by current MagicUI (Tailwind v4 branch); aligns with Next.js 15 default |
| Tailwind CSS | 4.x | Styling | Current MagicUI targets Tailwind v4 by default; v3 supported via v3.magicui.design if needed |
| MagicUI | current (copy-paste, no pkg version) | Animated UI components | MIT license; copy-paste model (no npm install); 20k+ GitHub stars; React 19 + Tailwind v4 by default; covers animated text, backgrounds, cards for a product page |
| Framer Motion | latest compatible | Animation engine for MagicUI | Required peer dependency of MagicUI components; declarative animation API |

### Supporting Libraries / Services

| Library / Service | Version / Tier | Purpose | When to Use |
|-------------------|---------------|---------|-------------|
| Impeccable CSS | current (npx impeccable detect) | AI-assisted design quality enforcement | Use as a Claude Code skill during website build; prevents common AI-generated UI mistakes (overused fonts, purple gradients, nested card abuse); install globally via `cp -r dist/claude-code/.claude ~/.claude/` |
| Tally.so | Free tier | Email capture / mailing list signup form | Unlimited forms + unlimited submissions on free; Tally branding on free tier; integrates with Mailchimp/ConvertKit via Zapier, Relay.app, or webhook; embed as iframe in site |
| Patreon | Standard (10% platform fee) | Creator funding / recurring patronage | No setup cost; 10% platform fee (new creators post-Aug 4 2025) + 2.9% + $0.30 payment processing; total effective ~12–15% of gross; free to set up and explore before earning |
| Netlify | Free tier | Static site hosting | 100 GB bandwidth + 300 build minutes/month free; commercial use allowed on free tier (unlike Vercel); custom domain + SSL free; suspends on overage (does not charge automatically) |
| TypeScript | 5.x | Type safety for Next.js codebase | Standard for Next.js + MagicUI projects; catches errors at build time |

### Development Tools

| Tool | Purpose | Notes |
|------|---------|-------|
| Impeccable CLI | Design anti-pattern detection | `npx impeccable detect src/` scans HTML/JSX for common design mistakes without needing an AI harness |
| GitHub Actions | CI/CD for Netlify deploys | Push-to-deploy; Netlify also offers drag-and-drop deploy and its own CI hooks as alternatives |
| shadcn/ui CLI | Install MagicUI components | MagicUI is a shadcn/ui-compatible copy-paste library; use the shadcn CLI to add individual components |

---

## Installation

```bash
# Bootstrap Next.js 15 app with TypeScript + Tailwind v4
npx create-next-app@latest openspatialdelay-site \
  --typescript --tailwind --app --src-dir

cd openspatialdelay-site

# Install Framer Motion (MagicUI peer dependency)
npm install framer-motion

# Install MagicUI components via shadcn CLI (per-component, copy-paste model)
npx shadcn@latest add "https://magicui.design/r/[component-name]"
# Example:
npx shadcn@latest add "https://magicui.design/r/animated-gradient-text"
npx shadcn@latest add "https://magicui.design/r/marquee"

# Install Impeccable for Claude Code globally (one-time)
# From: https://github.com/pbakaus/impeccable
cp -r dist/claude-code/.claude ~/.claude/
# Or run standalone scan:
npx impeccable detect src/
```

---

## Hosting Decision: Why Netlify Over Alternatives

| Host | Free Bandwidth | Commercial Use | Custom Domain | Build CI | Verdict |
|------|---------------|---------------|--------------|---------|---------|
| **Netlify** | 100 GB/mo | **Allowed** | Yes + SSL | 300 min/mo | **Recommended** |
| GitHub Pages | 100 GB/mo (soft) | Prohibited for business/SaaS | Yes | Via Actions only | Avoid — TOS prohibits commercial use |
| Vercel Hobby | 100 GB/mo | **Prohibited** (personal only) | Yes | Yes | Avoid — TOS requires Pro ($20/mo) for commercial |
| Cloudflare Pages | Unlimited | Allowed | Yes + SSL | 500 min/mo | Valid alternative with better bandwidth |

Netlify is the recommended choice: same generous free tier as Vercel, commercial use explicitly allowed, suspends rather than surprise-charges on overage. Cloudflare Pages is a valid upgrade path if bandwidth becomes a concern.

---

## Alternatives Considered

| Recommended | Alternative | When to Use Alternative |
|-------------|-------------|-------------------------|
| Netlify (hosting) | Cloudflare Pages | If CDN performance and bandwidth limits matter more than Netlify DX |
| Tally.so (forms) | Typeform | Only if you need branded, conversational forms — Typeform free tier is far more restrictive (10 responses/month) |
| Tally.so (forms) | Google Forms | If zero branding concern and no styling needed; functional but ugly |
| Next.js 15 (framework) | Astro | If site is pure static content with no interactive components; Astro ships zero JS by default and would be faster for a simple product page |
| Patreon (funding) | Ko-fi | Lower fees (~0% platform fee, just payment processing); better for one-time donations; weaker membership/tier tooling than Patreon |
| Patreon (funding) | Gumroad | Better fit if selling plugin licenses directly; not suited for recurring community funding |

---

## What NOT to Use

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| Vercel Hobby (free tier) | TOS explicitly prohibits commercial use; any plugin/product promotion counts as commercial | Netlify free tier |
| GitHub Pages (commercial) | TOS prohibits commercial transactions or promoting commercial software | Netlify free tier |
| WordPress / Squarespace | Unnecessary complexity and cost for a static product page; kills performance | Next.js + Netlify |
| Typeform free tier | 10 responses/month limit makes it useless for email capture | Tally.so free tier |
| MagicUI Pro (paid) | Paid templates unnecessary — free copy-paste components are sufficient for a single product page | Free MagicUI components via shadcn CLI |
| Separate email marketing platform (immediately) | Don't add Mailchimp/ConvertKit during launch — Tally webhook + Google Sheets is sufficient to capture emails at zero cost until list exceeds ~500; add ESP when list is real | Tally + Google Sheets webhook initially |

---

## Stack Patterns by Variant

**If the site is purely a product landing page (no blog, no CMS):**
- Use Next.js with `output: 'export'` for fully static HTML/CSS/JS output
- Deploy to Netlify via drag-and-drop or GitHub Actions
- No server-side features needed

**If SpatialMediaLab.org needs a blog / newsletter archive:**
- Keep the website builder for SpatialMediaLab.org (as scoped); don't rebuild it in Next.js
- The personal site (OpenSpatialDelay page) stays pure static

**If Tally email list grows past ~500 subscribers:**
- Add Kit (formerly ConvertKit) via Zapier Tally integration
- Kit free tier covers up to 10,000 subscribers with basic automations

**If Impeccable is already installed globally for Claude Code:**
- Skip project-local install; the global `~/.claude/` install applies to all projects
- Use `/audit` and `/polish` commands during the UI build phase

---

## Version Compatibility

| Package | Compatible With | Notes |
|---------|-----------------|-------|
| MagicUI (current / Tailwind v4 branch) | React 19, Tailwind CSS v4, Next.js 15 | Use v3.magicui.design if project uses Tailwind v3 |
| MagicUI (v3 branch at v3.magicui.design) | React 18, Tailwind CSS v3, Next.js 14 | Legacy branch; maintained but not updated with new components |
| Framer Motion (latest) | React 19 | framer-motion v11+ supports React 19 |
| Next.js 15 static export | Netlify, GitHub Pages, Cloudflare Pages | Requires `output: 'export'` in next.config; disables image optimization; add `.nojekyll` for GitHub Pages |

---

## Tally Integration Architecture

For email capture feeding a mailing list:

```
User fills Tally form
  → Tally webhook fires (free tier)
  → Google Sheets (via native Tally integration, free)
  → [Later] Zapier/Relay.app → Kit/Mailchimp when list is real
```

Tally's native Google Sheets integration is free and zero-latency. Start there. Add Zapier automation only when sending newsletters.

---

## Patreon Setup Summary

- No cost to create a page or explore features
- Standard fee: **10% platform** + **2.9% + $0.30 payment processing** per transaction
- Total effective take-rate: ~12–15% of gross
- No content restrictions for audio software tools / plugin funding
- Apple in-app purchase (iOS Patreon app): Apple takes 30% — accept this as a known loss; most supporters join via web
- Payout methods: PayPal, Stripe direct, bank transfer (fees vary by method and currency)

---

## Sources

- https://github.com/magicuidesign/magicui — MIT license, copy-paste model confirmed, 20k+ stars
- https://v3.magicui.design/docs/tailwind-v4 — Tailwind v4 / React 19 as current default, v3 legacy branch
- https://github.com/pbakaus/impeccable — Impeccable tool confirmed: Claude Code support, 18 commands, `npx impeccable detect` CLI, Apache 2.0
- https://tally.so/help/plans-and-pricing — Free tier: unlimited forms + submissions; Pro $29/mo for no branding
- https://support.patreon.com/hc/en-us/articles/36426991446797 — 10% standard fee for new creators post-Aug 4 2025
- https://www.netlify.com/pricing/ — 100 GB bandwidth, 300 build minutes, commercial use allowed (MEDIUM confidence — verified via search aggregation)
- https://vercel.com/docs/plans/hobby — Hobby plan non-commercial only (HIGH confidence — official docs)
- https://docs.github.com/en/pages/getting-started-with-github-pages/github-pages-limits — 100 GB soft limit; commercial use prohibited (HIGH confidence — official docs)
- https://nextjs.org/docs/app/guides/static-exports — Next.js static export for GitHub Pages/Netlify (HIGH confidence — official docs)
- WebSearch aggregation — Zapier/Relay.app Tally→Mailchimp/Kit integration paths (MEDIUM confidence)

---
*Stack research for: OpenSpatialDelay launch infrastructure*
*Researched: 2026-04-14*
