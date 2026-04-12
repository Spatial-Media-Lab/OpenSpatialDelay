---
name: daw-compatibility-guide
description: Use when troubleshooting DAW-specific plugin issues, fixing host quirks, passing AU validation, handling offline render, or debugging automation in Logic, Ableton, Pro Tools, Cubase, Reaper, FL Studio, or Bitwig.
---

# DAW Compatibility Guide

## Critical Cross-DAW Issues

| Issue | Affected DAWs | Fix |
|-------|--------------|-----|
| **AU validation fails** | Logic Pro | Fix `getTailLengthSeconds()`, handle zero-buffer `processBlock()`, pass `auval -a` |
| **Offline render differs** | Reaper, Logic, Ableton | Check `isNonRealtime()` — disable modulation, use deterministic processing |
| **Automation not working** | All | Use APVTS; never modify params from audio thread without `beginChangeGesture()`/`endChangeGesture()` |
| **State not restored** | All | Verify `getStateInformation()`/`setStateInformation()` round-trips correctly |
| **Side-chain routing broken** | Logic, Cubase | Declare side-chain bus in `isBusesLayoutSupported()`; test each DAW individually |

## Quick Compatibility Matrix

| Feature | Logic | Ableton | Pro Tools | FL Studio | Cubase | Reaper | Bitwig |
|---------|-------|---------|-----------|-----------|--------|--------|--------|
| AU | Required | Supported | No | No | No | Supported | Supported |
| VST3 | Supported | Required | No | Required | Required | Required | Required |
| AAX | No | No | Required | No | No | No | No |
| Side-chain | AU only | VST3 | AAX | Limited | VST3 | VST3 | VST3 |
| Offline render | Special handling | Works | Works | Works | Works | Special handling | Works |

## AU Validation Quick Fix

```cpp
// These are the most common auval failures:

// 1. Return non-zero tail length
double getTailLengthSeconds() const override { return 0.5; } // NOT 0.0

// 2. Handle zero-sample processBlock
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override {
    if (buffer.getNumSamples() == 0) return;  // Don't crash on empty buffer
    // ... normal processing
}

// 3. Support mono and stereo
bool isBusesLayoutSupported(const BusesLayout& layouts) const override {
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()) return true;
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()) return true;
    return false;
}
```

Run validation: `auval -v aufx MyPl MyMa` (effect) or `auval -v aumu MySy MyMa` (instrument)

## Testing Strategy

| Priority | Test | Why |
|----------|------|-----|
| 1 | `auval -a` (macOS) | Gate for Logic Pro — most common failure point |
| 2 | Load/save state in each DAW | State corruption is the #1 user-reported bug |
| 3 | Automation record/playback | Verify params automate smoothly without zipper noise |
| 4 | Offline render vs realtime | Must produce identical output |
| 5 | Multiple instances | Verify no shared state between instances |

## Common Mistakes

| Mistake | Fix |
|---|---|
| Not testing AU validation before release | Run `auval -a` in CI; it catches 90% of Logic issues |
| Ignoring offline render mode | Check `isNonRealtime()` and adjust behavior accordingly |
| Hardcoded 44100 sample rate | Always use `getSampleRate()` — DAWs run at 48k, 96k, etc. |
| Not handling buffer size changes | Reallocate in `prepareToPlay()`, not `processBlock()` |
| Assuming stereo — breaking mono/surround | Implement `isBusesLayoutSupported()` for all target layouts |
| Testing only in one DAW | Each DAW has unique quirks — test in at minimum Logic + Ableton + Reaper |

---

See **[REFERENCE.md](REFERENCE.md)** for full per-DAW details: Logic Pro (AU specifics, sidechain, freeze/flatten), Ableton Live (macro mapping, session view), Pro Tools (AAX, AudioSuite, latency comp), FL Studio (preset browser, instance isolation), Cubase/Nuendo (expression maps, Note Expression), Reaper (offline render, flexible configs), Bitwig (modulation rate), Studio One, format-specific considerations, emergency fixes, and complete testing matrix.
