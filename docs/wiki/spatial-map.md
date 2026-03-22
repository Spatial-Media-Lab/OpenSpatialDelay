# Spatial Map

The spatial map is the large circular visualization on the left side of the plugin window. It shows a top-down view of the 3D sound field, with the listener at the center. Each delay tap is represented as a colored dot positioned around the listener.

## Coordinate System

OpenSpatialDelay uses a spherical coordinate system with three dimensions:

### Azimuth (Horizontal Angle)

- **Range:** -180 to +180 degrees
- **0 degrees:** Directly in front of the listener
- **Positive values:** Right side (+90 = hard right)
- **Negative values:** Left side (-90 = hard left)
- **+/-180 degrees:** Directly behind

The spatial map displays compass labels at the cardinal positions:

- **F** -- Front (0 degrees)
- **B** -- Back (180 degrees)
- **L** -- Left (-90 degrees)
- **R** -- Right (+90 degrees)

### Elevation (Vertical Angle)

- **Range:** -90 to +90 degrees
- **0 degrees:** Ear level (horizontal plane)
- **+90 degrees:** Directly above
- **-90 degrees:** Directly below

Since the spatial map is a 2D top-down view, elevation is encoded visually through two properties of the tap dot:

- **Opacity:** Higher elevation = more opaque (1.0 at +90). Lower elevation = more transparent (0.3 at -90)
- **Size:** Higher elevation = larger dot. Lower elevation = smaller dot

This lets you see at a glance which taps are above or below the listener even on a flat display.

### Distance

- **Range:** 0.1 to 3.0 (normalized, not meters)
- **0.1:** Very close to the listener (near the center of the map)
- **3.0:** Far from the listener (near the edge of the map)

Distance affects perceived loudness (inverse-square attenuation) and, when air absorption is enabled, high-frequency rolloff.

## Distance Rings and Labels

The spatial map displays concentric rings at fixed real-world distances, labeled in meters:

- **1m** -- innermost ring
- **2m**
- **5m**
- **10m**
- **20m** -- outermost ring

These labels appear in a monospace font along the rings, helping you gauge approximate source distances. The mapping between the normalized distance parameter (0.1-3.0) and real-world meters is nonlinear, with closer distances having more resolution.

## Interacting with the Map

### Selecting a Tap

Click on a tap dot to select it. The selected tap is highlighted with a brighter glow, and its per-tap controls become active in the bottom panel.

### Moving a Tap

Click and drag a tap dot to reposition it on the map. Dragging updates two parameters simultaneously:

- **Azimuth** -- the angular position around the listener
- **Distance** -- how far from the center

> **Note:** Elevation cannot be changed by dragging on the map. Use the ELEV knob in the bottom panel to adjust elevation.

### Visual Feedback

Each tap has a unique color, consistent across the map and the tap selector at the bottom of the plugin. The 12 tap colors progress through the spectrum, making it easy to identify which tap is which.

**Enabled taps** appear as filled, glowing dots. **Disabled taps** are not shown on the map.

When a tap has an active trajectory, the map shows:

- A **glow trail** tracing the trajectory path (brightness varies along the trail -- brighter near the current position, dimmer further away)
- A **crosshair marker** at the tap's origin point (the knob position), showing where the trajectory is anchored
- The **animated dot** showing the tap's current position as it moves along the trajectory path

### Global Tap Drawer

The left edge of the spatial map features a collapsible drawer with 6 global offset knobs (AZIM, ELEV, DIST, DOPPLER, PITCH, SPEED). Click the **GLOBAL** handle to open or close it. The drawer overlays the spatial map with a semi-transparent background. See [Controls Reference -- Global Tap Controls](controls-reference.md#global-tap-controls-spatial-map-drawer) for full details.

### Star Field

The spatial map background features a subtle star field with twinkling dots, providing visual depth without distracting from the tap positions. This is purely decorative and does not represent any audio parameter.

## Relationship to Controls

The spatial map and the per-tap knobs in the bottom panel are linked:

| Map Interaction | Equivalent Knob |
|---|---|
| Horizontal drag position | AZIMUTH knob |
| Radial distance from center | DIST knob |
| Cannot change on map | ELEV knob |

Changes made on the map immediately update the corresponding knob values, and vice versa.

> **Tip:** For precise positioning, use the knobs directly. Double-click a knob to type an exact value. For quick spatial arrangement, drag taps on the map.
