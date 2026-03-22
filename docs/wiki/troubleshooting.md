# Troubleshooting

## No Audio Output

- **Check output format:** Ensure the selected output format matches your DAW bus configuration. If the DAW provides 2 channels but a surround format is selected, audio may not route correctly.
- **Check Dry/Wet:** If Dry/Wet is set to 0%, only the dry signal passes through. Increase to hear the spatial delay effect.
- **Check object state:** At least one object (delay tap) must be **enabled** (ON) for wet signal to be produced. Open the bottom panel and verify the selected object shows "ON."

## Plugin Not Found by DAW

Verify the plugin is installed in the correct location:

- **macOS AU:** `~/Library/Audio/Plug-Ins/Components/`
- **macOS VST3:** `~/Library/Audio/Plug-Ins/VST3/`
- **Windows VST3:** `C:\Program Files\Common Files\VST3\`

After placing the plugin file, trigger a **plugin rescan** in your DAW (the procedure varies by DAW).

## Glitchy or Chirpy Audio on Second Feedback Cycle

This is a known issue related to DAW bus layout renegotiation. The DAW may renegotiate the bus layout after the plugin reports its channel configuration, causing buffer discontinuities on subsequent feedback cycles.

For details and workarounds, see `docs/bug-reports/BUS_LAYOUT_BUG.md`.

**Immediate recovery:** Restart the DAW or reload the plugin to clear the corrupted state.

## OSC Not Receiving

- **Check the port:** Ensure the configured port (shown next to the OSC toggle, default 4002) is not in use by another application.
- **Check the toggle:** The OSC toggle in the header bar must be **ON** (highlighted).
- **Check the firewall:** Ensure your system firewall allows incoming UDP on the configured port.
- **Check the namespace:** The sender must use the `/adm/obj/N/` namespace with **1-based** object indices (N = 1-12). Verify with a tool like `oscdump` or the included test script.
- **Check the sender address:** The sender should target `localhost` (or the machine's IP address) on the correct port.

## HRTF Profile Loading Slowly

Loading a SOFA profile involves reading and resampling the HRIR data from the embedded SOFA file. Larger files (e.g., SADIE II D2 KU100 at approximately 35 MB) take longer to load.

This loading is **double-buffered** -- audio continues through the previous profile without dropout while the new profile loads in the background. The switch is atomic once loading completes.

## Trajectory Not Moving

- **Check the shape:** If the trajectory shape is set to "None," no animation occurs.
- **Check the speed:** If the trajectory speed is 0.0, the trajectory is frozen at its current phase position. Increase speed above 0 to see motion.
- **Check OSC override:** If ADM-OSC is actively sending position data for the object, trajectories are paused for that object. An "OSC" label appears on the spatial map when an object is under OSC control. Disable OSC or stop the sender to resume trajectory animation.

## Presets Not Saving

Verify write permissions on the user preset directory:

```
~/Library/Application Support/OpenSpatialDelay/Presets/
```

If the directory does not exist, the plugin creates it on the first save. If creation fails, check that `~/Library/Application Support/` is writable.

## Elevation Not Visible on Map

Elevation is encoded visually on the 2D spatial map using dot size and opacity, following the IEM standard:

- **Above ear level:** Larger, brighter dots (full opacity)
- **Below ear level:** Smaller, dimmer dots (0.3 alpha)

To see the exact elevation in degrees, **select an object** by clicking its button in the bottom panel or clicking the dot on the map. When the selected object has non-zero elevation (greater than +/-1 degree), a degree label appears near the dot.

## See Also

- [Getting Started](getting-started.md) -- initial setup and verification
- [ADM-OSC Integration](adm-osc-integration.md) -- OSC configuration details
- [Presets](presets.md) -- preset file format and locations
