# ADM-OSC Integration

OpenSpatialDelay receives object position data via the **ADM-OSC** protocol (Audio Definition Model over Open Sound Control), enabling real-time control from external spatializers, DAWs, and automation tools.

## Setup

1. **Enable OSC:** Click the **OSC** toggle button in the header bar.
2. **Set the port:** Click the port label (default **4002**) to edit. Valid range: 1024-65535. The port is persisted with the plugin state.
3. **Configure sender:** Point your ADM-OSC sender to `localhost` (or the machine's IP) on the configured UDP port.

## Protocol Details

- **Transport:** UDP
- **Namespace:** `/adm/obj/N/` where N is a **1-based** object index (1-12)
- **Convention:** ADM-OSC per EBU Tech 3396 / ITU-R BS.2127-0

## Message Reference

| Address Pattern | Arguments | Description |
|----------------|-----------|-------------|
| `/adm/obj/N/azim` | float (degrees) | Set azimuth. Range: -180 to +180. |
| `/adm/obj/N/elev` | float (degrees) | Set elevation. Range: -90 to +90. |
| `/adm/obj/N/dist` | float | Set normalized distance. Range: 0 to 1. |
| `/adm/obj/N/aed` | float, float, float | Set azimuth, elevation, and distance together. |
| `/adm/obj/N/xyz` | float, float, float | Set Cartesian position (auto-converted to polar). |
| `/adm/obj/N/x` | float | Set individual X coordinate. |
| `/adm/obj/N/y` | float | Set individual Y coordinate. |
| `/adm/obj/N/z` | float | Set individual Z coordinate. |

For individual `/x`, `/y`, `/z` messages, the plugin maintains internal Cartesian state per object and recomputes the full polar position on each update.

## Cartesian-to-Polar Conversion

When Cartesian coordinates are received (`/xyz`, `/x`, `/y`, `/z`), the conversion follows ITU-R BS.2127-0:

```
azimuth  = atan2(-x, y) * (180 / pi)
elevation = atan2(z, sqrt(x^2 + y^2)) * (180 / pi)
distance  = clamp(0, 1, sqrt(x^2 + y^2 + z^2))
```

## Behavior

- **Parameter update:** Received positions are written to APVTS parameters via `setValueNotifyingHost`, making them visible to the DAW's automation system and the plugin UI simultaneously.
- **Trajectory override:** When OSC messages arrive for an object, the trajectory animation for that object is **paused**. An "OSC" label appears on the spatial map beneath the affected object.
- **Timeout:** If no OSC message is received for an object within **500 ms**, the override is released and the trajectory animation resumes (if one was active).
- **Clamping:** Received values are clamped before application: azimuth to -180/+180, elevation to -90/+90, distance to 0/1.

## Compatible Senders

Any ADM-OSC compliant tool can control OpenSpatialDelay, including:

- **Nuendo** (Steinberg) -- native ADM-OSC output
- **SPAT Revolution** (FLUX::) -- ADM-OSC send
- **IEM StereoEncoder** / IEM MultiEncoder -- OSC output
- Custom scripts using the ADM-OSC namespace

## Test Script

A Python test script is included for verifying OSC connectivity:

```bash
pip install python-osc
python3 scripts/adm_osc_test.py
```

The script sends test position messages to all 12 objects, cycling through various positions.

## See Also

- [Trajectory Animation](trajectory-system.md) -- how trajectories interact with OSC override
- [Parameter Reference](parameters-reference.md) -- ADM-OSC enabled parameter and per-object position parameters
