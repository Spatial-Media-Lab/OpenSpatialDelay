# Troubleshooting

Common issues and their solutions. If your issue is not listed here, please [open an issue on GitHub](https://github.com/AndrewRahman/OpenSpatialDelay/issues).

## Plugin Not Appearing in DAW

| Symptom | Cause | Solution |
|---|---|---|
| Plugin not in plugin list | Plugin files not in the correct directory | **macOS:** Copy AU to `~/Library/Audio/Plug-Ins/Components/` and VST3 to `~/Library/Audio/Plug-Ins/VST3/`. **Windows:** Copy VST3 to `C:\Program Files\Common Files\VST3\` |
| Plugin was there before but disappeared | DAW plugin cache is stale | Rescan plugins in your DAW's preferences. Do **not** delete the entire plugin scan database -- just trigger a rescan. |
| AU validation fails (macOS) | Known issue with some AU host validation | Try using the VST3 version instead. AU validation is stricter and some hosts may flag non-standard behavior. |
| Plugin appears but cannot be loaded | Architecture mismatch | Verify you have the correct build: arm64 for Apple Silicon Macs, x64 for Windows. OpenSpatialDelay does not ship Intel (x86_64) macOS builds. |

## No Sound

| Symptom | Cause | Solution |
|---|---|---|
| No wet signal | DRY/WET at 0% | Increase the DRY/WET knob. Default is 50%. |
| No wet signal | All taps disabled | Enable at least one tap using the ON/OFF button in the bottom panel. |
| No wet signal | Input gain too low | Check the IN knob in the MIX section. Default is 0 dB. |
| Very quiet wet signal | Distance too far | Reduce the DIST value for your taps. Distance affects perceived volume via inverse-square attenuation. |
| No delay repeats | Feedback at 0% | Increase the FEEDBACK knob. You need some feedback for echoes beyond the first set of taps. |
| Dry signal but no wet | Plugin on a bus with no input signal | Ensure audio is routed to the track where the plugin is inserted. |

## Audio Issues

| Symptom | Cause | Solution |
|---|---|---|
| Harsh, building high frequencies | Self-oscillation without filtering | Enable FLT and set LP to 3000-6000 Hz. The feedback filters tame high-frequency buildup. |
| Rumbling low frequencies | Self-oscillation without HP filtering | Enable FLT and set HP to 100-300 Hz. |
| Clicking or pops | Buffer size too small | Increase your DAW's audio buffer size. 256 or 512 samples is recommended. |
| Audio cutting out | DAW speaker protection triggered | OpenSpatialDelay has a built-in output limiter, but extreme settings may still trigger some DAWs' protection. Reduce feedback or output gain. |
| Pitch sounds wrong | Cumulative pitch misunderstanding | Global pitch is cumulative (tap k = k * pitch). Per-tap pitch is additive. Check both values. See [Controls Reference](controls-reference.md). |
| Unexpected pitch warble | Wobble modulation enabled | Check the MOD section. If the toggle is on with a non-zero AMOUNT, delay time is being modulated. |
| Delay time not matching tempo | Tempo sync off | Enable SYNC in the DELAY section and set NOTE and MODE appropriately. |

## Spatial Issues

| Symptom | Cause | Solution |
|---|---|---|
| No spatial effect | Output format set to Stereo with Equal Power | Try Binaural for headphones or a Surround format for speakers. Stereo Equal Power produces standard L/R panning only. |
| Elevation not audible | Using a non-height output format | Elevation requires Binaural, height-enabled Surround (e.g., 7.1.4), or Ambisonics. Standard surround formats (5.1, 7.1) only pan in the horizontal plane. |
| Binaural sounds wrong | HRTF profile mismatch | Try different HRTF profiles -- spatial perception varies between individuals. Start with "Studio Reference" or "Immersive". |
| Binaural localization poor | Listening on speakers, not headphones | Binaural output is designed exclusively for headphone monitoring. On speakers, it will sound phasey and unfocused. Use Stereo or Surround for speaker playback. |
| Surround channels silent | Track channel count too low | Ensure your DAW track has enough output channels. A 7.1.4 format needs 12 channels. Check your DAW's track routing. |
| SML 13.1 channels silent | Track not configured for 14 discrete channels | In Reaper, set track channel count to 14 via the routing dialog. SpatialMediaLab 13.1 requires 14 discrete channels (13 speakers + 1 LFE on channel 14). |
| Algorithm dropdown disabled | Binaural or Ambisonics format selected | The algorithm selector is only active for Surround formats. Binaural uses HRTF convolution directly. Ambisonics uses spherical harmonic encoding. |
| Taps appear to jump when trajectory starts | Normal behavior | When enabling a trajectory, the tap moves from its knob position to the first point on the trajectory path. The knob position becomes the origin. |

## OSC Issues

| Symptom | Cause | Solution |
|---|---|---|
| OSC receive not working | Port conflict | Ensure no other application is using the same UDP port. Try a different port number. |
| OSC receive not working | Firewall blocking | Check your OS firewall settings. Allow UDP traffic on the configured port. |
| OSC positions not sticking | 500ms override timeout | OSC positions are temporary overrides. After 500ms of silence, trajectories resume. This is by design. |
| OSC send not reaching destination | Wrong IP or port | Verify the destination IP address and port number match the receiving application. Use `scripts/adm_osc_test.py --listen` to verify. |
| OSC coordinates seem inverted | Coordinate convention mismatch | OpenSpatialDelay uses ITU-R BS.2127-0 Cartesian convention: +x = right, +y = front, +z = up. Some tools use different conventions. |

## Preset Issues

| Symptom | Cause | Solution |
|---|---|---|
| Factory presets missing | Presets not installed | **macOS:** Rebuild the plugin (`cmake --build build --config Release`). The build installs presets automatically. **Windows:** Presets are installed on first launch. |
| Preset changes not saving | Saving to a factory preset location | Factory presets are overwritten on each build/update. Save user presets to the "User" category or a custom category. |
| Preset sounds different than expected | Output format differs | Presets do not store output format. A surround preset loaded in Binaural mode will translate positions but may sound different due to the rendering path. |
| Preset dropdown looks wrong | LookAndFeel issue | This was fixed in v0.9. Update to the latest version. |

## Performance Issues

| Symptom | Cause | Solution |
|---|---|---|
| High CPU usage | Many taps with HRTF convolution | Binaural mode with 12 active taps uses 12 HRTF convolvers. Reduce active taps, or switch to "Simple (Low CPU)" HRTF profile which uses no convolution. |
| High CPU usage | Many active trajectories | Trajectory animation is lightweight, but Doppler processing on moving taps adds CPU load. Reduce DOPPLER amount on taps where it is not needed. |
| High CPU usage | High Ambisonics order | 6th order Ambisonics (49 channels) requires significant processing. Use lower orders if possible. |
| Audio dropouts | Buffer size too small | Increase your DAW's audio buffer size to 512 or 1024 samples. |

## Build Issues (Source Compilation)

| Symptom | Cause | Solution |
|---|---|---|
| CMake error about JUCE | Submodule not initialized | Run `git submodule update --init --recursive` in the repository root. |
| zlib not found (macOS) | zlib not available | Install via Homebrew: `brew install zlib` |
| zlib not found (Windows) | vcpkg not configured | Follow the vcpkg setup instructions in the repository's CI workflow. |
| Link errors about mysofa | FetchContent failed | Ensure internet access during the first build. libmysofa is downloaded automatically. |
| Post-build install fails | Permission denied | The plugin installs to user-level directories (`~/Library/Audio/Plug-Ins/`). No sudo is needed. Check that the target directories exist. |
