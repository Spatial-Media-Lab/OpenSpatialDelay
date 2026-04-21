# Next-Session Prompt — Signal Flow Manual Regen

> Paste the block below into a fresh Claude Code session when you're
> ready to regenerate the DOCX and PDF manual with the new diagram.

---

You're picking up the last mile of OpenSpatialDelay issue #168.

The signal-flow PNG was redrawn in pass 4 ("Orbital Horizon") and
signed off by the user. PR #211 was merged to main
(`git log --oneline | head -5` should show merge commit
`11437c2`). The new PNG is at `docs/assets/signal-flow.png` —
4800 × 1800, 8:3 aspect, rendered by `docs/generate_signal_flow.py`
with `SCALE = 2`.

Remaining work: regenerate the DOCX and PDF manual so they pick up
the new diagram. Per the standing "no manual regen until images
final" rule, that rule is now satisfied — the diagram is final.

## Step-by-step

1. Start from clean main:
   ```
   git pull origin main
   git checkout -b AndrewRahman/manual-regen-signal-flow
   ```

2. Regenerate the PNG (deterministic, should produce byte-identical
   output to what's in main; run anyway so the build chain is
   exercised from scratch):
   ```
   python3 docs/generate_signal_flow.py
   ```

3. Regenerate the DOCX manual:
   ```
   NODE_PATH=/opt/homebrew/lib/node_modules node docs/generate_manual.js
   ```

4. Convert to PDF:
   ```
   soffice --headless --convert-to pdf \
     docs/OpenSpatialDelay_Manual_v1.0.docx \
     --outdir docs/
   ```

5. **Visually verify** the signal-flow section of the PDF at its
   actual render size (480 × 180 pt inside the A4 page) reads
   cleanly. Specifically check:
   - INPUT / OUTPUT block labels are legible.
   - 12-tap fan structure is readable.
   - Panel titles (PER-TAP DSP, SPATIAL RENDER) are legible.
   - No distortion, no clipping, no alignment issues.
   - Colour coding (cyan / amber / violet) survives the PDF
     compression and print rendering.

   If anything reads as too small or too faint at manual scale,
   bump `SCALE` in `generate_signal_flow.py` and / or adjust
   specific font sizes in the generator — but do NOT change the
   composition. The user signed off on the composition.

6. Commit atomically:
   ```
   git add docs/assets/signal-flow.png \
           docs/OpenSpatialDelay_Manual_v1.0.docx \
           docs/OpenSpatialDelay_Manual_v1.0.pdf
   git commit -m "docs(manual): regenerate with pass 4 signal flow diagram"
   ```

7. Push and open a PR against main:
   ```
   git push -u origin AndrewRahman/manual-regen-signal-flow
   gh pr create --base main --title "Manual regen — pass 4 signal flow" ...
   ```

8. After merge, close issue #168 if nothing else is outstanding on
   it. If there's a backlog, look at
   `docs/MANUAL_UPDATE_PLAN.md` for the next-up item.

## Context to carry

- `docs/SIGNAL_FLOW_READABILITY_PLAN.md` — full history of passes
  1-4, the aspect-ratio bug that was masking the geometry, the
  Orbital Cartography directives, structural truths preserved.
- `docs/design-philosophy.md` — the brand's own words on the
  orbital aesthetic. Honour them if you need to adjust anything.
- `agent_docs/architecture.md` — the actual DSP chain. The
  diagram must not misrepresent this.
- User memory `feedback_no_manual_regen_until_images_final.md` —
  reason the manual was frozen through passes 1-4.
- User memory `feedback_marketing_copy_depth.md` — no DSP jargon
  on public surfaces. Pitch Shift, not Phase Vocoder.

## What NOT to do

- Don't change the diagram's composition, colours, or labels.
  Signed off.
- Don't commit any DOCX/PDF produced by a dev soffice with the
  "sans" font fallback warning treated as an error — check the
  output visually. If fonts fall back in the DOCX, investigate
  `docs/generate_manual.js` font loading before committing.
- Don't introduce `killall AudioComponentRegistrar` or any other
  plugin-testing commands — this task is docs-only.
