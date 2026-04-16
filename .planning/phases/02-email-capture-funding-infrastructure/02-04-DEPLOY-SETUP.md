---
title: andrewrahman.com Deploy + DNS Cutover — User Walkthrough
phase: 02-email-capture-funding-infrastructure
plan: 02-04
purpose: Single-document walkthrough for Tasks 2, 3, 4 of Plan 02-04 — Netlify signup, bake-off, custom domain attach, Netfirms DNS cutover, HTTPS verification.
audience: Andrew (user) — dashboard + DNS work; agent handles verification after your return signal.
---

# Deploy andrewrahman.com on Netlify

The agent has pushed the Netlify config, security headers, and a production smoke script to
`AndrewRahman/andrewrahman-com`. Everything below happens **in your browser and at Netfirms**.

Total time: ~30 min hands-on + 10–60 min DNS propagation wait.

---

## Pre-flight

- [ ] `AndrewRahman/andrewrahman-com` on GitHub is up to date with `netlify.toml`, `_headers`,
      and `scripts/verify-production.sh` committed (the agent pushed these; you can confirm with
      `cd ~/conductor/repos/andrewrahman-com && git log --oneline -3 && git status`).
- [ ] You have access to: your GitHub account, your Netfirms control panel, and your email for
      Netlify signup.
- [ ] Create `docs/phase-02-evidence/` in the OSD repo (`mkdir -p
      ~/conductor/repos/openspatialdelay/docs/phase-02-evidence`) — you'll drop 4 screenshots there.

---

## Step 1 — Sign up for Netlify

1. Visit **app.netlify.com/signup**
2. Choose **"Sign up with GitHub"** (simplest — OAuth links your GitHub repos to Netlify)
3. Authorize the Netlify GitHub app for the `AndrewRahman` account (personal, not Spatial-Media-Lab
   org — matches D-21 personal-identity framing)
4. Free **Starter** plan is default. Do NOT pick Pro.
5. **Enable 2FA** on the Netlify account (T-02-16 mitigation) before touching DNS.

---

## Step 2 — Deploy to Netlify (bake-off half 1)

1. Netlify dashboard → **"Add new site"** → **"Import an existing project"**
2. Choose **GitHub** → select **`AndrewRahman/andrewrahman-com`** (you may need to grant repo
   access to Netlify's app first — click "Configure the Netlify app on GitHub" and scope it to
   this one repo, NOT all repos)
3. On the deploy config screen, Netlify should **auto-detect `netlify.toml`** — confirm it shows:
   - Build command: `npm run build`
   - Publish directory: `out`
   - Node version: `20` (from `[build.environment]` in netlify.toml)
4. Click **"Deploy site"**. Wait ~2 min for the first build (npm install + next build).
5. Netlify assigns a random subdomain like `eloquent-pascal-abc123.netlify.app`. **Write it down
   — you need it for the `www` CNAME in Step 5.**
6. Visit the assigned URL. Confirm all three pages:
   - [ ] `/` renders — "Andrew Rahman" heading, bio copy, "Get OpenSpatialDelay" button
   - [ ] `/privacy` renders — full privacy policy (no "[Prose pending]" strings)
   - [ ] `/get-osd` renders — shows "Tally form ID not yet configured" placeholder card
7. 📸 Screenshot the Netlify site overview → `docs/phase-02-evidence/netlify-preview.png`

---

## Step 3 — Deploy to Vercel (bake-off half 2 — throwaway)

D-18 mandates a Netlify-vs-Vercel bake-off. RESEARCH.md Pitfall 1 says Vercel Hobby tier bans
commercial use and Patreon-linked sites are commercial. **Do NOT put `andrewrahman.com` on Vercel.**
This step is for comparison only.

1. Visit **vercel.com/signup** → sign in with GitHub
2. **"Add New... → Project"** → import the same `AndrewRahman/andrewrahman-com` repo
3. Vercel auto-detects Next.js. On the Configure Project screen:
   - Framework: Next.js
   - Build Command: `npm run build`
   - **Output Directory: `out`** (Vercel doesn't infer `output: 'export'` reliably — set this explicitly)
4. Deploy. Vercel assigns `{repo-name}-{hash}.vercel.app`
5. Visit the Vercel URL. Spot-check `/`, `/privacy`, `/get-osd` render the same as Netlify.
6. 📸 Screenshot the Vercel project page → `docs/phase-02-evidence/vercel-preview.png`
7. **Do NOT attach `andrewrahman.com` as a custom domain in Vercel. Leave it as the preview URL only.**

---

## Step 4 — Attach andrewrahman.com in Netlify

(Back to Netlify — you've chosen it as production per RESEARCH.md Pitfall 1.)

1. Netlify → your site → **Site configuration → Domain management**
2. Click **"Add a domain you already own"** → type `andrewrahman.com` → **Verify** → **Add**
3. Netlify prompts to also add `www.andrewrahman.com` as alias → accept
4. Netlify shows a DNS verification panel — at this point DNS still points at the old Netfirms
   parking IP (66.96.149.1), so the check will **fail**. That's expected. Step 5 fixes DNS.
5. 📸 Screenshot the Domains panel showing `andrewrahman.com (Awaiting External DNS)` →
   `docs/phase-02-evidence/netlify-custom-domain-added.png`
6. **Do NOT click "Provision certificate" yet** — Let's Encrypt needs DNS to resolve first.

---

## Step 5 — Netfirms DNS cutover (CAREFUL — this is the production switch)

⚠️ **Before touching DNS**, verify the Plan 03 gate:

```bash
cd ~/conductor/repos/andrewrahman-com
grep -c 'Prose pending Plan 03' app/privacy/page.tsx
# Expected: 0
grep -c '<CONTROLLER>' app/privacy/page.tsx
# Expected: 0
```

If either returns non-zero, STOP. Re-run Plan 03 before cutting DNS — the first public view of
`andrewrahman.com/privacy` must be the authored policy, not placeholders.

### 5a. Verify current DNS state (rollback evidence)

```bash
dig andrewrahman.com +short A
# Expected: 66.96.149.1   (if different, document the surprise)

dig andrewrahman.com NS
# Expected: ns1.netfirms.com, ns2.netfirms.com   (if different, DNS authority is NOT at Netfirms
#                                                  — STOP, this plan's assumptions are wrong)
```

### 5b. Log into Netfirms

1. Log into the **Netfirms control panel** (credentials from your password manager)
2. **Enable 2FA** on the Netfirms account if not already on (T-02-16 mitigation)
3. Navigate: **Domains → andrewrahman.com → DNS records**
4. 📸 Screenshot the current records table → `docs/phase-02-evidence/netfirms-dns-before.png`
   (this is your rollback evidence — do NOT skip)

### 5c. Edit records (atomic — save only once)

Make these changes, don't save yet:

- **Apex A record** (replace the existing 66.96.149.1 entry):
  - Type: `A`
  - Host / Name: `@` (or blank — depends on Netfirms UI)
  - Value / Points to: `75.2.60.5`
  - TTL: leave default (3600 / 1 hour is fine)

- **www CNAME record** (add new, or replace if one exists):
  - Type: `CNAME`
  - Host / Name: `www`
  - Value / Points to: `<your-netlify-subdomain>.netlify.app.` (with trailing dot if Netfirms UI requires it — e.g. `eloquent-pascal-abc123.netlify.app.`)
  - TTL: default

⚠️ **DO NOT touch:** MX records (email), NS records (DNS authority), TXT records, SOA record.

Save. 📸 Screenshot the updated records table → `docs/phase-02-evidence/netfirms-dns-after.png`

### 5d. Wait for propagation

DNS propagation is typically 10 min – 2 h at Netfirms; can be up to 24 h globally. Poll:

```bash
dig andrewrahman.com +short A
# Target: 75.2.60.5

dig www.andrewrahman.com +short
# Target: <your-netlify-subdomain>.netlify.app + Netlify IP
```

If still 66.96.149.1 after 30 min, re-check the records in Netfirms. Maximum wait: 24 h.

### 5e. Trigger Let's Encrypt SSL

Once `dig andrewrahman.com +short A` returns `75.2.60.5`:

1. Netlify → Domain management → **HTTPS**
2. Click **"Verify DNS configuration"** → should pass now
3. Click **"Provision certificate"** → wait ~1 min for Let's Encrypt to issue

---

## Step 6 — Final verification

### 6a. Run the smoke script

```bash
cd ~/conductor/repos/andrewrahman-com
./scripts/verify-production.sh
```

Expected output: `ALL CHECKS PASSED` with 13 PASS lines. If any FAIL:

- **DNS fail** → wait longer for propagation
- **Security header fail** → check Netlify build log that `_headers` was detected (it should be
  in publish dir root)
- **Content fail on /privacy** → Netlify may be serving a stale build; trigger a **"Clear cache
  and deploy site"** from the Deploys tab

### 6b. Manual curl sanity (belt-and-braces)

```bash
curl -sI https://andrewrahman.com/ | head -3
curl -sI https://andrewrahman.com/privacy/ | head -3
curl -sI https://andrewrahman.com/get-osd/ | head -3
curl -sI https://andrewrahman.com/ | grep -i strict-transport-security
curl -s https://andrewrahman.com/privacy/ | grep -c "Tally Technologies SRL"
# Expected: "1" (or higher)
```

---

## Step 7 — Return signal to resume Plan 02-04

When all of Step 6 is green, reply in chat with:

```
dns cutover complete

Netlify subdomain used:     <eloquent-pascal-abc123>.netlify.app
Hosting decision:           netlify
Vercel bake-off result:     <"both rendered identically" or note any differences>

Screenshots saved:
  docs/phase-02-evidence/netlify-preview.png
  docs/phase-02-evidence/vercel-preview.png
  docs/phase-02-evidence/netlify-custom-domain-added.png
  docs/phase-02-evidence/netfirms-dns-before.png
  docs/phase-02-evidence/netfirms-dns-after.png

curl output (paste output of the 5 curl commands from Step 6b):
<paste>

./scripts/verify-production.sh output:
<paste — should end with "ALL CHECKS PASSED">
```

A continuation agent will then:
1. Finalize 02-04-SUMMARY.md with your hosting decision, subdomain name, and curl output
2. Mark Plan 02-04 complete in ROADMAP
3. Unblock Plan 02-05 (Tally form go-live) — Tally's consent checkbox can now link to the live
   `https://andrewrahman.com/privacy`

---

## Rollback (if anything goes wrong)

If the site is broken after cutover and you need to roll back:

1. Netfirms → DNS records → Apex A → change back to `66.96.149.1`
2. Remove the `www` CNAME (or point it back at the old Netfirms parking target)
3. Save. Re-check `dig` in 10–30 min.
4. Open a chat saying "02-04 rollback" with what went wrong — I'll document and re-plan.

The `docs/phase-02-evidence/netfirms-dns-before.png` screenshot from Step 5b is your
authoritative record of the pre-change state.
