# Presets

OpenSpatialDelay includes 70 factory presets across 9 categories, plus a user preset system for saving and loading your own settings.

## Factory Presets

### Categories

| Category | Count | Description |
|---|---|---|
| Classic Delays | 8 | Traditional delay effects adapted for spatial audio -- ping-pong, slapback, dotted eighth, tape echo, dub, and more |
| Spatial Movement | 18 | Taps with trajectories and spatial animation -- orbits, spirals, figure-8 weaves, bouncing patterns, and showcases of all 14 trajectory shapes |
| Ambient + Texture | 8 | Atmospheric, evolving textures -- shimmer, frozen cascades, dark matter, grain clouds, echo chambers |
| Height + 3D | 6 | Presets that emphasize vertical movement and 3D positioning -- falling cascades, overhead arcs, dome rings, vertical ping-pong |
| Surround Production | 6 | Designed for surround speaker layouts -- 5.1, 7.1, 7.1.4 Atmos, quad swirl, rhythmic, and wide stereo |
| Wobble + Modulated | 6 | Tape-inspired and LFO-modulated delay effects -- tape wow, flutter, warped echo, chorus delay, seasick, subtle motion |
| Creative + Experimental | 8 | Extreme and unusual effects -- self-oscillation, reverse spirals, stereo split, pitch ladders, Doppler storms |
| Rhythmic | 10 | Rhythmically patterned delays -- tresillo, cinquillo, polyrhythms, clave patterns, Morse code, Fibonacci spacing |

### Full Preset List

**Classic Delays:** Quad Ping-Pong, Stereo Ping-Pong, Slapback, Dotted Eighth, Tape Echo, Dub Delay, Quarter Note Bounce, Multi-Tap Cascade

**Spatial Movement:** Cardinal, Rising Spiral, Orbit Dance, Figure-8 Weave, Bouncing Balls, Spiral Descent, Random Walk, Pendulum, Heartbeat, Box Step, Trigonometry, Crossing Paths, Merry-Go-Round, Lemniscate, Back and Forth, Corkscrew Duo, Comet Trail, Kaleidoscope

**Ambient + Texture:** Shimmer, Frozen Cascade, Dark Matter, Ethereal Wash, Grain Cloud, Echo Chamber, Drift, Fifth Ghost

**Height + 3D:** Falling Cascade, Rain, Hemisphere Spread, Overhead Arc, Dome Ring, Vertical Ping-Pong

**Surround Production:** Surround 5.1, Surround 7.1, Atmos 7.1.4, Quad Swirl, 5.1 Rhythmic, Wide Stereo

**Wobble + Modulated:** Tape Wow, Tape Flutter, Warped Echo, Chorus Delay, Seasick, Subtle Motion

**Creative + Experimental:** Self-Oscillation, Reverse Spiral, Stereo Split, Pitch Ladder, Doppler Storm, Micro Delay, Wide Scatter, Broken Record

**Rhythmic:** Tresillo, Cinquillo, Offbeat Pong, West African Bell, Son Clave 3-2, Paradiddle Pong, Polyrhythm 3v4, Morse SOS, Fibonacci Scatter, Dotted Gallop

## Browsing Presets

### Dropdown Menu

Click the preset name in the header bar to open the preset browser. Presets are organized into category submenus. Click a category to expand it, then click a preset to load it.

### Previous / Next Navigation

Use the arrow buttons on either side of the preset name to step through presets sequentially. This cycles through all presets across all categories in order.

### Preset Indicator

The preset name in the header shows the currently loaded preset. If you modify any parameter after loading a preset, the name remains displayed but the settings may differ from the stored preset.

## Saving Presets

### Save Overlay

Click the **Save** button in the header bar to open the preset save overlay. This appears as a modal dialog within the plugin window.

| Field | Description |
|---|---|
| Name | The preset name. Pre-filled with the current preset name (with "Copy" appended for factory presets). |
| Category | The category folder for the preset. Defaults to "User". You can select any existing category or type a new one. |

Click **Save** to write the preset to disk. Click **Cancel** to close without saving.

### What Gets Saved

A preset stores all parameters except the output format:

- All global parameters (delay time, sync settings, feedback, filters, pitch, mix, wobble)
- All per-tap parameters (enabled state, position, pitch, Doppler, trajectory, input channel)
- Filter resonance values (HP Q, LP Q)
- Filter and air absorption toggle states

### What Does NOT Get Saved

- **Output format** -- intentionally excluded so presets work regardless of your monitoring setup
- **OSC settings** -- port, IP, and enable states are session-specific
- **HRTF profile** -- monitoring preference, not a creative setting
- **Algorithm** -- determined by output format context

> **Tip:** Because output format is not saved in presets, you can load a "Surround 5.1" preset while monitoring in Binaural -- the spatial positions translate naturally.

## File Format

Presets are saved as `.osdpreset` files in JSON format. Each file contains all parameter values in a human-readable structure.

### File Locations

| Platform | Path |
|---|---|
| macOS | `~/Library/Audio/Presets/OpenSpatialDelay/` |
| Windows | `%APPDATA%\OpenSpatialDelay\Presets\` |

Within the preset directory, presets are organized into subdirectories by category:

```
~/Library/Audio/Presets/OpenSpatialDelay/
  Classic Delays/
    Default.osdpreset
    Stereo Ping-Pong.osdpreset
    ...
  Spatial Movement/
    Circle (Quad).osdpreset
    Rising Spiral.osdpreset
    ...
  User/
    My Custom Preset.osdpreset
    ...
```

### Factory vs. User Presets

- **Factory presets** are installed during the build process (macOS) or on first launch (Windows). They are regenerated on each update and should not be manually edited -- your changes will be overwritten on the next update.
- **User presets** are stored in the "User" category subdirectory (or any custom category you create). These are never touched by updates.

> **Tip:** If you want to customize a factory preset, load it, modify the settings, then save it with a new name in the "User" category.

### JSON Structure

The `.osdpreset` file format is a simple JSON object:

```json
{
  "name": "My Preset",
  "category": "User",
  "delayTime": 500.0,
  "tempoSync": false,
  "noteDivision": 4.0,
  "syncMode": 0,
  "feedback": 0.3,
  "filterLP": 5000.0,
  "filterHP": 50.0,
  "filterLPQ": 0.707,
  "filterHPQ": 0.707,
  "pitchShift": 0.0,
  "dryWet": 0.5,
  "inputGain": 0.0,
  "outputGain": 0.0,
  "airAbsorption": false,
  "filterEnabled": false,
  "wobbleEnabled": false,
  "wobbleAmount": 0.0,
  "wobbleMorph": 0.0,
  "taps": [
    {
      "enabled": true,
      "azimuth": -45.0,
      "elevation": 0.0,
      "distance": 0.5,
      "doppler": 0.0,
      "pitchShift": 0.0,
      "trajectory": "none",
      "speed": 1.0,
      "direction": 0,
      "inputChannel": 0
    }
  ]
}
```

This format is designed for both machine parsing and manual editing. You can create presets by hand or generate them with scripts if needed.
