# Sender.net Setup Walkthrough (Plan 02-05 Task 1)

**Goal:** Stand up the Sender.net form, groups, and DOI automation that Plan 02-05 depends on. Site-side code already exists on `andrewrahman-com` origin/main (`DownloadForm` on homepage, `DownloadButtons` + `/get-osd/` post-confirm page, `lib/release.ts` asset constants, `/assets/*.zip` hosting). You provide the FORM_ID, I record it here, and the production flip happens when Plan 02-04 completes (Netlify deploy of andrewrahman-com + env var `NEXT_PUBLIC_SENDER_FORM_ID` + live E2E UAT).

**Source of truth:** 02-05-PLAN.md `<task>` block as amended by the 2026-04-22 design pivot (see `<objective>` block and `02-05-SENDING-DOMAIN-RESEARCH.md §13`). This doc is the distilled in-browser checklist. If plan and doc disagree, the plan wins.

> ✅ **DOMAIN AUTH COMPLETE 2026-04-22** — Option A resolved, ns1 synced, all three Sender checks green. See `02-05-SENDING-DOMAIN-RESEARCH.md §13` for continuation session log.
>
> **Decision (2026-04-21):** Option A chosen — sender domain = `andrewrahman.com`, sender identity = `hey@andrewrahman.com` (forwarded to Gmail via ImprovMX). DNS host = Netfirms (Plan 02-04 Netlify cutover deferred).
>
> **State (2026-04-22):** ImprovMX + Sender DNS records live at Netfirms; both `ns1` and `ns2` serving new zone. Mail forwarding verified end-to-end. Sender.net: SPF + DKIM + DMARC all green; groups `osd-unconfirmed` + `osd-confirmed` created; embedded form published; FORM_ID pending capture.
>
> **Design pivot (2026-04-22):** The DOI email no longer contains download buttons. Downloads live on `andrewrahman.com/get-osd/` (post-confirm landing page with `DownloadButtons` component). Sender's post-confirm redirect URL = `https://andrewrahman.com/get-osd/`. Rationale + full revised spec in research doc §13.
>
> **Remaining work on Sender side:** Step 3 (DOI automation — revised body below), Step 4 (data region), Step 5 (self-test, optional), then email `support@sender.net` requesting account verification to unlock double opt-in.
>
> **Sender terminology correction:** Sender.net has no standalone "Add sender" screen. The from-address is configured inside each campaign/automation's Details step, not globally.

**Plan status (2026-04-22):** Domain verified. Groups + form built. DOI automation pending. Site-side code already live on `andrewrahman-com` origin/main; remaining code work = spot-check `_headers` CSP + `app/privacy/page.tsx` processor prose + Playwright spec. Production flip gated on Plan 02-04 Netlify deploy.

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

## Step 3 — DOI automation (revised 2026-04-22)

**Design note:** The DOI email is confirm-button-only. Downloads live on `andrewrahman.com/get-osd/`, not in the email. Sender's post-confirm redirect takes the subscriber there. The old version of this Step — with macOS + Windows buttons in the email body — is obsolete; see `02-05-SENDING-DOMAIN-RESEARCH.md §13` for the pivot rationale.

Left sidebar → **Automation** → **New workflow**.

**Workflow name:** `OSD DOI — confirm subscription`

**Trigger:**
- Event: `Subscriber added to group`
- Group: `osd-unconfirmed`

**Step A: Send email**

| Field | Value |
|-------|-------|
| Subject | `Confirm your subscription to OpenSpatialDelay` |
| Preheader | `One click to confirm and grab your installer.` |
| From name | `Andrew Rahman` |
| From address | `hey@andrewrahman.com` (Option A sender identity; forwarded to Gmail via ImprovMX) |
| Reply-to | `hey@andrewrahman.com` (optional; keeps replies flowing through the forwarder) |

**Body** — use Sender's block editor, paste each block verbatim:

1. **Heading block:** `You're one click away from OpenSpatialDelay.`
2. **Paragraph block:** `Click below to confirm your subscription. We'll take you straight to the download.`
3. **Button block (confirm):**
   - Label: `Confirm my subscription`
   - URL: `{$double-optin-link}` ← **paste the literal mergetag including braces and dollar sign**; Sender resolves per-subscriber
   - Style: **primary** / brand colour
4. *(Optional)* **Paragraph block (Patreon CTA):** `If you find OpenSpatialDelay useful, you can support the Spatial Media Library pipeline on [Patreon](https://patreon.com/AndrewRahman). Every tier helps keep these tools free and open-source.`
   — OK to drop entirely since `/get-osd/` already carries a Patreon CTA below the downloads. Keeping it in the email is a soft duplicate; dropping it makes the email tighter.

**No download buttons in this email.** If Sender's template pre-fills any extra blocks, delete them.

**Post-confirm redirect URL:** locate the setting in Sender's form builder (Form settings → "After confirmation redirect URL" or similar) OR in the DOI automation's confirm-action step — wherever Sender exposes it. Set it to:

```
https://andrewrahman.com/get-osd/
```

This is what ties "click confirm" to "see downloads". If you can't find this setting during the build, flag it and we'll dig into Sender's UI together.

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

## Step 5 — Self-test (revised 2026-04-22)

You can skip this during Phase 2 close-out and do it as part of the live UAT once the site is deployed. But if you want to confirm the automation works before the site exists:

1. Grab the form's **share URL** from Sender (Forms → your form → Share / Public link) OR paste the embed snippet into a local `test.html` file and open it in a browser.
2. Submit your real email.
3. Confirm in the Sender dashboard: subscriber appears in `osd-unconfirmed`.
4. Wait ≤60s. DOI email arrives at `andrewjrahman@gmail.com` via ImprovMX (`hey@andrewrahman.com` forwarder).
5. Open email. Verify it contains a single primary button labelled `Confirm my subscription`. No download buttons in the email body. (Optional: soft Patreon CTA paragraph, if kept.)
6. Click confirm. Two things must happen:
   - Subscriber moves to `osd-confirmed` in Sender dashboard.
   - Browser lands on `https://andrewrahman.com/get-osd/` (once that URL is live) OR Sender's generic "subscribed" page if the post-confirm redirect isn't configured yet. Until the site is live on Netlify, you'll hit the Netfirms placeholder at `andrewrahman.com/get-osd/` (a 404 / Netfirms parking page) — that's expected and not a bug.
7. (Deferred to live UAT) On `/get-osd/`, both macOS and Windows `Download for ...` buttons render; clicking them downloads `OpenSpatialDelay-v1.0.0-{macOS-arm64|Windows-x64}.zip` from `/assets/`.

If any step 1–6 fails before the site goes live, fix in Sender UI before replying.

---

## What to reply with

Paste these two values in one message:

```
A. FORM_ID = <value from data-sender-form-id>
B. Data region = <value or "awaiting support">
```

(macOS + Windows URLs from the old checklist are no longer needed — downloads are served from `/assets/` on the site, wired up via `lib/release.ts`. No per-email URL capture required.)

Then I'll:
1. Write `02-05-SUMMARY.md` with `plan_status: sender-setup-complete` and embed A + B
2. Update STATE/ROADMAP to reflect 02-05 as substantially complete (Sender side); flag the remaining code/deploy work (Netlify env var + Netlify deploy of andrewrahman-com + live E2E UAT) as dependent on Plan 02-04 completion
3. Leave DIST-01 in-progress (not complete) until the live E2E UAT runs against the deployed site

## Evidence screenshots

Per `feedback_evidence_artifact_ceremony.md`: **waived** by default for this plan (solo-creator context). If you want to capture them anyway (the 4 in PLAN.md artifacts list), drop them in `docs/phase-02-evidence/` — they'll be a nice-to-have, not a gate.
