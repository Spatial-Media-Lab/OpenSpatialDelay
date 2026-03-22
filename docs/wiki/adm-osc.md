# ADM-OSC Integration

OpenSpatialDelay supports the ADM-OSC protocol for external position control, enabling integration with spatial audio workstations, renderers, and custom tools.

## What is ADM-OSC?

ADM-OSC (Audio Definition Model over Open Sound Control) is an open protocol for communicating object-based audio positions over a network. It was developed by the spatial audio community to enable interoperability between tools like SPAT Revolution, L-ISA Controller, and other object-based audio systems.

The protocol transmits position data as OSC (Open Sound Control) messages over UDP, using a standardized namespace that maps directly to the ITU-R BS.2076 Audio Definition Model.

## OSC Receive

When enabled, OpenSpatialDelay listens for incoming ADM-OSC messages and maps them to tap positions.

### Enabling Receive

1. Toggle **RECV** on in the OSC section of the right panel
2. Set the **Port** to match the sending application (default: 4002)

### Supported Messages

OpenSpatialDelay responds to the following ADM-OSC messages, where `N` is the object number (1-12, mapping to taps 1-12):

| Message | Arguments | Description |
|---|---|---|
| `/adm/obj/N/azim` | float (degrees) | Set azimuth. Positive = right, negative = left. |
| `/adm/obj/N/elev` | float (degrees) | Set elevation. Positive = above, negative = below. |
| `/adm/obj/N/dist` | float | Set distance (normalized). |
| `/adm/obj/N/aed` | float, float, float | Set azimuth, elevation, and distance simultaneously. |
| `/adm/obj/N/xyz` | float, float, float | Set position in Cartesian coordinates (x, y, z). Automatically converted to polar. |
| `/adm/obj/N/x` | float | Set X position only (left-right). |
| `/adm/obj/N/y` | float | Set Y position only (front-back). |
| `/adm/obj/N/z` | float | Set Z position only (up-down). |

### Cartesian to Polar Conversion

When Cartesian coordinates are received (`/xyz`, `/x`, `/y`, `/z`), they are converted to polar using the following convention (per ITU-R BS.2127-0):

- **Azimuth:** `atan2(-x, y) * (180 / pi)` -- positive x is right, positive y is front
- **Elevation:** `atan2(z, sqrt(x^2 + y^2)) * (180 / pi)` -- positive z is up
- **Distance:** `sqrt(x^2 + y^2 + z^2)`

### Override Timeout

When OSC messages are being received for a tap, external control takes priority:

- The tap's position is set by incoming OSC data
- If the tap has an active trajectory, the trajectory is **paused** during OSC control
- After **500 milliseconds** of silence (no incoming OSC messages for that tap), external control releases and the trajectory resumes

This timeout ensures smooth handoff between external automation and internal trajectories.

### Interaction with Trajectories

When a tap has an active trajectory and OSC receive is active:

- Incoming OSC data sets the **origin position** (the same value the knobs control)
- The trajectory continues to compute its animated position relative to the new origin
- During active OSC reception, the trajectory animation is paused and the OSC position is used directly
- When OSC messages stop (after the 500ms timeout), the trajectory resumes from the last OSC-set origin

## Custom `/osd/` Namespace

In addition to the ADM-OSC standard namespace, OpenSpatialDelay provides a custom `/osd/` namespace for full parameter control.

### Per-Object Parameters (`/osd/obj/N/`)

Where `N` is the object number (1-12):

| Message | Arguments | Description |
|---|---|---|
| `/osd/obj/N/azim` | float (degrees) | Set azimuth (alias for `/adm/obj/N/azim`). |
| `/osd/obj/N/elev` | float (degrees) | Set elevation (alias for `/adm/obj/N/elev`). |
| `/osd/obj/N/dist` | float | Set distance (alias for `/adm/obj/N/dist`). |
| `/osd/obj/N/aed` | float, float, float | Set position (alias for `/adm/obj/N/aed`). |
| `/osd/obj/N/enabled` | float (0/1) | Enable or disable the tap. |
| `/osd/obj/N/doppler` | float (0-1) | Set Doppler amount. |
| `/osd/obj/N/pitch` | float (-24 to 24) | Set per-tap pitch shift in semitones. |
| `/osd/obj/N/trajectory` | float (0-13) | Set trajectory shape index. |
| `/osd/obj/N/speed` | float (0-5) | Set trajectory speed in Hz. |
| `/osd/obj/N/direction` | float (0/1) | Set trajectory direction (0 = Forward, 1 = Reverse). |
| `/osd/obj/N/input` | float (0-2) | Set input channel (0 = L+R, 1 = L, 2 = R). |

### Global Parameters (`/osd/global/`)

| Message | Arguments | Description |
|---|---|---|
| `/osd/global/delaytime` | float (1-2000) | Set delay time in ms. |
| `/osd/global/temposync` | float (0/1) | Enable/disable tempo sync. |
| `/osd/global/notedivision` | float | Set note division. |
| `/osd/global/syncmode` | float (0-2) | Set sync mode (0=Straight, 1=Dotted, 2=Triplet). |
| `/osd/global/feedback` | float (0-1) | Set feedback amount. |
| `/osd/global/filterlp` | float (200-20000) | Set LP filter frequency. |
| `/osd/global/filterhp` | float (20-5000) | Set HP filter frequency. |
| `/osd/global/filterlpq` | float (0.5-8) | Set LP filter resonance. |
| `/osd/global/filterhpq` | float (0.5-8) | Set HP filter resonance. |
| `/osd/global/filterenabled` | float (0/1) | Enable/disable feedback filters. |
| `/osd/global/drywet` | float (0-1) | Set dry/wet mix. |
| `/osd/global/inputgain` | float (dB) | Set input gain. |
| `/osd/global/outputgain` | float (dB) | Set output gain. |
| `/osd/global/algorithm` | float (0-10) | Set spatialization algorithm. |
| `/osd/global/hrtfprofile` | float (0-5) | Set HRTF profile. |
| `/osd/global/outputformat` | float (0-21) | Set output format. |
| `/osd/global/air` | float (0/1) | Enable/disable air absorption. |
| `/osd/global/wobble` | float (0/1) | Enable/disable wobble modulation. |
| `/osd/global/wobbleamount` | float (0-1) | Set wobble depth. |
| `/osd/global/wobblemorph` | float (0-1) | Set wobble waveform. |

### Global Tap Offsets (`/osd/global/tap*`)

These control the global tap offset knobs (the collapsible drawer on the spatial map). Each message sets the absolute offset value -- the plugin computes and applies the delta to all enabled taps.

| Message | Arguments | Description |
|---|---|---|
| `/osd/global/tapazimuth` | float (-180 to 180) | Set global azimuth offset in degrees. |
| `/osd/global/tapelevation` | float (-90 to 90) | Set global elevation offset in degrees. |
| `/osd/global/tapdistance` | float (-1 to 1) | Set global distance offset. |
| `/osd/global/tapdoppler` | float (-1 to 1) | Set global Doppler offset. |
| `/osd/global/tappitch` | float (-24 to 24) | Set global pitch offset in semitones. |
| `/osd/global/tapspeed` | float (-5 to 5) | Set global trajectory speed offset in Hz. |

> **Note:** Global tap offsets are not saved in presets. Loading a preset resets all offsets to 0.

## OSC Send

When enabled, OpenSpatialDelay broadcasts the current position of all taps over OSC.

### Enabling Send

1. Toggle **SEND** on in the OSC section of the right panel
2. Set the destination **IP** address (default: 127.0.0.1 for localhost)
3. Set the **Send Port** (default: 9000)

### Broadcast Format

Tap positions are sent as `/adm/obj/N/aed` messages at **30 Hz** with three float arguments: azimuth (degrees), elevation (degrees), and distance (normalized).

```
/adm/obj/1/aed -45.0 0.0 0.5
/adm/obj/2/aed  45.0 0.0 0.5
/adm/obj/3/aed -135.0 0.0 0.5
...
```

### Position-Change Gating

To minimize network traffic, OSC send uses position-change gating: messages are only sent when a tap's position has actually changed. Static taps do not generate repeated messages.

### What Gets Broadcast

- **Positions:** `/adm/obj/N/aed` -- animated position when trajectory is active, knob position otherwise
- **Per-object params:** `/osd/obj/N/` -- enabled, doppler, pitch, trajectory, speed, direction, input
- **Global params:** `/osd/global/` -- all global parameters (delay time, feedback, filters, etc.)
- **Global tap offsets:** `/osd/global/tap*` -- all 6 tap offset knob values
- All messages are **change-gated** -- only sent when the value has actually changed

## Use Cases

### SPAT Revolution Integration

1. Configure SPAT Revolution to send ADM-OSC on port 4002
2. Enable RECV in OpenSpatialDelay on port 4002
3. Tap positions in OpenSpatialDelay now follow SPAT Revolution's object positions

### L-ISA Controller

1. Set up L-ISA Controller to broadcast object positions via ADM-OSC
2. Enable RECV in OpenSpatialDelay with the matching port
3. Delay taps follow L-ISA object movements in real time

### Bidirectional Control

Enable both RECV and SEND simultaneously:

- RECV: Accept positions from an external controller
- SEND: Broadcast animated positions to an external renderer

This creates a workflow where you control origins from an external tool, while OpenSpatialDelay's trajectories generate animated positions that are reflected back to the renderer.

### Custom Scripts

OpenSpatialDelay includes a Python test script for ADM-OSC at `scripts/adm_osc_test.py`:

```bash
# Send a single position
python3 scripts/adm_osc_test.py --aed 1 45.0 0.0 0.5

# Orbit object 1 around the listener
python3 scripts/adm_osc_test.py --orbit 1

# Set per-object params
python3 scripts/adm_osc_test.py --obj 1 pitch -12
python3 scripts/adm_osc_test.py --obj 1 doppler 0.8

# Set global params
python3 scripts/adm_osc_test.py --global feedback 0.8
python3 scripts/adm_osc_test.py --global tapazimuth 45

# Listen for outgoing OSC messages
python3 scripts/adm_osc_test.py --listen 9000
```

This script supports position modes, per-object params, global params, and listen mode. You can also write your own OSC scripts using any OSC library (e.g., `python-osc`, `liblo`, `oscpack`).

### DAW Automation Recording

To record trajectory motion as automation:

1. Enable SEND in OpenSpatialDelay
2. Set up an OSC-to-automation bridge in your DAW (e.g., via Reaper's OSC integration)
3. Run a trajectory -- the animated positions are broadcast as standard ADM-OSC
4. Record the incoming automation in the DAW

## Network Configuration

### Localhost (Same Machine)

For applications running on the same computer:

- **IP:** 127.0.0.1
- **Ports:** Use different ports for send and receive to avoid conflicts

### Networked Machines

For communication between separate computers:

- Ensure both machines are on the same network
- Use the destination machine's local IP address
- Verify that firewalls allow UDP traffic on the chosen ports

> **Note:** ADM-OSC uses UDP, which is connectionless. There is no handshake or acknowledgment -- messages are fire-and-forget. If a message is lost due to network issues, the next message will correct the position.
