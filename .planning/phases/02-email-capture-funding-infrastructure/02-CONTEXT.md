# Phase 2: Email Capture & Funding Infrastructure - Context

**Gathered:** 2026-04-15
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 2 delivers three publicly-live destinations that Phase 3's website will link to:

1. **Tally form** — required-email + GDPR-consent gate that redirects to the OpenSpatialDelay installer on GitHub Releases
2. **Privacy policy page** — hosted at `andrewrahman.com/privacy` as a stable URL linked from the Tally form, Patreon, and all future product communications
3. **Patreon page** — seeded with 3 posts under the "Andrew Rahman" creator identity, pitching the **Spatial Media Library** pipeline and crediting **Spatial Media Lab** as the org behind the source code

Scope also includes standing up the minimal `andrewrahman.com` Next.js static shell (placeholder homepage + `/privacy` + layout scaffolding) so that the privacy URL is stable from day 1 and Phase 3 inherits an existing site rather than starting cold.

**Not in scope:**
- Full multi-product personal hub (aspirational, not this milestone)
- OSD-specific landing page content (remains Phase 3 deliverable)
- Newsletter tool integration beyond Tally's dashboard (deferred to Phase 5)
- Influencer-outreach email draft + community-forum post drafts (folded into Phase 5)
- Registering the domain — user already owns `andrewrahman.com` via Netfirms or Namecheap

</domain>

<decisions>
## Implementation Decisions

### Identity & Branding

- **D-01:** Three distinct identities must be preserved consistently in all Phase 2 copy:
  - **Spatial Media Lab** (SML) — the org that owns the GitHub repo (`github.com/Spatial-Media-Lab/OpenSpatialDelay`) and `SpatialMediaLab.org`
  - **Spatial Media Library** — the pipeline/toolset brand funded via Patreon (new name introduced in Phase 2 — three words, not to be confused with SML)
  - **OpenSpatialDelay** — first product in the Spatial Media Library pipeline, developed under Spatial Media Lab
  - **Andrew Rahman** — personal creator identity on Patreon, maintainer and primary author
- **D-02:** All external artifacts (Tally form, privacy policy, Patreon page, andrewrahman.com shell) must reference Spatial Media Lab as the org behind the source code and link to `github.com/Spatial-Media-Lab/OpenSpatialDelay` where appropriate
- **D-03:** PROJECT.md must be updated during Phase 2 to introduce the "Spatial Media Library" pipeline brand alongside the existing "Spatial Media Lab" org reference — downstream agents (research, planner, executor) should not conflate them

### Gate Architecture

- **D-04:** **Email is the gate for the built installer**, not optional. Source code remains freely public on GitHub (GPL-3.0 compliance). The installer download is reached via the Tally form; anyone can bypass by browsing GitHub Releases directly. Gate-by-friction accepted — community sharing of direct installer URLs is an accepted cost, not something to prevent.
- **D-05:** Phase 2 must update `ROADMAP.md` Phase 2 success criterion 1 — remove "optional email" wording and replace with "required email" framing
- **D-06:** Phase 2 must update `REQUIREMENTS.md` — remove "Download gate / mandatory email" from the anti-features table (contradicts D-04) and revise DIST-01 to reflect required-email framing

### Tally Form

- **D-07:** Form fields = required email + single required GDPR-consent checkbox. No additional fields at launch.
- **D-08:** Consent checkbox wording = single required checkbox in plain English covering both storage and marketing use. Claude drafts wording in the plan for user review.
  - Suggested baseline: *"I agree that my email will be stored and used to send occasional updates about OpenSpatialDelay and other Spatial Media Library tools. [Privacy policy]"*
- **D-09:** Post-submit flow = **thank-you page with manual download buttons** (macOS, Windows) + soft Patreon CTA line. No auto-redirect. User clicks the download button themselves. Patreon CTA tone: subtle, not the primary focus.
- **D-10:** Email storage = **Tally dashboard only for Phase 2**. No newsletter-tool integration. CSV export + newsletter integration deferred to Phase 5 when the first outbound send actually happens.
- **D-11:** Form copy authorship = Claude drafts headline, microcopy, consent wording, and thank-you page text in the plan; user revises before execution.

### Privacy Policy

- **D-12:** Privacy policy hosted at **`andrewrahman.com/privacy`** (stable permanent URL). No Tally-hosted fallback, no temporary URL.
- **D-13:** Policy authored by Claude from scratch, tailored to the actual Phase 2 data flow (Tally form → stored in Tally dashboard → manual newsletter use in Phase 5+). User reviews before publishing.
- **D-14:** Policy explicitly covers **GDPR (EU) + CCPA (California) + UK DPA** jurisdictions.
- **D-15:** Email retention = indefinite until user-initiated unsubscribe. Unsubscribe via plain email request or standard email-footer unsubscribe link (once newsletter sends start in Phase 5).
- **D-16:** Data-controller designation = **RESOLVED 2026-04-16: Andrew Rahman personally** for privacy policy / newsletter / email handling. Rationale: the privacy policy governs emails collected through andrewrahman.com/get-osd → stored in Tally → used in Andrew's personal newsletter. Site is under personal identity, Patreon is under personal identity (D-21), and the newsletter is personal — so data controller must match that chain. **Boundary:** Plugin licenses and legal notices (GPL-3.0, third-party attributions, LICENSE file in the OSD repo) remain with Spatial Media Lab as the legal home of the code. Privacy policy contact email: andrewjrahman@gmail.com.

### Site Shell & Hosting

- **D-17:** Phase 2 stands up a **minimal Next.js static shell** at `andrewrahman.com` containing:
  - Placeholder homepage with "minimal bio + more coming" framing
  - `/privacy` page serving the drafted policy
  - Shared `<Layout>` scaffolding (nav slot, footer) so Phase 3 can extend without a rewrite
  - Footer includes attribution line: "code lives at github.com/Spatial-Media-Lab"
- **D-18:** Platform hosting = **Netlify vs Vercel bake-off during Phase 2**. Deploy the shell to both; pick winner before updating the custom domain's DNS. Decision must be made by end of Phase 2.
  - Known risk: Vercel Hobby tier prohibits commercial use; Patreon-linked site may qualify as commercial. Factor into the final pick.
- **D-19:** Domain DNS = user already owns `andrewrahman.com` via Netfirms or Namecheap. Registrar verification is the first execution step. Point an **A or CNAME record** (not MX — MX is for email routing) at the winning platform after bake-off.
- **D-20:** Homepage placeholder content = short personal bio paragraph, mention of OpenSpatialDelay (link to GitHub), note that more is coming. Footer with `/privacy` link and attribution. Claude drafts in plan; user reviews.

### Patreon

- **D-21:** Creator identity = **Andrew Rahman (personal)**. Not SML, not a new brand.
- **D-22:** Core patron-value pitch = "Funding the **Spatial Media Library** pipeline — OpenSpatialDelay is the first; more are coming. Source code lives at Spatial Media Lab. Your support keeps these tools free and open-source."
- **D-23:** Tier structure = **4 tiers + annual-payment option**. Claude drafts the following baseline for user review; pricing and benefits finalized in plan:
  - **$3 Supporter** — newsletter, public thanks, funding acknowledgment
  - **$10 Patron** — Supporter benefits + feature-request voting + early builds
  - **$25 Partner** — Patron benefits + 1:1 Discord/email access + early access to future Patreon-exclusive plugins
  - **$100 Founder** — Partner benefits + public thank-you credit in plugin About dialog
  - **Annual option** — ~15% discount on each tier
- **D-24:** Future Patreon-only plugins = soft-mentioned as a future plan, **no specific timeframe committed**. Copy example: *"OpenSpatialDelay is GPL and free for everyone. Some future plugins in the Spatial Media Library pipeline will launch on Patreon first or as patron-exclusive — those will be announced as they come."*
- **D-25:** Patreon page must include a **soft one-liner CTA for Berlin residents** to connect with Spatial Media Lab. Placement = sidebar or footer of the Patreon page (not seed posts, not the main pitch). Tone = casual, low-urgency. Suggested baseline: *"In Berlin? Come by Spatial Media Lab — we run events and collaborate with local artists. [link]"* Link target to be confirmed in planning (likely SpatialMediaLab.org About Us or a "join us" contact section).
- **D-26:** Patreon page links = (a) `github.com/Spatial-Media-Lab/OpenSpatialDelay` for source, (b) `SpatialMediaLab.org` for org identity, (c) `andrewrahman.com` as creator home, (d) `andrewrahman.com/privacy` in footer.

### Patreon Seed Posts

- **D-27:** **3 seed posts at launch** (exceeds DIST-03 minimum of 2). Required live before any public Patreon link is shared in launch communications.
- **D-28:** Topics + visibility:
  - **Post 1 — Welcome / why-Patreon / pipeline vision** → **public**. Anchor post: who Andrew is, what the Spatial Media Library is, why supporting here funds the pipeline, credit to Spatial Media Lab as source-code home.
  - **Post 2 — OpenSpatialDelay technical deep-dive** → **patron-only ($3+ tier)**. HRTF rendering, phase vocoder, trajectory engine, Doppler simulation. Demonstrates patron value immediately.
  - **Post 3 — Spatial Media Library roadmap** → **public**. Pipeline themes and future plugin categories (no specific promises). Acts as aliveness signal for visitors.
- **D-29:** Post drafting = Claude drafts all three in Phase 2 execution; user revises for voice before publishing on Patreon.

### Cross-Phase Flags (for planner/future phases)

- **D-30:** **Phase 5 scope expansion** — influencer-outreach email draft (for spatial-audio reviewers) and community-forum post drafts (KVR, Gearspace, Reddit audio subs, Hacker News, etc.) belong in Phase 5 (Launch Announcements). Planner should surface this when Phase 5 is discussed so ROADMAP.md Phase 5 success criteria can be extended.

### Claude's Discretion

- Tally form visual design (within Tally's template constraints)
- Specific wording of consent checkbox (within plain-English + GDPR-valid constraints)
- Specific bio copy for placeholder homepage (Andrew will edit for voice)
- Specific tier benefit descriptions (baseline in D-23, copy refinement is editorial)
- Specific seed-post prose (Andrew revises for voice)

### Folded Todos

None — zero pending todos matched Phase 2 scope per init check.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Roadmap & Requirements
- `.planning/ROADMAP.md` §Phase 2 — phase goal, dependencies, success criteria (criterion 1 wording requires update per D-05)
- `.planning/REQUIREMENTS.md` §DIST-01, §DIST-02, §DIST-03 — requirement definitions (anti-features table requires update per D-06)
- `.planning/REQUIREMENTS.md` §Anti-features — "Download gate / mandatory email" row is now obsolete (D-04 supersedes)
- `.planning/PROJECT.md` — project context, GPL-3.0 decision, Patreon-funding model decision, anti-features list (must be updated per D-03 to introduce Spatial Media Library brand)

### Prior Phase Context
- `.planning/phases/01-repo-license-readiness/01-CONTEXT.md` — GPL-3.0 transition decisions, license-delivery model (separate legal notices doc bundled in release ZIP)

### Research
- `.planning/research/SUMMARY.md` — original Phase 2 research (Tally free tier, /releases/latest redirect, hub-and-spoke architecture, KVR audience reference). Note: the original research framed email as optional — D-04 supersedes that framing.

### External Documentation (to be read during planning)
- Tally.so docs (free-tier limits, consent-checkbox patterns, redirect configuration, thank-you-page customization)
- Patreon creator docs (tier setup, annual billing, post visibility controls, page customization)
- Next.js 15 static export docs (for site shell)
- Netlify deploys docs + Vercel deploys docs (for bake-off)
- GDPR/CCPA/UK DPA privacy policy template references (planner selects authoritative sources during research)

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- **GPL-3.0 `LICENSE`** at repo root — reference for "source code" link language on Patreon page and andrewrahman.com footer
- **`docs/` directory structure** — pattern for hosting user-facing documents (legal notices generated here in Phase 1); privacy policy generation *could* live alongside if Phase 2 decides to co-locate drafts
- **No existing Tally / Patreon / web-hosting code** — Phase 2 is entirely new external infrastructure; repo contains only the plugin source

### Established Patterns
- **Script-generated documents** (from Phase 1) — `docs/generate_legal_notices.js` precedent for Node.js-scripted document generation; privacy policy could follow similar pattern if kept in-repo, but andrewrahman.com site is a separate project and likely gets its own repo
- **Separate repo expected** — based on Phase 3 research (Next.js 15 static export to Netlify), the personal site should live in a **new repository**, not in the plugin repo. Planner to confirm repo strategy.

### Integration Points
- **README.md** (repo root) — no changes expected in Phase 2 (Phase 1 already handled install docs and license). Planner should verify no Phase 2 changes to README are needed.
- **GitHub Releases v1.0.0** — Tally redirect target. Current release page URL: `github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/latest`
- **Phase 3 (future)** — andrewrahman.com shell created in Phase 2 must leave shared `<Layout>` scaffolding (nav + footer slots) intact so Phase 3 landing-page work extends rather than rewrites

</code_context>

<specifics>
## Specific Ideas

- **Netlify vs Vercel bake-off** is a deliberate technique, not indecision — both deploy the same shell from the same source; real-world comparison before pointing DNS. Deadline = end of Phase 2.
- **Gate-by-friction, not gate-by-obscurity** — user accepts that community members will redistribute direct GitHub Release URLs. Tally is the "official front door," not a secret.
- **Patreon always-present, never-primary** — link appears in Tally thank-you page, privacy policy footer, andrewrahman.com footer, all future email footers, and in-plugin About dialog (aspirational). Never the lead CTA.
- **Berlin one-liner** — intentionally soft. Not a recruiting pitch; a quiet "we're here, come hang out" in the Patreon sidebar/footer.
- **Spatial Media Library** is a new pipeline brand introduced in Phase 2. Distinct from Spatial Media Lab (org). Future product-naming decisions (e.g. plugin #2, plugin #3) will inherit this pipeline-brand umbrella.

</specifics>

<deferred>
## Deferred Ideas

Captured here so they aren't lost — not acted on in Phase 2.

### Deferred to Phase 3 (Personal Website)
- **Full personal-hub expansion** — multi-product architecture, per-product pages, standard navigation, dedicated bio page, blog. User answered "hub is aspirational for now" — Phase 3 stays scoped as an OSD-focused landing page built on the andrewrahman.com shell from Phase 2.

### Deferred to Phase 5 (Launch Announcements)
- **Influencer-outreach email draft** — outreach to spatial-audio reviewers / YouTubers to get OSD reviewed. Needs Phase 4 demo assets to be meaningful.
- **Community-forum post drafts** — KVR Audio forum, Gearspace, r/edmproduction, r/WeAreTheMusicMakers, Hacker News Show HN post, etc. Needs Phase 4 demo assets and Phase 3 landing page to link to.
- **Newsletter-tool integration** — auto-sync Tally → Mailchimp/Buttondown when first outbound send is imminent. Phase 2 keeps emails in Tally dashboard only.

### Deferred to a Future Milestone
- **Patreon-exclusive plugin release** — soft-mentioned on Patreon page per D-24 but no specific commitment or timeframe. Will be planned as part of a future Spatial Media Library plugin milestone.
- **Andrew's personal-hub build-out** — full multi-product site, blog, writing section. Becomes its own milestone once a second product is real.
- **In-plugin Patreon CTA / About-dialog enhancements** — mentioned aspirationally; would require plugin source changes (out of scope for this milestone which explicitly excludes plugin code changes).

### Reviewed Todos (not folded)

None — no pending todos existed at Phase 2 start.

</deferred>

---

*Phase: 02-email-capture-funding-infrastructure*
*Context gathered: 2026-04-15*
