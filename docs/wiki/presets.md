# Presets

OpenSpatialDelay includes 8 factory presets and supports saving/loading user presets as JSON files.

## Factory Presets

### Default
- **Taps:** 4, at -45, +45, -135, +135 degrees azimuth (diagonal cross)
- **Delay:** 500 ms | **Feedback:** 30% | **Algorithm:** VBAP
- **Pitch:** 0 st | **Filters:** HP 20 Hz, LP 20 kHz

### Stereo Ping-Pong
- **Taps:** 2, at -90 and +90 degrees azimuth (hard left/right)
- **Delay:** 350 ms | **Feedback:** 50% | **Algorithm:** VBAP
- **Pitch:** 0 st | **Filters:** HP 20 Hz, LP 18 kHz

### Circle (Quad)
- **Taps:** 4, at 0, +90, +180, -90 degrees azimuth (equidistant ring)
- **Delay:** 250 ms | **Feedback:** 40% | **Algorithm:** VBAP
- **Pitch:** 0 st | **Filters:** HP 20 Hz, LP 20 kHz

### Surround 5.1
- **Taps:** 5, at 0 (C), -30 (L), +30 (R), -110 (Ls), +110 (Rs) degrees
- **Delay:** 300 ms | **Feedback:** 35% | **Algorithm:** VBAP
- **Pitch:** 0 st | **Filters:** HP 20 Hz, LP 20 kHz

### Surround 7.1
- **Taps:** 7, at 0 (C), -30 (L), +30 (R), -90 (Ls), +90 (Rs), -135 (Lrs), +135 (Rrs) degrees
- **Delay:** 250 ms | **Feedback:** 35% | **Algorithm:** VBAP
- **Pitch:** 0 st | **Filters:** HP 20 Hz, LP 20 kHz

### Atmos 7.1.4
- **Taps:** 11 total
  - 7 ear-level: 0, -30, +30, -90, +90, -135, +135 degrees at 0 elevation
  - 4 height: -45, +45, -135, +135 degrees at +45 elevation
- **Delay:** 200 ms | **Feedback:** 30% | **Algorithm:** VBAP
- **Pitch:** 0 st | **Filters:** HP 20 Hz, LP 20 kHz

### Rising Spiral
- **Taps:** 8, spiraling upward from -20 to +50 degrees elevation with increasing azimuth
- **Trajectory:** Orbit on all taps, speeds from 1.5 to 3.2
- **Delay:** 200 ms | **Feedback:** 40% | **Algorithm:** VBAP
- **Pitch:** +2 st | **Doppler:** 0.2 on all taps | **Filters:** HP 40 Hz, LP 16 kHz

### Falling Cascade
- **Taps:** 6, descending from +40 to -35 degrees elevation at increasing distance
- **Delay:** 350 ms | **Feedback:** 45% | **Algorithm:** VBAP
- **Pitch:** -1 st | **Doppler:** 0.1 on all taps | **Filters:** HP 30 Hz, LP 14 kHz

## User Presets

### Saving

1. Click the **Save** button in the header bar.
2. Enter a name in the dialog.
3. The preset is saved as a JSON file.

### File Location

```
~/Library/Application Support/OpenSpatialDelay/Presets/{name}.json
```

### Browsing

- Use the **ComboBox dropdown** in the header bar to see all available presets.
- Use the **Prev** and **Next** arrow buttons to step through presets sequentially.
- Factory presets appear first in the list, followed by user presets sorted alphabetically.

## What Presets Store

Presets capture and restore **all** of the following:

**Global parameters:**
- Delay Time, Tempo Sync, Note Division, Sync Mode
- Feedback
- LP Filter, HP Filter
- Pitch Shift
- Dry/Wet, Input Gain, Output Gain
- Air Absorption toggle
- Algorithm, HRTF Profile

**Per-tap parameters (x12):**
- Enabled state
- Azimuth, Elevation, Distance
- Doppler Amount
- Trajectory Shape, Trajectory Speed

## What Presets Do NOT Store

The following are excluded from presets and persist independently:

- **Output Format** -- tied to DAW bus configuration
- **ADM-OSC Enabled** state
- **OSC Receive Port**

## See Also

- [Getting Started](getting-started.md) -- using presets in your first session
- [Parameter Reference](parameters-reference.md) -- full parameter list with IDs and ranges
