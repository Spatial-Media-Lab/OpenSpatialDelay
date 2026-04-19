# Sender.net Setup Walkthrough (Plan 02-05 Task 1)

**Goal:** Stand up the Sender.net form, groups, and DOI automation that Plan 02-05 depends on. The site-side code is already done (commits `fce5663` + `a4ef090` in `andrewrahman-com` repo). You provide the FORM_ID + asset URLs, I record them here, and the actual production flip happens in Phase-3 tail plan **03-09** (Netlify env var + clear-cache deploy + live E2E UAT) once the Phase-3 website is ready and DNS flips to Netlify.

**Source of truth:** 02-05-PLAN.md `<task status="complete">` block — this doc is the distilled in-browser checklist. If the plan and this doc disagree, the plan wins.

> ⚠️ **BLOCKED 2026-04-19 — read `02-05-SENDING-DOMAIN-RESEARCH.md` before proceeding.**
>
> Sender.net gates double opt-in behind account verification, which requires a verified sending domain with working SPF/DKIM/DMARC. Research found that DNS for `spatialmedialab.org` is hosted at InterNetX AutoDNS (not jackhost), and the AutoDNS credentials are not accessible to Andrew. The research doc enumerates three options (pivot sending domain to `andrewrahman.com` + forwarder · ask Timo Bittner to add DNS records · use Sender shared domain), picks a recommended path, and provides the ordered resumption checklist. **Do not execute any step below until the sending-domain decision is made per that doc.**

**Plan status (2026-04-19):** Paused pending sending-domain decision. Sender account exists (user-created earlier). Groups/form/automation not yet built because DOI is locked behind verification. The 2026-04-17 note below ("Task 1 was erroneously marked complete") remains accurate for historical context.

**Plan status (2026-04-17):** Sender account exists (user-created earlier). Nothing else has been done. Task 1 was erroneously marked complete in a prior session — we're now doing the actual dashboard work.

---

## Prereqs

- Signed in at https://www.sender.net under the email you'll use as the data-controller contact in the privacy policy (same address as the Andrew Rahman controller per D-16).
- Free Forever plan (no card required).
- If the UI shows a "Start 14-day trial" prompt, dismiss it — free plan covers launch scale (2,500 subs / 15,000 emails/month).

---

## Step 1 — Two groups

Left sidebar → **Subscribers** → **Groups** (sometimes called "Lists") → **New group**.

Create **two**:

| Name | Description |
|------|-------------|
| `osd-unconfirmed` | DOI holding group |
| `osd-confirmed` | Confirmed OSD newsletter subscribers |

(Exact casing matters — automation references these by name.)

**What to save:** nothing — the group names are fixed.

---

## Step 2 — Embedded signup form

Left sidebar → **Forms** → **New form** → pick **Embedded form** template.

**Form settings:**
- Title: `Get OpenSpatialDelay`
- Target group: `osd-unconfirmed` (NOT confirmed — DOI moves them after click)

**Fields** (delete any default extras; keep one):
| Field | Type | Label | Placeholder | Required |
|-------|------|-------|-------------|----------|
| Email | Email | `Email address` | `you@example.com` | Yes |

**Below the email field**, add a **Text block** with this copy **verbatim** (Markdown link renders clickable in Sender's builder):

```
By subscribing, you agree to receive occasional updates about OpenSpatialDelay and other Spatial Media Library tools. See our [Privacy policy](https://andrewrahman.com/privacy).
```

**Submit button label:** `Get the download`

**After-submit view:** keep the default success message ("Check your email to confirm your subscription") — preview to confirm it shows.

**Publish** the form. On the Publish / Embed code screen you'll see two `<script>` blocks. The second one contains `data-sender-form-id="XXXX"`.

**Save to reply** ▸ **A. FORM_ID** = the XXXX value.

---

## Step 3 — DOI automation (the core of Option A)

Left sidebar → **Automation** → **New workflow**.

**Workflow name:** `OSD DOI — confirm + deliver download`

**Trigger:**
- Event: `Subscriber added to group`
- Group: `osd-unconfirmed`

**Step A: Send email**

| Field | Value |
|-------|-------|
| Subject | `Confirm your subscription and download OpenSpatialDelay` |
| Preheader | `One click to confirm and grab your installer.` |
| From name | `Andrew Rahman` |
| From address | your controller email (Step 0) |

**Body** — use Sender's block editor, paste each block verbatim:

1. **Heading block:** `You're one click away from OpenSpatialDelay.`
2. **Paragraph block:** `Thanks for signing up. Click below to confirm your subscription, then grab your download for macOS or Windows.`
3. **Button block #1 (confirm):**
   - Label: `Confirm my subscription`
   - URL: `{$double-optin-link}` ← **paste the literal mergetag including braces and dollar sign**; Sender resolves per-subscriber
   - Style: **primary** / brand colour
4. **Paragraph block:** `Your downloads:`
5. **Button block #2 (macOS):**
   - Label: `Download for macOS`
   - URL: `https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/download/v1.0.0/<exact-macOS-filename>`
   - Style: secondary
6. **Button block #3 (Windows):**
   - Label: `Download for Windows`
   - URL: `https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/download/v1.0.0/<exact-Windows-filename>`
   - Style: secondary
7. **Paragraph block (Patreon CTA):** `If you find OpenSpatialDelay useful, you can support the Spatial Media Library pipeline on [Patreon](https://patreon.com/AndrewRahman). Every tier helps keep these tools free and open-source.`

**Exact asset filenames:** open https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0 in a new tab, copy the precise filenames from the Assets list. Do not guess. Plan expects `.pkg` for macOS and `.exe` for Windows but the real release may use `.dmg` or `.msi`.

**Save to reply** ▸ **B. macOS URL** and ▸ **C. Windows URL**.

**Step B: Wait** → `1 minute` (gives subscriber time to click before condition fires)

**Step C: Condition**
- Type: `If link was clicked in previous email`
- Link: `{$double-optin-link}`

**On Yes branch:** `Action` → `Move subscriber to group` → `osd-confirmed`

**On No branch:** leave empty (subscribers stay in unconfirmed; can prune later)

**Save workflow. Toggle to ACTIVE.** A paused automation does nothing.

---

## Step 4 — Data region check

Account settings (top-right avatar → Settings or Profile) → look for **"Data region"** or **"Data processing region"**.

**Save to reply** ▸ **D. Data region**:
- If visible and = `EU (Lithuania)`: perfect — privacy policy is already accurate.
- If visible and ≠ EU: reply with the actual value; privacy policy needs an amendment (DIST-01 gate).
- If NOT visible in UI: email Sender support: *"For a GDPR privacy policy I need to document where personal data is processed. What region/country hosts subscriber data for my account?"* Reply `awaiting support` + forward the reply when it arrives.

---

## Step 5 — Self-test (optional now; can defer to Phase-3 tail UAT)

You can skip this during Phase 2 close-out and do it as part of Plan 03-09 once the production site is live. But if you want to confirm the automation works before the site exists:

1. Grab the form's **share URL** from Sender (Forms → your form → Share / Public link) OR paste the embed snippet into a local `test.html` file and open it in a browser.
2. Submit your real email.
3. Confirm in the Sender dashboard: subscriber appears in `osd-unconfirmed`.
4. Wait ≤60s. DOI email arrives.
5. Open email. Verify four elements present in order: confirm button, macOS button, Windows button, Patreon CTA.
6. Click confirm. Subscriber moves to `osd-confirmed` in dashboard.
7. Click each download button. Correct installer downloads.

If any step fails, fix in Sender UI before replying.

---

## What to reply with

Paste these four values in one message:

```
A. FORM_ID = <value from data-sender-form-id>
B. macOS URL = https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/download/v1.0.0/<filename>
C. Windows URL = https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/download/v1.0.0/<filename>
D. Data region = <value or "awaiting support">
```

Then I'll:
1. Write `02-05-SUMMARY.md` with `plan_status: code-complete-UAT-deferred` and embed A–D
2. Create the Phase-3 tail plan `03-09-PLAN.md` that will, when the site is ready, set the Netlify env var + clear-cache deploy + live E2E UAT + flip DIST-01 to complete
3. Update STATE/ROADMAP to reflect 02-05 as substantially complete
4. Leave DIST-01 in-progress (not complete) until 03-09 runs

## Evidence screenshots

Per `feedback_evidence_artifact_ceremony.md`: **waived** by default for this plan (solo-creator context). If you want to capture them anyway (the 4 in PLAN.md artifacts list), drop them in `docs/phase-02-evidence/` — they'll be a nice-to-have, not a gate.
