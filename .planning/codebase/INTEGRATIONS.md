# External Integrations

**Analysis Date:** 2026-04-14

## APIs & External Services

**OSC (Open Sound Control):**
- ADM-OSC send: Broadcast spatial object positions at 30Hz to external control systems
  - Address patterns: `/adm/obj/{N}/aed` (azimuth, elevation, distance), `/osd/obj/{N}/*`, `/osd/global/*`
  - Implementation: `Source/PluginProcessor.cpp` lines 1631-1729, `oscMessageReceived()` at line 5493
  - Send via: `juce::OSCMessage` (JUCE OSC module)

- ADM-OSC receive: Accept spatial parameter updates via OSC messages
  - Purpose: External motion control from sequencers, motion capture systems, or mixing consoles
  - Override timeout: 500ms since last receive (line 1631)
  - Enable/disable toggle: Non-APVTS member `oscReceiveEnabled` (line 406)
  - Handler: `oscMessageReceived()` in `Source/PluginProcessor.cpp`

**Audio I/O:**
- VST3 plugin host integration via JUCE Audio Plugin Client
- AU plugin host integration (macOS only)
- No MIDI input/output (configured `NEEDS_MIDI_INPUT=FALSE`, `NEEDS_MIDI_OUTPUT=FALSE`)
- Surround output support: Configurable speaker layouts for 5.0, 5.1, 7.0, 7.1, 9.0 Ambisonics, HOA 25/36/49-channel

## Data Storage

**Databases:**
- Not applicable (no persistent database)

**File Storage:**
- **HRTF SOFA Files:** Read-only at runtime
  - 5 embedded HRTF profiles (binary data embedded at build time):
    - MIT KEMAR Large Pinna (`HRTF/mit_kemar_large_pinna.sofa`)
    - SADIE D2-KU100 (`HRTF/sadie_d2_ku100.sofa`)
    - CIPIC Subject 003 (`HRTF/cipic_subject_003.sofa`)
    - HUTUBS PP2 (`HRTF/hutubs_pp2.sofa`)
    - BernSchuetz KU100 (`HRTF/bernschuetz_ku100.sofa`)
  - Loaded via libmysofa: `mysofa_open_data()` in `Source/PluginProcessor.cpp` line 551
  - Loaded synchronously (required for test harness binaural tests)

- **Preset Data:** VST3 state save/restore (APVTS ValueTree)
  - Persisted by host DAW (Ableton, REAPER, Logic Pro, etc.)
  - Both spatial params and delay effect params included
  - Implementation: `Source/PresetData.cpp`, JUCE parameter attachment system

- **Fonts:** Read-only at runtime
  - 8 font files embedded at build time (DM Sans, JetBrains Mono, Roboto)
  - Loaded via FontData namespace (JUCE binary data)
  - Used by PluginEditor for UI rendering

**Caching:**
- Shared FFT Cache: `Source/SharedFFTCache.h` - Partition-based FFT caching for phase vocoder
  - Thread-safe access patterns
  - Partitioned convolver FFT reuse

## Authentication & Identity

**Auth Provider:**
- Not applicable (no user authentication required)

**Plugin Identification:**
- Company code: `Smli` (4-char code)
- Plugin code: Versioned (base `Os10` for v1.0, unique codes for each patch version via `build_version.sh`)
  - Example: `O100` (v1.0.0), `O101` (v1.0.1)
- CFBundleIdentifier (macOS):
  - Base: `com.spatialmedialab.OpenSpatialDelay`
  - Versioned suffix for multi-build coexistence: `com.spatialmedialab.OpenSpatialDelay.v1-0-Z`
  - Patched at build time in `build_version.sh` line 57

## Monitoring & Observability

**Error Tracking:**
- Not applicable (no external error tracking service)

**Logs:**
- JUCE logging via `juce::Logger` (internal only)
- Plugin state available to host via parameter callbacks (APVTS)

## CI/CD & Deployment

**Hosting:**
- GitHub Releases (frozen v0.1-v0.9 builds, never modified)
- GitHub Actions CI: macOS and Windows builds on each commit to main branch

**CI Pipeline:**
- **macOS:** `.github/workflows/build-macos.yml`
  - Runner: `macos-14` (Apple Silicon)
  - Builds: arm64 AU + VST3
  - Artifact retention: 90 days
  - Post-build: Remove quarantine flags with `xattr -cr`
  - Deployment: Manual versioned builds via `build_version.sh` with bundle ID patching and code signing

- **Windows:** `.github/workflows/build-windows.yml`
  - Runner: `windows-latest`
  - Builds: x64 VST3 only
  - Artifact retention: 90 days
  - Dependency: zlib via vcpkg (`x64-windows-static`)
  - Code signing: SignPath integration available (commented)

## Environment Configuration

**Required env vars:**
- None required at runtime (all configuration compile-time or host-provided)

**Secrets location:**
- Not applicable (no secrets)
- SignPath code signing token (GitHub Actions secret): Optional, commented in CI workflow

## Webhooks & Callbacks

**Incoming:**
- VST3/AU parameter callbacks from host DAW
  - JUCE APVTS (AudioProcessorValueTreeState) parameter change listeners
  - Both global parameters (algorithm, HRTF profile, sample rate, etc.) and per-object spatial parameters

**Outgoing:**
- OSC broadcast messages (ADM-OSC) to external control systems
  - Sent at 30Hz from processBlock
  - Addresses: `/adm/obj/{N}/aed`, `/osd/obj/{N}/*`, `/osd/global/*`
  - Implementation: `Source/PluginProcessor.cpp` lines 1631-1729

## Third-Party Licenses

**Bundled in Binary:**
- JUCE 8.0.3: GPL-3.0 (commercial license available)
- libmysofa v1.3.2: BSD 3-Clause
- zlib: zlib License
- Fonts (DM Sans, JetBrains Mono, Roboto): SIL OFL 1.1 (DM Sans, JetBrains), Apache 2.0 (Roboto)

**Attribution:**
- Legal notices generated via `docs/generate_legal_notices.js`
- License table in `agent_docs/licensing.md`

---

*Integration audit: 2026-04-14*
