---
plan: 05-07
phase: 05-launch-announcements
status: complete
completed: 2026-04-18
requirement: ANNC-06 (D-07 Berlin Superbooth DMs)
---

# Summary: 05-07 Berlin Superbooth DMs

## What was built

4 per-contact DM drafts + 1 send log:

- `drafts/berlin-dm-hainbach.md` — Hainbach DM (IG primary @hainbach101, ~340 chars, Timo IN)
- `drafts/berlin-dm-kirn.md` — Peter Kirn DM (email primary, friend-tone, Timo OUT)
- `drafts/berlin-dm-horstmann.md` — Eric Horstmann DM (LinkedIn DM primary, friend-tone, Timo OUT, email fallback)
- `drafts/berlin-dm-aes.md` — Ulli Scuda email (formal, ~810 chars, Timo OUT, AES Germany re-scope from CONTEXT D-07)
- `berlin-dm-log.md` — 4-row send log, all status=pending, send window 2026-04-21 through 2026-04-25

## Decisions

### Identity framing — Rule 8a (locked 2026-04-18 by Andrew override)

Andrew is **NOT** a "software developer" / "audio dev" / "solo dev". Identity = "Berlin-based spatial media expert and co-founder of Spatial Media Lab" (long form) or just "co-founder of Spatial Media Lab" (DM short form). OSD is the first piece of software Andrew has ever shipped.

Initial DMs drafted with "Berlin solo dev" / "Berlin-based audio developer" framing — rewritten per Rule 8a addition to `05-CONTEXT-SUPPLEMENT-2026-04-18.md`. See commits `69d146e` (Berlin DMs), `cd272be` (KVR news), `364719f` (LinkedIn).

### Per-contact Timo Bittner inclusion

| Contact | Timo IN/OUT | Reason |
|---------|-------------|--------|
| Hainbach | IN | Andrew knows him but not well; doesn't know if Hainbach knows Timo |
| Peter Kirn | OUT | Andrew knows Peter personally |
| Eric Horstmann | OUT | Andrew knows Eric personally |
| Ulli Scuda | OUT | Direct email (rule overrides peer-knowledge) |

### Plan deviation — DM channels routed via Andrew's actual platforms

Plan template specified Bluesky as Hainbach's + Peter's primary platform. Andrew has no Bluesky/Mastodon. Per Rule 8a platform-availability rule:

| Contact | Plan said | Actually using | Trade-off |
|---------|-----------|----------------|-----------|
| Hainbach | Bluesky primary, IG fallback | IG primary, no fallback | Acceptable — IG is a real channel for Hainbach |
| Peter Kirn | Bluesky primary, Mastodon + email fallback | Email primary, no fallback | Peter prefers Bluesky for unsolicited inbound, but Andrew/Peter relationship makes email an acceptable channel |
| Eric Horstmann | LinkedIn primary, email fallback | unchanged | none |
| Ulli Scuda | Email | unchanged | none |

### Tone refinement

Initial Peter + Eric drafts felt "awkward and not very personal" (Andrew feedback) — rewritten in friend-tone since both are personal acquaintances. Hainbach + Ulli kept formal-warm tone since relationship is thinner.

## Send timeline (pending)

| Step | When | Action |
|------|------|--------|
| 2026-04-21 to 2026-04-25 | Send window opens | Send all 4 DMs per primary platform |
| Per-send | At time of send | Update `berlin-dm-log.md` row: status=sent, fill Timestamp |
| Reply | When/if reply arrives | Update Reply column |
| Meeting confirmed | If meeting set | Fill Meeting-confirmed column |
| 2026-05-07 to 2026-05-10 | Superbooth at Messe Berlin | Face-to-face conversion if meetings confirmed |
| 2026-05-11+ | Post-Superbooth | Optional one-line nudge to non-repliers IF Andrew can check email during travel |

## Self-Check: PASSED

- All 5 files exist and committed (ec4386e, fd61c8f, 5e87b1c, 661b032, f23f74c, then rewritten in 69d146e)
- Rule 8a compliance: all "developer" / "solo dev" framing removed; identity = "co-founder of Spatial Media Lab"
- Timo per-contact rule honored (Hainbach IN; Kirn/Horstmann/Scuda OUT)
- Platform routing matches Andrew's actual capabilities (no Bluesky/Mastodon)
- Tone: relationship-first, friend-tone where Andrew knows the contact
- Send log structure ready for fill-as-you-go
- Andrew approved framing rewrite: 2026-04-18

## Forward-looking impact (carried into Wave 2/3)

- **05-06 review pitches**: Default Timo IN for unknown contacts; flag any contacts Andrew knows personally for Timo OUT.
- **05-08 guest pitches**: Default Timo IN (podcast/conference contacts are mostly unknown).
- **Live surfaces** to update separately (out of Phase 5 scope):
  - `andrewrahman.com` page.tsx + v2.tsx bio (line 651): "audio software developer" → spatial media expert + SML co-founder framing
  - `patreon.com/AndrewRahman` About section: same rewrite
