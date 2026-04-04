# Controls Reference

This page documents every parameter in OpenSpatialDelay, organized by its location in the UI. The plugin window is 820 x 580 pixels, divided into three main areas:

- **Spatial Map** (left) -- top-down visualization of tap positions
- **Right Panel** (264px wide) -- global parameters organized by signal flow
- **Bottom Panel** (150px tall) -- per-tap parameters for the selected tap

## Header Bar

The header bar spans the top of the plugin and contains navigation, routing, and format controls.

| Control | Type | Description |
|---|---|---|
| Preset Name | Dropdown | Click to open the preset browser. Shows current preset name. |
| Prev/Next Arrows | Buttons | Step through presets sequentially. |
| Save | Button | Opens the preset save overlay to save the current settings. |
| OSC RECV | Toggle | Enables ADM-OSC receive. See [ADM-OSC Integration](adm-osc.md). |
| Output Format | Dropdown | Selects one of 23 output formats. See [Output Formats](output-formats.md). |
| Algorithm / HRTF Profile | Dropdown | Context-dependent: shows spatialization algorithm for surround formats, HRTF profile for Binaural, stereo mode for Stereo. Hidden when not applicable. |
| SML Badge | Button | Links to spatialmedialab.org. |

## DELAY Section (Right Panel)

Controls the core delay engine.

| Parameter | Range | Default | Description |
|---|---|---|---|
| TIME | 1 - 2000 ms | 500 ms | Base delay time. Tap 1 plays at 1x this value, tap 2 at 2x, tap 3 at 3x, and so on. Displayed in seconds when >= 1000 ms. |
| SYNC | On/Off | Off | Enables tempo synchronization. When on, delay time is locked to the DAW's tempo using the NOTE and MODE settings. |
| NOTE | 1/32 - 2/1 | 1/4 | Note division for tempo sync. Available values: 1/32, 1/16, 1/8, 1/4, 1/2, 1/1, 2/1. Only active when SYNC is on. |
| MODE | Straight / Dotted / Triplet | Straight | Modifies the note division. Dotted = 1.5x duration. Triplet = 2/3 duration. Only active when SYNC is on. |
| FEEDBACK | 0 - 100% | 30% | Amount of output fed back into the delay input. Higher values = more repeats. Values above 95% produce self-oscillation (safe -- output limiter prevents clipping). |

## MOD Section (Right Panel)

Wobble modulation adds tape-style delay time variation.

| Parameter | Range | Default | Description |
|---|---|---|---|
| MOD Toggle | On/Off | Off | Enables wobble modulation. When off, no CPU is used. |
| AMOUNT | 0 - 100% | 0% | Depth of the delay time modulation. Higher values = more pronounced pitch warble. |
| MORPH | 0 - 100% | 0% | Waveform shape of the modulation LFO. 0% = smooth sine wave (gentle wow). 50% = triangle (classic tape flutter). 100% = square wave (dramatic pitch jumps). |

> **Tip:** For realistic tape echo, try AMOUNT around 30-50% with MORPH at 0% (sine). For glitchy digital effects, push MORPH toward 100% (square).

## TONE Section (Right Panel)

Feedback filters shape the tonal character of repeats over time.

| Parameter | Range | Default | Description |
|---|---|---|---|
| FLT Toggle | On/Off | Off | Enables the feedback filters. When off, filters are completely bypassed (zero CPU). |
| HP | 20 - 5000 Hz | 50 Hz | High-pass filter cutoff frequency. Removes low frequencies from the feedback path. Higher values = thinner repeats over time. |
| LP | 200 - 20000 Hz | 5000 Hz | Low-pass filter cutoff frequency. Removes high frequencies from the feedback path. Lower values = darker repeats over time. |
| HP RES | 0.50 - 8.00 | 0.71 | High-pass filter resonance (Q factor). 0.71 = Butterworth (flat). Higher values add a resonant peak at the cutoff frequency. |
| LP RES | 0.50 - 8.00 | 0.71 | Low-pass filter resonance (Q factor). 0.71 = Butterworth (flat). Higher values add a resonant peak at the cutoff frequency. |
| AIR | On/Off | Off | Enables air absorption simulation. Distant taps lose high-frequency content based on their distance from the listener, modeling real-world sound propagation. Uses a quadratic distance curve. |

> **Note:** The filters are in the feedback path, not the output path. They shape each successive repeat -- the first echo passes through the filters once, the second echo twice, and so on. This creates a natural darkening/thinning effect over time.

## MIX Section (Right Panel)

Output level controls.

| Parameter | Range | Default | Description |
|---|---|---|---|
| DRY/WET | 0 - 100% | 50% | Balance between the dry (unprocessed) and wet (delayed) signal. 0% = fully dry. 100% = fully wet. |
| IN | -inf - +40 dB | 0 dB | Input gain applied to the signal entering the delay engine. Does not affect the dry signal path. |
| OUT | -inf - +12 dB | 0 dB | Output gain applied to the final mixed signal. |

> **Note:** Input gain only affects the delay (wet) path. The dry signal passes through unmodified. This is intentional -- it allows you to boost or cut the delay input independently from the dry signal.

## OSC Section (Right Panel)

ADM-OSC networking controls for external position automation.

| Parameter | Range | Default | Description |
|---|---|---|---|
| RECV | On/Off | Off | Enables the ADM-OSC receiver. When on, incoming OSC messages can control tap positions. |
| Port | 1 - 65535 | 4002 | UDP port for receiving ADM-OSC messages. |
| SEND | On/Off | Off | Enables the ADM-OSC sender. When on, tap positions are broadcast over OSC at 30 Hz. |
| IP | IP address | 127.0.0.1 | Destination IP address for OSC send. |
| Send Port | 1 - 65535 | 9000 | UDP port for sending ADM-OSC messages. |

See [ADM-OSC Integration](adm-osc.md) for full details on the protocol and use cases.

## Per-Tap Controls (Bottom Panel)

The bottom panel shows controls for the currently selected tap. Click a tap number (1-12) to select it. The controls update to reflect that tap's settings.

| Parameter | Range | Default | Description |
|---|---|---|---|
| ON/OFF | On/Off | Taps 1-4: On, Taps 5-12: Off | Enables or disables this tap. Disabled taps produce no audio and are hidden from the spatial map. |
| AZIMUTH | -180.0 - +180.0 deg | Varies by tap | Horizontal angle. 0 = front, +90 = right, -90 = left, +/-180 = behind. The knob is reversed (clockwise rotation = clockwise movement on the map), matching the IEM StereoEncoder convention. |
| ELEV | -90.0 - +90.0 deg | 0 deg | Elevation angle. 0 = ear level, +90 = directly above, -90 = directly below. Only audible in binaural and surround formats with height speakers. |
| DIST | 0.00 - 1.00 | 0.50 | Distance from the listener (normalized). Affects perceived loudness and, when AIR is enabled, high-frequency attenuation. |
| DOPPLER | 0 - 100% | 0% | Doppler effect intensity. When a tap is moving (via trajectory or OSC), its pitch shifts based on approach/recede velocity, simulating the real-world Doppler effect. |
| PITCH | -12 - +12 st | 0 st | Per-tap pitch shift in semitones. Added on top of the global cumulative pitch. Uses Phase Vocoder (STFT with Laroche-Dolson phase locking) to preserve timing -- the delay rhythm stays locked regardless of pitch shift amount. |
| INPUT | L+R / L / R | L+R | Input channel routing (only active when the DAW provides a stereo input). L+R = summed mono. L = left channel only. R = right channel only. |
| TRAJ | 14 shapes | None | Trajectory shape for automated spatial movement. See [Trajectories](trajectories.md) for all shapes. |
| SPEED | 0.00 - 5.00 Hz | 0.30 Hz | Trajectory animation speed. Higher values = faster movement. |
| DIR | Forward / Reverse | Forward | Trajectory direction. Forward = the default motion path. Reverse = the path plays backward. |

### Tap Number Display

The bottom panel includes numbered buttons (1-12) for selecting taps. Each number is color-coded to match the tap's color on the spatial map. Active taps show a bright indicator; inactive taps are dimmed.

### Double-Click to Type

All rotary knobs support double-click to enter a precise value. Click on the knob, type the number, and press Enter to confirm.

## Global Tap Controls (Spatial Map Drawer)

A collapsible panel on the left edge of the spatial map provides 6 global offset knobs. Turning a global knob adjusts the matching parameter on **all enabled taps simultaneously** by the same delta, preserving their spatial arrangement.

**Example:** If taps are at AZ -45, +45, -135, +135 and the global AZ knob is turned +10, all taps shift to -35, +55, -125, +145.

### Opening the Drawer

Click the handle strip labeled **GLOBAL** on the left edge of the spatial map. The drawer slides open with a smooth animation. Click the handle again to close it. The open/close state is saved with your DAW session.

### Global Knobs

| Knob | Affects | Range | Behavior |
|---|---|---|---|
| AZIM | Azimuth | +/-180 deg | Wraps continuously at +/-180 (no hard stop) |
| ELEV | Elevation | +/-90 deg | Clamps at +/-90 |
| DIST | Distance | +/-1.0 | Clamps at parameter limits |
| DOPPLER | Doppler Amount | +/-1.0 | Clamps at parameter limits |
| PITCH | Pitch Shift | +/-12 st | Clamps at +/-12 semitones |
| SPEED | Trajectory Speed | +/-5.0 Hz | Clamps at parameter limits |

### Important Details

- **Enabled taps only** -- global knobs only affect taps that are currently enabled. Disabled taps are not modified.
- **Value readout** -- each knob displays its current offset value below the knob, with unit suffixes (deg, st, Hz).
- **Preset reset** -- all global offsets snap to 0 whenever a preset is loaded (via the menu, prev/next buttons, or save overlay). Per-tap knob values show the final result -- the offset is baked in.
- **Not automatable** -- global knobs are UI-only controls, not DAW parameters. They are not saved in presets and cannot be automated. Use [OSC control](adm-osc.md) for external automation.
- **Double-click** -- double-click any global knob to reset it to 0.

## Parameter Interaction Notes

### Per-Tap Pitch Shift

Each tap has an independent pitch shift (+/-12 semitones) using Phase Vocoder (STFT with Laroche-Dolson phase locking). This preserves the delay rhythm -- the tap's timing stays locked regardless of pitch shift amount. The global PITCH knob in the drawer offsets all enabled taps simultaneously.

### Feedback Read Position

The feedback path always reads from the last enabled tap in the chain. If taps 1, 2, and 4 are enabled but tap 3 is disabled, feedback reads from the position of tap 4 (not tap 3).

### Filter Toggle Independence

The FLT toggle and the AIR toggle are independent. You can have feedback filters on with air absorption off, or vice versa. They affect different parts of the signal:

- **FLT:** Shapes the feedback loop (affects all repeats progressively)
- **AIR:** Shapes each tap's output based on distance (affects the spatial output path)
