# Phase 5: Launch Announcements — Context

**Gathered:** 2026-04-17
**Status:** Ready for planning

<domain>
## Phase Boundary

Coordinated public announcement of OpenSpatialDelay v1.0.0 across 5 channels + 2 outreach modes:

**Channels (ANNC-01..05):**
1. SML blog post with demo embed (`spatialmedialab.org`)
2. SML newsletter email
3. LinkedIn post (link in first comment)
4. Instagram post / Reel
5. KVR Audio product listing

**Outreach (ANNC-06):**
6. Review-pitch outreach — to Segment 1-5 contacts (immersive tool reviewers, film/game sound designers, Atmos/Ambisonics educators, spatial practitioners, press) per `influencer-outreach-v1.0.md`
7. Guest/speaker-pitch outreach — to Segment 7 contacts (podcasts, meetups, AES Berlin, ADC)

Phase 5 clarifies **how to launch**. New channels (YouTube, own podcast, merch, Product Hunt) belong in v2 and are explicitly out of scope per PROJECT.md.

**Hard deadline:** All public links (site, Patreon, GitHub release, demo assets) live **before Superbooth 2026** (May 7–10). Target launch day: **Tuesday April 28, 2026**.
</domain>

<decisions>
## Implementation Decisions

### Launch Cadence
- **D-01 Coordinated D-day burst:** All 5 channels (blog, newsletter, LinkedIn, IG, KVR) publish within a 2-hour window on a single day. Not rolling-week.
- **D-02 Day-of-week = Tuesday.** Highest B2B engagement; avoids Monday-email-backlog and Friday-attention-drift.
- **D-03 Target launch date = Tuesday April 28, 2026.** Driven by hard constraint: all public URLs/assets must be live before Superbooth (Berlin, May 7–10, 2026) so Andrew can point attendees to live links face-to-face. Fallback date: Tuesday May 5, 2026 (tighter Superbooth buffer; overlaps with Andrew's travel window — avoid if possible). Planner must confirm Phase 3 site + Phase 4 demo content are ready in time.
- **D-04 User travel constraint:** Andrew is traveling for most of May 2026. Launch day itself (Apr 28) is pre-travel. Anything requiring Andrew's email response during May must NOT be triggered by the launch.

### Review-Pitch Outreach (Segments 1-5)
- **D-05 Send at T-0 launch morning.** Cold-send all review-pitch emails on launch day itself. No pre-launch embargo process. Rationale: simpler logistics; Andrew won't be in a state to manage embargo tracking + Superbooth prep + travel simultaneously. Accepted tradeoff: lower reply rate than embargo strategy, but all coverage falls inside the fresh-launch window.
- **D-05a Scope of T-0 batch:** Segment 1 (immersive tool reviewers — 10 contacts), Segment 2 (film/game sound designers — 9), Segment 3 (Atmos/Ambisonics educators — 8), Segment 4 (spatial practitioners — 7 minus Berlin-local trio which is handled separately), Segment 5 (press — selective).

### Guest/Speaker-Pitch Outreach (Segment 7)
- **D-06 Delay to late May / early June 2026.** Send AFTER Andrew's May travel period ends. Rationale: podcast/meetup replies trigger scheduling conversations that require Andrew's active inbox; launching cold pitches while Andrew is unreachable wastes replies.
- **D-06a Decouple from launch day.** Guest-pitch is NOT part of the launch-day blast. Treat as a separate outreach wave.

### Superbooth Berlin-Local Outreach (special case)
- **D-07 Pre-Superbooth DMs to Berlin-local contacts.** Before May 7, send lightweight "see you at Superbooth" intros (NOT full guest-pitch emails) to:
  - Hainbach
  - Peter Kirn (CDM)
  - Eric Horstmann (Immersive Lab / Genelec Immersive Audio Hub)
  - AES Germany Berlin section chair
- **D-07a Tone:** short, personal, face-to-face-first. "I'm launching a free spatial delay before Superbooth — grab a coffee at the Messe and I'll show you?" Not a review pitch. Not a guest pitch. A relationship-first move.
- **D-07b Send timing:** within the next 1-2 weeks (starting now, through launch day). These are time-sensitive; Berlin contacts plan their Superbooth schedules in advance.

### Newsletter Source (ANNC-02)
- **D-08 Use separate spatialmedialab.org newsletter.** NOT the osd-confirmed Sender.net list at andrewrahman.com/get-osd. Rationale:
  - SML newsletter presumably has an established audience; OSD Sender list starts at 0 subs and may have very few by launch day
  - Decouples ANNC-02 from DIST-01 and the ongoing Sender.net outage blocker
  - Aligns with PROJECT.md framing: OSD is the *first tool in the Spatial Media Library pipeline* — SML newsletter is the right primary channel for the launch narrative
- **D-08a Open question for planner:** Confirm who on the SML board manages the newsletter and the send-path. If Andrew doesn't directly own the send, coordinate with that person + factor their availability into the D-03 launch date.

### Claude's Discretion
- **Positioning hero angle** — User deferred to Claude. Recommendation for planner: lead with *"Free spatial delay — each echo has a position in 3D space"* (capability + free, in that order) for the blog post; let LinkedIn lean on the Berlin-indie-dev story; let IG/Reels lean on the audible demo. Per-channel positioning OK but keep the core capability sentence consistent.
- **Per-channel content format split** — Planner designs (blog = depth + DSP story + comparison to Sound Particles/dearVR free-tier context; newsletter = teaser + links; LinkedIn = personal narrative + first-comment link; IG = demo clip + screenshot carousel; KVR = product-metadata-first).
- **KVR listing category/tags** — Planner picks after reviewing current KVR taxonomy; suggest primary: `Delay`, secondary: `Spatial`, plus `Free`, `VST3`, `AU`, `macOS`, `Windows`.
- **Email template styling** — Planner decides plain-text vs branded HTML per mode. Recommendation: plain-text-looking for review-pitch (reads personal), light branded HTML for guest-pitch (signals legitimacy).

### Folded Todos
No existing todos matched Phase 5 scope at discussion time.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Outreach
- `.planning/research/influencer-outreach-v1.0.md` — v2 candidate list (34 individuals + 15 communities) across 7 segments. Every entry carries `verify-before-send` flag. Contains the Sound-Particles-reviewer heuristic, Berlin-local cluster, dual outreach modes (review vs guest). Primary source of truth for Segments 1-7.

### Phase dependencies
- `.planning/phases/03-personal-website/03-CONTEXT.md` — website copy, hero stats, feature slate (launch content must align with what visitors see when they click through)
- `.planning/phases/03-personal-website/03-09-PLAN.md` — cross-phase DIST-01 flip + Sender UAT (queued; does NOT block ANNC-02 per D-08)
- `.planning/phases/04-demo-content/` — DOES NOT YET EXIST. Phase 4 has no plans. CONT-01/02/03 are on Phase 5's critical path. Planner MUST flag this as a blocker in the Plan 05-00-PLAN.md preconditions.

### Voice / tone baseline
- `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-page-about.md` — "Spatial Media Library pipeline" framing
- `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-01-welcome.md` — first-person Andrew voice
- `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-02-technical.md` — capability-level technical-story framing (no DSP jargon)
- `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-03-roadmap.md` — roadmap narrative tone
- `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-post-04-project-links.md` — cross-linking template (GitHub, SML, andrewrahman.com, Patreon)
- `.planning/phases/02-email-capture-funding-infrastructure/drafts/patreon-tier-copy.md` — space-theme tier naming (Stargazer / Astronaut / Commander / Mission Control) — available as a gentle motif for launch copy

### Project-level
- `.planning/PROJECT.md` — core value statement, v2 out-of-scope list (do not invent new channels)
- `.planning/REQUIREMENTS.md` — ANNC-01..06 full scope definitions
- `CLAUDE.md` (project root) — build / voice / no-jargon rules
- User memory `feedback_marketing_copy_depth.md` — capability-level language only, no FFT size / paper-name jargon
- User memory `feedback_evidence_artifact_ceremony.md` — don't block plan closure on non-load-bearing evidence artifacts (screenshots etc.) for solo-creator context

### External anchors (no file — event dates only)
- **Superbooth 2026:** May 7–10, 2026, Messe Berlin (FEZ-Berlin). Live links must exist BEFORE.
- **Andrew's travel:** "most of May 2026" (specific dates TBD). Launch must land before travel starts.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- **Patreon draft files** (6 markdown docs) — already establish the "Spatial Media Library pipeline" framing + Berlin-solo-dev tone. Reuse liberally in blog post + LinkedIn + IG caption.
- **Sender.net confirmation email template** (built in Phase 02-05 Task 1 walkthrough) — contains download buttons + Patreon CTA. Launch blog can mirror this three-block structure (confirm, download, support).
- **Website copy locked in Phase 03** — hero headline, feature slate, taglines. Launch copy should propagate these not contradict them. Planner must cross-check.

### Established Patterns
- **First-person Andrew voice** in Patreon posts and About pages — extend to blog post + LinkedIn + IG caption.
- **SML-plural framing** (OSD is "the first tool in the Spatial Media Library") — preserve in all launch copy; never position OSD as the whole company.
- **Cross-link stack** (GitHub + SML + andrewrahman.com + Patreon) — canonicalised in Patreon Post 4; repeat in blog post + newsletter.

### Integration Points
- **SML blog publishing workflow** — spatialmedialab.org. Unclear if it's Next.js/Markdown/WordPress/etc. Planner must verify before drafting the blog post in the right format.
- **SML newsletter sending mechanism** — separate infrastructure from OSD Sender. Planner must identify who owns the send-path (see D-08a).
- **GitHub Release v1.0.0** — already published (768c248, per project_version_e15b.md memory). Launch day does NOT need to re-cut the release; only update release-notes wording if needed.
- **Patreon cross-post** — Patreon already live with 4 posts. Consider an additional "v1.0.0 is OUT" Patreon post on launch day as a sixth channel (not counted in ANNC-01..05 but a natural extension).

</code_context>

<specifics>
## Specific Ideas

- **Superbooth anchor is decisive.** The hard constraint reshapes everything — it's why launch is Tue Apr 28 (not "ASAP" or "May sometime"). Planner should treat this as an immovable deadline.
- **Andrew's May travel decouples the two outreach modes.** Review-pitch goes out at T-0 launch morning and is fire-and-forget (reviewers who reply can wait a week for Andrew to respond post-travel). Guest-pitch goes out late May / early June because scheduling podcasts requires active inbox management.
- **Berlin-local Superbooth intros are a category unto themselves.** Not review pitches, not guest pitches. Lightweight "see you there" DMs to 4 named contacts (Hainbach, Kirn, Horstmann, AES Berlin chair) in the next 1-2 weeks. Separate from ANNC-06 entirely but high-leverage.
- **"Free" is a credibility liability on KVR.** KVR users occasionally assume free = unfinished. Counterweight with: GPL-3.0 license + explicit "v1.0.0, fully tested, 162+ Catch2 tests, 70 factory presets" proof points in the listing body.

</specifics>

<deferred>
## Deferred Ideas

Ideas surfaced during discussion that belong in other phases or future milestones:

- **YouTube channel for Andrew** — new capability; belongs in v2 / COMM-01 (already logged as v2 requirement).
- **Own podcast** — new capability; v2.
- **Merchandise** — new capability; v2 at earliest.
- **Product Hunt launch** — already tracked as v2 requirement COMM-02; could promote to v1.1 if the v1.0 launch generates enough signal to warrant it.
- **Launch-day livestream** — considered as an amplification move; deferred because Andrew's May travel makes post-launch live appearances difficult. Revisit for v1.1.
- **Press-release distribution service** (e.g., PRWeb, PR Newswire) — corporate PR approach; doesn't fit indie-dev positioning. Skip.
- **Sound-design/film-scoring segment follow-up research** — flagged in `influencer-outreach-v1.0.md` Gaps section; worth a dedicated research round targeting film-scoring YouTubers + Sound Particles users if launch coverage feels thin.
- **AES Berlin speaker slot** — listed in Segment 7 meetup targets; contact is part of D-07 Superbooth DM batch but the formal talk proposal is late-May / June work (guest-pitch wave).

### Reviewed Todos (not folded)
No todos were reviewed-but-deferred at discussion time.

</deferred>

---

*Phase: 05-launch-announcements*
*Context gathered: 2026-04-17*
