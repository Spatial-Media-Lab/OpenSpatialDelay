# Plan 02-05 — Sending-Domain Blocker & Options

**Date:** 2026-04-19
**Status:** Plan 02-05 paused pending decision on sending domain.
**Blocker:** No direct DNS access to `spatialmedialab.org`; Sender.net requires SPF/DKIM/DMARC on the sending domain before double opt-in unlocks.

---

## 1 — What went wrong

Plan 02-05 assumed the Sender.net from-address would be `andrew@spatialmedialab.org`. To authenticate outbound mail for that domain, Sender.net requires three DNS records (SPF, DKIM CNAME, DMARC TXT) on `spatialmedialab.org`.

**Finding (2026-04-19):** DNS for `spatialmedialab.org` is **not** managed at jackhost.net where the website is hosted. The WHOIS shows:

```
Registrar:    InterNetX GmbH
Nameservers:  a.ns14.net / b.ns14.net / c.ns14.net / d.ns14.net
```

Those are **InterNetX AutoDNS** nameservers. The jackhost.net Plesk panel shows no DNS zone editor for this domain — only WHOIS info and webhosting settings — because DNS was never delegated to jackhost.

The Plesk `Systembenutzer` is `timobittner.de`, suggesting Timo Bittner (SML co-founder) registered the domain at InterNetX in 2018 and holds the AutoDNS credentials.

**Sender.net's blocker chain:**
1. Double opt-in feature is gated behind account verification.
2. Account verification requires a verified sending domain with working SPF/DKIM/DMARC.
3. We cannot add those records to `spatialmedialab.org` without AutoDNS access.

---

## 2 — Domains and state summary

| Domain | Registrar | DNS host | Who controls DNS | Website | Email |
|--------|-----------|----------|------------------|---------|-------|
| `spatialmedialab.org` | InterNetX GmbH | InterNetX AutoDNS (`*.ns14.net`) | Timo Bittner (presumed) | jackhost.net (Plesk, WordPress) | existing via jackhost; SPF: `v=spf1 a include:spf.jackhost.net -all` |
| `andrewrahman.com` | (Andrew's registrar — unconfirmed) | Currently Weebly; migrating to Netlify | Andrew | Weebly → Netlify (in progress, 2026-04-19) | **none** — domain only, no mail server |

---

## 3 — Three options considered

### Option A — Pivot sending domain to `andrewrahman.com` *(recommended)*

**Summary:** Use `andrew@andrewrahman.com` (or similar local-part) as the Sender.net from-address instead of `andrew@spatialmedialab.org`.

**Why it's actually better, not a compromise:**
- Matches `[Phase 02] D-16`: *Data Controller = Andrew Rahman (natural person), not SML — privacy policy governs personal newsletter; SML boundary is plugin-side only*.
- Signup form is on `andrewrahman.com` — same-domain sender gives stronger trust/deliverability signals.
- Removes Timo from the newsletter infrastructure dependency chain.
- SML's domain stays reserved for actual SML org comms later.

**Problem:** `andrewrahman.com` has no email infrastructure — it's a domain-only setup. **Solved by using a forwarder** (see section 4).

### Option B — Ask Timo to add the DNS records on `spatialmedialab.org`

**Summary:** One-time ask. Paste Timo the three exact values from Sender.net's panel; he clicks Save in AutoDNS.

**Values Timo would need:**
```
1. EDIT existing SPF TXT on @:
   Current: v=spf1 a include:spf.jackhost.net -all
   New:     v=spf1 a include:spf.jackhost.net include:sender.net ~all
   (Single SPF record only — must edit, not duplicate.)

2. ADD CNAME:
   Host:   sender._domainkey
   Target: (from Sender panel clipboard icon — account-unique)

3. ADD TXT:
   Host:   _dmarc
   Value:  v=DMARC1; p=none;
```

**Trade-offs:** Works if Timo is responsive. Leaves a Timo-dependency for any future DNS changes (DMARC upgrades, DKIM rotations, Sender domain changes).

### Option C — Use Sender's shared/provider-owned from-address

**Summary:** Send from a `@something.sender.net` or provider-branded address.

**Verdict: avoid.** Gmail/Yahoo sender requirements (April 2024) enforce DMARC alignment; shared-domain sending produces poor deliverability and often spam-bucketing. Not suitable for launch.

---

## 4 — Why a forwarder is enough for `andrewrahman.com`

You don't need a mail server to use `andrew@andrewrahman.com` as a sender identity. Sender.net only needs you to *receive* mail there for:

1. One-time sender-identity verification email
2. DMARC aggregate reports (monthly-ish, machine-readable)
3. Occasional subscriber replies

**All outbound newsletter sending happens from Sender's servers, not yours.** A forwarder — a free service that catches inbound mail and relays it to your existing Gmail — covers the three reasons above.

---

## 5 — Forwarder comparison

| Service | Cost | Setup | Lock-in | Notes |
|---------|------|-------|---------|-------|
| **ImprovMX** | Free forever | Add 2 MX + 1 TXT DNS records | None | **Recommended default.** Works with any DNS host. |
| **Cloudflare Email Routing** | Free | Requires moving DNS to Cloudflare | Must use Cloudflare nameservers | Only if you were switching to Cloudflare anyway |
| **Forwardemail.net** | Free | Add 3 DNS records | None | Open-source alternative to ImprovMX |
| **Fastmail** | ~$3/mo | Add MX records | Easy to leave | Use if you want a real inbox, not just forwarding |
| **Google Workspace** | ~$6/mo | Add MX records | Moderate | Use if you want @andrewrahman.com in the Gmail app |

---

## 6 — Recommended end-state DNS on `andrewrahman.com`

After the Netlify DNS migration completes and both ImprovMX + Sender are configured, the zone should look approximately:

```
Type   Host                Value
----   ----                -----
A/CNAME records for the Netlify site (already in place)
MX     @                   mx1.improvmx.com   (priority 10)
MX     @                   mx2.improvmx.com   (priority 20)
TXT    @                   v=spf1 include:spf.improvmx.com include:sender.net ~all
CNAME  sender._domainkey   (Sender's per-domain target — copy from Sender panel)
TXT    _dmarc              v=DMARC1; p=none; rua=mailto:andrew@andrewrahman.com
```

**Key merge:** SPF record must remain **single**. Both ImprovMX and Sender's includes live in one `v=spf1 ... ~all` TXT record.

---

## 7 — Why NOT send from `andrewjrahman@gmail.com`

Gmail's DMARC policy has been `p=reject` since 2024. If Sender.net tried to send mail with `From: andrewjrahman@gmail.com`, Gmail's receiving servers see that Sender's IPs aren't authorized for `gmail.com` and reject the message. Sender.net will refuse to verify a `gmail.com` sender identity for this reason. You must use a domain you control.

---

## 8 — Order of operations (when resuming, assuming Option A)

### Prerequisite diagnostic — run first

```bash
dig +short NS andrewrahman.com
```

- Netlify nameservers (e.g. `dns1.p01.nsone.net`) → DNS is already on Netlify; proceed.
- Weebly-branded nameservers → **wait** for Netlify migration to complete; otherwise records added now will be wiped when nameservers flip.
- Anything else → diagnose before adding records.

### Steps once DNS is stable on Netlify

1. **Netlify migration complete** (separate task — verify site loads + DNS resolves correctly).
2. **ImprovMX setup:**
   - Sign up at <https://improvmx.com>, add `andrewrahman.com`.
   - Create alias `andrew@andrewrahman.com` → `andrewjrahman@gmail.com`.
   - Add 2 MX + 1 TXT record in Netlify DNS panel (app.netlify.com → Domains → andrewrahman.com → DNS).
   - Click **Verify** in ImprovMX → green check.
   - Test: email `andrew@andrewrahman.com` from your phone → should arrive in Gmail within ~30s.
3. **Sender.net — add domain:**
   - Delete `spatialmedialab.org` from Sender's Domains list.
   - Add new domain `andrewrahman.com`.
   - Click **Set up DNS records** → copy the three per-domain values shown.
4. **Add Sender's records to Netlify DNS:**
   - **EDIT** the ImprovMX SPF record to merge in `include:sender.net` (single record only).
   - **ADD** CNAME `sender._domainkey` → Sender's target.
   - **ADD** TXT `_dmarc` → `v=DMARC1; p=none; rua=mailto:andrew@andrewrahman.com`.
5. **Verify in Sender** → click Check DNS Records → three green ticks.
6. **Sender identity:** add sender `andrew@andrewrahman.com`; confirm via the verification email (arrives in Gmail via ImprovMX).
7. **Request account verification from Sender support** — email `support@sender.net` (template in `02-05-SENDER-SETUP.md` — update domain + sender address to `andrewrahman.com` before sending).
8. **Once verified** → enable DOI on the form → complete Steps 2–5 in `02-05-SENDER-SETUP.md` → reply with A/B/C/D values.
9. **Plan doc updates** (handled by Claude):
   - `02-05-PLAN.md` frontmatter — sender identity reference
   - `site/app/privacy/page.tsx` — contact line (if desired: upgrade `andrewjrahman@gmail.com` → `andrew@andrewrahman.com`)
   - `STATE.md` Phase 02 decisions — log the sending-domain pivot
   - `02-05-SENDER-SETUP.md` — search/replace old domain references

---

## 9 — Fallback decision logic

If reaching Timo becomes feasible before you finish the Netlify migration:

- **Timo says yes and adds the records within 1–2 days** → revert to Option B, keep `andrew@spatialmedialab.org`, no pivot needed. Skip sections 4–8 above.
- **Timo unresponsive or declines** → proceed with Option A (pivot).
- **Can run both in parallel** — message Timo now while you continue Netlify migration. Whichever path clears first wins; no work is wasted either way because the Netlify migration needed to happen regardless.

---

## 10 — Research topics for next session

Things worth confirming before committing to Option A:

- [ ] What registrar is `andrewrahman.com` at? (Determines whether DNS is fully at Netlify or partially at registrar.)
- [ ] Is the Netlify DNS migration actually complete? (Run the `dig` check from section 8.)
- [ ] Does ImprovMX's free tier have any limits worth knowing? (Check current TOS — historically 25 aliases, unlimited forwards; verify still true.)
- [ ] Does Sender.net require the sender-identity email to be *deliverable for sending* (SMTP submission) or just *receiving*? (A forwarder handles inbound only — confirm Sender doesn't need SMTP-AUTH credentials for the address.)
- [ ] Has Timo been messaged? What was the response timeline?

---

## 11 — Key facts to carry forward

- **Sender.net requires account verification before double opt-in activates** — verification is a QA-team manual review, triggered by completing domain-auth + sender-identity + first-campaign milestones, sometimes requires an explicit email to `support@sender.net`.
- **A domain can have only ONE SPF record.** Multiple `v=spf1` TXT records = validation failure. Merging is required when adding a new sending service.
- **DKIM selectors are per-account unique.** You cannot pre-compute them; always use the exact value Sender's dashboard shows.
- **DMARC should start at `p=none`.** Monitor for 2–4 weeks before tightening to `p=quarantine` or `p=reject`.
- **Gmail/Yahoo April 2024 sender requirements** make DMARC non-optional for bulk senders; cutting corners here causes deliverability collapse.
- **jackhost.net uses Plesk** at `https://ssl.jackhost.net:8443`. Plesk has a DNS editor only when DNS is delegated to jackhost's nameservers — which is not the case for `spatialmedialab.org`.

---

## Related files

- `02-05-PLAN.md` — original plan (assumes domain access works)
- `02-05-SENDER-SETUP.md` — dashboard walkthrough (steps 1–5 for Sender UI)
- `.planning/STATE.md` — Phase 02 decisions list (update once sending domain is finalized)
- `site/app/privacy/page.tsx` — privacy policy contact line (may need update if pivoting)

---

## 12 — Session log 2026-04-21 (Option A execution)

**Decision:** Option A chosen — pivot sender domain to `andrewrahman.com` using ImprovMX forwarder.

**Alias refinement:** sender identity is `hey@andrewrahman.com` (not `andrew@andrewrahman.com` as originally drafted in §8). Forwards to `andrewjrahman@gmail.com` via ImprovMX.

**DNS-host discovery (contradicts §8 prerequisite assumption):** `andrewrahman.com` nameservers are `ns1.netfirms.com` / `ns2.netfirms.com` — DNS lives at **Netfirms, not Netlify**. Plan 02-04's DNS cutover to Netlify (Tasks 2/3/4) was never executed; the andrewrahman.com apex still resolves to `66.96.149.1` (Netfirms/HostGator parking page, last-modified 2023-09-27). Option A therefore proceeds with DNS records added at Netfirms, not Netlify. Plan 02-04 completion + Netlify apex migration is deferred to a later session — it does not block Sender.net sender-identity setup.

**Actions completed this session:**

1. ImprovMX account created; `andrewrahman.com` added; alias `hey@andrewrahman.com → andrewjrahman@gmail.com` configured.
2. Netfirms DNS — deleted stale `MX @ mx.andrewrahman.com` (priority 30) and stale `TXT @ v=spf1 ip4:66.96.128.0/18 include:websitewelcome.com ?all`. Deleted wildcard `MX * mx.andrewrahman.com` as cleanup. Added ImprovMX records:
   - `MX @ mx1.improvmx.com` priority 10
   - `MX @ mx2.improvmx.com` priority 20
   - `TXT @ v=spf1 include:spf.improvmx.com ~all`
3. Sender.net — domain `andrewrahman.com` added; DNS panel generated per-account values.
4. Netfirms DNS — merged Sender SPF into ImprovMX SPF, added Sender DKIM + DMARC:
   - **Edit** `TXT @` → `v=spf1 include:spf.improvmx.com include:sendersrv.com ~all`
   - **Add** `CNAME sender._domainkey → dkim.sendersrv.com`
   - **Add** `TXT _dmarc → v=DMARC1; p=none;`
5. Tested forwarder end-to-end: external email sent to `hey@andrewrahman.com` arrived in Gmail via ImprovMX. Forwarding is live.

**Current state at session pause:**

- **ns2.netfirms.com** — all new records serving correctly (SPF, DKIM, DMARC, MX verified via `dig @ns2.netfirms.com`)
- **ns1.netfirms.com** — still serving stale zone (old SPF, old MX, missing DMARC); DKIM CNAME present. Netfirms' internal sync between NS1 and NS2 is the bottleneck; panel warns "changes may take 4–8 hours".
- **Sender.net verification** — DKIM has hit green (queries that landed on ns2); SPF and DMARC still flip red/green depending on which NS Sender's check queries. Expected to stabilise all-green once ns1 catches up.
- **ImprovMX internal DNS check** — still red for MX records (checker queries public resolvers with cached old values); real mail flow works regardless.

**Existing unrelated Netfirms records preserved:**

- `CNAME dkim._domainkey → cur.dkim.veigmail.net` (old Netfirms email DKIM, different selector `dkim` vs our `sender`, no conflict)
- `A` records for site + legacy subdomains (`ftp`, `popmail`, `pop`, `imap`, `smtp`, `mail`, `webmail`, `email`, etc.) — orphaned from old Netfirms email setup; harmless
- `CNAME _acme-challenge → andrewrahman.com.letsencry...` — Let's Encrypt validation, leave alone

**Deviation from §8 order of operations:**

- §8 step 1 (Netlify migration complete) skipped — andrewrahman.com DNS stayed at Netfirms; records added there directly. When Netlify migration does happen in a later session, the mail-related DNS records (MX ×2, SPF TXT, DKIM CNAME, DMARC TXT) will need to be re-created at Netlify's DNS UI before nameservers flip. Track as a prerequisite for the Netlify cutover.

**Resume checklist (next session):**

1. Run `dig @ns1.netfirms.com TXT andrewrahman.com +short` — expect merged SPF. If still stale, wait.
2. Back in Sender.net Domains screen → click "Check SPF, DKIM and DMARC records" — expect all-green.
3. Continue with SENDER-SETUP.md steps 2–5 (form, DOI automation, data-region, self-test) using from-address `hey@andrewrahman.com` in the DOI automation.
4. Email `support@sender.net` requesting account verification (required to unlock double opt-in) — template in §8 step 7.
5. Capture FORM_ID; set aside for Plan 02-05 Task 2 (code changes, deferred until Netlify deploy of andrewrahman-com site).

**Dependency reminder:** Plan 02-05 Tasks 2–4 (site code + env var + E2E UAT) still depend on Plan 02-04 Tasks 2–4 (Netlify deploy + custom-domain attach + DNS cutover) being executed. Sender-side setup can complete fully before that happens; the FORM_ID sits in Sender waiting for the site to go live.

---

## 13 — Session continuation 2026-04-22 (ns1 synced + design correction)

**ns1 sync completion:** Netfirms ns1 caught up ~30 minutes after records were added (much faster than the 4–8h warning). Verified via `dig @ns1.netfirms.com`:
- SPF: `v=spf1 include:spf.improvmx.com include:sendersrv.com ~all` ✅
- MX: `10 mx1.improvmx.com`, `20 mx2.improvmx.com` ✅
- DKIM: `dkim.sendersrv.com` ✅
- DMARC still lagging on ns1 per direct query but Sender's panel shows all-green (cached verified state from earlier ns2 hits). Functionally complete.

**Sender.net — domain auth status:** ALL GREEN (SPF + DKIM + DMARC).

**Sender.net — progress this session:**
1. Groups created: `osd-unconfirmed`, `osd-confirmed` ✅
2. Embedded signup form published. **FORM_ID = `bkRxov`** (captured 2026-04-22). Wire into `NEXT_PUBLIC_SENDER_FORM_ID` in `andrewrahman-com` Netlify env during Plan 02-04 cutover.
3. DOI automation — NOT YET BUILT (next step).

**Critical design correction — `/get-osd/` is NOT the form page.** Fresh read of `andrewrahman-com` origin/main (local clone was 93 commits behind):
- **Homepage (`/`)** is the email-capture surface — `DownloadForm` component embeds the Sender form (`app/page.tsx` line ~992). `NEXT_PUBLIC_SENDER_FORM_ID` drives it.
- **`/get-osd/`** is the POST-DOI-CONFIRM landing page: heading "Confirmed · you're in" + `DownloadButtons` (OS-detected macOS + Windows .zip) + Patreon CTA.
- **Downloads are self-hosted** at `https://andrewrahman.com/assets/OpenSpatialDelay-v1.0.0-macOS-arm64.zip` and `…-Windows-x64.zip`. NOT GitHub releases. `lib/release.ts` is the single source of truth for filenames + paths.
- **Windows build IS live** (`WIN_ZIP_AVAILABLE = true` in `lib/release.ts`). Earlier concern about missing Windows build was based on stale GitHub-releases data; irrelevant under the self-hosted-assets design.

**Consequence — DOI email simplifies drastically:**
- **No download buttons in the email.** The email is pure confirm-action.
- **Post-confirm redirect URL** = `https://andrewrahman.com/get-osd/`. Configured in Sender's DOI/form settings.
- Patreon CTA already lives on `/get-osd/` — can optionally drop from email (or keep as soft duplicate).

**Revised DOI email body (replaces Plan 02-05 Task 1 Step C.14 spec):**
- Subject: `Confirm your subscription to OpenSpatialDelay`
- Preheader: `One click to confirm and grab your installer.`
- From name: `Andrew Rahman`
- From email: `hey@andrewrahman.com`
- Body blocks (in order):
  1. Heading: `You're one click away from OpenSpatialDelay.`
  2. Paragraph: `Click below to confirm your subscription. We'll take you straight to the download.`
  3. Button: `Confirm my subscription` → URL `{$double-optin-link}` (primary/brand)
  - (Optional) Paragraph: soft Patreon line — may be dropped since `/get-osd/` already carries a Patreon CTA.

**Dependency correction:** Plan 02-05 Task 2 site-code work is largely already done on `andrewrahman-com` origin/main. `DownloadForm` component, `DownloadButtons` component, `/get-osd/` post-confirm page, `lib/release.ts` constants, `/assets/` zip hosting — all present. Remaining code work scopes down to:
- Confirm `_headers` CSP covers `cdn.sender.net` + Sender runtime domains (probably already done)
- Confirm `app/privacy/page.tsx` names Sender.net as processor (probably already done per commit `986a898 chore(03-05): migrate .env.example env var TALLY_FORM_ID -> SENDER_FORM_ID`)
- Sender-embed Playwright spec (`tests/sender-embed.spec.ts`) may or may not exist — verify

**Local clone hygiene:** `andrewrahman-com` local at this workstation is 93 commits behind origin + 2 ahead (local Plan 02-05 work that never got pushed because the push was superseded by origin's redesign). Rebase/reset recommended before further code work — separate task, not blocking Sender setup.

**Resume checklist for next step (Step 3 in Sender):**
1. Build DOI automation per revised body above.
2. Set post-confirm redirect URL = `https://andrewrahman.com/get-osd/` (exact setting location: Sender's automation success-redirect OR the form's post-confirm URL — locate in-UI during build).
3. Toggle automation to ACTIVE.
4. Data region check (Step 4 in SENDER-SETUP).
5. Email `support@sender.net` requesting account verification to unlock double opt-in.
6. Paste FORM_ID into this log + STATE.md so downstream code work can wire it when the site deploys.

---

## 14 — Session continuation 2026-04-22 (DOI workflow built, account under review)

**Session window:** 2026-04-22 00:15 → 01:15 GMT+2 (~1 hour).

**FORM_ID:** `bkRxov` (captured from the form's Publishing Settings page). Form URL: `https://stats.sender.net/forms/bkRxov/view`.

**DOI automation built:** `OSD DOI — confirm subscription`. Structure:
1. **Trigger:** `Subscriber joins a group` → group `osd-unconfirmed`.
2. **Email step:** `Send an email` — From `Andrew Rahman <hey@andrewrahman.com>`, subject `Confirm your subscription to OpenSpatialDelay`, preheader `One click to confirm and grab your installer.`, blank-template body = Heading (`You're one click away from OpenSpatialDelay.`) + Paragraph (`Click below to confirm your subscription. We'll take you straight to the download.`) + Button (`Confirm my subscription`, URL `{$double-optin-link}`). Using Sender's free-tier default styling for now (orange button, auto-injected `Delivered using Sender` footer + unsubscribe line). Brand polish deferred — see STATE.md "Pending Todos" for brand-settings pass.
3. **Delay step:** `1 minute`. Required by Sender's documented DOI pattern — gives click event time to register before the condition evaluates. Invisible to end-user (their redirect fires instantly on button click).
4. **Condition step:** `Workflow email activity` → "Clicked a link" → email = this workflow's email step, link = `{$double-optin-link}`, window = 1 minute.
5. **Yes branch → Action:** `Move subscriber to group` → `osd-confirmed`.
6. **No branch:** empty (unconfirmed subscribers persist; manual prune later if needed).

**Form settings touched:** *Publishing > Redirect after submit* → **unchecked**. Rationale: `/get-osd/` is the POST-confirm landing page ("Confirmed · you're in" + DownloadButtons); redirecting there right after form submit would bypass the DOI gate. Sender's default inline success message ("Thanks! Check your email to confirm") is correct UX — user knows to check email, doesn't leave the site.

**Blockers discovered:**
- **Account verification** — clicking Activate on the workflow surfaced: *"You will be able to activate workflows as soon as your account is reviewed by our team. Usually this happens within 1 hour."* Sender auto-flagged the account for review the moment Activate was clicked — no proactive support ticket needed. Publishing-settings panel confirms: *"Account is under review... you'll receive an update within 1 hour."*
- **DOI toggle greyed out** on form's Publishing Settings → Double opt-in section — locked with warning *"Double opt-in is only available on verified accounts"*. Expected; unlocks with account verification.
- **Post-DOI redirect URL field** — NOT FOUND tonight. Almost certainly lives inside the DOI settings panel (currently locked). Assumption: once DOI toggle unlocks, the `/get-osd/` redirect field becomes visible. Next session will verify.

**Decisions & corrections this session:**
- **Trigger label:** Sender's actual UI label is `Subscriber joins a group`, not `Subscriber added to group` as written in earlier planning docs. Harmless naming drift.
- **Builder flow:** Sender splits email metadata (subject/sender/preview) from body content. "Create email content" button opens a separate drag-and-drop builder. Template picker lands on "Blank template" tile — chosen to avoid pre-designed marketing templates that would add hero images/logos we don't want.
- **Palette correction:** Session uncovered that `--accent-regal` lavender (Phase 03-03 D-12 "Patreon CTA only" token) is **abandoned** — Session 3 swapped Patreon CTA to `--accent-green` (`#3BCE6C`), Session 7 extended the through-line to §5 Pipeline. Globals.css:28 still carries a stale `/* Patreon CTA only */` comment. Memory entry `project_andrewrahman_site_cta_palette.md` written + indexed to prevent regression. Current CTA hierarchy: primary cyan `#80d8ff` / secondary green `#3BCE6C` / tertiary dim.
- **DOI email button styling:** Kept Sender's free-tier orange default for now. Recommended future swap = cyan `#80d8ff` (matches Download OSD primary CTA semantic). Deferred to brand-settings pass (STATE.md todo).

**Evidence artefacts (screenshots captured by Andrew during build):**
- `~/Desktop/Screenshot 2026-04-22 at 00.25.42.png` — Automations landing (pre-"Create from scratch")
- `~/Desktop/Screenshot 2026-04-22 at 00.28.30.png` — Trigger picker showing 9 options
- `~/Desktop/Screenshot 2026-04-22 at 00.38.45.png` — Email Setup panel with sender + subject filled
- `~/Desktop/Screenshot 2026-04-22 at 00.45.08.png` — Template picker with Blank template tile
- `~/Desktop/Screenshot 2026-04-22 at 00.50.19.png` — Built DOI email body
- `~/Desktop/Screenshot 2026-04-22 at 00.55.09.png` — Brand settings page (todo reference)
- `~/Desktop/Screenshot 2026-04-22 at 01.02.29.png` — Condition Setup with "Workflow email activity" option
- `~/Desktop/Screenshot 2026-04-22 at 01.06.56.png` — "Account under review" error on Activate click
- `~/Desktop/Screenshot 2026-04-22 at 01.10.22.png` — Form Settings showing "Redirect after submit"
- `~/Desktop/Screenshot 2026-04-22 at 01.13.10.png` — Form Publishing Settings with DOI locked + review banner

**Resume checklist for NEXT session:**
1. **First check:** has Sender's account review completed? Indicators = DOI toggle on form's Publishing Settings is no longer greyed out, AND the "Account is under review" banner is gone. If not, wait or email `support@sender.net` with context.
2. Enable **Double opt-in toggle** on the form's Publishing Settings (top-right corner of that panel).
3. Locate + set the **post-DOI redirect URL** → `https://andrewrahman.com/get-osd/`. Should surface inside the now-unlocked DOI settings panel OR in a new field that appears when DOI is toggled on.
4. Return to the automation (`Automations → OSD DOI — confirm subscription`) and toggle it to **ACTIVE**.
5. **Data region check** — account Settings/Profile → look for "Data region" / "Data processing region". If visible and = EU, log it. If not visible, ask Sender support.
6. **Optional self-test** — submit an email via Sender's hosted form URL (`https://stats.sender.net/forms/bkRxov/view`), verify DOI email arrives, click confirm, verify redirect lands on `/get-osd/` (will hit the OLD Netfirms placeholder until Plan 02-04 Netlify cutover — expected). Confirm subscriber moved from `osd-unconfirmed` → `osd-confirmed` in Sender.
7. **STILL DEFERRED (Plan 02-04):** Netlify deploy of `andrewrahman-com` + apex DNS cutover. Without this, the post-confirm redirect lands on the old Netfirms parking page. Production flip of Plan 02-05 depends on this.

## 15 — Session continuation 2026-04-22 (account verified; DOI redirect feature DOES NOT EXIST in Sender; Path A chosen)

**Session window:** 2026-04-22 08:20 → 09:00 GMT+2 (~40 min). Computer reboot pause.

**Sender account review completed** — DOI toggle on form `bkRxov` Publishing Settings is unlocked; "Account is under review" banner gone.

**DOI toggle flipped ON** on form `bkRxov` → Publishing Settings → "Double opt-in settings" panel. Panel contains only: DOI toggle, confirmation email subject (`Confirm your subscription`), sender name (`Andrew`), sender email (`hey@andrewrahman.com`), plus a preview card. Blue helper text reads: *"Automations will be triggered once subscribers confirm their email address."* **No post-DOI redirect URL field exists in this panel or anywhere else on the form.** Resume-checklist Step 2 done; Step 3 **cannot be completed as written — it was based on an invalid assumption.**

### Empirical test resolving H1 vs H2 (redirect timing)

Question: when DOI is enabled, does the form's "Redirect after submit" (Form editor → Settings tab → Options → Redirect after submit + "Redirect to" URL field) fire at step 1 (immediately on form submit, pre-DOI) or at step 3 (after clicking Confirm in DOI email)?

Test: set "Redirect after submit" → `https://andrewrahman.com/?sender-test=1`, submitted hosted form (`https://stats.sender.net/forms/bkRxov/view`) in incognito.

Result: **H1 confirmed.** Submit → brief flash of Sender's default "Oh thank you! / We are glad to have you on board" Success view → immediate redirect to `/?sender-test=1`. No DOI email click required to trigger redirect. **"Redirect after submit" fires at step 1, pre-DOI. Using this field for `/get-osd/` would gate-bypass the DOI.** Setting was reverted; field will remain unchecked going forward.

### Sender's architectural reality (confirmed from docs + test)

- Sender's Success view is a single hosted page per form. Per docs, the confirm button in the DOI email also lands on "success view page" — docs use the same term for both moments. Cannot use the Success view to host a download link, because step-1 users (not yet confirmed) see the same view and would click straight through, defeating DOI.
- **Sender has no native post-DOI-confirm redirect URL anywhere in its UI.** Form-level, DOI-panel, automation-action, account-settings — none of them expose a "where should the user land after confirming?" field.
- Four Sender help docs explicitly route this question to `support@sender.net`, and the implicit answer across all four is: *"no such setting, use automations."*
- Sender's own mental model is clear in the DOI panel's blue helper: **post-confirmation behaviour is delivered via automations, not redirects.**

### Path A chosen (recommended): follow-up automation email on Yes branch

Current automation structure:
`Trigger: added to osd-unconfirmed → Email (Confirm button) → Delay 1min → Condition: clicked {$double-optin-link} → Yes: Move to osd-confirmed / No: (empty)`

**New Yes branch:**
`Yes: Move to osd-confirmed → Send email (subject "Your OpenSpatialDelay download", body: single primary button "Get OpenSpatialDelay" → https://andrewrahman.com/get-osd/)`

User journey:
1. Submit form → Sender Success view ("Oh thank you!" — will be rewritten to set expectations)
2. Email #1 (DOI) arrives → click Confirm → same Sender Success view
3. Email #2 (download) arrives seconds later → click button → `/get-osd/`

Preserves DOI gate absolutely (only osd-confirmed subscribers get email #2). Zero custom infra. Matches Sender's native model.

### Decisions deferred to next session

- **Success view copy rewrite.** Default "Oh thank you! / We are glad to have you on board" doesn't tell the user to expect email #2. Rewrite to something like *"Almost there — check your email to confirm, and we'll send your download link right after."* (Same view is used at both step 1 and step 3; copy must work for both moments. After step 3 the user just read email #2 instructions, so it's only mildly redundant.)
- **Patreon soft-CTA placement.** Original plan had DOI email carry optional soft Patreon CTA. With downloads moving to email #2, open question: Patreon CTA in email #1 (DOI), email #2 (download), `/get-osd/` page, or all three? Recommend email #2 + `/get-osd/` (both are post-confirm, aligned with "thanks for subscribing" moment).
- **Email #2 button styling.** Defer to brand-settings pass (already on STATE.md todo).

### Evidence artefacts (screenshots captured by Andrew this session)

- `~/Desktop/Screenshot 2026-04-22 at 08.44.24.png` — DOI panel fully unlocked; toggle ON; only email-metadata fields present, no redirect field.
- `~/Desktop/Screenshot 2026-04-22 at 08.54.23.png` — Form editor Settings tab showing "Redirect after submit" checkbox + "Redirect to" URL field (used for H1 test).
- `~/Desktop/Screenshot 2026-04-22 at 08.56.14.png` — Sender's default "Oh thank you! We are glad to have you on board" Success view (shown briefly at step 1 before redirect fired).

### Revised resume checklist for NEXT session (supersedes §14 checklist steps 3 + 6)

**State at pause:** DOI toggle ON, confirmation email metadata saved, "Redirect after submit" UNCHECKED (reverted from test). Automation still in Paused state. Downloads still on `/get-osd/` only. No production (Netlify) flip yet.

1. Confirm automation is still in Paused state (`Automations → OSD DOI — confirm subscription`). If it somehow auto-activated, pause it before editing.
2. **Add Yes-branch email step to the automation:** click Yes branch under Condition → after "Move to osd-confirmed", add `Send an email`. Subject: `Your OpenSpatialDelay download`. Sender: `Andrew Rahman <hey@andrewrahman.com>`. Body (Blank template): short heading, 1-line paragraph, single primary button `Get OpenSpatialDelay` → `https://andrewrahman.com/get-osd/`. Optional soft Patreon paragraph below button (aligns with post-confirm "thanks" moment).
3. **Rewrite the form's Success view** (Form editor → Design tab → click "Success view" above the form preview). Replace `Oh thank you! / We are glad to have you on board` with something like `Almost there — check your email to confirm, and we'll send your download link right after.` Save.
4. **Activate the DOI automation workflow.**
5. **Data region check** (Settings/Profile → "Data region" / "Data processing region"). Log EU or email support if not visible.
6. **End-to-end self-test** in incognito: submit hosted form → receive DOI email → click Confirm → verify Sender Success view + receive email #2 within seconds → click "Get OpenSpatialDelay" → lands on `/get-osd/` (will still hit old Netfirms parking page until Plan 02-04 Netlify cutover — this is a known expected failure; verify instead that email #2 *arrived* and the button URL points at `andrewrahman.com/get-osd/`). Confirm subscriber moved osd-unconfirmed → osd-confirmed.
7. Commit this research doc update + update STATE.md's `stopped_at`, `Current Position`, and Pending Todos.
8. Plan 02-05 production flip still blocked on Plan 02-04 Netlify deploy/cutover (unchanged from §14).

### Why Path B and Path C were rejected

- **Path B (revert 2026-04-22 pivot: put download buttons back in DOI email):** rejected for design continuity with the pivot — downloads on `/get-osd/` is the locked design, email #1 stays confirm-button-only. (Weakens DOI gate marginally — user could click download before confirming — but downloads are free public GPL zips so the gate is soft either way. Kept Path B as a documented fallback if Path A friction proves unacceptable.)
- **Path C (put download button on Sender Success view):** rejected because the Success view is shown at BOTH step 1 and step 3 — a step-1 user would click the download button without confirming, totally defeating the DOI. Unrecoverable unless Sender distinguishes the two views (no doc evidence it does).
