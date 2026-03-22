# Frequently Asked Questions

## What does the spatial map represent?

The spatial map is a **top-down polar view** of the 3D sound field around the listener.

- **F** = front (top of map), **B** = back (bottom), **L** = left, **R** = right
- Distance from center = normalized object distance (0 at center, 1 at outer ring)
- Azimuth convention: 0 degrees = front, positive values = right (negative = left)
- The 4 concentric rings mark distance intervals of 0.25

## How does elevation show on the 2D map?

Elevation is encoded using the IEM-standard visual convention:

- Objects **above** ear level appear as **larger, brighter** dots (full opacity fill)
- Objects **below** ear level appear as **smaller, dimmer** dots (30% opacity fill)
- Dot size varies by +/-3 pixels based on the sine of the elevation angle

When you **select an object** (click its button or dot), its exact elevation in degrees is displayed as a text label near the dot, provided the elevation is more than +/-1 degree from horizontal.

## Can I use this with headphones?

Yes. Select **Binaural** as the output format (this is the default). The plugin will render spatial audio using HRTF-based binaural processing suitable for headphone listening. Choose an [HRTF Profile](hrtf-profiles.md) that suits your preference -- "Simple (Low CPU)" for minimal overhead, or one of the SOFA-based profiles for more realistic spatialization.

## What is the maximum Ambisonics order?

**6th order**, requiring 49 output channels. The available Ambisonics orders are:

| Order | Channels | Short Name |
|-------|----------|------------|
| 1st (FOA) | 4 | FOA |
| 2nd (SOA) | 9 | SOA |
| 3rd (HOA) | 16 | HOA |
| 4th | 25 | 4OA |
| 5th | 36 | 5OA |
| 6th | 49 | 6OA |

All Ambisonics output uses **AmbiX** format (ACN channel ordering, SN3D normalization).

## Can I automate parameters in my DAW?

Yes. All parameters are exposed through JUCE's `AudioProcessorValueTreeState` (APVTS) and are fully automatable. This includes global parameters (delay, feedback, filters, etc.) and all per-object parameters (azimuth, elevation, distance, trajectory, etc.). See the [Parameter Reference](parameters-reference.md) for the complete list of parameter IDs.

## What sample rates are supported?

Any sample rate supported by your DAW. The plugin adapts to the session sample rate in `prepareToPlay`. SOFA HRTF data is automatically resampled to match the session sample rate during profile loading.

## How do I test ADM-OSC?

Use the included Python test script:

```bash
pip install python-osc
python3 scripts/adm_osc_test.py
```

This sends test position messages to all 12 objects on the default port (4002). Make sure the OSC toggle is enabled in the plugin before running the script. See [ADM-OSC Integration](adm-osc-integration.md) for full protocol details.

## What is the maximum delay time?

**2000 ms** (2 seconds) per tap. This applies to both the global Delay Time parameter and the per-object Time parameter. With 12 taps active, the plugin maintains a delay buffer sufficient for all taps at maximum delay time.

## How many delay taps can be active?

Up to **12 simultaneously**. Each tap has independent spatial position (azimuth, elevation, distance), delay time, doppler amount, and trajectory animation. Enable or disable individual taps using the ON/OFF button in the bottom panel.

## Does the plugin support multichannel input?

Input is **folded to mono** internally before entering the delay engine. Regardless of how many input channels the DAW provides, all input channels are summed to a single mono signal before processing. The spatial positioning of delay taps then distributes this mono signal across the output channels according to the selected algorithm and output format.

## See Also

- [Getting Started](getting-started.md) -- initial setup walkthrough
- [Output Formats](output-formats.md) -- full list of supported output formats
- [ADM-OSC Integration](adm-osc-integration.md) -- OSC setup and message reference
