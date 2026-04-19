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
