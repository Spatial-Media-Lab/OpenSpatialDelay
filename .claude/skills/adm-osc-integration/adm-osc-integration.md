---
name: adm-osc-integration
description: >
  ADM-OSC protocol implementation for JUCE audio plugins. Covers receiving object position
  updates (/adm/obj/N/azim, elev, dist, aed, xyz), sending position broadcasts at 30Hz,
  Cartesian-to-Polar conversion per ITU-R BS.2127-0, override timeout management, port
  configuration, and ecosystem interoperability with SPAT Revolution, L-ISA Controller,
  Dolby Atmos Renderer, DiGiCo, and Lawo consoles.
  Use when implementing ADM-OSC receive or send, integrating with spatial audio workstations,
  or debugging OSC communication issues.
allowed-tools: Read,Write,Edit,Bash,Glob,Grep,WebFetch,WebSearch
metadata:
  category: Development & Engineering
  pairs-with:
    - skill: spatial-audio-dsp
      reason: ADM-OSC controls the position of spatialized objects
    - skill: spatialcore-architecture
      reason: ADM-OSC is part of the SpatialCore framework
  tags:
    - adm-osc
    - osc
    - spatial-audio
    - juce
    - interoperability
    - spat-revolution
    - dolby-atmos
---

# ADM-OSC Integration

Implementing the ADM-OSC protocol in JUCE audio plugins for spatial audio ecosystem interoperability.

## What is ADM-OSC?

ADM-OSC is an industry initiative for communicating object-based audio position data over OSC (Open Sound Control). It enables real-time position exchange between spatial audio tools — DAWs, renderers, consoles, and plugins.

**Specification:** [github.com/immersive-audio-live/ADM-OSC](https://github.com/immersive-audio-live/ADM-OSC)

**Key participants:** L-Acoustics, FLUX:: Immersive, Radio France, d&b audiotechnik

## OSC Namespace

### Object Position Messages

| Message | Type | Range | Description |
|---------|------|-------|-------------|
| `/adm/obj/N/azim` | float | -180 to +180 | Azimuth in degrees |
| `/adm/obj/N/elev` | float | -90 to +90 | Elevation in degrees |
| `/adm/obj/N/dist` | float | 0 to 1 | Normalized distance |
| `/adm/obj/N/aed` | fff | az, el, dist | Combined position (most common) |
| `/adm/obj/N/xyz` | fff | -1 to +1 each | Cartesian position |
| `/adm/obj/N/x` | float | -1 to +1 | X axis only |
| `/adm/obj/N/y` | float | -1 to +1 | Y axis only |
| `/adm/obj/N/z` | float | -1 to +1 | Z axis only |

`N` = 1-based object index (object 1 = `/adm/obj/1/aed`)

### Coordinate Conventions

**Polar (ADM/ITU-R BS.2127-0):**
- Azimuth: 0° = front, +90° = left, -90° = right, ±180° = behind
- Elevation: 0° = ear level, +90° = overhead, -90° = below
- Distance: 0.0 = at listener, 1.0 = far field

**Cartesian:**
- X: -1 (right) to +1 (left)
- Y: -1 (behind) to +1 (front)
- Z: -1 (below) to +1 (above)

### Cartesian → Polar Conversion

```cpp
// Per ITU-R BS.2127-0
float azDeg = std::atan2(-x, y) * (180.0f / juce::MathConstants<float>::pi);
float elDeg = std::atan2(z, std::sqrt(x * x + y * y)) * (180.0f / juce::MathConstants<float>::pi);
float dist = std::sqrt(x * x + y * y + z * z);
```

**Key:** Note the `-x` in atan2 — ADM convention has +X = left, but atan2 expects +X = right.

## JUCE Implementation

### OSC Receive

```cpp
// In your Processor class:
juce::OSCReceiver oscReceiver;

// Use MessageLoopCallback (message thread, safe for APVTS updates)
oscReceiver.addListener(this, juce::OSCReceiver::MessageLoopCallback);

// Connect to port
oscReceiver.connect(port);  // Default: 4002

// Handle messages
void oscMessageReceived(const juce::OSCMessage& msg) override {
    auto pattern = msg.getAddressPattern().toString();

    // Parse /adm/obj/N/aed
    if (pattern.matchesWildcard("/adm/obj/*/aed", false)) {
        int objIndex = pattern.substring(9, pattern.indexOf("/aed")).getIntValue();
        if (objIndex >= 1 && objIndex <= 12 && msg.size() >= 3) {
            float az = msg[0].getFloat32();
            float el = msg[1].getFloat32();
            float dist = msg[2].getFloat32();
            handleOSCPosition(objIndex - 1, az, el, dist);  // Convert to 0-based
        }
    }
}
```

### Override Timeout

When OSC is actively controlling an object, the plugin's internal position (knobs, trajectories) should be temporarily overridden. Use a 500ms timeout:

```cpp
static constexpr int OSC_OVERRIDE_TIMEOUT_MS = 500;
int64_t lastOscUpdateTime[MAX_OBJECTS] = {};

void handleOSCPosition(int objIdx, float az, float el, float dist) {
    lastOscUpdateTime[objIdx] = juce::Time::currentTimeMillis();

    // Update APVTS parameters (thread-safe via message thread callback)
    apvts.getParameter("object" + String(objIdx) + "_azimuth")->setValueNotifyingHost(
        apvts.getParameterRange("object" + String(objIdx) + "_azimuth").convertTo0to1(az));
    // ... same for elevation, distance
}

bool isOscOverriding(int objIdx) {
    return (juce::Time::currentTimeMillis() - lastOscUpdateTime[objIdx]) < OSC_OVERRIDE_TIMEOUT_MS;
}
```

### OSC Send

Broadcast object positions at 30Hz with position-change gating:

```cpp
juce::OSCSender oscSender;
float lastSentAz[MAX_OBJECTS] = {};
float lastSentEl[MAX_OBJECTS] = {};
float lastSentDist[MAX_OBJECTS] = {};

// Called from 60Hz timer (every other tick = 30Hz)
void sendOSCPositions() {
    if (!oscSendEnabled || !oscSender.isConnected()) return;

    for (int i = 0; i < numActiveObjects; ++i) {
        float az = currentAzimuth[i];
        float el = currentElevation[i];
        float dist = currentDistance[i];

        // Position-change gating — only send if position changed
        if (std::abs(az - lastSentAz[i]) > 0.1f ||
            std::abs(el - lastSentEl[i]) > 0.1f ||
            std::abs(dist - lastSentDist[i]) > 0.01f) {

            oscSender.send("/adm/obj/" + juce::String(i + 1) + "/aed",
                           az, el, dist);

            lastSentAz[i] = az;
            lastSentEl[i] = el;
            lastSentDist[i] = dist;
        }
    }
}
```

### Port and Connection Management

```cpp
// Persist port in plugin state (ValueTree, not APVTS)
void getStateInformation(juce::MemoryBlock& destData) override {
    auto state = apvts.copyState();
    state.setProperty("oscReceivePort", oscReceivePort, nullptr);
    state.setProperty("oscSendIP", oscSendIP, nullptr);
    state.setProperty("oscSendPort", oscSendPort, nullptr);
    // ... serialize
}

// Connection managed by timer (reconnect on port change)
void timerCallback() override {
    if (oscReceivePortChanged) {
        oscReceiver.disconnect();
        oscReceiver.connect(oscReceivePort);
        oscReceivePortChanged = false;
    }
}
```

## Ecosystem Interoperability

| System | Role | Port Convention |
|--------|------|-----------------|
| **SPAT Revolution** | Send + Receive | Configurable (typically 9000-9100) |
| **L-ISA Controller** | Send | Configurable |
| **Dolby Atmos Renderer** | Receive | 4001 |
| **DiGiCo SD consoles** | Send | Configurable |
| **Lawo consoles** | Send | Configurable |
| **SML Plugins** | Send + Receive | Default receive: 4002, send: 4003 |

### Typical Workflow

```
SPAT Revolution ──/adm/obj/N/aed──→ SML Plugin (receive on 4002)
SML Plugin ──/adm/obj/N/aed──→ External renderer (send to 4001)
```

## Interaction with Trajectories

When both OSC and trajectories are active:
- **OSC receive sets the origin position** (knob values) when trajectory is active
- The trajectory continues to animate relative to the new origin
- When OSC updates stop (500ms timeout), trajectory continues from last origin
- **OSC send broadcasts the animated position** (not the origin), so external systems see the actual moving position

## Testing

Use the included Python test script:
```bash
python3 scripts/adm_osc_test.py --mode send --port 4002     # Send test positions
python3 scripts/adm_osc_test.py --mode listen --port 4003   # Verify OSC Send output
python3 scripts/adm_osc_test.py --mode orbit --port 4002    # Orbit animation
```

## Key References

- ITU-R BS.2076-2 — Audio Definition Model
- ITU-R BS.2127-0 — ADM renderer reference
- ADM-OSC specification — [github.com/immersive-audio-live/ADM-OSC](https://github.com/immersive-audio-live/ADM-OSC)
- Wright (2002) — Open Sound Control 1.0 Specification, CNMAT
