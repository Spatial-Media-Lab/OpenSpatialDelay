# Pitfalls Research

**Domain:** Open-source audio plugin launch (GPL-3.0, Patreon, email capture, web, social)
**Researched:** 2026-04-14
**Confidence:** HIGH (legal), MEDIUM (platform-specific), MEDIUM (social/content)

---

## Critical Pitfalls

### Pitfall 1: Forgetting to Update Every File That References the Commercial License

**What goes wrong:**
You change the LICENSE file to GPL-3.0 only, but source file headers, README badges, the user manual legal chapter, CMakeLists.txt JUCE license identifier, the About dialog, and the website About page still say "dual-licensed" or reference commercial licensing terms. Users or contributors find contradictions and raise issues. More seriously, the dual-license header in the LICENSE file constitutes a continuing offer of commercial terms even after you intend to withdraw it.

**Why it happens:**
Developers treat the LICENSE file as the entire license change. It isn't. The license is expressed in every artefact that ships — source headers, compiled binary About dialogs, documentation PDFs, release notes, and marketing copy.

**How to avoid:**
Audit every location where the license is stated before going public. The minimum set for OSD is:
- `LICENSE` at repo root — replace dual-license header with standard GPL-3.0 boilerplate
- `CLAUDE.md` line 4 — update "Dual-licensed GPL-3.0 / Commercial" description
- `README.md` license badge and any license section
- Source file copyright headers in `Source/` — if any say "dual-licensed" or reference commercial
- `CMakeLists.txt` JUCE_DISPLAY_SPLASH_SCREEN or license tier identifier
- `docs/OpenSpatialDelay_Manual_v1.0.pdf` and `.docx` — legal notices chapter
- `agent_docs/licensing.md` — currently describes the dual-license model as active
- SML website About page and any press materials that mention commercial licensing

Grep the entire repo for "commercial license", "dual license", "dual-licensed", "spatialmedialab.org/commercial", and "commercial licensing" before making the repo public.

**Warning signs:**
- `git grep -i "commercial licen"` returns results outside of historical CHANGELOG entries
- The JUCE `CMakeLists.txt` line `JUCE_DISPLAY_SPLASH_SCREEN` is still set for a commercial-tier build

**Phase to address:**
License transition phase (before repo goes public).

---

### Pitfall 2: GPL-3.0 Plugin Loaded in Closed-Source DAW — Misrepresenting the Legal Reality

**What goes wrong:**
You either (a) warn users that they "cannot legally" load OSD in Ableton, Logic, or Pro Tools, which is technically unverified and will cause your launch post to get corrected in public, or (b) claim there is no issue at all without explaining the gray area. Either extreme damages credibility.

**Why it happens:**
The GPL's "combined work" clause is genuinely ambiguous when applied to plugin-host dynamic linking. The community has not reached consensus, and the FSF's guidance does not specifically address this scenario. Plugin API boundaries (VST3 MIT license, AU Apple framework) are the practical separator.

**Actual situation (MEDIUM confidence):**
The JUCE project's founder ("Jules") has explicitly stated he does not object to GPL JUCE code running in a proprietary DAW. The practical consensus in the JUCE/KVR community is that distributing a GPL plugin *separately* (not bundled with) a closed-source host is acceptable. The VST3 SDK moved to MIT in October 2025, which further reduces the concern at the API boundary.

**How to avoid:**
In the README and documentation, do not make definitive legal claims about host compatibility. Use: "OpenSpatialDelay is GPL-3.0. Load it in any DAW you own — the license governs source redistribution, not personal use." If you want a formal position, state that you follow the same practical approach as JUCE's own GPL users. Do not scare users with unnecessary legal warnings in the installation guide.

**Warning signs:**
- Draft README contains "you cannot use this in Logic Pro" or similar
- LICENSE contains a custom preamble adding distribution restrictions beyond GPL-3.0

**Phase to address:**
License transition phase; README/documentation review before going public.

---

### Pitfall 3: macOS Installation Fails Silently for Most Users Because of Missing Gatekeeper Instructions

**What goes wrong:**
You ship an ad-hoc signed (not notarized) AU and VST3. On macOS 15.1+ (Sequoia), the old ctrl-click → Open workflow is gone. On Apple Silicon Macs, unsigned code doesn't execute at all. Users download the plugin, it doesn't appear in their DAW, they assume the plugin is broken and leave. This is the highest-friction moment in the entire user funnel, and it happens before the user ever hears a sound.

**Why it happens:**
Code signing / notarization is deferred post-v1.0 (correctly — it requires a paid Apple Developer account). But the README and release notes don't account for the macOS 15.1 change, which removed the ctrl-click workaround as of November 2024.

**How to avoid:**
The download/installation page must include current, macOS-version-aware instructions:
1. After copying the plugin to `~/Library/Audio/Plug-Ins/VST3/` (or AU equivalent), run in Terminal: `sudo xattr -rd com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/OpenSpatialDelay.vst3"`
2. Note explicitly: "On Apple Silicon Macs, ad-hoc signing is required. The above xattr command handles this."
3. Mention: "These steps are needed because OSD is not yet notarized by Apple. We plan to add notarization in a future release."

Verify these instructions still work on macOS 15 Sequoia before launch — the workflow changed in November 2024.

**Warning signs:**
- README installation section says "ctrl-click to open" without the xattr alternative
- No mention of Apple Silicon vs. Intel difference
- GitHub issues from beta testers saying "plugin doesn't show up"

**Phase to address:**
Documentation/distribution phase, before the Tally form goes live (since the download link points to GitHub Releases).

---

### Pitfall 4: Patreon Page Is Empty When the Launch Post Links to It

**What goes wrong:**
You link to Patreon in the launch post before the page has any content. Visitors see an empty creator page with zero posts, zero patrons, and a description that says "support me to fund development." This reads as a charity ask, not a value exchange. Conversion rate is near zero and cannot easily recover — first impressions on the page are cached socially.

**Why it happens:**
Patreon page creation is treated as "set up tiers and publish." The page launch is considered parallel to the plugin launch. In practice, the Patreon page needs content (at minimum one pinned welcome post) before any external link is sent to it.

**How to avoid:**
- Write at least two posts before publishing the Patreon page: a welcome/about post explaining the spatial audio tools pipeline roadmap, and one "exclusive early look" post (can be a roadmap draft or behind-the-scenes build log)
- Frame every tier benefit as what the patron *receives*, not what you need the money for: "Get early access to upcoming spatial audio tools and direct input on the feature roadmap" rather than "Support the development of OSD"
- The page must be published with at least 1 patron (ideally yourself on a test account, or a friend) before the launch post links to it — a page showing "0 patrons" creates a cold-start problem

**Warning signs:**
- Patreon page description contains "support me" or "help fund" language
- Page has 0 posts when you paste the URL into the launch post
- Tiers list benefits like "patron-only updates" without specifying what kind or how often

**Phase to address:**
Patreon setup phase, before any social post that links to Patreon.

---

### Pitfall 5: Email Capture Form Is Not GDPR-Compliant at Launch

**What goes wrong:**
Tally form collects email addresses without an explicit opt-in checkbox and a linked privacy policy. Under GDPR (which applies to any EU resident who fills out the form, regardless of where you are based), this is illegal. Penalties reach €20M or 4% of global annual turnover. More practically, your email service provider (Mailchimp, ConvertKit, etc.) will suspend your account if complaint rates spike because users didn't clearly consent.

**Why it happens:**
Tally's free tier makes forms trivially easy to set up. The GDPR consent fields are optional in the form builder — they require deliberate addition, not automatic inclusion. Developers who are not marketers assume "people can unsubscribe" satisfies the law. Under GDPR, it does not — consent must be obtained *before* the first email, not escaped afterward.

**How to avoid:**
The Tally form for OSD email capture must include:
1. An **unchecked checkbox** with text: "I agree to receive email updates about OpenSpatialDelay and the Spatial Media Lab spatial audio tools pipeline. You can unsubscribe at any time."
2. A link to a published privacy policy (minimum: what data is collected, who holds it, how to request deletion, and the mailing list platform used)
3. A statement clarifying what the email will be used for before the submit button

For CAN-SPAM (US): every email sent must include a physical mailing address, a clear unsubscribe mechanism, and a non-deceptive subject line. The mailing list platform handles most of this automatically, but the physical address field must be configured.

Double opt-in (a confirmation email before adding to the list) is not legally required by CAN-SPAM but is best practice and reduces spam complaints.

**Warning signs:**
- Tally form has no checkbox — just an email field and a submit button
- No privacy policy page exists when the form goes live
- You have not yet chosen a mailing list platform (Mailchimp, ConvertKit, Kit, etc.) and configured your sender address and physical address

**Phase to address:**
Email capture / mailing list phase, before the Tally form link is shared publicly.

---

### Pitfall 6: Launch Post Leads With Technology, Not With What It Sounds Like

**What goes wrong:**
The launch post on LinkedIn, KVR, or the SML blog leads with: "JUCE-based, GPL-3.0, 12 echo objects, VBAP/Ambisonics/HRTF, phase vocoder PDC compensation..." This is a technical specification sheet, not a launch post. The audience that would actually download and use the plugin (producers, sound designers) sees a wall of acronyms and moves on. Developers who understand the acronyms are not the primary audience for this tool.

**Why it happens:**
The developer wrote the plugin. The developer is most comfortable describing what was built. The audience cares about what it sounds like and what they can create with it.

**How to avoid:**
Lead with the experience, not the specification:
- "Your delays move through space around you. Each echo has its own 3D position, pitch, and trajectory." — then link to audio demo
- Lead with the demo audio or video in the first 3 seconds of any video post
- Mention GPL-3.0 and JUCE once, at the end, for the audience that cares (audio developers)
- KVR announcement: required format is product page + audio demo. A text-only post without audio will be ignored
- LinkedIn: lead with a video clip showing the spatial map moving, not a screenshot of a parameter list

**Warning signs:**
- Draft post contains "VBAP", "HRTF", "Ambisonics", "GPL" in the first paragraph
- No audio or video demo is ready before writing launch posts
- The post was written before the demo content exists

**Phase to address:**
Demo content creation phase (before any social post); post writing phase.

---

### Pitfall 7: Repo Goes Public With No CONTRIBUTING.md and No Issue Templates

**What goes wrong:**
Within 24 hours of going public, someone files a GitHub issue that is actually a feature request, or opens a PR that doesn't follow your conventions (or breaks a JUCE submodule reference). Without CONTRIBUTING.md and issue templates, you spend your launch week triaging noise instead of amplifying signal. Worse, if someone files a GPL compliance question as an issue in public, an empty response policy is visible to everyone.

**Why it happens:**
CONTRIBUTING.md and issue templates feel like boilerplate for large projects, not a solo plugin. But they become necessary the moment the repo is public because contributors don't share your mental model.

**How to avoid:**
Before going public:
- `CONTRIBUTING.md` — minimum: build instructions (cmake command from CLAUDE.md), test instructions, branch/PR policy, note that JUCE is a submodule and must be initialized, and a line stating that DSP changes are out of scope for v1.x
- `.github/ISSUE_TEMPLATE/bug_report.md` — captures DAW, OS, plugin format, reproduction steps
- `.github/ISSUE_TEMPLATE/feature_request.md` — explicitly states "please note that DSP features are roadmapped via Patreon supporter input"
- `SECURITY.md` — even a minimal one, stating how to report security issues privately

The README already exists and is strong. These files extend it into contribution infrastructure.

**Warning signs:**
- `ls .github/` returns nothing before repo goes public
- First few beta tester issues are formatted however they want, with no structure
- `git submodule` status shows JUCE uninitialized and this isn't mentioned in build instructions

**Phase to address:**
Repo readiness phase, before going public.

---

### Pitfall 8: Personal Website Rebuild Delays the Entire Launch

**What goes wrong:**
The personal website rebuild (static site with MagicUI/Impeccable) becomes the blocking dependency for the mailing list, the Patreon link, and the social posts. MagicUI requires React/Next.js, Tailwind, and TypeScript, which are non-trivial to configure and deploy correctly for the first time. The website is still not live when the launch window closes (end of April 2026) because the developer is debugging CSS animations instead of writing launch copy.

**Why it happens:**
"Website first" is the stated phasing strategy. But a full static site rebuild is a multi-week project. Rebuilding the site and launching the plugin are two separate goals that got merged into one phase.

**How to avoid:**
Decouple the personal website rebuild from the plugin launch. The plugin launch only needs:
1. A working URL that describes OSD and links to the GitHub release and Patreon
2. The Tally form to be embeddable or linkable from somewhere

This can be the existing website builder page (updated content), a single GitHub Pages HTML file, or even a Notion page with a domain redirect. The MagicUI site rebuild can be done as a separate project after launch. If you choose to build the static site, cap the time-box strictly: if it's not deployable in 2 focused days, fall back to the website builder.

**Warning signs:**
- Day 3 of the milestone and the website is still being configured locally
- Attempting to add animations before the core content (bio, OSD description, links) is written
- "I just need to fix this one CSS issue" is the reason launch posts haven't been written yet

**Phase to address:**
Website planning phase — decide static site vs. update builder before starting, not after.

---

## Technical Debt Patterns

Shortcuts that seem reasonable but create long-term problems.

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Skipping notarization | No $99/year Apple Developer fee, no CI complexity | Every macOS user must run a terminal command; Apple Silicon users may be blocked entirely; increases support burden | Acceptable for v1.0.0 launch IF instructions are crystal clear and prominently placed |
| No double opt-in on email list | Higher conversion rate (less friction) | Higher spam complaint rate; violates email provider terms; non-GDPR-compliant best practice | Never for EU audience |
| Launching Patreon without content | Gets the page live faster | Zero conversion; page looks abandoned | Never — add at least 2 posts before first public link |
| Keeping website builder for launch | No rebuild time | Limits design options; content-editing UX can be slow | Acceptable for v1.0.0 if the builder supports adding a Tally embed or link |
| Single LICENSE file change without repo-wide audit | Fast license transition | Contradictions appear throughout repo; damages credibility | Never — audit must be complete |

---

## Integration Gotchas

Common mistakes when connecting infrastructure components.

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| Tally → mailing list platform | Tally form collected emails but the Zapier/native integration to Mailchimp/ConvertKit was never configured; emails sit in Tally and never reach the list | Connect and test the integration before the form goes live: submit a test entry and verify it appears in the mailing list |
| Patreon → website | Patreon link in nav/footer points to Patreon.com root instead of your creator page URL | Always test links in a private/incognito window before publishing |
| GitHub Release → download link | The Tally form "thank you" page links to a GitHub release that doesn't exist yet (draft, not published) | Create the GitHub release in draft first, verify the download URL, then publish the form |
| Social post → demo video | Launch post links to a YouTube video that is still set to "unlisted" and you forgot to make it public | Publish the video 24h before the post; verify it plays in an incognito browser |
| README → user manual PDF | README links to `docs/OpenSpatialDelay_Manual_v1.0.pdf` which is a relative path — this 404s on GitHub if the PDF is > 100 MB and was Git-LFS tracked but LFS isn't initialized for the public | Check `git lfs ls-files` before going public; verify the PDF renders in the GitHub UI |

---

## Performance Traps

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Static site with heavy animation framework | Site loads slowly on mobile; Lighthouse score < 50; users bounce before seeing the download link | If using MagicUI/Next.js, audit Core Web Vitals before launch; animations are optional, content is not | From day one if not budgeted — image-heavy sites with unoptimized animations fail immediately on mobile |
| GitHub Releases as CDN for large binaries | macOS .pkg or .zip > 100 MB downloads slowly from GitHub Release; GitHub has bandwidth limits for large files | GitHub Releases supports files up to 2 GB with no bandwidth fees for open-source public repos — this is fine for plugin binaries | Not a problem at OSD's binary size; only an issue if you add full SOFA libraries as separate downloads |
| Tally free tier submission limits | After N submissions, new form entries are rejected silently | Tally free tier supports unlimited submissions as of 2025 — not a current concern; verify before any viral moment | Only a concern if a single post goes unexpectedly viral |

---

## Security Mistakes

| Mistake | Risk | Prevention |
|---------|------|------------|
| Credentials or API tokens accidentally committed | Exposure of mailing list API keys, Patreon OAuth tokens, or CI secrets in git history | Run `git log --all --diff-filter=A --name-only | grep -iE '\.env|secret|key|credential|token|password'` before going public (already in the release plan) |
| Email list platform API key in a public config file | Attackers can spam your list or harvest email addresses | API keys for mailing list integrations must be in environment variables or platform secrets, never in committed files |
| Privacy policy references data handling you don't actually do | Creates legal liability if your actual practice differs from stated policy | Write the privacy policy based on what you *actually* collect (email, name from Tally, IP logged by Tally) — don't copy a template without reading it |

---

## UX Pitfalls

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| Installation instructions buried in a README section at the bottom | macOS users hit Gatekeeper and abandon before finding the fix | Put the xattr command prominently, in a callout box if possible, near the download link — not at line 200 of the README |
| Tally form asks for too much information before the download | Drop-off before completing form; users expected a direct download | Ask for email only at the form step; all other survey questions (DAW, use case, how they found OSD) should be optional or on a second page |
| Patreon tiers with vague or jargon-heavy benefit descriptions | Producers don't understand what "direct feature request access" means in practice | Be specific: "$5/month — Vote on the next plugin in the Spatial Media Library pipeline and get early beta builds before public release" |
| No audio demo on the product page | Producers cannot evaluate the plugin without installing it; many won't install something they can't pre-audition | Embed a SoundCloud or YouTube clip directly on the landing page before the download button |
| KVR plugin listing has no category or wrong category | Plugin is unfindable in KVR searches | KVR "Delay" + "Spatial/3D" tags; ensure the plugin is submitted to KVR DB with correct tags, not just announced in the forum |

---

## "Looks Done But Isn't" Checklist

- [ ] **License transition:** LICENSE file changed — but `git grep -i "commercial licen"` still returns results in source files, README, or docs
- [ ] **Patreon page:** Page is published — but has 0 posts and 0 patrons when you paste the URL; description uses "support me" framing
- [ ] **Tally form:** Form is live — but has no GDPR consent checkbox, no privacy policy link, and no integration to a mailing list platform
- [ ] **GitHub repo is public:** Visible publicly — but `.github/` is empty (no issue templates, no CONTRIBUTING.md)
- [ ] **macOS install instructions:** Instructions exist — but still say "ctrl-click to open" and don't include the xattr command for macOS 15+
- [ ] **Social posts are drafted:** Copy is written — but no audio or video demo exists to embed
- [ ] **GitHub Release is published:** The release page exists — but the download link in the Tally thank-you page still points to the draft/unpublished version
- [ ] **JUCE submodule in public repo:** Repo is cloned — but `JUCE/` is empty because the submodule wasn't pushed correctly or isn't initialized; contributors can't build

---

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Commercial license references found after going public | LOW | File-by-file grep and fix; push a single commit with all corrections; add a brief note to CHANGELOG |
| Email list had no GDPR consent checkbox | HIGH | Cannot retroactively obtain consent for already-collected emails; must either delete those addresses or re-contact with a consent request email; rebuild the form correctly; add a privacy policy |
| Patreon launched empty and went cold | MEDIUM | Add content immediately; reach out to first visitors personally; repost to social with "updated page" framing |
| macOS users can't install because instructions are wrong | MEDIUM | Update README and release notes; post a pinned GitHub issue with the correct commands; most users will find it if it's pinned |
| Website rebuild blocked the launch | MEDIUM | Redirect personal domain to GitHub repo or Linktree temporarily; publish launch posts with GitHub link as primary; resume website rebuild post-launch |
| Mailing list integration not wired | LOW | Tally submissions are stored in Tally's dashboard; bulk export CSV and import to list platform; wire the integration properly before sending any emails |

---

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Incomplete license transition | License audit phase (before repo goes public) | `git grep -i "commercial licen"` returns 0 results outside CHANGELOG |
| GPL host compatibility misrepresentation | README/docs review phase | README contains no definitive claim about DAW compatibility; links to JUCE community position |
| macOS Gatekeeper blocks installation | Documentation phase (before download link goes live) | Test fresh install on macOS 15 Sequoia following only the README instructions |
| Patreon page empty at launch | Patreon setup phase | Page has ≥2 posts and benefit language describes patron value, not creator need |
| Tally form not GDPR-compliant | Email capture setup phase | Form has unchecked consent checkbox, privacy policy link, and verified integration to mailing list platform |
| Launch post is a spec sheet | Demo content phase (before post is written) | Audio or video demo is embedded in the first screen of every launch post |
| Repo public with no CONTRIBUTING.md | Repo readiness phase | `ls .github/ISSUE_TEMPLATE/` returns at least bug_report.md; CONTRIBUTING.md exists at root |
| Website rebuild delays launch | Website planning phase (day 1 of milestone) | Decision made: static rebuild (time-boxed) or builder update; commitment documented in PROJECT.md |

---

## Sources

- [JUCE Forum: GPL audio plugins incompatible with closed-source hosts?](https://forum.juce.com/t/gpl-audio-plugins-incompatible-with-closed-source-hosts/22403) — community consensus on GPL/DAW compatibility
- [Tally GDPR compliance documentation](https://tally.so/help/how-to-create-a-gdpr-compliant-form) — consent checkbox requirements
- [Email Marketing Compliance 2026: GDPR, CAN-SPAM & Privacy Laws](https://www.hustlermarketing.com/email-marketing-compliance-in-2026-gdpr-can-spam-privacy-laws-explained/) — penalty amounts
- [Steinberg VST3 SDK moves to MIT license](https://www.kvraudio.com/news/steinberg-moves-vst-3-sdk-to-mit-open-source-license-asio-now-gplv3-65179) — October 2025, reduces plugin-host license friction
- [Patreon mistakes that are costing you money](https://bradguigar.substack.com/p/patreon-mistakes-that-are-costing) — "support me" framing vs. patron value
- [Apple Forces Signing of Applications in macOS Sequoia 15.1](https://hackaday.com/2024/11/01/apple-forces-the-signing-of-applications-in-macos-sequoia-15-1/) — November 2024 Gatekeeper change
- [How to run unsigned apps in macOS 15.1](https://ordonez.tv/2024/11/04/how-to-run-unsigned-apps-in-macos-15-1/) — xattr command for Sequoia
- [Audacity CLA discussion (GitHub)](https://github.com/audacity/audacity/discussions/932) — reference for why contributor agreements matter in license transitions
- [Open Source Guides: Starting a Project](https://opensource.guide/starting-a-project/) — repo readiness expectations (CONTRIBUTING.md, CODE_OF_CONDUCT.md)
- [GNU GPL FAQ on license changes](https://www.gnu.org/licenses/gpl-faq.en.html) — what must be present when distributing under GPL
- [Patreon standard platform fee for new creators (August 2025)](https://support.patreon.com/hc/en-us/articles/36426991446797-A-standard-platform-fee-for-new-creators-effective-after-August-4-2025) — 10% flat fee for new creators

---
*Pitfalls research for: open-source audio plugin launch (OSD v1.0.0)*
*Researched: 2026-04-14*
