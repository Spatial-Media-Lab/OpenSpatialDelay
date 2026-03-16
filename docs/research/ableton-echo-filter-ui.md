# Ableton Echo Filter UI — Research Notes

## Sources
- Official Ableton Live 12 manual
- Echo product page (ableton.com/en/packs/echo/)
- Related Auto Filter and Delay documentation (same Ableton design language)

## Filter Section Structure
Echo's filter section lives in the **Echo Tab** (one of three tabs: Echo, Modulation, Character):
- **Filter Toggle** — on/off switch for both HP and LP together
- **HP slider** — cutoff frequency of the high-pass filter
- **HP Res slider** — resonance of the HP filter (adjacent to HP slider)
- **LP slider** — cutoff frequency of the low-pass filter
- **LP Res slider** — resonance of the LP filter
- **Filter Display** — collapsible graphical visualization of filter curves
- **Triangular toggle** — shows/hides the Filter Display

## Filter Display — Visual Appearance
- Frequency response graph (frequency Hz axis left-to-right, amplitude vertical)
- Combined HP + LP frequency response drawn as a smooth curve
- The **passband baseline sits at the top** of the graph (0 dB line)
- Resonance peaks **rise ABOVE the passband baseline** — they are visible as bumps/peaks above the flat passband level at the cutoff frequencies
- Rolloff slopes descend below the baseline
- The area under the curve (passband) is filled/shaded

## Two Draggable Filter Dots — Interaction Model
From the manual: "You can also adjust the filter parameters by clicking and dragging either of the **filter dots** in the Filter Display."

- **Two separate dots** (not one combined handle)
- **One dot for HP filter** — sits on the curve at the HP cutoff
- **One dot for LP filter** — sits on the curve at the LP cutoff

## How Dragging Works
Each dot is an **XY control point**:
- **Horizontal drag = frequency** (cutoff point left/right)
- **Vertical drag = resonance (Q)** (up = more resonance, peak grows taller above baseline)

This matches Ableton's established interaction pattern:
- Auto Filter: "drag the handle to change the value of the Freq and Res controls"
- Delay: "click and drag on the vertical axis" for bandwidth
- Echo: Same paradigm but with TWO dots

## Resonance Visualization — KEY DETAIL
When resonance is increased:
- The **resonance peak rises ABOVE the passband** at the cutoff frequency
- The passband level stays fixed (doesn't drop)
- Higher Q = taller/sharper peak above the baseline
- This is standard EQ/filter visualization in all DAWs

**CRITICAL**: The normalization approach must NOT scale the entire curve down when peaks exceed 0 dB. Instead, allocate headroom above the passband for peaks to rise into. The passband should remain at a fixed Y position.

## Key Design Takeaways for OpenSpatialDelay
1. Two separate dots (HP and LP), not one combined handle
2. XY interaction per dot: X=frequency, Y=resonance
3. Separate HP Res and LP Res values
4. Resonance peaks visible ABOVE the passband baseline
5. Passband sits at a fixed Y position, peaks rise above it
6. Fill the area under the curve
7. Dots sit directly on the curve at their cutoff frequencies
