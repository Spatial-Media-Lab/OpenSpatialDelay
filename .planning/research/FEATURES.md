# Feature Research

**Domain:** Open-source audio plugin public launch (landing page, Patreon, social, email capture, blog, docs)
**Researched:** 2026-04-14
**Confidence:** MEDIUM — General launch best practices HIGH confidence. Audio-plugin-specific patterns MEDIUM (extrapolated from comparable plugin launches: Surge XT, Odin2, Vital, Angle Audio PH launch). No direct "spatial delay plugin launch" case studies found.

---

## Feature Landscape

### Landing Page

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Hero with plugin name + one-line value prop | First thing a visitor reads — determines if they stay | LOW | "Each echo lives in 3D space" is already a strong hook |
| Audio/video demo above the fold | Audio plugins sell by sound — no demo = no trust | MEDIUM | Even a 30-second clip works; full walkthrough optional |
| Single prominent download CTA | Free plugins expect zero friction to get the file | LOW | "Download Free" or "Get It Free" — one button, no forms |
| System requirements (macOS/Windows, VST3/AU) | Producers check format compatibility before downloading | LOW | Must be scannable, not buried in a table |
| GitHub link | GPL open-source plugins are expected to show source | LOW | Badge or footer link is sufficient |
| Screenshots of the plugin UI | Producers judge by UI before trying | LOW | 2-3 high-res screenshots; include the 3D trajectory view |
| Feature list (brief) | Scannable proof the plugin does what it claims | LOW | Bullets, not paragraphs — emphasize the spatial angle |
| License statement | GPL plugins need to state license clearly | LOW | One line: "GPL-3.0 — free to use, modify, and share" |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Audio embed demonstrating spatial effect | Spatial audio is hard to convey in screenshots — a headphone demo is the product | MEDIUM | A 15-30 second stereo headphone demo directly on the page converts skeptics; use SoundCloud embed or HTML5 audio |
| "How it works" 3-step explainer | Spatial delay is unfamiliar to most producers — reduces cognitive load | LOW | Step 1: Place echo in 3D / Step 2: Set trajectory / Step 3: Render binaural; simple icon + text layout |
| HRTF profile callout | 6 HRTF profiles is differentiating vs generic binaural plugins | LOW | Name the profiles by UI name (from CLAUDE.md: use UI names not dataset names) |
| Changelog / version history link | Shows the plugin is actively maintained; builds trust for open-source | LOW | Link to GitHub releases or a /changelog page |
| Patreon CTA (secondary) | Converts interested users into supporters to fund next tools | LOW | Secondary button or banner below the fold — not competing with the download CTA |
| OS-adaptive download button | Surge XT does this — shows Windows/Mac installer based on user's OS | MEDIUM | Nice DX touch; requires simple JS UA detection |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Email gate on download | "Build the list" | For a free GPL plugin, gating destroys adoption — spatial audio community is small and will share links anyway; creates friction with no upside | Offer optional email capture (Tally survey) as a secondary CTA: "Stay updated" — never required |
| Flashy animated hero | Looks impressive | Animations that auto-play or block content annoy producers who came to download, not to be wowed; also causes accessibility problems | Static hero with one great screenshot; video demo clearly opt-in |
| Forum / comment section on landing page | Community engagement | On a personal landing page this requires moderation, adds tech debt, and rarely stays active past launch week | Link to GitHub Discussions or a Discord if community is desired |
| Pricing tiers on the main page | Clarify commercial vs free | GPL-3.0 = no commercial tier; showing a "free vs pro" grid where everything is free reads as spam | Single tier: free. Patreon is for support, not for features |
| Newsletter subscription as primary CTA | "Grow the list" | Competing with the download CTA splits attention; the user came to download the plugin | Download is primary CTA, newsletter is secondary — separate page section or footer |

---

### Patreon Page

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Profile photo + real name | Patrons support people, not anonymous developers | LOW | Personal photo; consistent with LinkedIn/SML presence |
| Cover image showing plugin or studio context | Visual identity; makes the page feel real | LOW | Can reuse a hero asset from the landing page |
| About section: who you are + what the pipeline is | Patrons need to know what they're funding — "pipeline of spatial audio tools" is the pitch | LOW | 2-3 paragraphs: who you are, what you've built, what's next |
| At least one tier with a named reward | Tiers with concrete perks convert better than open-ended "support me" | LOW | Even one $5 tier with "early access to new tools + dev log" works |
| Welcome post explaining what patrons get | Patreon surfaces this to new subscribers — sets expectations | LOW | Pin it; write it before launching |
| Featured post (audio demo or dev video) | Patreon's new design surfaces featured content prominently | LOW | Embed the same audio demo from the landing page |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| "Spatial audio pipeline" framing | Not just funding one plugin — funding a series of tools for an underserved niche | LOW | Reference OSD as proof you ship; describe the next 1-2 tools vaguely enough to stay flexible |
| Dev log posts (monthly cadence) | The Audio Programmer's Patreon succeeds on transparency — showing the work builds loyalty | LOW | Even 200-word posts with a screenshot keep patrons engaged; consistency > length |
| Early beta access as a perk | Producers love being first; early access has near-zero marginal cost | LOW | Offer to beta tier ($10+); aligns with "building in public" narrative |
| Discord or community thread access as perk | Community is a sticky retention mechanism | MEDIUM | Only add if you'll actually engage there; empty Discords are worse than none |
| Naming in plugin credits as perk | Low cost, high perceived value for top-tier patrons | LOW | "Listed as supporter in plugin About screen" — easy to fulfill |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Many tiers (5+) | "Cover all price points" | Analysis paralysis for new patrons; too many tiers is a known Patreon anti-pattern | Start with 2-3 tiers: $3 (follow the work), $10 (beta access), $25 (named in credits) |
| Physical rewards (stickers, merch) | "Tangible value" | Fulfillment is expensive and unpredictable at low patron counts; kills time-to-create | Digital-only perks scale; physical rewards only if patron count justifies it |
| Per-creation billing | Flexibility in theory | Patrons hate surprise charges when a creator has a productive month; feels adversarial | Monthly flat billing only |
| Immediate exclusive content behind paywall | Maximize Patreon revenue from day 1 | On launch with zero patrons, this looks like nothing is there; exclusivity before trust backfires | Free content first, locked content after first 10 patrons are established |

---

### Email Capture Survey (Tally)

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Email field + submit | The whole point | LOW | Tally handles this natively |
| Confirmation / thank-you message with download link | User expects immediate access after submitting | LOW | Tally supports redirect URL or custom thank-you message |
| Clear statement of what they're signing up for | GDPR/anti-spam trust signal; reduces fake emails | LOW | "You'll get updates on new spatial audio tools from Spatial Media Lab — no spam" |
| Unsubscribe info | Legal requirement in most jurisdictions | LOW | Handled by your email platform (SML mailing list provider) |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| 1-2 optional survey questions post-submit | Segmentation data with near-zero friction cost — "What DAW do you use?" tells you where to focus support efforts | LOW | Tally supports multi-page forms; make questions optional and post-email-submit |
| Framing as "stay updated on spatial audio tools" not just "download link" | Frames the list as ongoing value, not a one-time transaction | LOW | Copy change only; sets user expectation that they'll hear from you again |
| Immediate redirect to GitHub Releases page | Fastest path to the file; no waiting for an email | LOW | Alternatively redirect to landing page /download section; avoids "where's my email" support tickets |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Required email gate on the primary download | "Build the list first" | In the free open-source audio plugin world, gating download with email destroys adoption — the plugin will show up on every free plugin aggregator site anyway | Make email optional; put Tally link as "Get updates" alongside a direct download link |
| Long survey before download | More data = better segmentation | Users abandon forms with 3+ fields before getting what they came for | Max 1 required field (email); 1-2 optional questions on a thank-you page |
| Double opt-in that delays the download | Email hygiene | Creates a friction loop: submit → wait for email → confirm → then download. Plugin downloads are not SaaS trials | Single opt-in; deliver download link immediately in thank-you redirect |

---

### Blog Post (SpatialMediaLab.org)

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Headline that names the plugin + what it does | Readers need to know immediately what this is about | LOW | "Introducing OpenSpatialDelay: A Free 3D Delay Plugin for VST3 and AU" |
| One screenshot of the UI in the post | Blog posts about plugins without images feel unfinished | LOW | Reuse landing page screenshot |
| Download / landing page link (early + end) | The whole point of the blog post is to drive traffic | LOW | Link in first paragraph and in a closing CTA block |
| Technical capabilities overview | Core audience (audio/spatial developers) expect spec-level detail | LOW | Algorithms, HRTF profiles, platform support — can be bullets |
| Why it's GPL / open-source | Relevant to the SML audience; distinguishes from commercial plugins | LOW | One paragraph; frame as intentional community decision |
| Author byline | Establishes credibility and personal brand | LOW | Your name + "founder, Spatial Media Lab" |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| "Building in public" narrative thread | SML audience cares about the development journey, not just the artifact | LOW | Include 1-2 technical decisions made during development (e.g., choosing HRTF convolution approach, Doppler fix) — creates authentic developer voice |
| Embedded audio demo | Readers can hear the plugin without leaving the blog | MEDIUM | SoundCloud embed or Bandcamp clip; 30 seconds is enough |
| Patreon callout section | Blog post readers who connect with the story are warm Patreon prospects | LOW | One paragraph + link: "If you want to support more tools like this, consider backing on Patreon" |
| Link to GitHub with contributor invite | Signals openness; some SML readers are developers who might contribute | LOW | "Source on GitHub — contributions welcome" |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Full technical deep-dive (all 175KB of PluginProcessor.cpp context) | Thorough documentation | Blog posts with excessive technical depth lose the non-developer SML audience and the post becomes a reference doc, not an announcement | Link to the architecture docs on GitHub for the deep-dive; keep post accessible |
| Changelog-style list of every feature | Complete coverage | Launches read as dry spec sheets when they list every parameter — kills narrative momentum | Pick 3-5 headline features that tell a story; link to full feature list on landing page |

---

### Newsletter Email (SML Mailing List)

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Subject line that names the plugin | Email subject determines open rate; vague subjects get deleted | LOW | "We just released a free 3D spatial delay plugin" outperforms "Big news from SML" |
| One clear CTA button linking to the landing page or download | Email readers act on one clear action | LOW | "Download Free" or "Get OpenSpatialDelay" |
| 2-3 sentences of context (what it is, why it matters) | Subscribers need enough context to act — not a wall of text | LOW | Newsletter is a teaser, not the full blog post |
| Unsubscribe link | Legal requirement | LOW | Handled by your email platform |
| Plain-text fallback readable | Some subscribers use text-only email clients | LOW | Write copy that reads cleanly without HTML formatting |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Personal tone ("I built this because...") | Newsletters from a person outperform newsletters from a brand for small lists | LOW | One sentence of personal context goes a long way — "I spent 6 months building this because I couldn't find a delay that let me place echoes in real 3D space" |
| Patreon mention (secondary CTA) | Warm list readers are the highest-probability Patreon converts | LOW | P.S. line: "If you want to fund the next one, I'm on Patreon" — P.S. lines have unusually high read rates |
| Preview of next tool (tease) | Creates anticipation and reason to stay subscribed | LOW | One vague line about what's in the spatial audio pipeline |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| HTML-heavy newsletter with banners and multi-column layout | "Looks professional" | For a one-person developer mailing list, heavy HTML templates read as impersonal marketing, not genuine communication | Plain or lightly styled single-column email; personal voice > design polish |
| Multiple CTAs (download + Patreon + blog + social) | "Cover all channels" | Multiple competing CTAs in email reduce click-through on all of them | Primary CTA: download link. Secondary: Patreon P.S. Everything else is a footer link |

---

### LinkedIn Announcement Post

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Hook in first 1-2 lines (before "see more" truncation) | LinkedIn truncates at ~3 lines — the hook determines whether anyone reads the rest | LOW | Lead with the result or the story, not "I'm excited to announce..." |
| Plugin name + what it does | Readers need to know the subject immediately | LOW | "OpenSpatialDelay is a free 3D spatial delay plugin for VST3/AU" |
| Screenshot or short demo clip attached | LinkedIn's algorithm heavily favors posts with media | LOW | Single UI screenshot performs well; short Reel-style video clip performs better |
| Download/landing page link | Drive traffic | LOW | In the post body or first comment (LinkedIn reportedly deprioritizes posts with external links in body — test putting link in first comment) |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| "I built this in public" framing | LinkedIn's algorithm rewards personal narrative and vulnerability; "I spent months on this" outperforms press release tone | LOW | Share one specific challenge overcome during development (the Doppler fix is a compelling technical story) |
| 3-5 hashtags targeting audio/music production communities | Extends reach beyond direct network | LOW | #AudioDevelopment #SpatialAudio #JUCE #VST #OpenSource |
| Carousel post version (if doing multiple posts) | Carousel format has highest engagement rate on LinkedIn for multi-image content | MEDIUM | Slides: 1. What is it / 2. How 3D positioning works / 3. HRTF profiles / 4. Download CTA |
| Tag relevant community accounts (ADC, JUCE, SML) | Expands reach via notification to tagged accounts | LOW | Only tag accounts that are genuinely relevant — over-tagging reads as spam |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| External link in post body | "Drive traffic directly" | LinkedIn algorithm reportedly suppresses posts with external links in the body text | Put the link in the first comment, reference "link in comments" in the post |
| "Like and share if you find this useful" engagement bait | Boost algorithm reach | LinkedIn has been actively reducing reach of engagement bait posts since 2024 | Write content that naturally earns shares by being genuinely interesting |
| Formal press release tone | "Looks professional" | LinkedIn users respond to people, not PR — especially for indie developer posts | First-person, conversational narrative performs consistently better |

---

### Instagram Announcement Post

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Visual-first: plugin UI screenshot or short Reel | Instagram is visual; text-only posts are ignored | LOW | A Reel of the 3D trajectory moving in real-time to music is a natural fit |
| Caption with hook in first line | Caption truncates after 1-2 lines; first line determines "more" clicks | LOW | "Each echo lives in a different place in 3D space." beats "Excited to announce..." |
| Link-in-bio reference | Instagram doesn't allow body links; standard CTA | LOW | Update bio link to landing page; mention "link in bio" in caption |
| Relevant hashtags | Discoverability for audio/producer community | LOW | #VST #AudioPlugin #SpatialAudio #MusicProduction #OpenSource #JUCE — mix niche and broad |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Reel showing 3D trajectory animation in real-time | The 3D spatial concept is invisible in screenshots — motion shows it naturally | MEDIUM | Screen recording of the trajectory UI with audio playing through headphones; 15-30 seconds |
| Before/after audio demo via Reel | "Dry signal vs with OSD" is an instantly understandable demo format | MEDIUM | Requires a short produced example; pairs perfectly with the Reel format |
| Behind-the-scenes dev story in caption | Instagram producers respond to authentic creator stories; builds emotional connection before they download | LOW | One specific detail from development (e.g., "I kept hearing this artifact every time I moved the echo — spent a week tracking it down") |
| Story with swipe-up / link sticker for direct conversion | Stories with link stickers have measurable conversion to landing page | LOW | Post a Story with the plugin screenshot + link sticker on launch day |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Static infographic with feature list | "Tell them everything" | Feature-list infographics are low-engagement on Instagram — producers don't come to IG to read specs | Demo video or audio-visual clip; save the spec list for the landing page |
| Multiple simultaneous posts on launch day | "Maximize coverage" | Posting 3+ times in a day reads as spam and Instagram's algorithm may throttle reach | 1 Reel + 1 Story on launch day; follow-up posts 2-3 days later |
| Link in caption | Convenience | Instagram captions don't render clickable links; wastes copy space | Always "link in bio" only |

---

### Demo Video / Audio Content

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Headphone listening call-out ("best experienced with headphones") | Binaural demos require headphones — without the call-out, listeners on speakers will think the plugin is broken | LOW | Text overlay on video or spoken intro |
| Audible demonstration of spatial positioning | The core value prop must be heard, not described | MEDIUM | Move an echo in 3D while music plays — the ear should track the movement |
| Brief feature callout (overlay text or voiceover) | Viewers want to understand what they just heard | LOW | "3D positioning, 7 spatialization algorithms, 6 HRTF profiles" as lower-thirds |
| Duration 60-120 seconds max for social | Longer = drop-off on social platforms | LOW | Full walkthrough can be longer on YouTube; social cuts should be 30-60s |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Multiple spatial movements in a musical context (not just a test tone) | Test tones demonstrate the tech; music demonstrates the art — music demos convert better | MEDIUM | Use a drum loop or ambient pad to make the spatial movement musical; test tones feel clinical |
| Side-by-side comparison: no delay vs OSD binaural delay | Immediately answers "why does this matter?" without requiring any explanation | MEDIUM | Split-screen or A/B audio cuts; very effective for social |
| Trajectory animation screen recording | Shows the "why" of the plugin interface — 3D path visualization is visually compelling | LOW | Screen record with DAW timeline playing; overlay a camera of headphones optional |
| Short version (30s) + long version (2-3min walkthrough) | Short for social, long for YouTube/landing page | MEDIUM | Two edits from the same session; short is clips from long |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Extensive DAW tutorial showing every parameter | "Teach them to use it" | Tutorial-length content is appropriate for existing users, not a launch announcement; intro content buries the lead | 30-second "wow moment" demo for launch; link to tutorial content post-launch |
| Speaker listening demo for a binaural plugin | "Show more people" | Binaural demos on speakers sound flat and identical to stereo — risks making the plugin seem unimpressive | Explicitly call out headphone requirement; create a separate stereo version demo for speaker audiences |

---

### GPL-3.0 License Transition Announcement

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Clear statement that the commercial license is dropped | Any prior awareness of dual-licensing needs resolution | LOW | One sentence: "OpenSpatialDelay is now exclusively GPL-3.0 — free to use, modify, and redistribute" |
| What GPL-3.0 means for users in plain English | Most producers don't know what GPL means | LOW | "You can use it in your projects, modify it, and share it. If you redistribute modified versions, you must keep them GPL-3.0." |
| GitHub link showing the LICENSE file | Verifiable proof of the license | LOW | Link to the file directly |
| Note on JUCE AGPL compliance (briefly) | Technically relevant for the developer audience | LOW | "Uses JUCE under AGPL; full attribution in the repository" — keep brief |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Long legal explanation of GPL | "Be thorough" | Producers abandon legal walls of text; creates more confusion than clarity | 3-4 sentence plain English summary + link to GPL FAQ for those who want depth |
| Explaining what you can't do under GPL prominently | "Legal protection" | Leading with restrictions frames the license as adversarial; most users will never trigger the copyleft requirements | Lead with what users can do (everything, for free) — restrictions are a footnote |

---

### Documentation Cleanup (Public Readiness)

#### Table Stakes (Users Expect These)

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| README with install instructions for both macOS and Windows | First thing GitHub visitors read | LOW | VST3 and AU paths, minimum OS versions |
| Getting Started section (first 5 minutes UX) | New users need a path from install to "aha moment" | LOW | How to load the plugin, place an object, hear the effect |
| Known issues / limitations section | Sets honest expectations; reduces support burden | LOW | Platform-specific notes (e.g., AU cache behavior) without exposing internal debugging details |
| License block in README | GPL requires it | LOW | SPDX identifier + link to LICENSE file |
| Screenshot in README | GitHub README with no screenshot looks abandoned | LOW | One in-situ screenshot with a DAW visible |

#### Differentiators (Competitive Advantage)

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Contribution guide (CONTRIBUTING.md) | Invites open-source contributors; signals the project is maintained | LOW | Even a one-page "how to build from source + how to submit a PR" doc serves this purpose |
| Headphone-required callout in README | Unique to binaural plugins; reduces support tickets from confused users | LOW | One prominent note at the top of the README |
| Prebuilt binary links in README (not just build-from-source) | Most interested users are producers, not developers — they will not build from source | LOW | Link directly to the GitHub Releases page for the latest binaries |

#### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Exhaustive API/developer documentation at launch | "Complete docs" | Over-documentation at launch delays it; most users will never read it | Minimal docs that get a user to a working plugin; expand docs iteratively based on actual questions |
| Changelog going back to v0.1 in the README | "Historical completeness" | Scrolling through 169 patches in a README is unusable | Link to GitHub Releases for full history; README shows only the current version summary |

---

## Feature Dependencies

```
Landing Page
    └──requires──> Audio demo content (screenshots + audio clip)
    └──requires──> GitHub repo is public with LICENSE file
    └──requires──> Download binaries exist (GitHub Releases)
    └──enhances──> Email capture (Tally form) [secondary CTA]
    └──enhances──> Patreon page [secondary CTA]

Email Capture (Tally)
    └──requires──> Landing page (or standalone URL to embed)
    └──requires──> SML mailing list is set up to receive new subscribers
    └──enhances──> Newsletter email [feeds the list]

Demo Video / Audio Content
    └──required by──> Landing page (above-fold demo)
    └──required by──> Instagram Reel (social post)
    └──required by──> Newsletter email (embedded or linked)
    └──enhances──> Blog post (embed)

Blog Post (SML)
    └──requires──> Landing page is live (links there)
    └──requires──> Demo content exists (screenshot minimum)
    └──enhances──> Newsletter email [blog post IS the email content]

Newsletter Email
    └──requires──> SML mailing list platform configured
    └──requires──> Landing page is live
    └──depends on──> Blog post (can link or summarize it)

LinkedIn + Instagram Posts
    └──requires──> Landing page is live (link target)
    └──requires──> At least one visual asset (screenshot or Reel)
    └──enhances──> Landing page traffic

Patreon Page
    └──requires──> Landing page is live (links there and back)
    └──independent of──> Email capture (separate funnel)

GPL Transition Announcement
    └──requires──> LICENSE file updated in repo
    └──requires──> README updated
    └──part of──> Landing page (license section)
    └──part of──> Blog post (license section)

Documentation Cleanup
    └──blocks──> All public launch components (repo must be public-ready before linking to it)
    └──required by──> Landing page GitHub link
    └──required by──> Blog post source link
```

### Dependency Notes

- **Documentation Cleanup blocks everything:** The GitHub repo must be public-ready before any launch component can link to it. This is the first dependency to resolve.
- **Demo content enables landing page, social, and email:** Without an audio demo clip and at least one clean screenshot, the landing page, Instagram post, and newsletter email are all weakened. Create demo content before building other launch assets.
- **Landing page is the hub:** Every other launch component (LinkedIn, Instagram, newsletter, blog) points to the landing page as the conversion endpoint. Build the landing page before publishing social or email content.
- **Tally email capture is independent of Patreon:** These are separate conversion funnels (list vs. patron). They can be built in parallel and cross-linked, but neither depends on the other.
- **Blog post and newsletter email are coupled:** The blog post can serve as the editorial source for the newsletter email — write the post first, then condense to newsletter format.

---

## MVP Definition

### Launch With (v1)

Minimum required for a credible public launch.

- [ ] Documentation cleanup (README, LICENSE, install instructions) — unlocks all GitHub links
- [ ] Landing page with hero, audio demo embed, download CTA, GitHub link, license statement
- [ ] Demo audio clip (30s binaural demo for embedding) — required by landing page and social
- [ ] One clean plugin UI screenshot — required by blog, social, landing page
- [ ] GPL-3.0 license transition complete in repo — required before any public link to GitHub
- [ ] Blog post on SpatialMediaLab.org — primary launch editorial; drives SEO and newsletter
- [ ] Newsletter email to SML mailing list — activates existing audience at launch
- [ ] LinkedIn announcement post — professional network reach
- [ ] KVR Audio product listing — primary audio plugin directory; major free plugin discovery channel

### Add After Validation (v1.x)

Features to add once core launch is live and getting traffic.

- [ ] Tally email capture survey — add once landing page has traffic to capture; not needed day-1 if list is already seeded via newsletter
- [ ] Patreon page — launch Patreon within 1-2 weeks of plugin launch, after initial audience response validates interest in the pipeline framing
- [ ] Instagram Reel with 3D trajectory animation — higher production effort; can follow 1-3 days after initial launch
- [ ] Full demo video walkthrough (2-3 min, YouTube) — tutorial-level content; more valuable after users are downloading and asking "how do I use this?"
- [ ] Product Hunt listing — requires coordinated upvote campaign; better to queue 1-2 weeks after organic launch, with a prepared supporter list

### Future Consideration (v2+)

Defer until there is a user base to justify.

- [ ] Discord server — only valuable when there are users asking questions; before that it looks abandoned
- [ ] Contribution guide (CONTRIBUTING.md) — add once a contributor actually shows up or is expected
- [ ] Multi-language landing page — only when analytics show a specific non-English-speaking audience
- [ ] Plugin tutorial video series — post-launch content strategy; not launch blocker

---

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| Documentation cleanup (README + LICENSE) | HIGH | LOW | P1 |
| Landing page (hero + download CTA + demo) | HIGH | MEDIUM | P1 |
| Demo audio clip (30s binaural) | HIGH | MEDIUM | P1 |
| Plugin UI screenshot (1 high-res) | HIGH | LOW | P1 |
| Blog post (SML) | HIGH | LOW | P1 |
| Newsletter email | HIGH | LOW | P1 |
| LinkedIn announcement | MEDIUM | LOW | P1 |
| KVR Audio listing | HIGH | LOW | P1 |
| GPL license transition (repo + comms) | HIGH | LOW | P1 |
| Patreon page | MEDIUM | LOW | P2 |
| Tally email capture | MEDIUM | LOW | P2 |
| Instagram Reel (3D trajectory) | MEDIUM | MEDIUM | P2 |
| Full YouTube walkthrough video | MEDIUM | HIGH | P3 |
| Product Hunt launch | MEDIUM | MEDIUM | P3 |
| Discord server | LOW | MEDIUM | P3 |

**Priority key:**
- P1: Must have for launch day
- P2: Launch within 1-2 weeks of initial release
- P3: Nice to have, post-launch when user base exists

---

## Competitor / Comparable Launch Analysis

| Feature | Surge XT | Vital | Odin2 | OSD Approach |
|---------|----------|-------|-------|--------------|
| Primary website | GitHub Pages, clean, CTA-focused | Dedicated product site, freemium CTA | GitHub README + KVR announcement | Personal landing page (static) with download CTA |
| Demo content | SoundCloud community playlist embeds | Video demo in hero section | KVR forum screenshots | 30s binaural audio clip + screen recording Reel |
| Email capture | None | Freemium tier captures email at registration | None | Optional Tally survey; not gated |
| Patreon / funding | Open Collective (team) | Freemium + paid tiers | None visible | Personal Patreon as spatial audio pipeline funder |
| Social announcement | Community-driven | Product launch buzz | KVR post | LinkedIn + Instagram + newsletter coordinated |
| KVR listing | Yes — critical for discovery | Yes | Yes — launch thread drove initial traction | Yes — list before or on launch day |
| GitHub transparency | Full open source, contributor guide | Not fully open source | Full open source | GPL-3.0, clean README |

Key observation: **KVR Audio is the primary discovery channel for free plugins** — Odin2's KVR launch thread drove thousands of downloads in the first week. A KVR listing and launch post is as important as the landing page for free plugin discovery.

---

## Sources

- [Surge XT homepage structure](https://surge-synthesizer.github.io/) — observed directly (HIGH confidence)
- [Bedroom Producers Blog: Surge XT launch article](https://bedroomproducersblog.com/2022/01/19/surge-xt/) — structure observed (HIGH confidence)
- [ADC 2025: Crowded Market Launch Playbook — Randy Young](https://conference.audio.dev/session/2025/crowded-market-launch-playbook) — "Replace hope with clarity" pre-launch funnel framework (MEDIUM confidence)
- [Angle Audio: Product Hunt launch post-mortem](https://medium.com/angle-audio/product-hunt-launch-ae503cf7993b) — audio product launch lessons (MEDIUM confidence)
- [KVR Audio DSP Forum: indie dev survival discussion](https://www.kvraudio.com/forum/viewtopic.php?p=9227586) — Patreon for audio developers (MEDIUM confidence)
- [Patreon tier structure best practices](https://passionfru.it/patreon-tiers-6423/) — 2-3 tier recommendation (MEDIUM confidence)
- [Gated content best practices — Zapier](https://zapier.com/blog/gated-content-best-practices/) — email capture friction (MEDIUM confidence)
- [Odin2 GitHub + KVR thread](https://github.com/TheWaveWarden/odin2) — open source plugin launch pattern (HIGH confidence)
- [iMusician: viral Reels for musicians 2026](https://imusician.pro/en/resources/blog/viral-reels-for-musicians) — Instagram Reel best practices (MEDIUM confidence)

---
*Feature research for: Open-source audio plugin launch infrastructure*
*Researched: 2026-04-14*
