# Interface Overview

The OpenSpatialDelay interface is divided into four zones arranged around a central spatial map. The window size is **820 x 580 pixels**.

## Layout Zones

```
+---------------------------------------------------------------+
|                        HEADER BAR (56px)                      |
+------------------+----------------------------+---------------+
|                  |                            |               |
|                  |       SPATIAL MAP          |  RIGHT PANEL  |
|                  |       (center)             |  (264px wide) |
|                  |                            |               |
+------------------+----------------------------+---------------+
|                   BOTTOM PANEL (150px)                        |
+---------------------------------------------------------------+
```

## Header Bar

From left to right:

| Element | Description |
|---------|-------------|
| **Plugin Title** | "OpenSpatialDelay v0.6" displayed left-aligned |
| **Preset Browser** | ComboBox dropdown (130px) + Prev/Next buttons (24px each) + Save button (44px) |
| **OSC Toggle + Port** | "OSC" toggle button (42px) + editable port label (50px, default 4002) |
| **Output Format** | Dropdown (120px) selecting the output format (Binaural, Stereo, Surround, Ambisonics) |
| **Algorithm / HRTF Profile** | Dropdown (120px) -- shows Algorithm for surround output, or HRTF Profile for binaural. These share the same position; only one is visible at a time. |

## Spatial Map (Center)

The spatial map is a **top-down polar view** of the 3D sound field.

- **Orientation labels:** F (front/top), B (back/bottom), L (left), R (right)
- **Distance rings:** 4 concentric rings at 0.25, 0.50, 0.75, and 1.0 normalized distance
- **Center dot:** White dot at the origin representing the listener position
- **Objects:** Colored numbered dots that can be **dragged** to reposition in azimuth and distance
- **Elevation encoding (IEM standard):**
  - Objects above ear level appear as **larger, brighter** dots (full opacity fill)
  - Objects below ear level appear as **smaller, dimmer** dots (0.3 alpha fill)
  - Dot size varies by +/- 3 pixels based on elevation angle
  - When an object is selected and has non-zero elevation, its **elevation in degrees** is shown as a text label near the dot
- **OSC override indicator:** When an object is receiving ADM-OSC position data, an "OSC" label appears in cyan beneath the dot

## Right Panel (264px)

The right panel contains three sections of rotary knobs:

### DELAY Section

| Control | Description |
|---------|-------------|
| **Input Gain** | -60 to +12 dB (default 0 dB) |
| **Delay Time** | 1 to 2000 ms (default 500 ms). Replaced by Note Division knob when Tempo Sync is active. |
| **Tempo Sync** | Toggle button. When ON, delay time locks to musical note divisions. |
| **Sync Mode** | Dropdown: Notes, Triplet, Dotted, 16th |
| **Feedback** | 0% to 100% (default 30%) |
| **Pitch Shift** | -24 to +24 semitones (default 0 st) |

### TONE Section

| Control | Description |
|---------|-------------|
| **HP Filter** | High-pass, 20 Hz to 5000 Hz (default 20 Hz) |
| **LP Filter** | Low-pass, 200 Hz to 20000 Hz (default 20000 Hz) |
| **AIR** | Air Absorption toggle (right-aligned in section header) |

### MIX Section

| Control | Description |
|---------|-------------|
| **Dry/Wet** | 0% to 100% (default 50%) |
| **Output Gain** | -60 to +12 dB (default 0 dB) |

## Bottom Panel (150px)

### Object Selector Row

12 color-coded buttons arranged horizontally. Each button selects the corresponding delay tap for editing. Enabled objects show their number in full color; disabled objects appear dimmed.

Object colors are evenly distributed across the hue spectrum (HSV with 0.85 saturation, 0.95 value):

| Object | Color |
|--------|-------|
| 1 | Red |
| 2 | Orange |
| 3 | Yellow |
| 4 | Chartreuse |
| 5 | Green |
| 6 | Spring |
| 7 | Cyan |
| 8 | Azure |
| 9 | Blue |
| 10 | Violet |
| 11 | Magenta |
| 12 | Rose |

### Per-Object Controls

When an object is selected, the following controls appear:

| Control | Description |
|---------|-------------|
| **ON/OFF** | Enable/disable toggle for this delay tap |
| **Azimuth** | -180 to +180 degrees (rotary knob) |
| **Elevation** | -90 to +90 degrees (vertical slider) |
| **Distance** | 0.0 to 1.0 normalized (rotary knob) |
| **Doppler** | 0% to 100% doppler effect amount (rotary knob) |
| **Trajectory** | Shape dropdown (None, Spiral, Orbit, Bounce, Figure-8, Random) |
| **Speed** | Trajectory speed 0.0 to 10.0 (rotary knob) |

## Color Scheme

| Token | Hex | Usage |
|-------|-----|-------|
| `bg` | `#0a0a14` | Main background |
| `headerBg` | `#111827` | Header bar background |
| `panelBorder` | `#1e293b` | Borders, distance rings, dividers |
| `textPrimary` | `#e2e8f0` | Primary text, listener dot |
| `textSecondary` | `#b0bec5` | Secondary labels |
| `textDim` | `#64748b` | Disabled/inactive text |
| `accentCyan` | `#00d4ff` | Plugin title, OSC labels, active toggles |
| `accentPurple` | `#8B5CF6` | Filter/pitch knob thumbs, sync ON state |
| `accentAmber` | `#f5c542` | Doppler and trajectory speed knob thumbs |
| `sectionText` | `#7c8da0` | Section headers (DELAY, TONE, MIX) |
| `knobBg` | `#1e293b` | Knob background fill |

## See Also

- [Parameter Reference](parameters-reference.md) -- full parameter details and ID naming conventions
- [Spatialization Algorithms](algorithms-guide.md) -- algorithm selection and behavior
