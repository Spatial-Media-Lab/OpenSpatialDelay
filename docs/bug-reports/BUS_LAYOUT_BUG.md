# Recurring Bug: isBusesLayoutSupported Causes Tap System Failure

## Symptom Pattern (ALWAYS THE SAME)
1. Taps play one time through correctly (first cycle)
2. Second cycle (feedback) is glitchy, chirpy, sounds pitched up even with pitch at 0
3. After second cycle, taps never play again until DAW is closed and re-opened
4. Stop/Play in DAW does NOT fix it — only full DAW restart

## Root Cause
Adding new `AudioChannelSet` entries to `isBusesLayoutSupported()` causes DAWs
(confirmed on macOS) to **renegotiate the bus layout during or shortly after playback starts**.
This renegotiation triggers the JUCE lifecycle sequence:

```
releaseResources() → prepareToPlay() → processBlock()
```

This mid-session `prepareToPlay()` call:
- **Zeros the entire delay buffer** (`delayBuffer.assign(size, 0.0f)`)
- **Resets writePosition to 0**
- **Resets feedbackSample to 0**
- **Resets all pitch shifter phases**

The delay buffer corruption produces the "glitchy/pitched" second cycle (residual
audio draining from the buffer with wrong timing), followed by permanent silence
(empty buffer, no feedback, no new audio entering).

## Why It Recurs
Every time new output formats are added, the natural instinct is to add their
corresponding `AudioChannelSet` to `isBusesLayoutSupported()`. This changes the
DAW-facing bus API, triggering renegotiation. The bug re-appears with identical symptoms.

## The Fix (Permanent)
**NEVER modify `isBusesLayoutSupported()`** when adding new output formats.

The function must contain a **stable, frozen set** of bus layouts matching the v0.2
release. New formats are handled entirely INTERNALLY:

```cpp
// STABLE — DO NOT ADD NEW ENTRIES (see docs/bug-reports/BUS_LAYOUT_BUG.md)
auto outputSet = layouts.getMainOutputChannelSet();
if (outputSet == juce::AudioChannelSet::stereo())              return true;  // Binaural
if (outputSet == juce::AudioChannelSet::quadraphonic())        return true;  // Quad
if (outputSet == juce::AudioChannelSet::create5point1())       return true;  // 5.1
if (outputSet == juce::AudioChannelSet::create7point1())       return true;  // 7.1
if (outputSet == juce::AudioChannelSet::create7point1point4()) return true;  // 7.1.4
if (outputSet == juce::AudioChannelSet::create9point1point6()) return true;  // 9.1.6
if (outputSet == juce::AudioChannelSet::octagonal())           return true;  // Octaphonic
if (outputSet == juce::AudioChannelSet::discreteChannels(8))   return true;  // 8ch discrete
return false;
```

New formats (5.0, 7.0, 5.1.2, 5.1.4, 7.1.2, 7.1.6, Ambisonics FOA/SOA/HOA)
are all available via the **output format dropdown**. The plugin renders internally
to whatever channels the bus provides:

- User selects "5.1.2 Atmos" → plugin renders 5.1.2 to 8 channels of a 7.1 bus
- User selects "1st Order Ambi" → plugin writes AmbiX SH coefficients to 4 channels of a Quad bus
- `resolveEffectiveFormat()` handles fallback if the bus has insufficient channels

## Key Principles

1. `isBusesLayoutSupported()` is a **DAW-facing API** — changes affect host behavior
2. Output format selection is an **internal concern** — handled by the dropdown + `resolveEffectiveFormat()`
3. `detectOutputFormat()` should only auto-detect from the stable set of bus layouts
4. NEVER add `ambisonic()` channel sets — these trigger special Ambisonics routing in some DAWs

## History
- **v0.2 development**: Bug first appeared when adding surround formats. Fixed by stabilizing bus layouts.
- **v0.3 output format expansion**: Bug re-appeared when 10 new entries were added to `isBusesLayoutSupported()`. Fixed by reverting to v0.2 stable set.
