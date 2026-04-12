---
name: synth-ui-components
description: Synthesizer-specific UI widget designs for JUCE plugins including oscillator waveform display, filter response curve, modulation matrix grid, FM operator topology diagram, envelope shape editor, and wavetable viewer
---

# Synthesizer UI Components

Design patterns and implementation guidance for synth-specific UI widgets in SML plugins. All components follow the SMLLookAndFeel dark theme (#0A0A14 bg, cyan/purple/amber accents).

## Oscillator Waveform Display

### Purpose
Shows the current waveform shape in real-time as the user adjusts oscillator parameters.

### Design
- **Size:** 120x60px (compact), 200x100px (expanded)
- **Background:** Lifted dark (#14141E)
- **Waveform stroke:** Cyan (#00D4FF) for active oscillator, muted gray (#4A4A5A) for inactive
- **Stroke width:** 1.5px
- **Center line:** Subtle horizontal at y=0 (0.3 opacity)
- **Label:** Oscillator name below (e.g., "OSC A")

### Implementation Notes
- Render wavetable at display resolution (one pixel per sample)
- For FM: show the modulated waveform, not just the carrier
- For wavetable: show the current frame based on morph position
- Update at 30Hz (timer-based), not per-sample
- Use `juce::Path` for anti-aliased waveform rendering

### Interaction
- Click to cycle through waveform types (saw, square, triangle, sine)
- Drag vertically to adjust level
- Right-click for extended menu (fine-tune, waveform import)

## Filter Response Curve

### Purpose
Visualizes the filter frequency response (magnitude vs frequency) with cutoff, resonance, and type clearly shown.

### Design
- **Size:** 200x80px minimum
- **Background:** Lifted dark (#14141E)
- **Curve stroke:** Purple (#8B5CF6) for filter A, amber (#F5C542) for filter B
- **Stroke width:** 2px
- **Fill:** Gradient below curve at 15% opacity
- **Frequency axis:** Logarithmic (20Hz-20kHz), subtle tick marks at 100, 1k, 10k
- **Magnitude axis:** -24dB to +12dB (for resonance peaks)
- **Cutoff indicator:** Vertical dashed line at cutoff frequency
- **Resonance peak:** Visible bump at cutoff point

### Implementation Notes
- Compute magnitude response from filter coefficients at ~200 frequency points
- Use SVF coefficients for accurate visualization (matches audio filter)
- Animate smoothly when cutoff/resonance change (10ms lerp)
- Show bypass state as grayed-out curve with "OFF" overlay

### Interaction
- Drag horizontally to adjust cutoff frequency
- Drag vertically to adjust resonance
- Scroll to change filter type (LP, HP, BP, notch)

## Modulation Matrix Grid

### Purpose
Visual routing table showing all modulation source-to-destination connections with amounts.

### Design
- **Layout:** Grid with sources as rows, destinations as columns
- **Cell size:** 32x32px per intersection
- **Active connection:** Filled circle with amount (cyan for positive, rose for negative)
- **Inactive cell:** Empty or subtle dot
- **Row headers (sources):** ENV 1, ENV 2, LFO 1, LFO 2, VEL, MW, AT, SLIDE
- **Column headers (destinations):** PITCH, CUTOFF, RES, AMP, AZ, EL, DIST, MIX, FM

### Color Coding
- **Positive amount:** Cyan (#00D4FF) circle, size proportional to amount
- **Negative amount:** Rose (#FF6B8A) circle
- **Zero/inactive:** No circle, subtle grid line
- **Hover:** Highlight row and column

### Implementation Notes
- Max 8 sources x 12 destinations = 96 cells
- Only show destinations relevant to current synth mode
- Spatial destinations (AZ, EL, DIST) highlighted with spatial icon
- Each cell is a drag target: drag up/down to set amount (-100% to +100%)
- Double-click to type exact value

### Interaction
- Click cell to toggle on/off
- Drag cell up/down to adjust amount
- Right-click for menu (exact value, remove, copy routing)

## FM Operator Topology Diagram

### Purpose
Shows the DX7-style algorithm — which operators connect to which, and which are carriers vs modulators.

### Design
- **Size:** 200x150px
- **Background:** Lifted dark (#14141E)
- **Operator boxes:** 36x36px rounded squares
  - Carrier: Cyan border, bright fill
  - Modulator: Purple border, dim fill
  - Inactive: Gray border, no fill
- **Connection lines:** 1.5px stroke, arrows showing signal flow direction
- **Feedback loop:** Curved arrow from operator back to itself
- **Operator label:** "1" through "6" centered in box
- **Level indicator:** Small horizontal bar below each operator (0-99)

### Layout Pattern
```
Operators arranged to match standard DX7 algorithm diagrams:
- Carriers at bottom (output)
- Modulators above (signal flows downward)
- Feedback arrows curve right and back
```

### Implementation Notes
- Pre-define layout coordinates for all 32 algorithms
- Animate connections when switching algorithms (fade out old, fade in new)
- Highlight the selected operator for editing
- Show envelope shape mini-preview inside each operator box

### Interaction
- Click operator to select it for editing (shows detail panel)
- Click algorithm number to switch algorithms (1-32)
- Drag between operators to create/remove connections (advanced mode)

## Envelope Shape Editor

### Purpose
Visual ADSR (or DX7 4-rate/4-level) envelope editor with draggable breakpoints.

### Design
- **Size:** 180x80px (compact), 300x120px (expanded)
- **Background:** Lifted dark (#14141E)
- **Envelope line:** 2px stroke, color matches envelope destination:
  - Amplitude: Cyan (#00D4FF)
  - Filter: Purple (#8B5CF6)
  - Pitch: Amber (#F5C542)
- **Fill:** Gradient below line at 10% opacity
- **Breakpoints:** 6px circles at A, D, S, R transitions
- **Time axis:** Proportional (auto-scales to fit)
- **Level axis:** 0% to 100%
- **Grid:** Subtle horizontal lines at 25%, 50%, 75%

### ADSR Mode
```
    /\
   /  \___________
  /    \          \
 /      \          \
A   D      S       R
```

### DX7 Mode (4-rate, 4-level)
```
    L1──────L2
   /          \──────L3
  / R1     R2  \  R3  \
 /              \       \──L4
Start            Release
```

### Implementation Notes
- Render curve using `juce::Path` with quadratic bezier segments
- Breakpoints are draggable: horizontal = time, vertical = level
- Show current playback position as animated dot moving along curve
- Snap to grid when holding Shift

### Interaction
- Drag breakpoints to adjust attack, decay, sustain level, release
- Double-click breakpoint to type exact value
- Right-click for curve type (linear, exponential, logarithmic)

## Wavetable 3D Viewer

### Purpose
Shows all frames of a wavetable as a pseudo-3D "waterfall" display, with the current morph position highlighted.

### Design
- **Size:** 200x150px minimum
- **Background:** Lifted dark (#14141E)
- **Waveform lines:** Stacked with perspective offset (each frame shifted right and up)
- **Current frame:** Full opacity cyan (#00D4FF)
- **Other frames:** Reduced opacity (0.15-0.4), gradient from dark to light
- **Morph position indicator:** Highlighted frame with glow
- **Frame count label:** "Frame 23/64" in corner

### Layout
```
Frame 64: ~~~~~~~~~~~~  (back, dimmest)
Frame 48: ~~~~~~~~~~~~
Frame 32: ~~~~~~~~~~~~
Frame 16: ~~~~~~~~~~~~  (front, brightest)
Frame 1:  ~~~~~~~~~~~~  (selected, cyan glow)
```

### Implementation Notes
- Render 8-16 evenly spaced frames (not all frames, for performance)
- Perspective: each successive frame offset by (+2px, -4px) from previous
- Current frame rendered last (on top) with full opacity and glow effect
- Animate morph position changes (smooth scroll through frames)
- For wavetable files with 256+ frames, subsample to show every Nth frame

### Interaction
- Drag vertically to morph through frames
- Scroll to zoom in/out on frame detail
- Click to set morph position to specific frame

## General Design Rules (SML Consistency)

1. **4px grid alignment** for all widget positioning
2. **8px padding** inside component backgrounds
3. **6px corner radius** for component containers
4. **1px border** using lifted-background color
5. **Font:** System default at 10-11px for labels, 9px for axis ticks
6. **Colors:** Always use SMLLookAndFeel color constants, never hardcode
7. **Animation:** 10ms lerp for parameter changes, 200ms for mode switches
8. **Hover state:** +10% lightness on interactive elements
9. **Active/selected:** Cyan border or glow, +20% lightness
10. **Disabled:** 30% opacity, no interaction response
