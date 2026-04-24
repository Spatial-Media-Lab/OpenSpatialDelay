# Plan 02-05: Sender.net DOI — Research Log

Date: 2026-04-24 (updated 2026-04-24)
Purpose: Prevent re-researching the same topics. Every entry is evidence-grounded.

---

## Current Status

**DOI gate: WORKING.** Validated end-to-end on 2026-04-24:
1. User fills form → added to `osd-unconfirmed`
2. User receives DOI confirmation email → clicks confirm link
3. Subscriber group changes to `osd-confirmed` + download email sent automatically
4. User clicks link in download email → lands on `andrewrahman.com/get-osd/`

**Next step:** Integrate the Sender form embed onto the local andrewrahman-com site. Validate it works correctly and matches the site's design language. Once the site is live on Netlify, come back to simplify the workflow (change confirm button URL to `/get-osd/` so the user lands directly on the download page on confirm click, eliminating the need for the second download email).

---

## Confirmed Facts (from Sender docs + live testing)

### F1. Sender has TWO isolated DOI systems
- **Form-level DOI**: Sender's internal engine sends a native confirmation email using `{$double-optin-link}`. Confirmation flips subscriber status from `non-subscribed` → `Active`. No automation triggers fire.
- **Automation-based DOI**: Form is single opt-in. Automation sends a custom email with a plain URL. Click tracking via conditions or triggers.
- Source: [Sender automation DOI](https://www.sender.net/help/automation/double-opt-in/), internal testing §14–§15

### F2. Group-add timing: ALWAYS immediate on form submit
When a subscriber signs up via a form, they are added to the form's target group **immediately on form submit**, regardless of whether form-DOI is ON or OFF. The DOI only changes their status, not group membership.
- Source: [Sender DOI for forms](https://www.sender.net/help/lead-capture/double-opt-in-for-forms/)

### F3. "Subscriber status changed" trigger DOES NOT EXIST
Complete trigger list (10 triggers): A Date, An Anniversary Of A Date, Subscriber Added to a Group, Subscriber Is Removed From a Group, A Link Is Clicked, Cart Is Abandoned, A Product Is Purchased, An API Call Is Made, Subscriber unsubscribed, Subscriber field updated.
- No "Subscriber status changed" trigger exists.
- Source: [Sender automation triggers](https://www.sender.net/help/automation/choose-starting-trigger/)

### F4. `{$double-optin-link}` mergetag only works in form-DOI context
Using `{$double-optin-link}` in an automation email when form-DOI is ON → renders correctly (form-DOI handles confirmation). Using it in an automation email when form-DOI is OFF → 404.
- Source: failure chain item #1 in §14

### F5. Sender has NO post-DOI redirect URL
Confirmed by checking every UI surface: form settings, DOI panel, automation actions, account settings. Sender's helper text: "post-confirmation behaviour is delivered via automations, not redirects."
- Source: §15, screenshots `~/Desktop/Screenshot 2026-04-22 at 08.44.24.png`

### F6. "Redirect after submit" fires PRE-DOI
Empirically tested on form `bkRxov`: submit → brief success view → immediate redirect to configured URL. No DOI click required.
- Source: §15 H1 test

### F7. Condition step "Link is clicked" evaluates ONCE after delay
"You should add a DELAY before the Condition and Email, otherwise this step will be taken immediately after email is sent, which would lead all your recipients to the NO path." The condition does NOT listen continuously.
- Source: [Sender automation steps](https://help.sender.net/knowledgebase/choosing-the-right-automation-step/)
- Implication: delay = the click window. Short delay misses slow clickers. Long delay delays download delivery.

### F8. "A link is clicked" as a STARTING TRIGGER fires immediately
"Automation would start its first action whenever a subscriber clicks on a specific defined link." As a trigger (not condition), it fires the moment the click happens.
- Source: [Sender automation triggers](https://www.sender.net/help/automation/choose-starting-trigger/)
- Key quote: "After subscribers click on the link, they automatically participate in a secondary automation sequence which starts after the 'A link is clicked' trigger."
- This enables a two-automation chain where Automation 2 fires instantly on confirm click.

### F9. German DOI law — click in confirmation email IS valid proof
German BGH + UWG §7 + DSGVO Art. 7: DOI is not legally mandated but is the standard proof of consent. The click on a confirmation link is a "clear affirmative act." No distinction between form-DOI and automation-DOI at the legal level.
- Source: [dr-datenschutz.de](https://www.dr-datenschutz.de/alles-ueber-das-double-opt-in-verfahren/), [simon-erklaert.com](https://www.simon-erklaert.com/email-marketing/double-opt-in-pflicht/)
- Legal prohibition: DOI email must NOT contain advertising (Werbung). Only the confirmation link.
- Source: [dr-datenschutz.de](https://www.dr-datenschutz.de/werbung-in-double-opt-in-bestaetigungsmail-unzulaessig/)

### F10. Data region — unresolved, waiting on Sender support
Email sent to support@sender.net asking to confirm EU data processing. No UI field found in account settings. Status: pending response.

---

## Failed Approaches (do NOT retry)

### X1. Form-DOI + automation trigger on confirmation
**Why it fails:** group-add fires on submit (F2), not confirmation. No trigger fires on DOI status change (F3). All attempts to chain form-DOI → automation → download email fail because the automation fires before the user confirms.

### X2. `{$double-optin-link}` in automation email (form-DOI OFF)
**Why it fails:** mergetag produces 404 when form-DOI is OFF (F4).

### X3. `{$double-optin-link}` in automation email (form-DOI ON)
**Why it fails:** when form-DOI is ON, Sender sends its own native DOI email AND the automation fires on group-add (pre-DOI). User gets TWO emails — Sender's native DOI email AND the automation's email. Confusing and the automation email fires before DOI.

### X4. Post-DOI redirect URL
**Why it fails:** field does not exist anywhere in Sender's UI (F5).

### X5. "Redirect after submit" as DOI gate
**Why it fails:** fires on form submit, before DOI confirmation (F6).

### X6. Single-automation DOI with short delay + condition
**Why it's risky:** condition evaluates once after delay (F7). Short delay (1–5 min) misses slow clickers. Long delay (24–48h) delays download delivery unacceptably.

---

## Open Questions (need live testing in Sender UI)

### Q1. Does "Subscriber field updated" trigger detect DOI status change?
Trigger fires "when any of the chosen fields receives new information." Can subscriber subscription-status (non-subscribed → Active) be selected as a watched field? If yes, this would be a clean way to chain form-DOI → automation.
**Test:** create a test automation, select "Subscriber field updated" trigger, examine which fields are available in the picker.

### Q2. Can the "A link is clicked" starting trigger reference a link from an automation email?
The trigger may only work for links in manual campaign emails (not automation-sent emails). If it works across automations, the two-automation chain (Automation 1 sends DOI email → link click triggers Automation 2) fires instantly.
**Test:** create Automation 2 with "A link is clicked" trigger, examine whether links from Automation 1's email are selectable.

### Q3. Does form-DOI ON + automation-based email create a double-email problem?
With form-DOI ON, Sender sends its native DOI email. Simultaneously, group-add fires the automation which sends another email. Would the user receive two confirmation emails?
**Test:** toggle form-DOI ON, activate an automation triggered by group-add, submit test email, count emails received.

---

## Recommended Architecture: Two-Automation Chain

Based on F7 (condition evaluates once) and F8 ("A link is clicked" trigger fires immediately), the cleanest pattern is:

### Setup

**Form DOI = OFF** (automation handles everything)

**Automation 1: "OSD — confirm subscription"**
- Trigger: "Subscriber added to a group" → `osd-unconfirmed`
- Step 1: Send email
  - Subject: `Confirm your subscription to OpenSpatialDelay`
  - From: `Andrew Rahman <hey@andrewrahman.com>`
  - Body: heading + one-line paragraph + single button
  - Button label: `Confirm my subscription`
  - Button URL: `https://andrewrahman.com/?confirmed=osd` (unique, non-download URL; on live site will show homepage, on pre-deploy will show parking page — irrelevant, only the click matters)
- NO further steps. This automation just sends the DOI email.

**Automation 2: "OSD — deliver download"**
- Trigger: "A link is clicked" → URL = `https://andrewrahman.com/?confirmed=osd` (same URL as the button in Automation 1)
- Step 1: Move subscriber to group `osd-confirmed`
- Step 2: Send email
  - Subject: `Your OpenSpatialDelay download`
  - From: `Andrew Rahman <hey@andrewrahman.com>`
  - Body: heading + paragraph + primary button
  - Button label: `Get OpenSpatialDelay`
  - Button URL: `https://andrewrahman.com/get-osd/`

### User journey

1. Submit form → inline success: "Check your email to confirm"
2. DOI email arrives (seconds) → click "Confirm my subscription" → lands on homepage (or parking page pre-deploy)
3. **Immediately**: Automation 2 fires → subscriber moved to `osd-confirmed` → download email sent
4. Download email arrives (seconds) → click "Get OpenSpatialDelay" → `/get-osd/`

### Why this satisfies R6 + R7

- **R6 (download gated behind subscription):** Download email (#2) only sends via Automation 2, which fires on the confirm click. No click = no download email.
- **R7 (DOI is distinct from download):** The confirm button goes to `/?confirmed=osd` (NOT `/get-osd/`). Consent action and download delivery are logically and temporally separate.

### Fallback if Q2 fails

If "A link is clicked" trigger cannot reference links from automation emails (Q2), fall back to:
- Single automation with DELAY + CONDITION pattern
- Delay = 4 hours (reasonable window; stated in success view: "download link within a few hours")
- Condition: link clicked → Yes: move to confirmed + send download email

This is worse UX (delayed delivery) but functionally correct.

---

## Alternative Provider Assessment

If both the two-automation chain AND fallback fail, the migration shortlist is:
1. **Brevo Free** — native post-DOI redirect on free plan, France-based, 300 emails/day
2. **MailerLite Paid ($10/mo)** — native post-DOI redirect, Lithuania-based

Full evaluation in `02-05-MAILING-LIST-ANALYSIS.md`.
